// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameMode.h"
#include "ABPlayerController.h"
#include "ABSection.h"
#include "ABCharacter.h"
#include "ABItemBox.h" 

// Sets default values
AABSection::AABSection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;  //틱 함수는 필요없으니까 false로 하고. 

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MESH"));  //MESH라는 이름을 지어주고, CreateDefaultSubobject로 메쉬 만들기.
	RootComponent = Mesh;

	FString AssetPath = TEXT("/Game/Book/StaticMesh/SM_SQUARE.SM_SQUARE");  //AssetPath로 경로 저장
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_SQUARE(*AssetPath);  //FObjectFinder로 에셋 가져오기
	if (SM_SQUARE.Succeeded())  //애셋을 불러오는 데에 성공하면
	{
		Mesh->SetStaticMesh(SM_SQUARE.Object);
	}
	else
	{
		ABLOG(Error, TEXT("Faled to load staticmesh asset. : %s"), *AssetPath);  //실패하면 로그 발생
	}

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TRIGGER"));  //트리거만들고
	Trigger->SetBoxExtent(FVector(775.0f, 775.0f, 300.0f));  
	Trigger->SetupAttachment(RootComponent);
	Trigger->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
	Trigger->SetCollisionProfileName(TEXT("ABTrigger"));

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABSection::OnTriggerBeginOverlap);  
	//딜리게이트가 미리 존재한다 여기에 OnTriggerBeginOverlap을 바인딩한다. 이러면 오버랩됬늘 때 호출된다. 

	FString GateAssetPath = TEXT("/Game/Book/StaticMesh/SM_GATE.SM_GATE");  //메쉬 가져오기
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_GATE(*GateAssetPath);  //
	if (!SM_GATE.Succeeded())  //메쉬를 가져오는 데에 실패하면 
	{
		ABLOG(Error, TEXT("Failed to load staticmesh asset. : %s"), *GateAssetPath);
	}

	static FName GateSockets[] = { { TEXT("+XGate") } ,{ TEXT("-XGate") } ,{ TEXT("+YGate") } ,{ TEXT("-YGate") } };
	for (FName GateSocket : GateSockets)  //범위기반for문으로서 GateSocket에 33 라인의 4개를 하나씩 담는다. 
	{
		ABCHECK(Mesh->DoesSocketExist(GateSocket));  //소켓이 존재하는지 ABCHECK로 보기
		UStaticMeshComponent* NewGate = CreateDefaultSubobject<UStaticMeshComponent>(*GateSocket.ToString());  
		//NewGate에 CreateAbstractDefaultSubobject이거를 이용해서... CreateAbstractDefaultSubobject의 첫 번째 인자는 FName이므로 이름짓기. (12라인에서 MESH라고 이름을 지었으므로 이것은 33라인과 같이 이름 짓기), 이후 뉴게이트로 게이트 생성. 
		NewGate->SetStaticMesh(SM_GATE.Object);
		NewGate->SetupAttachment(RootComponent, GateSocket);
		NewGate->SetRelativeLocation(FVector(0.0f, -80.5f, 0.0f));
		GateMeshes.Add(NewGate);  //GateMeshes에 NewGate add하기. 

		UBoxComponent* NewGateTrigger = CreateDefaultSubobject<UBoxComponent>(*GateSocket.ToString().Append(TEXT("Trigger")));
		NewGateTrigger->SetBoxExtent(FVector(100.0f, 100.0f, 300.0f));
		NewGateTrigger->SetupAttachment(RootComponent, GateSocket);
		NewGateTrigger->SetRelativeLocation(FVector(70.0f, 0.0f, 250.0f));
		NewGateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));
		GateTriggers.Add(NewGateTrigger);

		NewGateTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABSection::OnGateTriggerBeginOverlap);
		NewGateTrigger->ComponentTags.Add(GateSocket);
	}
	bNoBattle = false;

	EnemySpawnTime = 2.0f;
	ItemBoxSpawnTime = 5.0f;
}

// Called when the game starts or when spawned  게임이 시작되고 객체가 만들어졌을 때 비긴플레이가 호출된다. 
void AABSection::BeginPlay()
{
	Super::BeginPlay();
	
	SetState(bNoBattle ? ESectionState::COMPLETE : ESectionState::READY);  
	//SetState가 호출되는데 bNoBattle이 false로 되고 이는 스테이트가 ready 임을 의미한다. 그러면 82라인으로 넘어간다. 
}

