// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Blueprint/UserWidget.h"
#include "ABCharacterWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindCharacterStat(class UABCharacterStatComponent* NewCharacterStat);

protected:
	virtual void NativeConstruct() override;
	void UpdateHPWidget();

private:
	TWeakObjectPtr<class UABCharacterStatComponent> CurrentCharacterStat;

	UPROPERTY(meta = (BindWidget))  //BindWidget을 사용할 때에는 C++과 블루프린트의 변수 이름이 같아야 한다. 그래서 여기서는 PB_HPBar로 모두 바꿨다. 이 부분 이게 더 쉽게 하는 방법이라서 책이랑 다름.
	class UProgressBar* PB_HPBar;
};
