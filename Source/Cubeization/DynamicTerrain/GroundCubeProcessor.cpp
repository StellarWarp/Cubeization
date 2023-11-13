// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundCubeProcessor.h"

#include "FieldGlobalData.h"
#include "GroundCubeTrait.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Kismet/GameplayStatics.h"

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
	const auto elapsedTime = UGameplayStatics::GetTimeSeconds(Context.GetWorld());
	UFieldDataSubsystem* FieldData = Context.GetWorld()->GetSubsystem<UFieldDataSubsystem>();
	auto& HeightField = FieldData->HeightField;
	auto& VelocityField = FieldData->VelocityField;
	const auto& FieldParameter = FieldData->FieldParameter;
	float DeltaTime = Context.GetDeltaTimeSeconds();

	//near by grid transfer
	static constexpr float TransferKernel[3][3] = {
		{0.2f, 0.4f, 0.2f},
		{0.4f, -2.4, 0.4f},
		{0.2f, 0.4f, 0.2f}
	};
	{
		FIntVector2 Begin = FieldData->Offset;
		FIntVector2 End = FieldData->Offset + FieldData->DynamicFieldSize;
		auto& LastHeightField = FieldData->LastHeightField;

		auto VelocityFieldRead = VelocityField;

		for (auto y = Begin.Y; y < End.Y; y++)
		{
			for (auto x = Begin.X; x < End.X; x++)
			{
				float Force = 0;
				for (int32 i = -1; i <= 1; ++i)
				{
					for (int32 j = -1; j <= 1; ++j)
					{
						float ChangingHeight = HeightField[FIntVector2(x + i, y + j)] - LastHeightField[FIntVector2(
							x + i, y + j)];
						Force += FieldParameter.Transfer * ChangingHeight * TransferKernel[i + 1][j + 1];
					}
				}
				VelocityField[FIntVector2(x, y)] += Force * DeltaTime;
			}
		}

		LastHeightField = HeightField;
	}


	//force apply
	{
		auto& Queue = FieldData->DeferredApplyForceQueue;
		while (!Queue.IsEmpty())
		{
			const auto Info = Queue.Peek();
			FieldData->ForRangeVelocity(Info->Center, Info->Radius, [=](const FVector& Distance, float& Val)
			{
				Val += UFieldDataSubsystem::GaussianLUT.LUT128[Distance / Info->Radius]
					* Info->Force * DeltaTime;
			});
			Queue.Pop();
		}
	}


	//field update
	{
		auto& RawBaseHeightField = FieldData->BaseHeightField.GetData();
		auto& RawHeightField = HeightField.GetData();
		auto& RawVelocityField = VelocityField.GetData();

		//test

		for (int i = 0; i < RawHeightField.Num(); ++i)
		{
			float ElasticForce = -FieldParameter.Elasticity * (RawHeightField[i] - RawBaseHeightField[i]);
			float DampingForce = -FieldParameter.Damping * RawVelocityField[i];
			RawVelocityField[i] += (ElasticForce + DampingForce) * DeltaTime;
			RawHeightField[i] += RawVelocityField[i] * DeltaTime;
		}
	}


	//entity sync
	Query.ForEachEntityChunk(EntityManager, Context, [=](FMassExecutionContext& Context) mutable
	{
		const TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FGroundCubeFragment> CubeFragments = Context.GetFragmentView<FGroundCubeFragment>();
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			FTransformFragment& TransformFragment = TransformFragments[i];
			auto& Transform = TransformFragment.GetMutableTransform();
			FIntVector2 LocalIndex = CubeFragments[i].Index;
			FIntVector2 LogicalIndex = FieldData->LocalIndexToLogicalIndex(LocalIndex);
			FVector Location = FieldData->LogicalIndexToWorldPosition(LogicalIndex);
			Location.Z = HeightField[LogicalIndex];
			Transform.SetLocation(Location);


			// DrawDebugLine(Context.GetWorld(),
			// 	Location,
			// 	Location + FVector(0, 0, 100),
			// 	FColor::Red,
			// 	false, 0.1f, 0, 1);
		}
	});
}
