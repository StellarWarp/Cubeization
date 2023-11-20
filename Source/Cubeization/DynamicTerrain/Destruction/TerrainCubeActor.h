// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "GameFramework/Actor.h"
#include "TerrainCubeActor.generated.h"

UCLASS()
class CUBEIZATION_API ATerrainCubeActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> StaticMeshComponents;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<class UGeometryCollectionComponent>> GeometryCollectionComponents;
	
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<class AGeometryCollectionActor>> GeometryCollectionActors;

	TArray<bool> BreakCubeIndex;

	UPROPERTY(VisibleAnywhere)
	int CubeCount = 0;

	//the top relative to the actor
	float TopOffset;
	//the bottom relative to the actor
	float BottomOffset;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UActorComponent> CubeMeshComponentClass;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AGeometryCollectionActor>> CubeGeometryCollectionActorClasses;

	UPROPERTY(EditAnywhere)
	FVector ComponentScaler = {0.99f,0.99f,1.0f};

	UPROPERTY(EditAnywhere)
	float InverseGravityMultiplier = 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	bool Debug;
#endif

public:
	// Sets default values for this actor's properties
	ATerrainCubeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void OnGeometryCollectionBreak(const FChaosBreakEvent& Event);

private:

	struct FDeferredInitParams
	{
		FIntVector2 Index;
		int CubeReserveCount;
		FVector OriginalLocation;
		FVector HitLocation;
	}DeferredInitParams;
	
public:

	void SetupInitalizationParams(const FIntVector2 FieldIndex, const FVector& HitLocation, const int CubeReserveCount,
	                              int& OutCubeCount);
	void DeferredInit();

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
