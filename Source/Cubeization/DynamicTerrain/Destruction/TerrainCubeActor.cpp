// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainCubeActor.h"

#include "DynamicTerrain/TerrainDataSubsystem.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"


// Sets default values
ATerrainCubeActor::ATerrainCubeActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

}

// Called when the game starts or when spawned
void ATerrainCubeActor::BeginPlay()
{
	Super::BeginPlay();
}


void ATerrainCubeActor::OnGeometryCollectionBreak_Implementation(const FChaosBreakEvent& Event)
{
	// //log on screen
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("OnGeometryCollectionBreak")));
	//
	// //locate hit index
	// auto HitHeight = Event.Location.Z;
	//
	// float ActorHeight = GetActorLocation().Z;
	// float ActorBottom = ActorHeight + BottomOffset;
	//
	// auto TerrainSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();
	// float CubeLength = TerrainSubsystem->CubeLength;
	//
	// float HitAffectBegin = HitHeight - CubeLength / 2 * 0.8;
	// float HitAffectEnd = HitHeight + CubeLength / 2 * 0.8;
	// int HitAffectBeginIndex = FMath::FloorToInt((HitAffectBegin - ActorBottom) / CubeLength);
	// int HitAffectEndIndex = FMath::CeilToInt((HitAffectEnd - ActorBottom) / CubeLength);
	// HitAffectBeginIndex = FMath::Max(HitAffectBeginIndex, 0);
	// HitAffectEndIndex = FMath::Min(HitAffectEndIndex, CubeCount);
}

void ATerrainCubeActor::SetupInitalizationParams(
	const FIntVector2 FieldIndex, const FVector& HitLocation, const int CubeReserveCount,
	int& OutCubeCount
)
{
	auto TerrainSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();
	auto OriginalLocation = TerrainSubsystem->LogicalIndexToWorldPosition(FieldIndex);
	float CubeLength = TerrainSubsystem->CubeLength;
	float Top = OriginalLocation.Z + CubeLength / 2;
	float DeltaDistance = Top - HitLocation.Z;
	DeltaDistance = FMath::Max(DeltaDistance, 0.f);
	OutCubeCount = FMath::CeilToInt(DeltaDistance / CubeLength) + CubeReserveCount;

	DeferredInitParams = {
		FieldIndex,
		CubeReserveCount,
		OriginalLocation,
		HitLocation,
	};
}


