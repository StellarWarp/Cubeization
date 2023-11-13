// Fill out your copyright notice in the Description page of Project Settings.


#include "AtomProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AAtomProjectile::AAtomProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
}

// Called when the game starts or when spawned
void AAtomProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAtomProjectile::SetVelocity(const FVector_NetQuantizeNormal& ImpactNormal)
{
	ProjectileMovement->Velocity = ImpactNormal * ProjectileMovement->InitialSpeed;
}
