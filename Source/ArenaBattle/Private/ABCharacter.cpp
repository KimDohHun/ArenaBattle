// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"  
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"
#include "ABAIController.h"
#include "ABCharacterSetting.h"
#include "ABGameInstance.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"

// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
    CharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));  //CharacterStatComponent가 생성되는 시점은 AABCharacter 생성자가 호출되는 시점ㅇ.다
    HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARWIDGET"));   //위젯 컴포넌트를  HPBar위젯어 부착

    SpringArm->SetupAttachment(GetCapsuleComponent());
    Camera->SetupAttachment(SpringArm);
    HPBarWidget->SetupAttachment(GetMesh());

    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
    if (SK_CARDBOARD.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

    static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C"));
    if (WARRIOR_ANIM.Succeeded())  
    {
        GetMesh()->SetAnimInstanceClass(WARRIOR_ANIM.Class);
    }

    SetControlMode(EControlMode::DIABLO);

    ArmLengthSpeed = 3.0f;
    ArmRotationSpeed = 10.0f;
    GetCharacterMovement()->JumpZVelocity = 800.0f;
    IsAttacking = false;

    MaxCombo = 4;
    AttackEndComboState();

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));

    AttackRange = 200.0f;
    AttackRadius = 50.0f;

    //381페이지 라인. 여기에 작성하는 게 확실하지 않습니다. 
    HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);   //4개의 좌표계 중 하나인 스크린 좌표계. 만약 world 좌표게를 사용하면 약간 홀로그램같이 된다. 
    static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("/Game/Book/UI/UI_HPBar.UI_HPBar_C"));  //클래스파인더로 클래스를 가져옴
    if (UI_HUD.Succeeded())
    {
        HPBarWidget->SetWidgetClass(UI_HUD.Class);   //HP바도 UIHUD를 하나 가지고 있다. UIHud가 곧 위젯이다. 이때에는 상속이 아니라 그냥 가지고 있는 거다. 위젯 컴포넌트는 상속 관계가 아니라 위젯을 그냥 가지고 있다. 셋위젯 클래스를 하는 이유는 ...  HPBar가 UI로 변하는 게 아니라 HP바가 위젯 안에 변수로 들어감. 
        HPBarWidget->SetDrawSize(FVector2D(150.0f, 50.0f));
    }
    AIControllerClass = AABAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    auto DefaultSetting = GetDefault<UABCharacterSetting>();
    if (DefaultSetting->CharacterAssets.Num() > 0)
    {
        for (auto CharacterAsset : DefaultSetting->CharacterAssets)
        {
            ABLOG(Warning, TEXT("Character Asset : %s"), *CharacterAsset.ToString());
        }
    }
    AssetIndex = 4;

    SetActorHiddenInGame(true);
    HPBarWidget->SetHiddenInGame(true);
    SetCanBeDamaged(false);
    //bCanBeDamaged = false;
    DeadTimer = 5.0f;
}

void AABCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != ABAnim);

    ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);  //OnMontageEnded에 OnAttackMontageEnded를 바인딩. OnMontageEnded가 호출되면 여기에 바인딩된 함수들도 함께 호출됨. 
    //OnAttackMontageEnded여기에 바인딩된 함수가 뭐냐하면 313라인들, OnAttackMontageEnded는 공격이 끝나면 호출됨. 
    ABAnim->OnNextAttackCheck.AddLambda([this]() -> void 
        {
        ABLOG(Warning, TEXT("OnNextAttackCheck"));
        CanNextCombo = false;

        if (IsComboInputOn)
        {
            AttackStartComboState();
            ABAnim->JumpToAttackMontageSection(CurrentCombo);
        }
    });
    ABAnim->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck); 

    CharacterStat->OnHPIsZero.AddLambda([this]() -> void 
        {
        ABLOG(Warning, TEXT("OnHPIsZero"));
        ABAnim->SetDeadAnim();
        SetActorEnableCollision(false);
        });

}

//370페이지 라인입니다. 원래 357라인에 있던 것을 이 위치로 옮겼습니다. 
float AABCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{   //여기서부터는 맞은애로 넘어온다. 370라인에서 여기로 넘어옴. 
    float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    ABLOG(Warning, TEXT("Actor : %s took Damage : %f"), *GetName(), FinalDamage);

    // 임시 테스트용
    if (!IsPlayerControlled())
    {
        FinalDamage *= 3.0f;
    }

    CharacterStat->SetDamage(FinalDamage);  //테이크데미지
    return FinalDamage;    //이 라인이 원래 여기에 없었는데 370페이지에 적혀 있어서 새로 작성했습니다. 102라인과 겹쳐서 문제가 되지는 않을까요??

}

