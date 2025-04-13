#pragma once
#include <math.h>
#include "v2.h"

union V3
{
	struct
	{
		f32 x;
		f32 y;
		f32 z;
	};

	struct
	{
		f32 r;
		f32 g;
		f32 b;
	};

	struct
	{
		V2 xy;
		f32 ignored_0;
	};

	struct
	{
		f32 ignored_1;
		V2 yz;
	};

	f32 e[3];

	V3();
	V3(f32 X, f32 Y, f32 Z);

	V3 operator-() const;
	V3 operator+(const V3& other) const;
	V3 operator-(const V3& other) const;
	V3 operator*(const f32 scalar) const;
	V3 operator/(const f32 scalar) const;
	V3& operator+=(const V3& other);
	V3& operator-=(const V3& other);

	friend V3 operator*(f32 scalar, const V3& v3);

	V2 getXY() const;
	V2 getYZ() const;

	static V3 Normalize(const V3& A);
};
