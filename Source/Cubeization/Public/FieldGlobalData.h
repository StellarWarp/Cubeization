#pragma once

#include "CoreMinimal.h"
#include "FieldGlobalData.generated.h"

inline FIntVector2 operator-(const FIntVector2& Lhs, const FIntVector2& Rhs)
{
	return FIntVector2(Lhs.X - Rhs.X, Lhs.Y - Rhs.Y);
}

inline FIntVector2 operator+(const FIntVector2& Lhs, const FIntVector2& Rhs)
{
	return FIntVector2(Lhs.X + Rhs.X, Lhs.Y + Rhs.Y);
}

UCLASS()
class UFieldDataSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void GenerateMap();


	template <typename T, bool bLoop = true>
	struct TFieldArray
	{
	private:
		TArray<T> Data;
		uint32 SizeX;
		uint32 SizeY;

	public:
		TFieldArray() = default;

		TFieldArray(const uint32 x, const uint32 y)
			: SizeX(x), SizeY(y)
		{
			Data.Init(0, x * y);
		}

		//copy constructor
		TFieldArray(const TFieldArray& Other)
			: SizeX(Other.SizeX), SizeY(Other.SizeY)
		{
			Data = Other.Data;
		}

		//copy assignment
		TFieldArray& operator=(const TFieldArray& Other)
		{
			SizeX = Other.SizeX;
			SizeY = Other.SizeY;
			Data = Other.Data;
			return *this;
		}

		void Init(const uint32 x, const uint32 y)
		{
			SizeX = x;
			SizeY = y;
			Data.Init(0, x * y);
		}

		T& operator[](const FIntVector2& Pos)
		{
			if constexpr (bLoop)
				//return Data[(Pos.X % SizeX + SizeX) % SizeX + (Pos.Y % SizeY + SizeY) % SizeY * SizeX];
				//optimization for 2^n Size
				return Data[(Pos.X & (SizeX - 1)) + (Pos.Y & (SizeY - 1)) * SizeX];
			else
				return Data[Pos.X + Pos.Y * SizeX];
		}

		const T& operator[](const FIntVector2& Pos) const
		{
			if constexpr (bLoop)
				//return Data[(Pos.X % SizeX + SizeX) % SizeX + (Pos.Y % SizeY + SizeY) % SizeY * SizeX];
				//optimization for 2^n Size
				return Data[(Pos.X & (SizeX - 1)) + (Pos.Y & (SizeY - 1)) * SizeX];
			else
				return Data[Pos.X + Pos.Y * SizeX];
		}

		T GetInterpreted(const FVector& Pos)
		{
			//biliner interpolation
			FIntVector2 P0 = FIntVector2(FMath::FloorToInt(Pos.X), FMath::FloorToInt(Pos.Y));
			float u = Pos.X - P0.X;
			float inv_u = 1 - u;
			float v = Pos.Y - P0.Y;
			float inv_v = 1 - v;
			T P00 = (*this)[P0];
			T P01 = (*this)[FIntVector2(P0.X, P0.Y + 1)];
			T P10 = (*this)[FIntVector2(P0.X + 1, P0.Y)];
			T P11 = (*this)[FIntVector2(P0.X + 1, P0.Y + 1)];
			return P00 * inv_u * inv_v + P01 * inv_u * v + P10 * u * inv_v + P11 * u * v;
		}

		void ForRange(const FVector& Center, const uint32 HalfExtent, TFunctionRef<void(const FVector&, T&)> Func)
		{
			const FIntVector2 GridCenter = FIntVector2(FMath::FloorToInt(Center.X), FMath::FloorToInt(Center.Y));
			FIntVector2 Begin = FIntVector2(GridCenter.X - HalfExtent, GridCenter.Y - HalfExtent);
			FIntVector2 End = FIntVector2(GridCenter.X + HalfExtent, GridCenter.Y + HalfExtent);
			for (int i = Begin.X; i <= End.X; ++i)
			{
				for (int j = Begin.Y; j <= End.Y; ++j)
				{
					Func(FVector(i - Center.X, j - Center.Y, 0), (*this)[{i, j}]);
					// DrawDebugLine(GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World(),
					// 	FVector(i, j, 0) * 100,
					// 	FVector(i, j, 0) * 100 + FVector(0, 0, (*this)[{i, j}]) * 100,
					// 	FColor::Red);
				}
			}
		}

		TArray<T>& GetData()
		{
			return Data;
		}

		bool IsValidIndex(const FIntVector2& Pos) const
		{
			return Pos.X >= 0 && Pos.X < SizeX && Pos.Y >= 0 && Pos.Y < SizeY;
		}

		int32 GetSizeX() const
		{
			return SizeX;
		}

		int32 GetSizeY() const
		{
			return SizeY;
		}
	};

	// FTransform Transform;
	float CubeLength = 100;
	FVector WorldOffset;
	FIntVector2 Offset;
	FIntVector2 DynamicFieldSize;
	TFieldArray<float> MapHeightField;
	TFieldArray<float> BaseHeightField;
	TFieldArray<float> LastHeightField;
	TFieldArray<float> HeightField;
	TFieldArray<float> VelocityField;
	TFieldArray<float> AccelerationField;

	struct FDeferredApplyForce
	{
		FVector Center;
		float Force;
		float Radius;
	};

	TQueue<FDeferredApplyForce> DeferredApplyForceQueue;

	FVector LocalPosition(const FVector& WorldPos) const
	{
		return (WorldPos - WorldOffset) / CubeLength - FVector(Offset.X, Offset.Y, 0);
	}