// Called when the game starts or when spawned

void AABCharacter::BeginPlay()
{
    Super::BeginPlay();

    bIsPlayer = IsPlayerControlled();
    if (bIsPlayer)
    {
        ABPlayerController = Cast<AABPlayerController>(GetController());
        ABCHECK(nullptr != ABPlayerController);
    }
    else
    {
        ABAIController = Cast<AABAIController>(GetController());
        ABCHECK(nullptr != ABAIController);
    }

    auto DefaultSetting = GetDefault<UABCharacterSetting>();

    if (bIsPlayer)
    {
        AssetIndex = 4;
    }
    else
    {
        AssetIndex = FMath::RandRange(0, DefaultSetting->CharacterAssets.Num() - 1);
    }

    CharacterAssetToLoad = DefaultSetting->CharacterAssets[AssetIndex];
    auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
    ABCHECK(nullptr != ABGameInstance);
    AssetStreamingHandle = ABGameInstance->StreamableManager.RequestAsyncLoad(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
    SetCharacterState(ECharacterState::LOADING);

    /*
    if (!IsPlayerControlled())  //만약 AI라면 즉 플레이어 컨트롤러가 아니라면
    {
        auto DafaultSetting = GetDefault<UABCharacterSetting>();  //
        int32 RandIndex = FMath::RandRange(0, DafaultSetting->CharacterAssets.Num() - 1);  //RandRange는 랜덤한 범위를 의미. CharacterAssets이 5개라고 한다면 이때 -1을 해주는 까닭은 배열은 0부터 시작하기 때문. 
        // 여기서 랜덤한 값을 가지므로 메쉬의 값들이 랜덤하게 달라진다. 
        CharacterAssetToLoad = DafaultSetting->CharacterAssets[RandIndex];  //RandIndex 여기서 인덱스를 받는데 인덱스 5가 들어오면 0부터 시작하므로 out of range가 나타난다. 

        auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
        if (nullptr != ABGameInstance)  //
        {
            AssetStreamingHandle = ABGameInstance->StreamableManager.RequestAsyncLoad(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
            
        //RequestAsyncLoad이 말은 비동기. 동기는 순서대로 실행하는 것이고 비동기는 순서에 상관없이 일단 실행하고 보는 것. 
        //StreamableManager를 통해서 RequestAsyncLoad 함수를 실행한다. 이때 인자를 보면 FSoftObjectPath인데 이것은 경로를 넣으라는 뜻. 
        //Fstreamabledelegate는 ㄴㅅㅁ샻 함수로서 전역함수.  
        //RequestAsyncLoad 이게 로드되면 OnAssetLoadCompleted 이 함수가 실행된다.
        }
    }

    //470페이지. 원래 이 코드가 있었습니다. 이 코드를 지우고 470페이지를 작성하니 위젯이 나타나지 않아서 다시 작성했습니다. 
    auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
    if (nullptr != CharacterWidget)
    {
        CharacterWidget->BindCharacterStat(CharacterStat);  //이게 있어야 한다. CharacterStat을 BindCharacterStat에 바인드 해준다. 
    }    506페이지 작성 중에 교재에 이 라인들이 없어서 주석처리했습니다. */
}

void AABCharacter::SetCharacterState(ECharacterState NewState)
{
    ABCHECK(CurrentState != NewState);
    CurrentState = NewState;

    switch (CurrentState)
    {
    case ECharacterState::LOADING:
    {
        if (bIsPlayer)
        {
            DisableInput(ABPlayerController);

            auto ABPlayerState = Cast<AABPlayerState>(GetPlayerState());
            ABCHECK(nullptr != ABPlayerState);
            CharacterStat->SetNewLevel(ABPlayerState->GetCharacterLevel());
        }

        SetActorHiddenInGame(true);
        HPBarWidget->SetHiddenInGame(true);
        SetCanBeDamaged(false);
        //bCanBeDamaged = false;
        break;
    }
    case ECharacterState::READY:
    {
        SetActorHiddenInGame(false);
        HPBarWidget->SetHiddenInGame(false);
        SetCanBeDamaged(true);
        //bCanBeDamaged = true;

        CharacterStat->OnHPIsZero.AddLambda([this]() -> void {  //람다함수를 바인딩하고 람다함수 정의부에 236라인이 있다. 

            SetCharacterState(ECharacterState::DEAD);  //dead하면 257라인으로 이동

            });

        auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
        ABCHECK(nullptr != CharacterWidget);
        CharacterWidget->BindCharacterStat(CharacterStat);

        if (bIsPlayer)
        {
            SetControlMode(EControlMode::DIABLO);
            GetCharacterMovement()->MaxWalkSpeed = 600.0f;
            EnableInput(ABPlayerController);
        }
        else
        {
            SetControlMode(EControlMode::NPC);
            GetCharacterMovement()->MaxWalkSpeed = 400.0f;
            ABAIController->RunAI();
        }

        break;
    }
    case ECharacterState::DEAD:
    {
        SetActorEnableCollision(false);
        HPBarWidget->SetHiddenInGame(true);  //죽었으니까  hp바를 숨긴다. . 
        ABAnim->SetDeadAnim();  //죽는 야ㅐ니메이션 실행시키고
        SetCanBeDamaged(false);
        //bCanBeDamaged = false;

        if (bIsPlayer)  //죽은 게 플레이어인지 검사한다. 플레이어였다면...
        {
            DisableInput(ABPlayerController);
        }
        else  //AI가 죽었다면...
        {
            ABAIController->StopAI();
        }

        GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
            //여기서부터는 죽은 후에 벌어지느 일
            if (bIsPlayer)  //죽은 후, 즉 5초 후에도 람다 함수 실행
            {
                ABPlayerController->RestartLevel();  //죽은 게 플레이어였으면 재시작
            }
            else
            {
                Destroy();
            }
            }), DeadTimer, false);  //죽은 게 AI 였으면 디스트로이. 

        break;
    }
    }
}

