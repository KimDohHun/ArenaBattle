// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"  
#include "ABCharacterStatComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"

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

}

void AABCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
    ABCHECK(nullptr != ABAnim);

    ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

    ABAnim->OnNextAttackCheck.AddLambda([this]()
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

    CharacterStat->OnHPIsZero.AddLambda([this]() -> void {

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

    CharacterStat->SetDamage(FinalDamage);
    return FinalDamage;    //이 라인이 원래 여기에 없었는데 370페이지에 적혀 있어서 새로 작성했습니다. 102라인과 겹쳐서 문제가 되지는 않을까요??

}

// Called when the game starts or when spawned

void AABCharacter::BeginPlay()
{
    Super::BeginPlay();

    //390페이지의 버전 관련 코드입니다. 

    auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
    if (nullptr != CharacterWidget)
    {
        CharacterWidget->BindCharacterStat(CharacterStat);
    }
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
    if (IsAttacking) 
    {
        ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
        if (CanNextCombo)
        {
            IsComboInputOn = true;
        }
    }
    else
    {
        ABCHECK(CurrentCombo == 0);
        AttackStartComboState();
        ABAnim->PlayAttackMontage();
        ABAnim->JumpToAttackMontageSection(CurrentCombo);
        IsAttacking = true;
    }
    
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    ABCHECK(IsAttacking);
    ABCHECK(CurrentCombo > 0);
    IsAttacking = false;
    AttackEndComboState();
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
        }   //여기서 액터로 돼 있지만 사실 실형식은 A캐릭터
    }
}

