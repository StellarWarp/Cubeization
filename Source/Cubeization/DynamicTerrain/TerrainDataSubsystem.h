#pragma once

#include "CoreMinimal.h"
#include "Utils/FieldArray.h"
#include "Utils/MathematicsExtension.h"
#include "TerrainDataSubsystem.generated.h"


UCLASS()
class UTerrainDataSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void GenerateMap();


	float CubeLength = 100;
	FVector WorldOffset;
	FIntVector2 Offset;
	FIntVector2 DynamicFieldSize;
	FIntVector2 ActiveFieldSize;

	//todo move this to a separate class and use editor to generate or modify map
	TFieldArray<float> MapHeightField;
	TFieldArray<float> MapModifyField;

	TFieldArray<float> BaseHeightField;
	TFieldArray<float> LastHeightField;
	TFieldArray<float> HeightField;
	TFieldArray<float> VelocityField;
	TFieldArray<float> AccelerationField;
	TFieldArray<uint32> InstanceIndexId;

	struct FDeferredApplyForce
	{
		FVector Center;
		float Force;
		float Radius;
	};

	TQueue<FDeferredApplyForce> DeferredApplyForceQueue;

private:

	

public:
	float GetBaseHeight(const FIntVector2& Pos) { return BaseHeightField[Pos]; }
	float GetHeight(const FIntVector2& Pos) { return HeightField[Pos]; }
	float GetVelocity(const FIntVector2& Pos) { return VelocityField[Pos]; }
	float GetAcceleration(const FIntVector2& Pos) { return AccelerationField[Pos]; }

	float GetHeight(const FVector& Pos) { return HeightField.GetInterpreted(WorldPositionToLogicalPosition(Pos)); }
	float GetVelocity(const FVector& Pos) { return VelocityField.GetInterpreted(WorldPositionToLogicalPosition(Pos)); }

	float GetAcceleration(const FVector& Pos)
	{
		return AccelerationField.GetInterpreted(WorldPositionToLogicalPosition(Pos));
	}

	void ForRangeHeight(const FVector& Center, const uint32 HalfExtent,
	                    const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		HeightField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
		DrawDebugBox(GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World(),
		             FVector(Center.X, Center.Y, 0),
		             FVector(HalfExtent * 100, HalfExtent * 100, 100), FColor::Red);
	}

	void ForRangeVelocity(const FVector& Center, const uint32 HalfExtent,
	                      const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		VelocityField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
	}

	void ForRangeAcceleration(const FVector& Center, const uint32 HalfExtent,
	                          const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		AccelerationField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
	}

	/*
	 * index - position API
	 *
	 * LocalIndex: index in the local space, which is the index on the simulation field area
	 * LogicalIndex: index in the logical space, which is the index on the map
	 * LogicalPosition: position in the logical space, float version of LogicalIndex
	 * 
	 */
	FIntVector2 IndexMap(uint32 Index) const
	{
		return FIntVector2(Index % DynamicFieldSize.X, Index / DynamicFieldSize.X);
	}

	FVector IndexMap(const FIntVector2& Index) const
	{
		return FVector(Index.X + Offset.X, Index.Y + Offset.Y, 0) * CubeLength + WorldOffset;
	}

	FIntVector2 WorldPositionToLogicalIndex(const FVector& Pos) const
	{
		const FVector LogicalPos = WorldPositionToLogicalPosition(Pos);
		return FIntVector2(FMath::RoundToInt(LogicalPos.X), FMath::RoundToInt(LogicalPos.Y));
	}

	FVector WorldPositionToLogicalPosition(const FVector& Pos) const
	{
		return (Pos - WorldOffset) / CubeLength;
	}

	FIntVector2 LocalIndexToLogicalIndex(const FIntVector2& LocalIndex) const
	{
		return LocalIndex + Offset;
	}

	FIntVector2 LogicalIndexToLocalIndex(const FIntVector2& LogicalIndex) const
	{
		return LogicalIndex - Offset;
	}

	FVector LogicalIndexToWorldPosition(const FIntVector2& LogicalIndex) const
	{
		return FVector(LogicalIndex.X * CubeLength, LogicalIndex.Y * CubeLength, HeightField[LogicalIndex]) +
			WorldOffset;
	}

	FVector LocalIndexToWorldPosition(const FIntVector2& LocalIndex) const
	{
		return LogicalIndexToWorldPosition(LocalIndexToLogicalIndex(LocalIndex));
	}

	//end index - position API

	void ApplyForce(const FVector& Center, const float Force, const float Radius)
	{
		DeferredApplyForceQueue.Enqueue({Center, Force, Radius});
	}


	void ShiftField(FIntVector2 OffsetIncrement);

	void UpdateCenter(FVector Center);

