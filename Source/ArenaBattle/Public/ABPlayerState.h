// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/PlayerState.h"
#include "ABPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABPlayerState : public APlayerState
{
	GENERATED_BODY()

public:  //스테이트에서 플레이어의 상태 저장
	AABPlayerState();

	int32 GetGameScore() const;
	int32 GetCharacterLevel() const;

	void InitPlayerData();

protected:
	UPROPERTY(Transient)
	int32 GameScore;  //스코어와 레벨 저장

	UPROPERTY(Transient)
	int32 CharacterLevel;
	
};
