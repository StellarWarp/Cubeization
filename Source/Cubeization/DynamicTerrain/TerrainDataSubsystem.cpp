#include "TerrainDataSubsystem.h"

void UTerrainDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FIntVector2 MapSize = FIntVector2(512, 512);
	FIntVector2 ComputeSize = FIntVector2(64, 64);

	CubeLength = 100;
	WorldOffset = FVector(-50 - MapSize.X * CubeLength / 2, -50 - MapSize.X * CubeLength / 2, 0);
	Offset = FIntVector2(MapSize.X / 2 - ComputeSize.X / 2, MapSize.Y / 2 - ComputeSize.Y / 2);
	DynamicFieldSize.X = (ComputeSize.X);
	DynamicFieldSize.Y = (ComputeSize.Y);
	
	MapHeightField.Init(MapSize.X, MapSize.Y);
	MapModifyField.Init(MapSize.X, MapSize.Y);

	
	BaseHeightField.Init(ComputeSize.X, ComputeSize.Y);
	LastHeightField.Init(ComputeSize.X, ComputeSize.Y);
	ActivationShiftField.Init(ComputeSize.X, ComputeSize.Y);
	HeightField.Init(ComputeSize.X, ComputeSize.Y);
	VelocityField.Init(ComputeSize.X, ComputeSize.Y);
	AccelerationField.Init(ComputeSize.X, ComputeSize.Y);
	InstanceIndexId.Init(ComputeSize.X, ComputeSize.Y);

	
	CubeIndexAllocator = {this};

	//todo this is a temp solution
	GenerateMap();
}


void UTerrainDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UTerrainDataSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bExternalAnimationControl) return;
	
	//near by grid transfer
	static constexpr float TransferKernel[3][3] = {
		{0.2f, 0.4f, 0.2f},
		{0.4f, -2.4, 0.4f},
		{0.2f, 0.4f, 0.2f}
	};
	{
		FIntVector2 Begin = Offset;
		FIntVector2 End = Offset + DynamicFieldSize;

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
						float ChangingHeight =
							HeightField[FIntVector2(x + i, y + j)] - LastHeightField[FIntVector2(x + i, y + j)];
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
		auto& Queue = DeferredApplyForceQueue;
		while (!Queue.IsEmpty())
		{
			const auto Info = Queue.Peek();
			ForRangeVelocity(Info->Center, Info->Radius, [=](const FVector& Distance, float& Val)
			{
				Val += UTerrainDataSubsystem::GaussianLUT.LUT128[Distance / Info->Radius]
					* Info->Force * DeltaTime;
			});
			Queue.Pop();
		}
	}


	//field update
	{
		auto& RawBaseHeightField = BaseHeightField.GetData();
		auto& RawHeightField = HeightField.GetData();
		auto& RawActivationField = ActivationShiftField.GetData();
		auto& RawVelocityField = VelocityField.GetData();


		for (int i = 0; i < RawHeightField.Num(); ++i)
		{
			float TargetHeight = RawBaseHeightField[i] + RawActivationField[i];
			float ElasticForce = -FieldParameter.Elasticity * (RawHeightField[i] - TargetHeight);
			float DampingForce = -FieldParameter.Damping * RawVelocityField[i];
			RawVelocityField[i] += (ElasticForce + DampingForce) * DeltaTime;
			RawHeightField[i] += RawVelocityField[i] * DeltaTime;
		}
	}

	FieldParameterLerpTick(DeltaTime);
}

