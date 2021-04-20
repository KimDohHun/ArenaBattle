// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacterWidget.h"
#include "ABCharacterStatComponent.h"

void UABCharacterWidget::BindCharacterStat(UABCharacterStatComponent* NewCharacterStat)  //캐릭터의 hp비율을 인자로 받아야 위젯으로 출력이 가능하다. 
{
	ABCHECK(nullptr != NewCharacterStat);

	CurrentCharacterStat = NewCharacterStat;
	NewCharacterStat->OnHPChanged.AddLambda( [this]() -> void {

		if (CurrentCharacterStat.IsValid())
		{
			ABLOG(Warning, TEXT("HPRatio : %f"), CurrentCharacterStat->GetHPRatio());   //이건 단순히 로그 출력, 테이크데미지가 호출돼야 람다 로그가 뜬다. 그니까 공격받는 대상이 있어야함.
		}

		});
}
