// Fill out your copyright notice in the Description page of Project Settings.


#include "ABSection.h"

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
		UStaticMeshComponent* NewGate = CreateAbstractDefaultSubobject<UStaticMeshComponent>(*GateSocket.ToString());  
		//NewGate에 CreateAbstractDefaultSubobject이거를 이용해서... CreateAbstractDefaultSubobject의 첫 번째 인자는 FName이므로 이름짓기. (12라인에서 MESH라고 이름을 지었으므로 이것은 33라인과 같이 이름 짓기), 이후 뉴게이트로 게이트 생성. 
		NewGate->SetStaticMesh(SM_GATE.Object);
		NewGate->SetupAttachment(RootComponent, GateSocket);
		NewGate->SetRelativeLocation(FVector(0.0f, -80.5f, 0.0f));
		GateMeshes.Add(NewGate);  //GateMeshes에 NewGate add하기. 
	}

}

// Called when the game starts or when spawned
void AABSection::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
/*  이 부분에 에러가 나오고 교재에 이 부분이 기입돼 있지 않아서 일단 주석처리했습니다.  void AABSection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}  */

