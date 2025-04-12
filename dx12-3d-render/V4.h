#pragma once

#include "V2.h"
#include "V3.h"

union V4
{
	struct
	{
		float x, y, z, w;
	};

	struct
	{
		V3 xyz;
		float Ignored0;
	};

	struct
	{
		V2 xy;
		V2 Ignored1;
	};

	float e[4];

	inline V4();
	inline V4(float X, float Y, float Z, float W);
	inline V4(V3 v3, float W);

	inline V4 operator+(const V4& B) const;
	inline V4 operator*(float B) const;
};

inline V4::V4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

inline V4::V4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}

inline V4::V4(V3 v3, float W) : x(v3.x), y(v3.y), z(v3.z), w(W) {}

inline V4 V4::operator+(const V4& B) const
{
	return V4(x + B.x, y + B.y, z + B.z, w + B.w);
}

inline V4 V4::operator*(float B) const
{
	return V4(x * B, y * B, z * B, w * B);
}
