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
		V2f xy;
		V2f ignored_1;
	};

	struct
	{
		f32 ignored_2;
		V3 yzw;
	};

	V4();
	V4(const f32 X, const f32 Y, const f32 Z, const f32 W);
	V4(const V3& v3, const f32 W);
	V4(const V3& v3);

	f32& operator[](const u32 index);
	V4 operator+(const V4& B) const;
	V4 operator*(const f32 B) const;

	friend V4 operator*(const f32 scalar, const V4& v4);
};
