// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPatrolPos.h"
#include "ABAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindPatrolPos::UBTTask_FindPatrolPos()
{
	NodeName = TEXT("FindPatrolPos");

}

EBTNodeResult::Type UBTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)  //ExecuteTask는 테스크를 실행한다. 그러면 ExecuteTask의 실행 주체인 OwnerComp가 첫 번째 인자로 들어간다.  
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);  //이때 OwnerComp에는 우리가 만든 BT(비헤비어트리)가 들어간다. 

	auto ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)  //ControllingPawn 여기에 값을 받으면 널포인터가 아니니까 fail을 받지 않고 널포인터면 fail.
		return EBTNodeResult::Failed;   //failed를 받는다.

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());  //네비게이션시스텝값을 NavSystem에 대입
	if (nullptr == NavSystem)
		return EBTNodeResult::Failed;

	FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AABAIController::HomePosKey);  //HomePosKey키는 아까 블랙보드에서 만든 것. 블랙보드와 비헤비어 트리느 묶여있다. 
	FNavLocation NextPatrol;
	
	if (NavSystem->GetRandomPointInNavigableRadius(Origin, 500.0f, NextPatrol))  //GetRandomPointInNavigableRadius는 반환형을 불 타입으로 받는다. 어떻게 불 타입으로 받냐? 불 타입으로 받으려면 목적지에 도착했냐 못했냐를 따져야 하는데 이건 랜덤 값으로 목적지를 정한다. 이때 주목해야 할 것이 레퍼런스다. 이 함수는 오리진 기준으로 범위 레디우스 안에 랜덤 좌표를 세 번째 인자로 받는다. 이후 이동 후에 그 위치가 원본과 같은지 판단한다(?, 이거 확실하지 않음)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AABAIController::PatrolPosKey, NextPatrol.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}


/*    BlackBoard:            인공지능의 판단에 사용하는 데이터 집합.   블랙보드는 사실상 애님인스턴스와 비슷하다. 애님인스턴스가 데이터를 가져오는 것처럼 작동하면 그걸 애님 그래프에서 받는다. 
   BehaviorTree:         블랙보드를 기반으로 비헤비어트리를 시각화해 저장   비헤비어트리는 결국 그래서 애님그래프. 블랙보드에서 가져오는 데이터를 가져와서 처리하니까. 
   Composite Node:         Selector:   자손 노드를 왼쪽에서 오른쪽 순서로 실행하며, 
                              자손 노드 중 하나가 실행에 성공하면 자손의 실행을 멈춘다
                     Sequence:   자손 노드를 왼쪽에서 오른쪽 순서로 실행하며, 
                              자손 노드중 하나가 실행에 실패하면 자손의 실행을 멈춘다.
                     Simple Parallel:   자손 노드를 순차적이 아닌 한번에, 동시에 실행
   Service Node:         컴포짓 노드에 부착되는 서브노드 (설정한 주기마다 실행)   컴포짓노드에 부가적 명령들
   Decorator Node:         블랙보드 값을 기반으로 특정 컴포짓 노드의 실행여부를 결정 (조건절)
   Task Node:            행동. 독립적으로 실행이 불가하며 컴포짓 노드를 거쳐야한다.  컴포넌트와 유사. 컴포짓노드를 통해서 실행됨. 
   NavMeshBoundsVolume : 이 영역 안에서는 내비게이션을 사용하겠단 것. */