void ATerrainCubeActor::DeferredInit()
{
	auto OriginalLocation = DeferredInitParams.OriginalLocation;
	auto CubeReserveCount = DeferredInitParams.CubeReserveCount;
	auto HitLocation = DeferredInitParams.HitLocation;
	auto TerrainSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();
	SetActorLocation(OriginalLocation);
	float ActorHeight = OriginalLocation.Z;
	float CubeLength = TerrainSubsystem->CubeLength;
	float Top = OriginalLocation.Z + CubeLength / 2;
	float DeltaDistance = Top - HitLocation.Z;
	DeltaDistance = FMath::Max(DeltaDistance, 0.f);
	CubeCount = FMath::CeilToInt(DeltaDistance / CubeLength) + CubeReserveCount;
	float ActorTop = Top;
	float ActorBottom = ActorTop - CubeLength * CubeCount;
	TopOffset = ActorTop - ActorHeight;
	BottomOffset = ActorBottom - ActorHeight;
	BreakCubeIndex.Init(false, CubeCount);
	//hit pos to nearest cube index
	float HitAffectBegin = HitLocation.Z - CubeLength / 2 * 0.8;
	float HitAffectEnd = HitLocation.Z + CubeLength / 2 * 0.8;
	int HitAffectBeginIndex = FMath::FloorToInt((HitAffectBegin - ActorBottom) / CubeLength);
	int HitAffectEndIndex = FMath::CeilToInt((HitAffectEnd - ActorBottom) / CubeLength);
	HitAffectBeginIndex = FMath::Max(HitAffectBeginIndex, 0);
	HitAffectEndIndex = FMath::Min(HitAffectEndIndex, CubeCount);
	for (int i = HitAffectBeginIndex; i < HitAffectEndIndex; i++)
	{
		BreakCubeIndex[i] = true;
	}

	if (CubeCount > 10)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CubeCount = %d"), CubeCount));
		return;
	}
	FVector LocalLocation = FVector(0, 0, (ActorTop + ActorBottom) / 2 - ActorHeight);
	FVector Scale = ComponentScaler;
	int GeometryCollectionIndex = CubeCount - 1;

	FVector Location = FVector(
		OriginalLocation.X,
		OriginalLocation.Y,
		(ActorTop + ActorBottom) / 2);

	FTransform Transform = FTransform(FRotator::ZeroRotator, Location, Scale);

	//box collision test in spawn area

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Location + FVector(0, 0, 1000),
		Location,
		ECC_Visibility);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
	                                 FString::Printf(
		                                 TEXT("HitResult = %s"),
		                                 HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("None")));
	//draw hit point
	// DrawDebugPoint(GetWorld(), HitResult.ImpactPoint,50 ,FColor::Red,false, 10.f, 0.5f);

	AGeometryCollectionActor* GeometryCollectionActor = Cast<AGeometryCollectionActor>(
		GetWorld()->SpawnActor(CubeGeometryCollectionActorClasses[GeometryCollectionIndex], &Transform));

	GeometryCollectionActors.Add(GeometryCollectionActor);

	// auto Component = Cast<UGeometryCollectionComponent>(AddComponentByClass(
	// 	CubeGeometryCollectionComponentClasses[GeometryCollectionComponentIndex],
	// 	false,
	// 	FTransform(FRotator::ZeroRotator, LocalLocation, Scale),
	// 	false));

	// TScriptDelegate<FWeakObjectPtr > Delegate;
	// Delegate.BindUFunction(this, "OnGeometryCollectionBreak");
	// Component->OnChaosBreakEvent.Add(Delegate);
	// GeometryCollectionComponents.Add(Component);

	// UE_LOG(LogTemp, Warning,
	//        TEXT(
	// 	       "OriginalLocation %s\n ActorHeight %f\n CubeLength %f\n Top %f\n DeltaDistance %f\n CubeCount %d\n ActorTop %f\n ActorBottom %f\n HitAffectBegin %f\n HitAffectEnd %f\n HitAffectBeginIndex %d\n HitAffectEndIndex %d\n"
	//        ),
	//        *OriginalLocation.ToString(),
	//        ActorHeight,
	//        CubeLength,
	//        Top,
	//        DeltaDistance,
	//        CubeCount,
	//        ActorTop,
	//        ActorBottom,
	//        HitAffectBegin,
	//        HitAffectEnd,
	//        HitAffectBeginIndex,
	//        HitAffectEndIndex
	// );
	// DrawDebugCircle(GetWorld(),
	// 			FTransform(
	// 				FRotator(90, 0, 0),
	// 				OriginalLocation + FVector(0, 0, TopOffset),
	// 				FVector(1, 1, 1)).ToMatrixNoScale(),
	// 			100, 32, FColor::Yellow, true, 10.f, 0, 5.f);
	// DrawDebugCircle(GetWorld(),
	// 		FTransform(
	// 			FRotator(90, 0, 0),
	// 			OriginalLocation + FVector(0, 0, BottomOffset),
	// 			FVector(1, 1, 1)).ToMatrixNoScale(),
	// 		100, 32, FColor::Yellow, true, 10.f, 0, 5.f);

