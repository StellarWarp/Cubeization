#include "FieldGlobalData.h"

void UFieldDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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
	BaseHeightField.Init(ComputeSize.X, ComputeSize.Y);
	LastHeightField.Init(ComputeSize.X, ComputeSize.Y);
	HeightField.Init(ComputeSize.X, ComputeSize.Y);
	VelocityField.Init(ComputeSize.X, ComputeSize.Y);
	AccelerationField.Init(ComputeSize.X, ComputeSize.Y);
	CubeIndexAllocator = {this};

	GenerateMap();
}


void UFieldDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UFieldDataSubsystem::GenerateMap()
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

	//copy to base
	for (int32 j = 0; j < DynamicFieldSize.Y; ++j)
	{
		for (int32 i = 0; i < DynamicFieldSize.X; ++i)
		{
			auto LogicalIndex = LocalIndexToLogicalIndex(FIntVector2(i, j));
			BaseHeightField[LogicalIndex] = MapHeightField[LogicalIndex];
			LastHeightField[LogicalIndex] = MapHeightField[LogicalIndex];
			HeightField[LogicalIndex] = MapHeightField[LogicalIndex];
		}
	}
}


void UFieldDataSubsystem::ShiftField(FIntVector2 OffsetIncrement)
{
	auto AppendData = [this](const FIntVector2& Begin, const FIntVector2& End)
	{
		for (int32 j = Begin.Y; j < End.Y; ++j)
		{
			for (int32 i = Begin.X; i < End.X; ++i)
			{
				BaseHeightField[FIntVector2(i, j)] = MapHeightField[FIntVector2(i, j)];
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

void UFieldDataSubsystem::UpdateCenter(FVector Center)
{
	auto Pos = LogicalIndex(Center) - FIntVector2(DynamicFieldSize.X / 2, DynamicFieldSize.Y / 2);
	if (Pos != Offset)
	{
		const auto OffsetIncrement = Pos - Offset;
		ShiftField(OffsetIncrement);
	}
}
