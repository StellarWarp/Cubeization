#pragma once

#include "CoreMinimal.h"
#include "Utils/FieldArray.h"
#include "Utils/MathematicsExtension.h"
#include "TerrainDataSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFieldParameter
{
	GENERATED_BODY()

	FFieldParameter() = default;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Elasticity = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damping = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Transfer = 0;

	//return is finished
	bool Lerp(const FFieldParameter& Other, const float Alpha)
	{
		Elasticity = FMath::Lerp(Elasticity, Other.Elasticity, Alpha);
		Damping = FMath::Lerp(Damping, Other.Damping, Alpha);
		Transfer = FMath::Lerp(Transfer, Other.Transfer, Alpha);

		return FMath::IsNearlyEqual(Elasticity, Other.Elasticity);
	}
};

UCLASS()
class UTerrainDataSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()


	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	void GenerateMap();

private:
	bool bPreGridParameter = false;

public:
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
	TFieldArray<float> ActivationShiftField;
	TFieldArray<float> HeightField;
	TFieldArray<float> VelocityField;
	TFieldArray<float> AccelerationField;
	TFieldArray<uint32> InstanceIndexId;

	TMap<FIntVector2, TFunction<void(const FIntVector2&)>> EventTriggerMap;

	struct FDeferredApplyForce
	{
		FVector Center;
		float Force;
		float Radius;
	};

	TQueue<FDeferredApplyForce> DeferredApplyForceQueue;

public:
	bool bExternalAnimationControl = false;
	void MapDataToField();

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

	void AddPointForce(const FVector& Pos, const float Acceleration)
	{
		const float DeltaTime = GetWorld()->GetDeltaSeconds();
		VelocityField[WorldPositionToLogicalIndex(Pos)] += Acceleration * DeltaTime;
	}

	void ForRangeBaseHeight(const FVector& Center, const uint32 HalfExtent,
	                        const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		BaseHeightField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
	}

	void ForRangeHeight(const FVector& Center, const uint32 HalfExtent,
	                    const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		HeightField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
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

	void ForRangeActivationShift(const FVector& Center, const uint32 HalfExtent,
	                             const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		ActivationShiftField.ForRange(WorldPositionToLogicalPosition(Center), HalfExtent, Func);
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

	void ActivationShiftSetUniform(float Value)
	{
		auto& Raw = ActivationShiftField.GetData();
		for (int i = 0; i < Raw.Num(); ++i)
		{
			Raw[i] = Value;
		}
	}

	void CenterActiveField(const FVector& Center,
	                       const float HalfExtent, const float FallOff,
	                       const float Max, const float Min)
	{
		ForRangeActivationShift(Center, HalfExtent + FallOff + 1, [=](const FVector& Distance, float& Val)
		{
			float X_Dist = FMath::Abs(Distance.X);
			float Y_Dist = FMath::Abs(Distance.Y);

			float Dist = FMath::Max(X_Dist, Y_Dist);
			if (Dist < FallOff)
			{
				Val = Max;
			}
			else
			{
				Val = (Dist - HalfExtent) * (Min - Max) / (FallOff) + Max;
			}
		});
	}

	void TriggerQuery(const FVector& WorldPosition)
	{
		auto LogicalIndex = WorldPositionToLogicalIndex(WorldPosition);
		if (EventTriggerMap.Contains(LogicalIndex))
		{
			EventTriggerMap[LogicalIndex](LogicalIndex);
		}
	}

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

private:
	bool bFieldParameterLerp = false;
	float FieldParameterLerpSpeed = 1;
	FFieldParameter FieldParameter;
	FFieldParameter NewFieldParameter;
	TFieldArray<FFieldParameter> PreGridParameterField;
public:

	UFUNCTION(BlueprintCallable)
	void SetFieldParameter(const FFieldParameter& Parameter, float Speed = 1);

	void FieldParameterLerpTick(float DeltaTime);
};
