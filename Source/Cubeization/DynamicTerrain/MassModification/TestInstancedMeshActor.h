// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestInstancedMeshActor.generated.h"

UCLASS()
class CUBEIZATION_API ATestInstancedMeshActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UMassVisualizationComponent> VisComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UInstancedStaticMeshComponent> ISMC;
	
public:	
	// Sets default values for this actor's properties
	ATestInstancedMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