ECharacterState AABCharacter::GetCharacterState() const
{
    return CurrentState;
}

bool AABCharacter::CanSetWeapon()
{
    return (nullptr == CurrentWeapon);
}

void AABCharacter::SetWeapon(AABWeapon* NewWeapon)
{
    ABCHECK(nullptr != NewWeapon && nullptr == CurrentWeapon);

    FName WeaponSocket(TEXT("hand_rSocket"));
    NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
    NewWeapon->SetOwner(this);
    CurrentWeapon = NewWeapon;
    
}

FName AABCharacter::GetCurrentStateNodeName() const
{
    return ABAnim->GetCurrentStateName(ABAnim->GetStateMachineIndex(TEXT("BaseAction")));  //이 부분은 수업 중에 그라운드에 있을 때, 점프할 때, 좌우로 움직일 때 공격을 하지 못하도록 수정한 코드.
}

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
    CurrentControlMode = NewControlMode;

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:

        ArmLengthTo = 450.0f;
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritRoll = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bDoCollisionTest = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case EControlMode::DIABLO:
        
        ArmLengthTo = 800.0f;
        ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);
        SpringArm->bUsePawnControlRotation = false;
        SpringArm->bInheritPitch = false;
        SpringArm->bInheritRoll = false;
        SpringArm->bInheritYaw = false;
        SpringArm->bDoCollisionTest = false;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
        break;
    case EControlMode::NPC:
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
        break;
    }
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

    switch (CurrentControlMode)
    {
    case EControlMode::DIABLO:
        SpringArm->SetRelativeRotation (FMath::RInterpTo(SpringArm->GetRelativeRotation(), ArmRotationTo, DeltaTime, ArmRotationSpeed));
        break;

    }

    switch (CurrentControlMode)
    {
    case EControlMode::DIABLO:
        if (DirectionToMove.SizeSquared() > 0.0f)
        {
            GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
            AddMovementInput(DirectionToMove);
        }
        break;
    }
}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
    PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AABCharacter::Jump);  
    PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);

    PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
    PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
}

void AABCharacter::UpDown(float NewAxisValue)
{
    if (GetCurrentStateNodeName() == TEXT("") || ABAnim->GetCurrentActiveMontage())  //
    {

        return;
    }

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X), NewAxisValue);
        break;
    case EControlMode::DIABLO:
        DirectionToMove.X = NewAxisValue;
        break;
    }
}

void AABCharacter::LeftRight(float NewAxisValue)
{
    if (GetCurrentStateNodeName() == TEXT("") || ABAnim->GetCurrentActiveMontage())  //
    {
        return;
    }

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddMovementInput(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y), NewAxisValue);
        break;
    case EControlMode::DIABLO:
        DirectionToMove.Y = NewAxisValue;
        break;
    }
}

void AABCharacter::LookUp(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddControllerPitchInput(NewAxisValue);
        break;
    }
}

void AABCharacter::Turn(float NewAxisValue)
{
    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        AddControllerYawInput(NewAxisValue);
        break;
    }
}

void AABCharacter::Jump()
{
    if (IsAttacking == true)
    {
        return;
    }

    bPressedJump = true;
    JumpKeyHoldTime = 0.0f;
}

