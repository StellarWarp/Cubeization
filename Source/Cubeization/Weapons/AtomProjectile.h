// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "AtomProjectile.generated.h"

UCLASS()
class CUBEIZATION_API AAtomProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAtomProjectile();

	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovement;

	void SetVelocity(const FVector_NetQuantizeNormal& ImpactNormal);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
