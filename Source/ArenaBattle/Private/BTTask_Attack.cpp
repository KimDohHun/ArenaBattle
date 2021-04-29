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
	ABCharacter->OnAttackEnd.AddLambda([this]() -> void {
		IsAttacking = false;
	});

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	if (!IsAttacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}


