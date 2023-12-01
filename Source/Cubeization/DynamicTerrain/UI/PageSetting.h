// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicTerrain/TerrainDataSubsystem.h"
#include "GameFramework/Actor.h"
#include "PageSetting.generated.h"


class UPageConfig;

UCLASS()
class CUBEIZATION_API APageSetting : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPageConfig> StartPage;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPageConfig> PausePage;

	UPROPERTY(EditAnywhere)
	FFieldParameter DeactivationParameter;

	UPROPERTY(EditAnywhere)
	FFieldParameter ActivationParameter;

	UPROPERTY(EditAnywhere)
	float ActivationHeight = 200;
	UPROPERTY(EditAnywhere)
	float DeactivationHeight = -500;
	UPROPERTY(EditAnywhere)
	float SequentialActivationTime = 5;
	UPROPERTY(EditAnywhere)
	float InterpolationTime = 1.0;
	UPROPERTY(EditAnywhere)
	float ExternalWaitTime = 5;
	UPROPERTY(EditAnywhere)
	float MaxDistance = 1000;
	UPROPERTY(EditAnywhere)
	bool bMainMenuInit = true;

public:
	// Sets default values for this actor's properties
	APageSetting();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void InitCurrentPage();

	UFUNCTION(BlueprintCallable)
	void EnterGame();

public:

};
