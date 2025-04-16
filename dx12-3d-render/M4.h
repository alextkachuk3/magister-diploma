#pragma once

#include "V4.h"
#include <cmath>

union M4
{
	V4 v[4];
	f32 e[16];

	M4();
	M4(const V4& row0, const V4& row1, const V4& row2, const V4& row3);

	static M4 Identity();
	static M4 Scale(const f32 X, const f32 Y, const f32 Z);
	static M4 Rotation(const f32 X, const f32 Y, const f32 Z);
	static M4 Translation(const f32 X, const f32 Y, const f32 Z);
	static M4 Translation(const V3& A);
	static M4 Perspective(const f32 aspectRatio, const f32 fov, const f32 nearZ, const f32 farZ);

	V4 operator*(const V4& B) const;
	M4 operator*(const M4& B) const;
};
