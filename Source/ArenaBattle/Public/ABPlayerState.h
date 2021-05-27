// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/PlayerState.h"
#include "ABPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlayerStateChangedDelegate);

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
	int32 GetGameHighScore() const;
	FString SaveSlotName;
	int32 GetCharacterLevel() const;
	float GetExpRatio() const;
	bool AddExp(int32 IncomeExp);
	void AddGameScore();

	void InitPlayerData();
	void SavePlayerData();

	FOnPlayerStateChangedDelegate OnPlayerStateChanged;  //스탯에 변화가 생겼을 때 변화사항을 호출하기 위한 딜리게이트

protected:
	UPROPERTY(Transient)
	int32 GameScore;  //스코어와 레벨 저장

	UPROPERTY(Transient)
	int32 GameHighScore;

	UPROPERTY(Transient)
	int32 CharacterLevel;

	UPROPERTY(Transient)
	int32 Exp;

private:
	void SetCharacterLevel(int32 NewCharacterLevel);
	struct FABCharacterData* CurrentStatData;
	
};