void AABSection::SetState(ESectionState NewState)
{
	switch (NewState)
	{
	case ESectionState::READY:  //ready가 하는 일 : AB트리거로 세팅,   NoCollision으로 세팅,  OperateGates(true)이렇게 해서 문을 열고 
		//타이머로 NPC와 박스를 언제 어떻게 만들지 정한다. 
	{
		Trigger->SetCollisionProfileName(TEXT("ABTrigger"));  //Trigger박스의 콜리전을 ABTrigger로 바꿈. 이 Trigger박스는 섹션 전체를 감싸서 충돌을 감지. 
		//캐릭터만 감지하는 트리거.
		for (UBoxComponent* GateTrigger : GateTriggers)  //GateTriggers는 배열이다. 4개니까. 이 부분을 범위기반 for문으로 한다. 
		{
			GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));  //GateTrigger르 NoCollision으로 한다. 
		}

		OperateGates(true);  //문을 회전시킨 후

		GetWorld()->GetTimerManager().SetTimer(SpawnNPCTimerHandle, FTimerDelegate::CreateUObject(this, &AABSection::OnNPCSpawn), EnemySpawnTime, false);
		//EnemySpawnTime은 위에서 2초로 정했다. false가 되면 FTimerDelegate로 바인딩 한 함수를 실행한다. 
		GetWorld()->GetTimerManager().SetTimer(SpawnItemBoxTimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
			FVector2D RandXY = FMath::RandPointInCircle(600.0f);  //RandPointInCircle 반지름 600범위에서 아무데나 찍는다. 
			GetWorld()->SpawnActor<AABItemBox>(GetActorLocation() + FVector(RandXY, 30.0f), FRotator::ZeroRotator);
			//GetActorLocation에서 
			}), ItemBoxSpawnTime, false);
		//ItemBoxSpawnTime(5초)에서 을 지정하고 0000을 false로 한다. 
		break;
	}

	case ESectionState::BATTLE:
	{
		Trigger->SetCollisionProfileName(TEXT("NoCollision"));
		for (UBoxComponent* GateTrigger : GateTriggers)
		{
			GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
		}

		OperateGates(false);  //OperateGates(false) 회전이 없단 것. 문이 닫힌단 것. 
		break;
	}
	case ESectionState::COMPLETE:
	{  //NoBattle이 true면 다음 맵으로 갈 수 있고 false면 battle가 시작된다. 
		Trigger->SetCollisionProfileName(TEXT("NoCollision"));
		for (UBoxComponent* GateTrigger : GateTriggers)  //GateTrigger는 네 개의 문에 대한 배열
		{
			GateTrigger->SetCollisionProfileName(TEXT("ABTrigger"));  //GateTrigger의 컴포넌트ㄴ를 ABTrigger(엔진에서 만듦)로 한다. 
		
		}

		OperateGates(true);  //문을 회전시킨다. 
		break;
	}
	}

	CurrentState = NewState;
}

void AABSection::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetState(bNoBattle ? ESectionState::COMPLETE : ESectionState::READY);
}

void AABSection::OperateGates(bool bOpen)  //bOpen에 트루가 들어오면, bOpen이 false면 닫혀 있다. 
{
	for (UStaticMeshComponent* Gate : GateMeshes)  //4개의 게이트를 받는다. 
	{
		Gate->SetRelativeRotation(bOpen ? FRotator(0.0f, -90.0f, 0.0f) : FRotator::ZeroRotator);  //삼항연산자. bOpen이 트루이면 FRotator만큼 회전한단 것. 
	}  //트루면 메쉬를 yaw 축으로 회전시킨단 것. 
}

void AABSection::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{   //OnTriggerBeginOverlap 섹션 트리거의 온비긴오버랩이란 딜리게이트가 있는데 거기에 바인딩 된 함수가 이거고 이게... 
	if (CurrentState == ESectionState::READY)  //CurrentState  가 READY면 배틀BATTLE로 바꾼다. 
	{
		SetState(ESectionState::BATTLE);
	}
}

void AABSection::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABCHECK(OverlappedComponent->ComponentTags.Num() == 1);
	FName ComponentTag = OverlappedComponent->ComponentTags[0];  //OverlappedComponent에서 ComponentTags가 나온다. 
	FName SocketName = FName(*ComponentTag.ToString().Left(2));
	if (!Mesh->DoesSocketExist(SocketName))  //Mesh이거는 4개의 문에 대한 소켓이 존재한다. 이 소켓이 존재하냐... 
		return;

	FVector NewLocation = Mesh->GetSocketLocation(SocketName);  //소켓 위치를 반환해서 NewLocation에 넣고

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
	FCollisionObjectQueryParams ObjectQueryParam(FCollisionObjectQueryParams::InitType::AllObjects);
	bool bResult = GetWorld()->OverlapMultiByObjectType(   //멀티.. 여러 개가 된단 것. 
		OverlapResults,
		NewLocation,
		FQuat::Identity,
		ObjectQueryParam,
		FCollisionShape::MakeSphere(775.0f),
		CollisionQueryParam
	);

	if (!bResult)   //
	{
		auto NewSection = GetWorld()->SpawnActor<AABSection>(NewLocation, FRotator::ZeroRotator);
	}
	else
	{
		ABLOG(Warning, TEXT("New section area is not empty."));
	}
}

void AABSection::OnNPCSpawn()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnNPCTimerHandle);
	auto KeyNPC = GetWorld()->SpawnActor<AABCharacter>(GetActorLocation() + FVector::UpVector * 88.0f, FRotator::ZeroRotator);  //544페이지 코드 14-32번에 없던 라인이 생겼고 있던 라인은 교재에 보이지 않습니다. 
	if (nullptr != KeyNPC)
	{
		KeyNPC->OnDestroyed.AddDynamic(this, &AABSection::OnKeyNPCDestroyed);
	}
}

void AABSection::OnKeyNPCDestroyed(AActor* DestroyedActor)  //파괴된 액터가 매개변수로 들어온다. 
{
	auto ABCharacter = Cast<AABCharacter>(DestroyedActor);
	ABCHECK(nullptr != ABCharacter);  //파괴된 애가 AB캐릭터인지 확인한다. 

	auto ABPlayerController = Cast<AABPlayerController>(ABCharacter->LastHitBy);  //누구에게 맞았는지 확인한다. 
	ABCHECK(nullptr != ABPlayerController);  //zjsxmfhffj ghkrdls

	auto ABGameMode = Cast<AABGameMode>(GetWorld()->GetAuthGameMode());
	ABCHECK(nullptr != ABGameMode);  //
	ABGameMode->AddScore(ABPlayerController);

	SetState(ESectionState::COMPLETE);
}
