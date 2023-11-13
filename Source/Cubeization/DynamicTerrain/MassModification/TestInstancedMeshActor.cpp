// Fill out your copyright notice in the Description page of Project Settings.


#include "TestInstancedMeshActor.h"

#include "ExMassVisualizationComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
ATestInstancedMeshActor::ATestInstancedMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// VisComponent = CreateDefaultSubobject<UExMassVisualizationComponent>(TEXT("ExVisualizerComponent"));
	ISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(L"InstancedStaticMeshComponent");
	// ISMC = NewObject<UInstancedStaticMeshComponent>(this);
}


// Called when the game starts or when spawned
void ATestInstancedMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATestInstancedMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

