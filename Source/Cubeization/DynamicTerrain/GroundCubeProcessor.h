// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "GroundCubeProcessor.generated.h"

/**
 * 
 */
UCLASS()
class UGroundCubeProcessor : public UMassProcessor
{
	GENERATED_BODY()

	FMassEntityQuery Query;

public:
	UGroundCubeProcessor();

private:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
};
