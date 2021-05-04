// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/Actor.h"
#include "ABSection.generated.h"

UCLASS()
class ARENABATTLE_API AABSection : public AActor  //애터를 상속하는 ABSection 생성자
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABSection();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UPROPERTY(VisibleAnywhere, Category = Mesh, Meta = (AllowPrivateAccess = true))
	TArray<UStaticMeshComponent*> GateMeshes;  //4개라서 어레이. 멤버함수(?)

	UPROPERTY(VisibleAnywhere, Category = Mesh, Meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;  // 이때 메쉬는 전체 맴에 대한 메쉬.

};
