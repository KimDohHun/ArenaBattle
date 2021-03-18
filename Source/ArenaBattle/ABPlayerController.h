// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/PlayerController.h"
#include "ABPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;
	virtual void Possess(APawn* aPawn) override;

protected:
	//virtual void SetupInoutComponent() override;  이 라인 삭제 후 159페이지 라인 작성?
	virtual void BeginPlay() override;

private:
	void LeftRight(float NewAxisValue);
	
};
