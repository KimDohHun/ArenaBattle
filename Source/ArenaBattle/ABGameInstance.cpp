// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameInstance.h"

UABGameInstance::UABGameInstance()
{
	FString CharacterDataPath = TEXT("/Game/Book/GameData/ABCharacterData.ABCharacterData");
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_ABCHARACTER(*CharacterDataPath);
	ABCHECK(DT_ABCHARACTER.Succeeded());
	ABCharacterTable = DT_ABCHARACTER.Object;
	ABCHECK(ABCharacterTable->GetRowMap().Num() > 0);    //여기서 row는 '행'을 의미하고 이 행을 가져온단 것. 이때 이 행은 0보다 커야 한단 것. 
}

void UABGameInstance::Init()
{
	Super::Init();
}  

FABCharacterData* UABGameInstance::GetABCharacterData(int32 Level)  //여기서 Get을 사용해야 하는 이유는 이게 프라이빗으로 선언돼 있기도 해서지만 퍼블릭으로 선언돼 있어도 유지보수가 쉽기에 get을 사용한다. 
{   //GetABCharacterData에서 레벨을 숫자로 받은 후 FString 네임으로 바꿔서 역참조. 여기서 GetABCharacterData를 사용할 수 있는 이유는 8행에서 F오브젝트파인더로 파일을 찾았기 때문. 
	return ABCharacterTable->FindRow<FABCharacterData>(*FString::FromInt(Level), TEXT(""));  //이때 FABCharacterData타입으로 가져오는 이유는 우리가 이러한 타입으로 데이터는 넘겼기 때문. *는 역참조. 
  //
}
