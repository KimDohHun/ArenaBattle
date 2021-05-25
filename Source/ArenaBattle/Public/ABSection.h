// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "AI/Navigation/NavModifierComponent.h"
#include "GameFramework/Actor.h"
#include "ABSection.generated.h"

UCLASS()
class ARENABATTLE_API AABSection : public AActor  //애터를 상속하는 ABSection 생성자
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABSection();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	enum class ESectionState : uint8  //이넘클래스, uint는 언사인드캐릭터형태로 만든단 것. 이 세 가지를. 
	{
		READY = 0,    //이넘은 int인데 이름만 레디라고 지은 것. 
		BATTLE,
		COMPLETE
	};

	//이넘은 열거형. 이넘은 원래 인트형인데 글자로 시각화한다. 이넘은 네이밍 중복이 발생할 수 있다. 묵시적 형변환이 가능하다는 문제도 있다. 묵시적 형변환보다는 명시적 형변환이 안전하다. 묵시적 형변환은 타입이 다른데 내부에서 타입을 자동으로 변형시키는 것. static_cast를 활용하는 게 명시적 형변환. 
	//이넘 클래스는 이넘과 다르다. 이넘 클래스는 이넘의 문제를 해결한다. 이넘 클래스는 이름을 앞에 붙여서 네이밍 중복이 나올 일이 없다. 그리고ㅗ 이것은 묵시적 형변환이 안 된다. 

	void SetState(ESectionState NewState);  //스테이트를 수정. 인자로는 ESectionState
	ESectionState CurrentState = ESectionState::READY;  //CurrentState를 레디로 초기화. 

	void OperateGates(bool bOpen = true);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnNPCSpawn();

	UFUNCTION()
	void OnKeyNPCDestroyed(AActor* DestroyedActor);

private:
	UPROPERTY(VisibleAnywhere, Category = "Mesh", Meta = (AllowPrivateAccess = true))
	TArray<UStaticMeshComponent*> GateMeshes;  //4개라서 어레이. 멤버함수(?)

	UPROPERTY(VisibleAnywhere, Category = "Trigger", Meta = (AllowPrivateAccess = true))
	TArray<UBoxComponent*> GateTriggers;

	UPROPERTY(VisibleAnywhere, Category = "Mesh", Meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;  // 이때 메쉬는 전체 맴에 대한 메쉬.

	UPROPERTY(VisibleAnywhere, Category = "Trigger", Meta = (AllowPrivateAccess = true))
	UBoxComponent* Trigger;

	UPROPERTY(EditAnywhere, Category = "State", Meta = (AllowPrivateAccess = true))
	bool bNoBattle;  //이 섹션에서 배틀 유무 판별

	UPROPERTY(EditAnywhere, Category = "Spawn", Meta = (AllowPrivateAccess = true))
	float EnemySpawnTime;

	UPROPERTY(EditAnywhere, Category = "Spawn", Meta = (AllowPrivateAccess = true))
	float ItemBoxSpawnTime;

	FTimerHandle SpawNPCTimerHandle = {};  //타이머핸들
	FTimerHandle SpawnItemBoxTimerHandle = {};
};
