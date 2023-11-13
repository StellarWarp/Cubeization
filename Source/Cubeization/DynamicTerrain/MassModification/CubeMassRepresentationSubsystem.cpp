// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeMassRepresentationSubsystem.h"

#include "ExMassVisualizer.h"
#include "ExMassVisualizationComponent.h"
#include "MassSimulationSubsystem.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "MassAgentSubsystem.h"
#include "MassSimulationSettings.h"
#include "MassEntityUtils.h"

void UCubeMassRepresentationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMassSimulationSubsystem::StaticClass());
	Collection.InitializeDependency(UMassActorSpawnerSubsystem::StaticClass());
	Collection.InitializeDependency(UMassAgentSubsystem::StaticClass());
	UWorldSubsystem::Initialize(Collection);

	if (UWorld* World = GetWorld())
	{
		EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World).AsShared();

		ActorSpawnerSubsystem = World->GetSubsystem<UMassActorSpawnerSubsystem>();
		WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();

		if (Visualizer == nullptr)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// The helper actor is only once per world so we can allow it to spawn during construction script.
			SpawnInfo.bAllowDuringConstructionScript = true;
			Visualizer = World->SpawnActor<AExMassVisualizer>(SpawnInfo);// this is the only modification
			check(Visualizer);
			VisualizationComponent = &Visualizer->GetVisualizationComponent();

#if WITH_EDITOR
			Visualizer->SetActorLabel(FString::Printf(TEXT("%sVisualizer"), *GetClass()->GetName()), /*bMarkDirty*/
			                          false);
#endif
		}

		UMassSimulationSubsystem* SimSystem = World->GetSubsystem<UMassSimulationSubsystem>();
		check(SimSystem);
		SimSystem->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics).AddUObject(
			this, &UCubeMassRepresentationSubsystem::ExOnProcessingPhaseStarted, EMassProcessingPhase::PrePhysics);
		SimSystem->GetOnProcessingPhaseFinished(EMassProcessingPhase::PostPhysics).AddUObject(
			this, &UCubeMassRepresentationSubsystem::ExOnProcessingPhaseStarted, EMassProcessingPhase::PostPhysics);

		UMassAgentSubsystem* AgentSystem = World->GetSubsystem<UMassAgentSubsystem>();
		check(AgentSystem);
		AgentSystem->GetOnMassAgentComponentEntityAssociated().AddUObject(
			this, &UCubeMassRepresentationSubsystem::OnMassAgentComponentEntityAssociated);
		AgentSystem->GetOnMassAgentComponentEntityDetaching().AddUObject(
			this, &UCubeMassRepresentationSubsystem::OnMassAgentComponentEntityDetaching);
	}

	RetryMovedDistanceSq = FMath::Square(GET_MASSSIMULATION_CONFIG_VALUE(DesiredActorFailedSpawningRetryMoveDistance));
	RetryTimeInterval = GET_MASSSIMULATION_CONFIG_VALUE(DesiredActorFailedSpawningRetryTimeInterval);
}

void UCubeMassRepresentationSubsystem::ExOnProcessingPhaseStarted(const float DeltaSeconds,const EMassProcessingPhase Phase) const
{
	check(VisualizationComponent);
	switch (Phase)
	{
	case EMassProcessingPhase::PrePhysics:
		{
			if(const auto ExComp = dynamic_cast<UExMassVisualizationComponent*>(VisualizationComponent.Get()))
				ExComp->ExBeginVisualChanges();
		}
		break;
	case EMassProcessingPhase::PostPhysics:/* Currently this is the end of phases signal */
		VisualizationComponent->EndVisualChanges();
		break;
	default:
		check(false); // Need to handle this case
		break;
	}
}
