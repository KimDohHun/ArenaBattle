// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsInAttackRange.h"
#include "ABAIController.h"
#include "ABCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsInAttackRange::UBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("CanAttack");  //여기서 CanAttack은 이름뿐이고 실제로는 블랙보드의 isinattackrange.
}

bool UBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const  //const 함수는 get터에서 자주 사용한다. 
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);  //부모의 CalculateRawConditionValue를 불러오고

	auto ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
		return false;

	auto Target = Cast<AABCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AABAIController::TargetKey));  //TargetKey는 블랙보드에서 만든 오브젝트 타입이고 TargetKey를 누르고 f12 를 누르면 Target를 가지고 오는 것을 볼 수 있다. 이때 target이 블랙보드에서 만든 오브젝트. 
	//GetBlackboardComponent에 있는 GetValueAsObject를 가져오고 이것의 매개변수는 TargetKey(키네임)이다. OwnerComp는 14행의 UBehaviorTreeComponent에서 가져온다. 
	if (nullptr == Target)  //타깃이 널포인터면 false 반환
		return false;
	bResult = (Target->GetDistanceTo(ControllingPawn) <= 200.0f);
	return bResult;
}
