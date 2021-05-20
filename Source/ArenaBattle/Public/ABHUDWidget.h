// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Blueprint/UserWidget.h"
#include "ABHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindCharacterStat(class UABCharacterStatComponent* CharacterStat);
	void BindPlayerState(class AABPlayerState* PlayerState);  //여기에 classㄹ르 왜 적었는가? 전방선언 공부

protected:
	virtual void NativeConstruct() override;
	void UpdateCharacterStat();
	void UpdatePlayerState();

private:
	TWeakObjectPtr<class UABCharacterStatComponent> CurrentCharacterStat;  //스마트 포인터, 위크 포인터라느 게 있는게 이걸 언리얼만의 것으로ㅗ 재탄생 시킨 게 이것. 포인터를 담은 그릇. ptr이라는 클래스로 감싸서 ... 특징은 언리얼은 가비지.. 자동으로 해제되는 데 이걸 현재 사용하는 곳이 있냐 없냐를 기준으로 가비지 콜렉팅을 한다. 
	//TWeakObjectPtr는 CurrentCharacterStat이 소멸하더라고 현재 사용되고 있는 것까지는 사용한 후에 가비지 콜렉팅에 ㅔ의해서 소멸하도록 한 것. 즉 다른 곳에서 CurrentCharacterStat이 사라지더라도 현재 이곳에서 사용중인 CurrentCharacterStat은 사라지지 않는다. 
	TWeakObjectPtr<class AABPlayerState> CurrentPlayerState;

	UPROPERTY()
	class UProgressBar* HPBar;

	UPROPERTY()
	class UProgressBar* ExpBar;

	UPROPERTY()
	class UTextBlock* PlayerName;

	UPROPERTY()
	class UTextBlock* PlayerLevel;

	UPROPERTY()
	class UTextBlock* CurrentScore;

	UPROPERTY()
	class UTextBlock* HighScore;
	
};