void AABCharacter::ViewChange()
{
    switch (CurrentControlMode)
    {
    case EControlMode::GTA:
        GetController()->SetControlRotation(GetActorRotation());
        SetControlMode(EControlMode::DIABLO);
        break;
    case EControlMode::DIABLO:
        GetController()->SetControlRotation(SpringArm->GetRelativeRotation());
        SetControlMode(EControlMode::GTA);
        break;
    }
}

void AABCharacter::Attack()
{
    if (GetCurrentStateNodeName() != TEXT("Ground"))  //GetCurrentStateNodeName는 현재 노드 네임을 불러온다. 이때에는 Ground만 아니면 나머지를 다 불러오므로 Jump를 의미한다. 
    {
        return;  //이 부분은 수업 중에 그라운드에 있을 때, 점프할 때, 좌우로 움직일 때 공격을 하지 못하도록 수정한 코드. 
    }

    if (IsAttacking) 
    {
        ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
        if (CanNextCombo)
        {
            IsComboInputOn = true;
        }
    }
    else  //여기가 처음 공격할 때 호출되는 함수. 왜냐하면 IsAttacking은 공격 중에 호출되는 함수니까. 
    {
        ABCHECK(CurrentCombo == 0);
        AttackStartComboState();
        ABAnim->PlayAttackMontage();  //여기에서 PlayAttackMontage에 행동을 인자로 받아온다. 
        ABAnim->JumpToAttackMontageSection(CurrentCombo);
        IsAttacking = true;
    }
    
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)   //81번 라인에 바인딩된 함수들. 
{
    ABCHECK(IsAttacking);
    ABCHECK(CurrentCombo > 0);
    IsAttacking = false;  //  IsAttacking = false로 할지, false=IsAttacking 으로할지정해서 수정하기. 
    AttackEndComboState();
    OnAttackEnd.Broadcast();
}

void AABCharacter::AttackStartComboState()
{
    CanNextCombo = true;
    IsComboInputOn = false;
    ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1));
    CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
    IsComboInputOn = false;
    CanNextCombo = false;
    CurrentCombo = 0;
}

void AABCharacter::AttackCheck()
{
    FHitResult HitResult;
    FCollisionQueryParams Params(NAME_None, false, this);
    bool bResult = GetWorld()->SweepSingleByChannel(
        HitResult,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * AttackRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel2,
        FCollisionShape::MakeSphere(AttackRadius),
        Params);

#if ENABLE_DRAW_DEBUG

    FVector TraceVec = GetActorForwardVector() * AttackRange;
    FVector Center = GetActorLocation() + TraceVec * 0.5f;
    float HalfHeight = AttackRange * 0.5f + AttackRadius;
    FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
    FColor DrawColor = bResult ? FColor::Green : FColor::Red;
    float DebugLifeTime = 5.0f;

    DrawDebugCapsule(GetWorld(),
        Center,
        HalfHeight,
        AttackRadius,
        CapsuleRot,
        DrawColor,
        false,
        DebugLifeTime);

#endif

    if (bResult)
    {
        if (HitResult.Actor.IsValid())
        {
            ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.Actor->GetName());

            FDamageEvent DamageEvent;
            HitResult.Actor->TakeDamage(CharacterStat->GetAttack(), DamageEvent, GetController(), this);  //맞은애의 테이크대미지를 호출, 맞은대미지를 TakeDamage의 첫번째 인자로 호출. 
        }   //여기서 액터로 돼 있지만 사실 실형식은 A캐릭터, 여기서 테이크대미지 호출. 
    }
}

void AABCharacter::PossessedBy(AController* NewController)  //432페이지 작성 위치가 맞는지 확실하지 않습니다. 
{
    Super::PossessedBy(NewController);

    if (IsPlayerControlled())  //플레이어에 의해서 컨트롤되고 있다면
    {
        SetControlMode(EControlMode::DIABLO);  //컨트롤모드를 디아블로로 바꾸고
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;  //최대 속력은 600
    }
    else   //플레이어에 의해서 컨트롤되고 있지 않다면, 즉 AI라면.
    {
        SetControlMode(EControlMode::NPC);
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    }
}

void AABCharacter::OnAssetLoadCompleted()
{
    USkeletalMesh* AssetLoaded = Cast<USkeletalMesh>(AssetStreamingHandle->GetLoadedAsset());
    AssetStreamingHandle.Reset();
    ABCHECK(nullptr != AssetLoaded);
    GetMesh()->SetSkeletalMesh(AssetLoaded);  //SetSkeletalMesh에 AssetLoaded 에셋을 로드해서 캐릭터가 등장할 때마다 메쉬가 달라진다.

    SetCharacterState(ECharacterState::READY);

    /*  if (nullptr != AssetLoaded)  //널포인터가 아니라면, 즉 주소가 없는게 아니라면.
    {
         
    }  */
}