// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainDataSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "UI/UPageConfig.h"
#include "WorldDynamicTerrainControlSubsystem.generated.h"

class UTerrainDataSubsystem;


UCLASS()
class CUBEIZATION_API UWorldDynamicTerrainControlSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }


	UTerrainDataSubsystem* TerrainDataSubsystem;

	FFieldParameter ActivationParameter;
	FFieldParameter DeactivationParameter;
	bool bFieldActivation = true;

public:
	float DeactivationHeight = -10000;
	float ActivationHeight = 200;
	float ActivationParamLerpSpeed = 1;
	float DeactivationParamLerpSpeed = 10;

public:
	UFUNCTION(BlueprintCallable)
	void ResetActivationArea();

	UFUNCTION(BlueprintCallable)
	void DeactivateAll();

	UFUNCTION(BlueprintCallable)
	void PartialActiveArea(
		const FVector& Center,
		float HaftExtend,
		float FallOff);

	void PartialActiveIndex(
		const FIntVector2& Index,
		float HaftExtend,
		float FallOff);

	UFUNCTION(BlueprintCallable)
	void SetFieldActivation(bool bActive)
	{
		if (bActive && !bFieldActivation)
		{
			bFieldActivation = true;
			TerrainDataSubsystem->SetFieldParameter(ActivationParameter, ActivationParamLerpSpeed);
		}
		else if (!bActive && bFieldActivation)
		{
			bFieldActivation = false;
			TerrainDataSubsystem->SetFieldParameter(DeactivationParameter, DeactivationParamLerpSpeed);
		}
	}

	UFUNCTION(BlueprintCallable)
	void SetFieldActivationParameter(const FFieldParameter& Parameter) { ActivationParameter = Parameter; }

	UFUNCTION(BlueprintCallable)
	void SetFieldDeactivationParameter(const FFieldParameter& Parameter) { DeactivationParameter = Parameter; }

private:
	bool bOnSequentialUpdate = false;
	float SequentialActiveBeginTime = 0;
	TFieldArray<float> SignedDistanceField;
	TFieldArray<float> TargetHeightField;
	//interpolation factors
	TFieldArray<FVector4d> InterpolationFactorField;
	TFieldArray<TTuple<float, float>> InterpolationBeginEndField;

	//preserved field
	TFieldArray<float> PrevHeightField;
	TFieldArray<float> PrevLastHeightField;
	TFieldArray<float> PrevVelocityField;

public:
	float SequentialActiveLastingTime = 0;
	float SequentialActiveInterpolationTime = 1;
	float SequentialActiveMaxDistance = 1000;
	float SequentialActiveExternalWaitTime = 5;

public:
	UFUNCTION(BlueprintCallable)
	void LeaveMainWorld();
	UFUNCTION(BlueprintCallable)
	void RecoverMainWorld();


	UFUNCTION(BlueprintCallable)
	void SequentialFieldInit(
		const TArray<FVector>& Centers,
		const TArray<float>& HaftExtends,
		const float BeginTime, const float LastingTime
	);

	void InterpolationInit(
		const TFieldArray<float>* X0,
		const TFieldArray<float>* V0,
		const TFieldArray<float>* X1,
		const TFieldArray<float>* V1);


	void SequentialFieldUpdate(const float Time);

	// UFUNCTION(BlueprintCallable)
	// void SequentialFieldUpdateDefault(const float Time, const float InterpolationTime, const float MaxDistance);


	//UI
	//todo move this to a new subsystem
private:
	bool bUIActive = false;

	struct FUIElement
	{
		FIntVector2 Index;
		AWidgetActor* WidgetActor;
	};

	TArray<FUIElement> UIElements;
	FVector UICenter;

	UPROPERTY()
	TArray<TObjectPtr<UPageConfig>> PageStack;

public:
	UPROPERTY()
	TObjectPtr<UPageConfig> StartPage;
	UPROPERTY()
	TObjectPtr<UPageConfig> PausePage;
	//UI interface

	UFUNCTION(BlueprintCallable)
	void OnHoverFieldReaction(const FVector& Position);

	UFUNCTION(BlueprintCallable)
	void OnClickFieldReaction(const FVector& Position);

	UFUNCTION(BlueprintCallable)
	void InitCurrentPage();

	UFUNCTION(BlueprintNativeEvent)
	void OnPageFinishInit();


	UFUNCTION(BlueprintCallable)
	void ReleaseCurrentPage();

	void UIActorSyncTick();

	//todo move these code to widget action
	//UI action and control
	UFUNCTION(BlueprintCallable)
	void AddPage(UPageConfig* PageConfig);

	UFUNCTION(BlueprintCallable)
	void ToNextPage(UPageConfig* PageConfig);
	UFUNCTION(BlueprintCallable)
	void ToPrevPage();

	UFUNCTION(BlueprintCallable)
	void OnClickStart();

	UFUNCTION(BlueprintCallable)
	void OnClickAbout();

	UFUNCTION(BlueprintCallable)
	void OnClickPause();

	UFUNCTION(BlueprintCallable)
	void OnClickContinue();

	UFUNCTION(BlueprintCallable)
	void OnClickReturn();

	UFUNCTION(BlueprintCallable)
	void OnClickExit();
};
