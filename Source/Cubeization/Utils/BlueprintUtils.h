// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/CubeizationPlayerController.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtils.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static void GetWorldGravityZ(const AActor* Actor, float& GravityZ)
	{
		GravityZ = Actor->GetWorld()->GetGravityZ();
	}


	UFUNCTION(BlueprintCallable)
	static void IsGameSuspend(bool& bSuspend)
	{
		auto PC =
			Cast<ACubeizationPlayerController>(
				GEngine->GetFirstLocalPlayerController(GWorld));

		if (PC)
			bSuspend = PC->IsSuspended();
		else
		{
			bSuspend = false;
			//log error
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			                                 TEXT("UBuleprintUtils::IsGameSuspend: PC is null"));
		}
	}

	UFUNCTION(BlueprintCallable)
	static void SetGameSuspend(bool bSuspend)
	{
		auto PC =
			Cast<ACubeizationPlayerController>(
				GEngine->GetFirstLocalPlayerController(GWorld));

		if (PC)
			PC->SetSuspended(bSuspend);
		else
		{
			//log error
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			                                 TEXT("UBuleprintUtils::SetGameSuspend: PC is null"));
		}
	}


	// UFUNCTION(BlueprintCallable)
	// static void MakeFieldParameter(
	// 	const float Elasticity,
	// 	const float Damping,
	// 	const float Transfer,
	// 	const float Debug1,
	// 	const float Debug2,
	// 	const FVector Debug3,
	// 	FFieldParameter& Parameter
	// )
	// {
	// }
};
