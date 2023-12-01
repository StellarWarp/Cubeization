#pragma once

#include "CoreMinimal.h"


struct FMathUtils
{
	struct HermiteInterpolation
	{
		static void Params(const float x0, const float v0, const float x1, const float v1, const float time, FVector4d& factors)
		{
			ensure(time != 0);
			const float time2 = (time * time);
			const float invTime = 1 / time;
			const float invTime2 = 1 / time2;
			const float invTime3 = 1 / (time2 * time);

			factors = FVector4d
			{
				 x0,
				 v0,
				 -3 * invTime2 * x0 - 2 * invTime * v0 + 3 * invTime2 * x1 - invTime * v1,
				 2 * invTime3 * x0 + invTime2 * v0 - 2 * invTime3 * x1 + invTime2 * v1,
			};
		}

		static void MotionVectors(const FVector4d& factors, const float t, float& x)
		{
			const float t2 = t * t;
			const float t3 = t2 * t;
			x = factors.X + factors.Y * t + factors.Z * t2 + factors.W * t3;
		}
		
		static void MotionVectors(const FVector4d& factors, const float t, float& x, float& v)
		{
			const float t2 = t * t;
			const float t3 = t2 * t;
			x = factors.X + factors.Y * t + factors.Z * t2 + factors.W * t3;
			v = factors.Y + factors.Z * 2 * t + factors.W * 3 * t2;
		}
	};
};
