// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundCubeProcessor.h"

#include "TerrainDataSubsystem.h"
#include "GroundCubeTrait.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "TerrainRepresentationSubsystem.h"

UGroundCubeProcessor::UGroundCubeProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UGroundCubeProcessor::ConfigureQueries()
{
	Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	Query.AddRequirement<FGroundCubeFragment>(EMassFragmentAccess::ReadOnly);
	Query.RegisterWithProcessor(*this);
}

void UGroundCubeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UTerrainDataSubsystem* FieldData = Context.GetWorld()->GetSubsystem<UTerrainDataSubsystem>();
	auto& HeightField = FieldData->HeightField;	
	//entity sync
	Query.ForEachEntityChunk(EntityManager, Context, [=](FMassExecutionContext& Context) mutable
	{
		const TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FGroundCubeFragment> CubeFragments = Context.GetFragmentView<FGroundCubeFragment>();
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			FTransformFragment& TransformFragment = TransformFragments[i];
			auto& Transform = TransformFragment.GetMutableTransform();

			auto ModToPositive = [] (int32& Value, const int32 Mod)
			{
				Value = (Value % Mod + Mod) % Mod;
			};

			
			FIntVector2 LocalIndex = CubeFragments[i].Index - FieldData->Offset;
			ModToPositive(LocalIndex.X, FieldData->DynamicFieldSize.X);
			ModToPositive(LocalIndex.Y, FieldData->DynamicFieldSize.Y);
			
			FIntVector2 LogicalIndex = FieldData->LocalIndexToLogicalIndex(LocalIndex);
			FVector Location = FieldData->LogicalIndexToWorldPosition(LogicalIndex);
			Location.Z = HeightField[LogicalIndex];
			Transform.SetLocation(Location);
		}
	});


}
