// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Animation/AnimInstance.h"
#include "UABAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UUABAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UABAnimInstance();  //블루프린트 에러를 해결하지 않고 작성한 코드라 에러 처리되는 듯합니다.

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
		float CurrentPawnSpeed;
};
