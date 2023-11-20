// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassVisualizationComponent.h"
#include "Weapons/WeaponRespond.h"
#include "ExMassVisualizationComponent.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UExMassVisualizationComponent : public UMassVisualizationComponent , public IWeaponRespond
{
	GENERATED_BODY()

public:
	UExMassVisualizationComponent();
protected:
	void ExConstructStaticMeshComponents();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	void ExBeginVisualChanges();

	
	// IWeaponRespond interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void NotifyWeaponHit(FWeaponHitInfo HitInfo);
	virtual void NotifyWeaponHit_Implementation(FWeaponHitInfo HitInfo) override;


protected:
	UPROPERTY(EditAnywhere)
	int InstanceBodiesSyncBatchSize = 64;
	
	int BatchUpdateCounter = 0;
	int TotalBatchUpdateCount = 0;
	int TotalInstanceCount = 0;
	int BatchUpdateBeginIndex = 0;
	int BatchUpdateEndIndex = 0;
	
};