public:
	struct FGaussianLUT
	{
		template <int SIZE>
		struct PrecomputeLUT
		{
			float LUT[SIZE][SIZE];

			constexpr PrecomputeLUT()
			{
				float Sum = 0;
				for (int i = 0; i < SIZE; ++i)
				{
					for (int j = 0; j < SIZE; ++j)
					{
						LUT[i][j] = exp(-(i * i + j * j) / (2.0f) * (3 * 3) / (SIZE * SIZE));
						Sum += LUT[i][j];
					}
				}
				// for (int i = 0; i < SIZE; ++i)
				// {
				// 	for (int j = 0; j < SIZE; ++j)
				// 	{
				// 		LUT[i][j] /= Sum;
				// 	}
				// }
			}

			float operator()(const int x, const int y)
			{
				return LUT[abs(x)][abs(y)];
			}

			float operator[](FVector Pos)
			{
				//biliner interpolation
				Pos *= SIZE;
				Pos = Pos.GetAbs();
				FIntVector2 P0 = FIntVector2(FMath::FloorToInt(Pos.X), FMath::FloorToInt(Pos.Y));
				if (P0.X + 1 >= SIZE || P0.Y + 1 >= SIZE) return 0;
				const float U = Pos.X - P0.X;
				const float Inv_U = 1 - U;
				const float V = Pos.Y - P0.Y;
				const float Inv_V = 1 - V;
				const float P00 = LUT[P0.X][P0.Y];
				const float P01 = LUT[P0.X][P0.Y + 1];
				const float P10 = LUT[P0.X + 1][P0.Y];
				const float P11 = LUT[P0.X + 1][P0.Y + 1];
				return P00 * Inv_U * Inv_V +
					P01 * Inv_U * V +
					P10 * U * Inv_V +
					P11 * U * V;
			}
		};

		PrecomputeLUT<128> LUT128;
		PrecomputeLUT<256> LUT256;
	};

	inline static FGaussianLUT GaussianLUT;

public:
	struct FCubeIndexAllocator
	{
		UTerrainDataSubsystem* Attach;
		uint32 Count = 0;

		FCubeIndexAllocator() = default;

		FCubeIndexAllocator(UTerrainDataSubsystem* Attach): Attach(Attach)
		{
		}

		FIntVector2 Allocate()
		{
			const FIntVector2 Index = FIntVector2(Count % Attach->DynamicFieldSize.X,
			                                      Count / Attach->DynamicFieldSize.X);
			Count++;
			return Index;
		}
	};

	FCubeIndexAllocator CubeIndexAllocator;

public:
	struct FFieldParameter
	{
		float Elasticity = 0;
		float Damping = 0;
		float Transfer = 0;
		float Debug1 = 0;
		float Debug2 = 0;
		FVector Debug3 = {0, 0, 0};
	} FieldParameter;

	UFUNCTION(BlueprintCallable)
	void SetFieldParameter(
		const float Elasticity,
		const float Damping,
		const float Transfer,
		const float Debug1,
		const float Debug2,
		const FVector Debug3
	)
	{
		FieldParameter = {
			Elasticity,
			Damping,
			Transfer,
			Debug1,
			Debug2,
			Debug3
		};
	}
};
