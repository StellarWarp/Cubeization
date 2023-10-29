// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "GroundCubeObserverProcessor.generated.h"

/**
 * 
 */
UCLASS()
class UGroundCubeObserverProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	
	UGroundCubeObserverProcessor();

	FMassEntityQuery Query;

	void ConfigureQueries() override;

	void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
};
