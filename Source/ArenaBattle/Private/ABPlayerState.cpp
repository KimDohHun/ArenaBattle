// Fill out your copyright notice in the Description page of Project Settings.


#include "ABPlayerState.h"

AABPlayerState::AABPlayerState()
{
	CharacterLevel = 1;
	GameScore = 0;
}

int32 AABPlayerState::GetGameScore() const   //함수의 우측에 const 가 붙으면 정의부에서 수정을 못한단 것. 
{  //get 함수는 값이 바뀌지 않고 그냥 얻기만 하는 경우가 많다. 이때 get과 const는 함께 자주 사용한다. . 
	return GameScore;
}

int32 AABPlayerState::GetCharacterLevel() const
{
	return CharacterLevel;
}

void AABPlayerState::InitPlayerData()  //임의의 값들이 들어간다. 
{
	SetPlayerName(TEXT("Destiny"));
	CharacterLevel = 5;
	GameScore = 0;
}