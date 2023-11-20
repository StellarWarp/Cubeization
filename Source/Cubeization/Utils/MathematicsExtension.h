#pragma once


inline FIntVector2 operator-(const FIntVector2& Lhs, const FIntVector2& Rhs)
{
	return FIntVector2(Lhs.X - Rhs.X, Lhs.Y - Rhs.Y);
}

inline FIntVector2 operator+(const FIntVector2& Lhs, const FIntVector2& Rhs)
{
	return FIntVector2(Lhs.X + Rhs.X, Lhs.Y + Rhs.Y);
}