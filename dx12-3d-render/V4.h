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
	V4(const f32 X, const f32 Y, const f32 Z, const f32 W);
	V4(const V3& v3, const f32 W);

	V4 operator+(const V4& B) const;
	V4 operator*(const f32 B) const;
};
