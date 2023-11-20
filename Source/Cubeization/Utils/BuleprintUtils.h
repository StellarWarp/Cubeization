// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuleprintUtils.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UBuleprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static void GetWorldGravityZ(const AActor* Actor, float& GravityZ)
	{
		GravityZ = Actor->GetWorld()->GetGravityZ();
	}
};
