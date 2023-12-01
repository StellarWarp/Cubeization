// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainRepresentationSubsystem.h"

#include "Destruction/TerrainCubeActor.h"
#include "TerrainDataSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"

void UTerrainRepresentationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UTerrainDataSubsystem::StaticClass());
	
	Super::Initialize(Collection);
}

void UTerrainRepresentationSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UTerrainRepresentationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//actor sync
	UpdateReplacingActor();
}

void UTerrainRepresentationSubsystem::NotifyDestruction(FWeaponHitInfo Info)
{
	auto TerrainDataSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();

	FVector InnerPosition = Info.HitLocation - Info.HitNormal * 0.5f * TerrainDataSubsystem->CubeLength;

	//test for debug
	// DrawDebugLine(GetWorld(), Info.HitLocation, InnerPosition, FColor::Red, true, 10.f, 0, 5.f);
	//find the index of replacing cube
	FIntVector2 CubeIndex = TerrainDataSubsystem->WorldPositionToLogicalIndex(InnerPosition);

	DeferredReplacementQueue.Enqueue({CubeIndex, Info.HitLocation});
}


void UTerrainRepresentationSubsystem::DeferredInstanceUpdate(UInstancedStaticMeshComponent* ISMC)
{
	auto TerrainDataSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();

	auto& InstanceTransforms = ISMC->PerInstanceSMData;
	auto& InstanceBodies = ISMC->InstanceBodies;

	while (!DeferredReplacementQueue.IsEmpty())
	{
		DeferredReplacementParams Params;
		DeferredReplacementQueue.Dequeue(Params);

		FIntVector2 CubeIndex = Params.Index;
		FVector CubePosition = TerrainDataSubsystem->LogicalIndexToWorldPosition(CubeIndex);
		// DrawDebugLine(GetWorld(), Params.HitLocation, CubePosition, FColor::Green, true, 10.f, 0, 5.f);
		//creating new actor
		ATerrainCubeActor* Actor = Cast<ATerrainCubeActor>(
			GetWorld()->SpawnActor(ReplacingActorClass, &CubePosition, &FRotator::ZeroRotator));


		int ShiftCube;
		Actor->SetupInitalizationParams(CubeIndex, Params.HitLocation, 2, ShiftCube);

		float Offset = -ShiftCube * TerrainDataSubsystem->CubeLength;
		TerrainDataSubsystem->MapModifyField[CubeIndex] += Offset;
		TerrainDataSubsystem->BaseHeightField[CubeIndex] += Offset;
		TerrainDataSubsystem->LastHeightField[CubeIndex] = TerrainDataSubsystem->BaseHeightField[CubeIndex];
		TerrainDataSubsystem->HeightField[CubeIndex] = TerrainDataSubsystem->BaseHeightField[CubeIndex];

		RepresentActors.Add(Actor);
		ActorToIndex.Add(Actor, CubeIndex);

		FTransform TerrainCubeTransform = FTransform(
			FVector(CubePosition.X, CubePosition.Y, TerrainDataSubsystem->HeightField[CubeIndex]));

		uint32 InstanceId = TerrainDataSubsystem->InstanceIndexId[Params.Index];
		int32* pInstanceIndex = ISMC->InstanceIdToInstanceIndexMap.Find(InstanceId);
		if (!pInstanceIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("InstanceIdToInstanceIndexMap not found"));
			continue;
		}
		int32 InstanceIndex = *pInstanceIndex;

		// DrawDebugBox(GetWorld(), FTransform(InstanceTransforms[InstanceIndex].Transform).GetLocation(),
		//              FVector(50, 50, 50), FColor::Red, false, 10.f, 0, 5.f);
		InstanceTransforms[InstanceIndex].Transform = TerrainCubeTransform.ToMatrixWithScale();
		InstanceBodies[InstanceIndex]->SetBodyTransform(TerrainCubeTransform, ETeleportType::TeleportPhysics);

		Actor->DeferredInit();

		// DrawDebugLine(GetWorld(), TerrainCubeTransform.GetLocation(),
		// TerrainCubeTransform.GetLocation() + FVector(0, 0, 100), FColor::Yellow,
		// false, 0.5, 0, 5.f);
	}

}



void UTerrainRepresentationSubsystem::UpdateReplacingActor()
{
	// for(const auto& ActorIndex: ActorToIndex)
	// {
	// 	FIntVector2 Index = ActorIndex.Value;
	// 	FVector Position = GetWorld()->GetSubsystem<UTerrainDataSubsystem>()->LogicalIndexToWorldPosition(Index);
	// 	ActorIndex.Key->SetActorLocation(Position);
	// }
}
