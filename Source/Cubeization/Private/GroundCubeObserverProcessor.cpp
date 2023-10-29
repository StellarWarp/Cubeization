// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundCubeObserverProcessor.h"

#include "GroundCubeTrait.h"
#include "MassExecutionContext.h"
#include "FieldGlobalData.h"
#include "MassCommonFragments.h"


UGroundCubeObserverProcessor::UGroundCubeObserverProcessor()
{
	ObservedType = FGroundCubeFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UGroundCubeObserverProcessor::ConfigureQueries()
{
	Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FGroundCubeFragment>(EMassFragmentAccess::ReadWrite);
	Query.RegisterWithProcessor(*this);
}

void UGroundCubeObserverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UFieldDataSubsystem* FieldData = Context.GetWorld()->GetSubsystem<UFieldDataSubsystem>();

	Query.ForEachEntityChunk(EntityManager, Context, [=](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FGroundCubeFragment> CubeFragments = Context.GetMutableFragmentView<FGroundCubeFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			auto& Index = CubeFragments[i].Index;
			Index = FieldData->CubeIndexAllocator.Allocate();
		}
	});
}
