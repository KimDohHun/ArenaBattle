// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameMode.h"
#include "ABCharacter.h"
#include "ABPlayerController.h"
#include "ABPlayerState.h"
#include "ABGameState.h"

AABGameMode::AABGameMode()
{
	DefaultPawnClass = AABCharacter::StaticClass();
	PlayerControllerClass = AABPlayerController::StaticClass();
	PlayerStateClass = AABPlayerState::StaticClass();
	GameStateClass = AABGameState::StaticClass();
}

void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ABGameState = Cast<AABGameState>(GameState);
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
	
	Super::PostLogin(NewPlayer);

	auto ABPlayerState = Cast<AABPlayerState>(NewPlayer->PlayerState);
	ABCHECK(nullptr != ABPlayerState);
	ABPlayerState->InitPlayerData();
}

void AABGameMode::AddScore(AABPlayerController* ScorePlayer)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)  //이터레이터는 반복자로서 인덱스를 증가시키는 것과 유사하다. It는 여기에서 그냥 변수일 뿐. 
		//플레이어 컨트롤러의 반복자를 가져와서 블레이어 컨트롤러들을 모두 순회하려고 사용. 하지만 이 게임에서는 플레이어가 하나라서 별 의미가 없다. 
	{
		const auto ABPlayerController = Cast<AABPlayerController>(It->Get());  //이건 포인터를 역참조하는 것과 같다. 포인터를 역참조하면 주소가 아니라 값이 나오듯이 이터레이터도 역참조하면 값이 나온다. 
		//이때 역참조를 도와주는 함수가 get이다. 역참조를 하면 플레이어 컨트롤러가 나온다. 
		if ((nullptr != ABPlayerController) && (ScorePlayer == ABPlayerController))
		{
			ABPlayerController->AddGameScore();
			break;
		}
	}

	ABGameState->AddGameScore();
}