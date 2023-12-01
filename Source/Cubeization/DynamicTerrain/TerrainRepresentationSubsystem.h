// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Weapons/WeaponRespond.h"
#include "TerrainRepresentationSubsystem.generated.h"

struct DeferredReplacementParams
{
	FIntVector2 Index;
	FVector HitLocation;
};

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UTerrainRepresentationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	TArray<TObjectPtr<AActor>> RepresentActors;
	TMap<AActor*, FIntVector2> ActorToIndex;

	UPROPERTY()
	TSubclassOf<class ATerrainCubeActor> ReplacingActorClass;

	TQueue<DeferredReplacementParams> DeferredReplacementQueue;

public:

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }


	void SetReplacingActorClass(TSubclassOf<ATerrainCubeActor> Class) { ReplacingActorClass = Class; }

	void NotifyDestruction(FWeaponHitInfo Index);
	
	void DeferredInstanceUpdate(UInstancedStaticMeshComponent* ISMC);

	void UpdateReplacingActor();
	
};
