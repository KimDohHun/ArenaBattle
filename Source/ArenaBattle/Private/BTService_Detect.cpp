// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Detect.h"
#include "ABAIController.h"
#include "ABCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

UBTService_Detect::UBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn) return;

	UWorld* World = ControllingPawn->GetWorld();
	FVector Center = ControllingPawn->GetActorLocation();
	float DetectRadius = 600.0f;

	if (nullptr == World) return;
	TArray<FOverlapResult> OverlapResults;  
	//TArray<FOverlapResult> 이거 자체가 타입임. TArray는 FOverlapResult 자체를 담는 배열임.< > 이 꺽쇠가 템플릿이다. OverlapResults는 컨테이너
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, ControllingPawn);
	bool bResult = World->OverlapMultiByChannel(    //OverlapMultiByChannel에 값이 담길 수 있는 이유는 레퍼런스 &, Outoverlaps 레퍼런스를 사용하고 있기 때문. OverlapMultiByChannel은 앞에서 공격 부분을 할 때 sweep 간련 함수로 사용했음. 이건 31행에서 여러 개를 받겠다는 의미에서 multi를 붙였음. 멀티이기때문에 Tarry로서 배열 컨테이너를 사용. 
		OverlapResults,
		Center,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel12,
		FCollisionShape::MakeSphere(DetectRadius),
		CollisionQueryParams
	);

	if (bResult)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(AABAIController::TargetKey, nullptr);  //여기에는 처음엔 널포인터가 들어온다.target 자체는 플레이어의 캐릭터는 담는 그릇.
		for(const FOverlapResult& OverlapResult : OverlapResults)  //범위기반for문, OverlapResults라는 범위 안에 있는 요소(element )들을 OverlapResult(이건 위에서 컨테이너라고 했음)라는 컨테이너에 변수로서 담는 반복문. 이때 담기는 요소들의 타입은 FOverlapResult(28라인)다.
			//이때 이건 '복사'다. 레퍼런스는 주소값만 가지고 와서 자칫하면 본래의 값을 바꿀 수도 있다. 하지만 복사를 하면 두 개의 주소값이 달라서 메모리 공간을 따로 잡는다. 그래서 이 경우 책에서는 복사로 했지만 레퍼런스(&)로바꿔도 무방하다. 왜냐하면 레퍼런스가 더 빠르기 때무문디ㅏ. 
			//만약 여기서 걸리는 게 없으면 nullptr을 반환해서 if 문을 통과함. 
		{
			AABCharacter* ABCharacter = Cast<AABCharacter>(OverlapResult.GetActor());  //여기에는 처음엔 널포인터가 들어온다. (비헤비어트리에의해서 0.9초마다...
			//429페이지에는 OverlapResult라고 돼 있지만 VS에서 찾을 때에는 OverlapResults만 나옵니다. OverlapResult라고 하면 OverlapResult에서 에러가 나타나고 OverlapResults라고 하면 GetActor에서 에러가 나타납니다. 
			//43라인에서 말했듯이 이건 값을 수정하는 함수가 아니다. GetActor는 액터를 가져오기만 하는 함수이지 set하는 함수가 아니다. 그래서 레퍼런스 사용이 가능한데, 만약 값이 바뀔것이 걱정된다면 const를 붙여서 값이 바뀔 여지를 아예 차단하는 것도 방법이다. 
			if (ABCharacter && ABCharacter->GetController()->IsPlayerController())  //ABCharacter의 GetController가 IsPlayerController(플레이어 컨트롤러)냐고 여기서 묻고 있다. 이때 AI랑 플레이어 둘 다 캐릭터를 상속하고 있는데 이를 구분해야 하는 이유는 이 교재에선 AI는 치우고 캐릭터에 한해서만 그리려고(드로우디버그)하고 있기 때문이다. 그래서 Ai와 캐릭터는 같은 '타입'이다. 
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(AABAIController::TargetKey, ABCharacter);
				DrawDebugSphere(World, Center, DetectRadius, 16, FColor::Green, false, 0.2f);
				DrawDebugPoint(World, ABCharacter->GetActorLocation(), 10.0f, FColor::Blue, false, 0.2f);
				DrawDebugLine(World, ControllingPawn->GetActorLocation(), ABCharacter->GetActorLocation(), FColor::Blue, false, 0.2f);
				return;
			}
		}
	}

	DrawDebugSphere(World, Center, DetectRadius, 16, FColor::Red, false, 0.2f);
}


//언리얼에서 클래스를 새로 만들 때 잘못 만들면 