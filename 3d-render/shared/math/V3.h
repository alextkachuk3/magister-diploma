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
		V2f xy;
		f32 ignored_0;
	};

	struct
	{
		f32 ignored_1;
		V2f yz;
	};

	V3();
	V3(f32 X, f32 Y, f32 Z);

	f32 operator[](int index) const;
	V3 operator-() const;
	V3 operator+(const V3& other) const;
	V3 operator-(const V3& other) const;
	V3 operator*(const f32 scalar) const;
	V3 operator/(const f32 scalar) const;
	V3& operator+=(const V3& other);
	V3& operator-=(const V3& other);
	V3& operator*=(const f32 scalar);
	V3& operator/=(const f32 scalar);

	friend V3 operator*(f32 scalar, const V3& v3);

	V2f getXY() const;
	V2f getYZ() const;

	static V3 Normalize(const V3& A);
};
