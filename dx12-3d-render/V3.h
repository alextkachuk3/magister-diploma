#pragma once
#include "v2.h"

union V3
{
	struct
	{
		f32 x, y, z;
	};

	struct
	{
		f32 r, g, b;
	};

	struct
	{
		V2 xy;
		f32 Ignored0;
	};

	struct
	{
		float Ignored1;
		V2 yz;
	};

	f32 e[3];

	inline V3();
	inline V3(f32 X, f32 Y, f32 Z);

	inline V3 operator+(const V3& other) const;
	inline V3 operator*(f32 scalar) const;

	inline friend V3 operator*(f32 scalar, const V3& v3);

	inline V2 getXY() const;
	inline V2 getYZ() const;
};

inline V3::V3() : x(0), y(0), z(0) {}

inline V3::V3(f32 X, f32 Y, f32 Z) : x(X), y(Y), z(Z) {}

inline V3 V3::operator+(const V3& other) const
{
	return V3(x + other.x, y + other.y, z + other.z);
}

inline V2 V3::getXY() const
{
	return V2(x, y);
}

inline V2 V3::getYZ() const
{
	return V2(y, z);
}

inline V3 V3::operator*(f32 scalar) const
{
	return V3(x * scalar, y * scalar, z * scalar);
}

inline V3 operator*(f32 scalar, const V3& v3)
{
	return V3(v3.x * scalar, v3.y * scalar, v3.z * scalar);
}
