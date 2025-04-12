#pragma once

#include "Typedefs.h"

union V2
{
	struct
	{
		f32 x, y;
	};

	f32 e[2];

	inline V2();
	inline V2(f32 value);
	inline V2(f32 X, f32 Y);

	inline V2 operator+(const V2& other) const;
	inline V2 operator-(const V2& other) const;
	inline V2 operator*(f32 scalar) const;
	inline V2 operator*(const V2& other) const;
	inline V2 operator/(f32 scalar) const;

	inline friend V2 operator*(f32 scalar, const V2& v2);

	inline static f32 CrossProduct(V2 A, V2 B);
};

inline V2::V2() : x(0), y(0) {}

inline V2::V2(f32 value) : x(value), y(value) {}

inline V2::V2(f32 X, f32 Y) : x(X), y(Y) {}

inline V2 V2::operator+(const V2& other) const
{
	return V2(x + other.x, y + other.y);
}

inline V2 V2::operator-(const V2& other) const
{
	return V2(x - other.x, y - other.y);
}

inline V2 V2::operator*(f32 scalar) const
{
	return V2(x * scalar, y * scalar);
}

inline V2 V2::operator*(const V2& other) const
{
	return V2(x * other.x, y * other.y);
}

inline V2 V2::operator/(f32 scalar) const
{
	return V2(x / scalar, y / scalar);
}

f32 V2::CrossProduct(V2 A, V2 B)
{
	return A.x * B.y - A.y * B.x;
}

V2 operator*(f32 scalar, const V2& v2)
{
	return V2(scalar * v2.x, scalar * v2.y);
}
