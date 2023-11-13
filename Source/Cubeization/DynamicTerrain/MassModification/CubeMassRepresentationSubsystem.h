// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationSubsystem.h"
#include "CubeMassRepresentationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UCubeMassRepresentationSubsystem :public UMassRepresentationSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ExOnProcessingPhaseStarted(const float DeltaSeconds, const EMassProcessingPhase Phase) const;

};
