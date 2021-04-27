// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "AIController.h"
#include "ABAIController.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABAIController : public AAIController
{
	GENERATED_BODY()

public:
	AABAIController();
	virtual void OnPossess(APawn* InPawn) override;  //403페이지에 Possess가 언리언 버전에 따라 OnPossess로 바뀐다고 해 적용했습니다. 하지만 UnPossess는 어찌 해야 하는지 몰라서 ..
	// 408페이지에 다라 이 라인 주석 처리  virtual void OnUnPossess() override;   //403페이지 적용 

	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName TargetKey;

private:
	UPROPERTY()
	class UBehaviorTree* BTAsset;
	//void OnRepeatTimer();

	UPROPERTY()
	class UBlackboardData* BBAsset;
	//FTimerHandle RepeatTimerHandle;
	//float RepeatInterval;
	
};
