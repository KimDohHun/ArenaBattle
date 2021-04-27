// Fill out your copyright notice in the Description page of Project Settings.


#include "ABAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName AABAIController::HomePosKey(TEXT("HomePos"));
const FName AABAIController::PatrolPosKey(TEXT("PatrolPos"));
const FName AABAIController::TargetKey(TEXT("Target"));

AABAIController::AABAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("/Game/Book/AI/BB_ABCharacter.BB_ABCharacter")); 
	if (BBObject.Succeeded())
	{
		BBAsset = BBObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("/Game/Book/AI/BT_ABCharacter.BT_ABCharacter"));
	if (BTObject.Succeeded())
	{
		BTAsset = BTObject.Object;
	}
}

void AABAIController::OnPossess(APawn* InPawn)  //403페이지에 다라 On 첨가, OnPossess는 폰이 컨트롤러에 빙의할 때 호룿된다. 즉 사용자가 조종을 시작할 때. 
{
	Super::OnPossess(InPawn);  //이후 부모의 OnPossess를 불러오고 인자로는 컨트롤러가 빙의한 폰이 들어간다. 
	if (UseBlackboard(BBAsset, Blackboard))  //이게 성공하면 14라인의 BBAsset을 불러온다. 이때 블랙보드값이 BBAsset에 들어간다. 
	{
		Blackboard->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());  //폰의 현재 위치를 HomePosKey에 넣는다. 
		if (!RunBehaviorTree(BTAsset))   //20라인의 BTAsset을 불러온다. 
		{
			ABLOG(Error, TEXT("AIController couldn't run behavior tree!"));   //조건문에 실패하면 에러 메시지를 나타낸다. 
		}
	}
}


	/*  GetWorld()->GetTimerManager().SetTimer(RepeatTimerHandle, this, &AABAIController::OnRepeatTimer, RepeatInterval, true);
}

void AABAIController::OnUnPossess()  //403페이지 적용
{
	Super::OnUnPossess();  //403페이지 적용
	GetWorld()->GetTimerManager().ClearTimer(RepeatTimerHandle);
}

void AABAIController::OnRepeatTimer()
{
	auto CurrentPawn = GetPawn();
	ABCHECK(nullptr != CurrentPawn);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (nullptr == NavSystem) return;

	FNavLocation NextLocation;
	if (NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, 500.0f, NextLocation))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NextLocation.Location);
		ABLOG(Warning, TEXT("Next Location : %s"), *NextLocation.Location.ToString());
	}
}  409에 보면 이 라인들이 전부 없어집니다. 맞는지요?  */