void UTerrainDataSubsystem::GenerateMap()
{
	int32 Xmid = MapHeightField.GetSizeX() / 2;
	int32 Ymid = MapHeightField.GetSizeY() / 2;

	for (int32 j = 0; j < Xmid; ++j)
	{
		for (int32 i = 0; i < Ymid; ++i)
		{
			if (rand() % 100 < 3)
			{
				MapHeightField[FIntVector2(i, j)] = FMath::FRandRange(300.0f, 800.0f);
			}
			else
			{
				MapHeightField[FIntVector2(i, j)] = 0;
			}
		}
	}

	for (int32 j = Ymid; j < MapHeightField.GetSizeY(); ++j)
	{
		for (int32 i = 0; i < Ymid; ++i)
		{
			if (i % 5 == 0 || j % 5 == 0)
			{
				MapHeightField[FIntVector2(i, j)] = 100;
			}
			else
			{
				MapHeightField[FIntVector2(i, j)] = 0;
			}
		}
	}

	for (int32 j = 0; j < Xmid; ++j)
	{
		for (int32 i = Ymid; i < MapHeightField.GetSizeX(); ++i)
		{
			if (i % 5 == 0 || j % 5 == 0)
			{
				MapHeightField[FIntVector2(i, j)] = 0;
			}
			else
			{
				MapHeightField[FIntVector2(i, j)] = 100;
			}
		}
	}

	for (int32 j = Ymid; j < MapHeightField.GetSizeY(); ++j)
	{
		for (int32 i = Ymid; i < MapHeightField.GetSizeX(); ++i)
		{
			if (rand() % 100 < 3)
			{
				MapHeightField[FIntVector2(i, j)] = FMath::FRandRange(300.0f, 800.0f);
			}
			else
			{
				MapHeightField[FIntVector2(i, j)] = 0;
			}
		}
	}

	// //copy to base
	// for (int32 j = 0; j < DynamicFieldSize.Y; ++j)
	// {
	// 	for (int32 i = 0; i < DynamicFieldSize.X; ++i)
	// 	{
	// 		auto LogicalIndex = LocalIndexToLogicalIndex(FIntVector2(i, j));
	// 		BaseHeightField[LogicalIndex] = MapHeightField[LogicalIndex];
	// 		LastHeightField[LogicalIndex] = MapHeightField[LogicalIndex];
	// 		HeightField[LogicalIndex] = MapHeightField[LogicalIndex];
	// 	}
	// }
}


void UTerrainDataSubsystem::MapDataToField()
{
	for (int32 j = 0; j < DynamicFieldSize.Y; ++j)
	{
		for (int32 i = 0; i < DynamicFieldSize.X; ++i)
		{
			auto LogicalIndex = LocalIndexToLogicalIndex(FIntVector2(i, j));
			BaseHeightField[LogicalIndex] = MapHeightField[LogicalIndex];
		}
	}
}


void UTerrainDataSubsystem::ShiftField(FIntVector2 OffsetIncrement)
{
	auto AppendData = [this](const FIntVector2& Begin, const FIntVector2& End)
	{
		for (int32 j = Begin.Y; j < End.Y; ++j)
		{
			for (int32 i = Begin.X; i < End.X; ++i)
			{
				BaseHeightField[FIntVector2(i, j)] = MapHeightField[FIntVector2(i, j)] + MapModifyField[FIntVector2(i, j)];
				LastHeightField[FIntVector2(i, j)] = 0;
				HeightField[FIntVector2(i, j)] = 0;
				VelocityField[FIntVector2(i, j)] = 0;
			}
		}
	};

	if (OffsetIncrement.X != 0)
	{
		Offset.X += OffsetIncrement.X;
		FIntVector2 XBegin = Offset;
		FIntVector2 XEnd = XBegin + FIntVector2(DynamicFieldSize.X, DynamicFieldSize.Y);
		if (OffsetIncrement.X > 0)
		{
			XBegin.X += DynamicFieldSize.X - OffsetIncrement.X;
		}
		else
		{
			XEnd.X += -DynamicFieldSize.X - OffsetIncrement.X;
		}
		AppendData(XBegin, XEnd);
	}
	if (OffsetIncrement.Y != 0)
	{
		Offset.Y += OffsetIncrement.Y;
		FIntVector2 YBegin = Offset;
		FIntVector2 YEnd = YBegin + FIntVector2(DynamicFieldSize.X, DynamicFieldSize.Y);
		if (OffsetIncrement.Y > 0)
		{
			YBegin.Y += DynamicFieldSize.Y - OffsetIncrement.Y;
		}
		else
		{
			YEnd.Y += -DynamicFieldSize.Y - OffsetIncrement.Y;
		}
		AppendData(YBegin, YEnd);
	}
}

void UTerrainDataSubsystem::UpdateCenter(FVector Center)
{
	auto Pos = WorldPositionToLogicalIndex(Center) - FIntVector2(DynamicFieldSize.X / 2, DynamicFieldSize.Y / 2);
	if (Pos != Offset)
	{
		const auto OffsetIncrement = Pos - Offset;
		ShiftField(OffsetIncrement);
	}
}

void UTerrainDataSubsystem::SetFieldParameter(const FFieldParameter& Parameter, float Speed)
{
	bFieldParameterLerp = true;
	NewFieldParameter = Parameter;
	FieldParameterLerpSpeed = Speed;
}

void UTerrainDataSubsystem::FieldParameterLerpTick(float DeltaTime)
{
	if(!bFieldParameterLerp) return;
	if(FieldParameter.Lerp(NewFieldParameter, DeltaTime * FieldParameterLerpSpeed))
		bFieldParameterLerp = false;

}
