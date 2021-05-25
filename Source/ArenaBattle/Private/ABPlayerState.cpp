// Fill out your copyright notice in the Description page of Project Settings.


#include "ABPlayerState.h"
#include "ABGameInstance.h"
#include "ABSaveGame.h"

AABPlayerState::AABPlayerState()
{
	CharacterLevel = 1;
	GameScore = 0;
	GameHighScore = 0;
	Exp = 0;
	SaveSlotName = TEXT("Player1");
}

int32 AABPlayerState::GetGameScore() const   //함수의 우측에 const 가 붙으면 정의부에서 수정을 못한단 것. 
{  //get 함수는 값이 바뀌지 않고 그냥 얻기만 하는 경우가 많다. 이때 get과 const는 함께 자주 사용한다. . 
	return GameScore;
}

int32 AABPlayerState::GetGameHighScore() const
{
	return GameHighScore;
}

int32 AABPlayerState::GetCharacterLevel() const
{
	return CharacterLevel;
}

void AABPlayerState::InitPlayerData()  //임의의 값들이 들어간다. 
{	//플레이어가 접속할 때 플레이어 정보 초기화
	auto ABSaveGame = Cast<UABSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));  //인자로 슬롯. 
	if (nullptr == ABSaveGame)
	{
		ABSaveGame = GetMutableDefault<UABSaveGame>();  //이건 디폴트 오브젝트 가져오기. 
	}

	SetPlayerName(ABSaveGame->PlayerName);
	SetCharacterLevel(ABSaveGame->Level);
	GameScore = 0;
	GameHighScore = ABSaveGame->HighScore;  //오른쪽 값들은 저장되는 값들이고 왼쪽 값은 처음 값.
	Exp = ABSaveGame->Exp;

	/*SetPlayerName(TEXT("Destiny"));
	SetCharacterLevel (5);
	GameScore = 0;
	Exp = 0;  555페이지 주석처리 했습니다. */
}

void AABPlayerState::AddGameScore()  //
{
	GameScore++;
	if (GameScore >= GameHighScore)
	{
		GameHighScore = GameScore;
	}
	OnPlayerStateChanged.Broadcast();
}

float AABPlayerState::GetExpRatio() const  //const는 함수의 정의부에서 값을 수정할 수 없단 것. 
{
	if (CurrentStatData->NextExp <= KINDA_SMALL_NUMBER)
		return 0.0f;

	float Result = (float)Exp / (float)CurrentStatData->NextExp;
	ABLOG(Warning, TEXT("Ratio : %D, Current : %d, Next : %d"), Result, Exp, CurrentStatData->NextExp);
	return Result;
}

bool AABPlayerState::AddExp(int32 IncomeExp)
{
	if (CurrentStatData->NextExp == -1)
		return false;

	bool DidLevelUp = false;
	Exp = Exp + IncomeExp;
	if (Exp >= CurrentStatData->NextExp)
	{
		Exp -= CurrentStatData->NextExp;
		SetCharacterLevel(CharacterLevel + 1);
		DidLevelUp = true;
	}

	OnPlayerStateChanged.Broadcast();
	return DidLevelUp;
}

void AABPlayerState::SetCharacterLevel(int32 NewCharacterLevel)
{
	auto ABGameInstance = Cast<UABGameInstance>(GetGameInstance());
	ABCHECK(nullptr != ABGameInstance);

	CurrentStatData = ABGameInstance->GetABCharacterData(NewCharacterLevel);
	ABCHECK(nullptr != CurrentStatData);

	CharacterLevel = NewCharacterLevel;
}