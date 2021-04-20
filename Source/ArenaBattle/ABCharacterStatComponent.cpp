// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacterStatComponent.h"
#include "ABGameInstance.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;  //이게 호출되면 밑에 UABCharacterStatComponent::InitializeComponent()가 호출되고 false면 호출 안 됨.

	Level = 1;

	// ...
}


// Called when the game starts
void UABCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetNewLevel(Level);
}

void UABCharacterStatComponent::SetNewLevel(int32 NewLevel)
{
	auto ABGameInstance = Cast<UABGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));  //GetGameInstance를 가지고 오는 이유는 여기에 스탯이 저장돼 있기 때문. 이것을 온전히 사용하기 위해 UABGameInstance를 실형식으로 가져옴. 

	ABCHECK(nullptr != ABGameInstance);
	CurrentStatData = ABGameInstance->GetABCharacterData(NewLevel);  //프라이빗은 온전히 자기 클래스 안에서만 접근 가능하다. ABGameInstance.cpp의 20행에서 스탯데이터를 받으면 여기로 넘어옴. 
	if (nullptr != CurrentStatData)  //만약 커런트데이터가 유효한 데이터면
	{
		Level = NewLevel;
		SetHP(CurrentStatData->MaxHP);    // 원래 이 라인이 있었는데 이거 삭제하고 385페이지 작성했습니다. CurrentHP = CurrentStatData->MaxHP;
	}
	else
	{
		ABLOG(Error, TEXT("Level (%d) data doesn't exist"), NewLevel);
	}
}


// Called every frame
/* void UABCharacterStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}     위에서 bCanEverTick를 false로 했디 깨문이다.    */

void UABCharacterStatComponent::SetDamage(float NewDamage)
{
	ABCHECK(nullptr != CurrentStatData);
	SetHP(FMath::Clamp<float>(CurrentHP - NewDamage, 0.0f, CurrentStatData->MaxHP));   //이때 클램프엑스는 음수가 들어올 수도 있다. 미니멈보다 작으면 미니멈으로세팅. 대미지가 아무리 커도 Hp의 최소값은 0. 

	/*   CurrentHP = FMath::Clamp<float>(CurrentHP - NewDamage, 0.0f, CurrentStatData->MaxHP);
	if (CurrentHP <= 0.0f)
	{
		OnHPIsZero.Broadcast();
	}   386페이지에 이 라인들 없어서 일단 주석처리했습니다.  */
}

void UABCharacterStatComponent::SetHP(float NewHP)  //65라인의 셋HP가 여기에서 뉴Hp로 들어옴. 
{
	CurrentHP = NewHP;
	OnHPChanged.Broadcast();  //셋Hp가 불려졌단 건 Hp에 변화가 있었단 것이고
	if (CurrentHP < KINDA_SMALL_NUMBER)   //이때 Hp변화가 있어서 딜리게이트가 호출됨. 현재 Hp가 010001보다 작아지면(0이 안 될수도 있으니까. 플롯은)
	{
		CurrentHP = 0.0f;
		OnHPIsZero.Broadcast();   //OnHPIsZero에 등록된 함수들을 Broadcast한다. 
	}
}

float UABCharacterStatComponent::GetAttack()  //어택이라는 변수를 반환하는 함수
{
	ABCHECK(nullptr != CurrentStatData, 0.0f);
	return CurrentStatData->Attack;
}

float UABCharacterStatComponent::GetHPRatio()
{
	ABCHECK(nullptr != CurrentStatData, 0.0f);

	return (CurrentHP < KINDA_SMALL_NUMBER) ? 0.0f : (CurrentHP / CurrentStatData->MaxHP);  //킨다스몰넘버를사용, 큐런트hp가 100일 때 맥스 hp가 200이면 0.5를 반환. 
}