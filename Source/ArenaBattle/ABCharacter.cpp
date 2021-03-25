// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"

// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

    SpringArm->SetupAttachment(GetCapsuleComponent());
    Camera->SetupAttachment(SpringArm);

    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacter/SK_CharM_Cardboard.SK_CharM_Cardboard"));
    if (SK_CARDBOARD.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

    static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANiM(TEXT("/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C"));
    if (WARRIOR_ANIM.Succeeded())  //선언되지 않은 식별자라는데 잘 모르겠습니다. 뒤에 블루프린트 관련 에러들도 모두 여기서 비롯되는 듯합니다.
    {
        GetMesh()->SetAnimInstanceClass(WARRIOR_ANIM.Class);
    }

    //SetControlMode(0);
    SetControlMode(EControlMode::DIABLO);

    ArmLengthSpeed = 3.0f;
    ArmRotationSpeed = 10.0f;
    GetCharacterMovement()->JumpZVelocity = 800.0f;
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

/*void AABCharacter::SetControlMode(int32 ControlMode)
{
    if (ControlMode == 0)
    {
        SpringArm->TargetArmLength = 450.0f;
        SpringArm->SetRelativeRotation(FRotator::ZeroRotater);
        SpringArm->bUsePawnControlRotation = true;
        SpringArm->bInheritPitch = true;
        SpringArm->bInheritRoll = true;
        SpringArm->bInheritYaw = true;
        SpringArm->bDoCollisionTest = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    }
}  이 라인들 삭제한 후 밑에 라인들 작성했습니다. 193P */

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
    CurrentControlMode = NewControlMode;

    switch (CurrentControlMode)
    {
    case EControlMode::GTA:

        //SpringArm->TargetArmLength = 450.0f
        //SpringArm->SetRelativeRotation(FRotator::ZeroRotater);
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
        //SpringArm->TargetArmLength = 800.0f
        //SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
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
        SpringArm->RelativeRotation = FMath::RInterpTo(SpringArm->RelativeRotation, ArmRotationTo, DeltaTime, ArmRotationSpeed);
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
    PlayerInputComponent->BindAction(TEXT("JUMP"), EInputEvent::IE_Pressed, this, &AABCharacter::JUMP);  //블루프린트에서 점프 에러를 해결하지 않고 작성해서 빨간줄이 생기는 듯합니다. 

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
        GetController()->SetControlRotation(SpringArm->RelativeRotation);
        SetControlMode(EControlMode::GTA);
        break;
    }
}