#ifdef SPLIT_CREATE_ACTOR
	//scan BreakCubeIndex and create actors
	auto CreateNonBreakCube = [=](int Length, float StartHeight)
	{
		if (Length <= 0) return;
		float Top = StartHeight + Length * CubeLength;
		float Bottom = StartHeight;
		FVector LocalLocation = FVector(0, 0, (Top + Bottom) / 2 - ActorHeight);
		FVector Scale = FVector(1, 1, Length);
		
		auto Component = AddComponentByClass(CubeMeshComponentClass,
		                                     false,
		                                     FTransform(FRotator::ZeroRotator, LocalLocation, Scale),
		                                     false);
		
		StaticMeshComponents.Add(Cast<UStaticMeshComponent>(Component));

#if WITH_EDITORONLY_DATA
		//circle on top and bottom
		// DrawDebugCircle(GetWorld(),
		//                 FTransform(
		// 	                FRotator(90, 0, 0),
		// 	                OriginalLocation + FVector(0, 0, Bottom - ActorHeight),
		// 	                FVector(1, 1, 1)).ToMatrixNoScale(),
		//                 100, 32, FColor::Green, true, 10.f, 0, 5.f);
		// DrawDebugCircle(GetWorld(),
		// 		FTransform(
		// 			FRotator(90, 0, 0),
		// 			OriginalLocation + FVector(0, 0, Top - ActorHeight),
		// 			FVector(1, 1, 1)).ToMatrixNoScale(),
		// 		100, 32, FColor::Green, true, 10.f, 0, 5.f);
		DrawDebugBox(GetWorld(), OriginalLocation + LocalLocation, Scale * 50, FColor::Green, true, 10.f, 0, 5.f);
#endif
	};
	auto CreateBreakCube = [=](int Length, float StartHeight)
	{
		if (Length <= 0) return;
// 		float CubeHeight = StartHeight + CubeLength / 2 - ActorHeight;
// 		for (int i = 0; i < Length; i++)
// 		{
// 			auto Component = Cast<UGeometryCollectionComponent>(AddComponentByClass(
// 				CubeGeometryCollectionComponentClass,
// 				false,
// 				FTransform(FVector(0, 0, CubeHeight)),
// 				false));
// 			TScriptDelegate<FWeakObjectPtr> Delegate;
// 			Delegate.BindUFunction(this, "OnGeometryCollectionBreak");
// 			Component->OnChaosBreakEvent.Add(Delegate);
// 			GeometryCollectionComponents.Add(Component);
//
// #if WITH_EDITORONLY_DATA
// 			DrawDebugBox(GetWorld(), OriginalLocation + FVector(0, 0, CubeHeight), FVector(50, 50, 50), FColor::Red,
// 			             true, 10.f, 0, 5.f);
// #endif
// 			CubeHeight += CubeLength;
// 		}
	};


	//non break cubes
	int IndexBegin = 0;
	int IndexEnd = 0;
	float HeightBegin = ActorBottom;
	float Height = ActorBottom;
	for (; IndexEnd < BreakCubeIndex.Num() && BreakCubeIndex[IndexEnd] == false; IndexEnd++, Height += CubeLength);
	if (IndexEnd > BreakCubeIndex.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("sec1 : NonBreakCubeLength >  BreakCubeIndex.Num()"));
	}
	CreateNonBreakCube(IndexEnd - IndexBegin, HeightBegin);
	IndexBegin = IndexEnd;
	HeightBegin = Height;
	//break cubes
	for (; IndexEnd < BreakCubeIndex.Num() && BreakCubeIndex[IndexEnd] == true; IndexEnd++, Height += CubeLength);
	
	CreateBreakCube(IndexEnd - IndexBegin, HeightBegin);
	IndexBegin = IndexEnd;
	HeightBegin = Height;
	//non break cubes
	IndexEnd = BreakCubeIndex.Num();
	CreateNonBreakCube(IndexEnd - IndexBegin, HeightBegin);
#endif
}


// Called every frame
void ATerrainCubeActor::Tick(float DeltaTime)
{
	// Super::Tick(DeltaTime);
	//
	// for (auto GeometryCollectionComponent : GeometryCollectionComponents)
	// {
	// 	GeometryCollectionComponent->SetRenderStateDirty();
	// }
	//
	// for(auto GeometryCollectionActor : GeometryCollectionActors)
	// {
	// 	//inverse gravity force
	// 	FVector WorldGravity = GetWorld()->GetGravityZ() * FVector(0, 0, -1) * InverseGravityMultiplier;
	// 	auto Component = GeometryCollectionActor->GeometryCollectionComponent;
	// 	Component->AddForce(WorldGravity * Component->GetMass());
	//
	// 	DrawDebugLine(GetWorld(), Component->GetComponentLocation(),
	// 	              Component->GetComponentLocation() + WorldGravity * 100, FColor::Red, false, 0.1, 0, 5.f);
	// }
}
