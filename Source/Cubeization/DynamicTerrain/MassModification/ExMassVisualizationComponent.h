// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassVisualizationComponent.h"
#include "ExMassVisualizationComponent.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UExMassVisualizationComponent : public UMassVisualizationComponent
{
	GENERATED_BODY()

public:
	UExMassVisualizationComponent();
protected:
	void ExConstructStaticMeshComponents();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	void EnableInstancesCollision(bool bCond);
	void ExBeginVisualChanges();
};
