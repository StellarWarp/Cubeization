// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassVisualizer.h"
#include "ExMassVisualizer.generated.h"


UCLASS()
class CUBEIZATION_API AExMassVisualizer : public AMassVisualizer
{
	GENERATED_BODY()
public:
	AExMassVisualizer();

	//this hides the base class function
	class UMassVisualizationComponent& GetVisualizationComponent() const { return *VisComponent; }

// protected:
// 	UPROPERTY()
// 	TObjectPtr<class UMassVisualizationComponent> ExVisComponent;

protected:

};
