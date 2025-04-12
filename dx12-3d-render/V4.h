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

	V4();
	V4(float X, float Y, float Z, float W);
	V4(V3 v3, float W);

	V4 operator+(const V4& B) const;
	V4 operator*(const float B) const;
};
