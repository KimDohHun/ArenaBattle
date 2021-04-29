// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TurnToTarget.h"
#include "ABAIController.h"
#include "ABCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("Turn");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)  //ExecuteTask는 태스크가 호출될 때 항상 호출되는 것이다. 
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);  //부모의 ExecuteTask를 호출하고

	auto ABCharacter = Cast<AABCharacter>(OwnerComp.GetAIOwner()->GetPawn());  //GetAIOwner에서 GetPawn을 호출하고(?)
	if (nullptr == ABCharacter)
		return EBTNodeResult::Failed;

	auto Target = Cast<AABCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AABAIController::TargetKey));   //OwnerComp는 비헤비어트리. TargetKey는 플레이어컨트롤러, 이것을 AABCharacter로 캐스팅한다. 
	if (nullptr == Target)   //이때 Target에는 사실상 nullptr가 들어갈 수 없다. 왜냐하면 비헤비어트리에서 애초에 Targeton에서 타겟, 즉 플레이어가 감지돼야 그 다음 노드들이 실행하므로. 
		return EBTNodeResult::Failed;

	//벡터 = 크기 + 방향
	//단위벡터 = 크기가 1인 벡터 (일반적으로 벡터는 크기와 방향이 나오는데 단위벡터는 방향만 나온다.) -> 각 x, y, z 좌표마다 벡터의 크기를 나눈다. / 벡터의 크기 = sqrt(x^2 + y^2 + z^3)      이때 sqrt는 루트
	FVector LookVector = Target->GetActorLocation() - ABCharacter->GetActorLocation();  //LookVector에 벡터를 저장한다. (ㅌ1-ㅌ2, ㅛ1-ㅛ2, ㅋ1-ㅋ2) / sqrt(x^2 + y^2 + z^3) 이렇게 하면 단위벡터가 ㅇ나온다. 여기에 sqrt(x^2 + y^2 + z^3)  이거를 곱하면 원래 좌표가 나온다.  
	//이때 LookVector에는 적을 바라보는 방향이 나온다. 이 라인은 플레이어 위치에서 적의 위치를 뺀다는 의미. 
	LookVector.Z = 0.0f;  //z  로는 회전하지 않으므로 0
	FRotator TargetRot = FRotationMatrix::MakeFromX(LookVector).Rotator();  //MakeFromX(LookVector)이거는 x 방향으로 LookVector, 즉 방향을 넣는단 것(?)
	//MakeFromX의 인자를 보면 Xais 이고 이는 LookVector를 x축으로 삼겠단 것. 그러면 나머지 y, z도 위치가 달라진다. MakeFrom*는 직교하는 단위벡터를 반환. 
	ABCharacter->SetActorRotation(FMath::RInterpTo(ABCharacter->GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 2.0f));  //RInterpTo보관한다. RInterpTo는 현재위치에서 타깃쪽으로 방향을 반환. 여기에 인자로 속도를 RInterpTospeed를 받음. 
	//이때 단위 벡터를 사용하는 까닭은 단위벡터는 크기가 없고 방향만 있기 때문. 즉 크기가 있다면 그 방향으로 '이동'을 하는데 이건 이동이 아니라 플레이어가 있는 쪽으로 '회전해서 바라보기'만 하면 되기 때문.
	//Inv는 뒤집는 단 것으로서 invsqrt는 그 뒤에 오는 숫자를 분모로 만든단 것이다. 
	return EBTNodeResult::Succeeded;

}