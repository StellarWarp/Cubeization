// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "CubeGeometryCollectionComponent.generated.h"


UCLASS(Blueprintable)
class CUBEIZATION_API UCubeGeometryCollectionComponent : public UGeometryCollectionComponent
{
	GENERATED_BODY()

	UFUNCTION()
	void OnChaosRemoval(const FChaosRemovalEvent& RemovalEvent);
	UCubeGeometryCollectionComponent();
};
