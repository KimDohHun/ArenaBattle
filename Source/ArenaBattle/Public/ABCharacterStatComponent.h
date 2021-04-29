// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Components/ActorComponent.h"
#include "ABCharacterStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHPIsZeroDelegate);
DECLARE_MULTICAST_DELEGATE(FOnHPChangedDelegate);    //여러개 함수 포함하는 멀티캐스트딜리게이트

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENABATTLE_API UABCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UABCharacterStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:	
	void SetNewLevel(int32 NewLevel);
	void SetDamage(float NewDamage);
	void SetHP(float NewHP);
	float GetAttack();
	float GetHPRatio();

	FOnHPIsZeroDelegate OnHPIsZero;
	FOnHPChangedDelegate OnHPChanged;

	// Called every frame
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;   이 라인이 365페이지에 적혀 있지 않아서 주석 처리했습니다.
	// 컴포넌트란 독립적으로 존재하는 게 아니라 어떤 객체에 부착시켜야 하는 것
private:
	struct FABCharacterData* CurrentStatData = nullptr;

	UPROPERTY(EditInstanceOnly, Category = "Stat", Meta = (AllowPrivateAccess = true))
	int32 Level;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Stat", Meta = (AllowPrivateAccess = true))
	float CurrentHP;
		
};
