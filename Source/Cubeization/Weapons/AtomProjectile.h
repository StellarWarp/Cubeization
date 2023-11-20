// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "AtomProjectile.generated.h"

UCLASS()
class CUBEIZATION_API AAtomProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAtomProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* SphereComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SphereRadius = 5.0f;
	
	void SetVelocity(const FVector_NetQuantizeNormal& ImpactNormal);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