public:
	float GetBaseHeight(const FIntVector2& Pos) { return BaseHeightField[Pos]; }
	float GetHeight(const FIntVector2& Pos) { return HeightField[Pos]; }
	float GetVelocity(const FIntVector2& Pos) { return VelocityField[Pos]; }
	float GetAcceleration(const FIntVector2& Pos) { return AccelerationField[Pos]; }

	float GetHeight(const FVector& Pos) { return HeightField.GetInterpreted(LogicalPosition(Pos)); }
	float GetVelocity(const FVector& Pos) { return VelocityField.GetInterpreted(LogicalPosition(Pos)); }
	float GetAcceleration(const FVector& Pos) { return AccelerationField.GetInterpreted(LogicalPosition(Pos)); }

	void ForRangeHeight(const FVector& Center, const uint32 HalfExtent,
	                    const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		HeightField.ForRange(LogicalPosition(Center), HalfExtent, Func);
		DrawDebugBox(GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)->World(),
		             FVector(Center.X, Center.Y, 0),
		             FVector(HalfExtent * 100, HalfExtent * 100, 100), FColor::Red);
	}

	void ForRangeVelocity(const FVector& Center, const uint32 HalfExtent,
	                      const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		VelocityField.ForRange(LogicalPosition(Center), HalfExtent, Func);
	}

	void ForRangeAcceleration(const FVector& Center, const uint32 HalfExtent,
	                          const TFunctionRef<void(const FVector&, float&)>& Func)
	{
		AccelerationField.ForRange(LogicalPosition(Center), HalfExtent, Func);
	}

	FIntVector2 IndexMap(uint32 Index) const
	{
		return FIntVector2(Index % DynamicFieldSize.X, Index / DynamicFieldSize.X);
	}

	FVector IndexMap(const FIntVector2& Index) const
	{
		return FVector(Index.X + Offset.X, Index.Y + Offset.Y, 0) * CubeLength + WorldOffset;
	}

	FIntVector2 LogicalIndex(const FVector& Pos) const
	{
		const FVector LogicalPos = LogicalPosition(Pos);
		return FIntVector2(FMath::RoundToInt(LogicalPos.X), FMath::RoundToInt(LogicalPos.Y));
	}

	FVector LogicalPosition(const FVector& Pos) const
	{
		return (Pos - WorldOffset) / CubeLength;
	}

	FIntVector2 LocalIndexToLogicalIndex(const FIntVector2& LocalIndex) const
	{
		return LocalIndex + Offset;
	}

	FVector LogicalIndexToWorldPosition(const FIntVector2& LogicalIndex) const
	{
		return FVector(LogicalIndex.X, LogicalIndex.Y, HeightField[LogicalIndex]) * CubeLength + WorldOffset;
	}

	FVector LocalIndexToWorldPosition(const FIntVector2& LocalIndex) const
	{
		return LogicalIndexToWorldPosition(LocalIndexToLogicalIndex(LocalIndex));
	}

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
		UFieldDataSubsystem* Attach;
		uint32 Count = 0;

		FCubeIndexAllocator() = default;

		FCubeIndexAllocator(UFieldDataSubsystem* Attach): Attach(Attach)
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
