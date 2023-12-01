#pragma once


template <typename T, bool bLoop = true>
struct TFieldArray
{
private:
	
	TArray<T> Data;
	uint32 SizeX;
	uint32 SizeY;

public:
	TFieldArray();

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
		Data.Init(T{}, x * y);
	}

	void Init(const T& val, const uint32 x, const uint32 y)
	{
		SizeX = x;
		SizeY = y;
		Data.Init(val, x * y);
	}

	TFieldArray(const TArray<T>& In, const uint32 InSizeX, const uint32 InSizeY)
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		Data = In;
	}

	void ToPrimitiveTypes(TArray<float>& Out,uint32& OutSizeX,uint32& OutSizeY)
	{
		OutSizeX = SizeX;
		OutSizeY = SizeY;
		Out = Data;
	}

	void LoadFromPrimitiveTypes(const TArray<float>& In, const uint32 InSizeX, const uint32 InSizeY)
	{
		SizeX = InSizeX;
		SizeY = InSizeY;
		Data = In;
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

	void UniformSet(const T& Val)
	{
		for(auto& Elem : Data)
		{
			Elem = Val;
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

template <typename T, bool bLoop>
TFieldArray<T, bLoop>::TFieldArray()
{
}
