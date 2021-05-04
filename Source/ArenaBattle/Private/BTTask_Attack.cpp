// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "ABAIController.h"
#include "ABCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
	bNotifyTick = true;
	IsAttacking = false;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	auto ABCharacter = Cast<AABCharacter>(OwnerComp.GetAIOwner()->GetPawn());  //여기서 에러가 나타나는데 컴파일을 하니 비헤비어트리는 제대로 작동합니다. (AI가 캐릭터에게 다가와 attack을 합니다.) 해결했습니다. 
	
	if (nullptr == ABCharacter)
		return EBTNodeResult::Failed;

	ABCharacter->Attack();
	IsAttacking = true;
	ABCharacter->OnAttackEnd.AddLambda([this]() -> void {  //람다함수 바인딩   이게 이 위에허 바인딩한 함수들
		IsAttacking = false;  //IsAttacking이 false이면 한 번의 공격을 끝낸다. 여기서 
	});

	return EBTNodeResult::InProgress;  //InProgress는 succeeded도 아니고 false도 아니다. InProgress는 한 번 실핼하고 있는 중임을 나타낸다. 그렇지 않으면 atttack는 계속 공격 시도만 반복할 것이다. 
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)  //TickTask
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	if (!IsAttacking)  //공격이 아니라면 (bool형 반환)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);  //공격이 아닌 게 성공이면 , 함수를 끝낸다. 
	}
}


