#pragma once

#include "Typedefs.h"

union V2
{
	struct
	{
		f32 x, y;
	};

	f32 e[2];

	V2();
	V2(f32 value);
	V2(f32 X, f32 Y);

	V2 operator+(const V2& other) const;
	V2 operator-(const V2& other) const;
	V2 operator*(const f32 scalar) const;
	V2 operator*(const V2& other) const;
	V2 operator/(const f32 scalar) const;

	friend V2 operator*(f32 scalar, const V2& v2);

	static f32 CrossProduct(const V2& A, const V2& B);
};
