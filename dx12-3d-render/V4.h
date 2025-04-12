#pragma once

#include "V2.h"
#include "V3.h"

union V4
{
	struct
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	struct
	{
		V3 xyz;
		f32 ignored_0;
	};

	struct
	{
		V2 xy;
		V2 ignored_1;
	};

	float e[4];

	V4();
	V4(f32 X, f32 Y, f32 Z, f32 W);
	V4(V3 v3, f32 W);

	V4 operator+(const V4& B) const;
	V4 operator*(const f32 B) const;
};
