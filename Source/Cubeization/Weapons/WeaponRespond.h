// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponRespond.generated.h"

USTRUCT(BlueprintType)
struct FWeaponHitInfo
{
	GENERATED_BODY()

	
	
	UPROPERTY()
	FVector HitLocation;

	UPROPERTY()
	FVector HitNormal;

	UPROPERTY()
	FVector Impulse;

	UPROPERTY()
	FHitResult HitResult;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponRespond : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CUBEIZATION_API IWeaponRespond
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NotifyWeaponHit(FWeaponHitInfo HitInfo);
	virtual void NotifyWeaponHit_Implementation(FWeaponHitInfo HitInfo);
};
