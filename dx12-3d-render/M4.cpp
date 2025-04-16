#include "M4.h"

M4::M4() : v{ V4(), V4(), V4(), V4() } {}

M4::M4(const V4& row0, const V4& row1, const V4& row2, const V4& row3)
{
	v[0] = row0;
	v[1] = row1;
	v[2] = row2;
	v[3] = row3;
}

M4 M4::Identity()
{
	return M4(
		V4(1.0f, 0.0f, 0.0f, 0.0f),
		V4(0.0f, 1.0f, 0.0f, 0.0f),
		V4(0.0f, 0.0f, 1.0f, 0.0f),
		V4(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

M4 M4::Scale(const f32 X, const f32 Y, const f32 Z)
{
	return M4(
		V4(X, 0.0f, 0.0f, 0.0f),
		V4(0.0f, Y, 0.0f, 0.0f),
		V4(0.0f, 0.0f, Z, 0.0f),
		V4(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

M4 M4::Rotation(const f32 X, const f32 Y, const f32 Z)
{
	M4 RotateX(
		V4(1.0f, 0.0f, 0.0f, 0.0f),
		V4(0.0f, cos(X), -sin(X), 0.0f),
		V4(0.0f, sin(X), cos(X), 0.0f),
		V4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	M4 RotateY(
		V4(cos(Y), 0.0f, sin(Y), 0.0f),
		V4(0.0f, 1.0f, 0.0f, 0.0f),
		V4(-sin(Y), 0.0f, cos(Y), 0.0f),
		V4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	M4 RotateZ(
		V4(cos(Z), -sin(Z), 0.0f, 0.0f),
		V4(sin(Z), cos(Z), 0.0f, 0.0f),
		V4(0.0f, 0.0f, 1.0f, 0.0f),
		V4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return RotateZ * RotateY * RotateX;
}

M4 M4::Translation(const f32 X, const f32 Y, const f32 Z)
{
	return M4(
		V4(1.0f, 0.0f, 0.0f, 0.0f),
		V4(0.0f, 1.0f, 0.0f, 0.0f),
		V4(0.0f, 0.0f, 1.0f, 0.0f),
		V4(X, Y, Z, 1.0f)
	);
}

M4 M4::Translation(const V3& A)
{
	return Translation(A.x, A.y, A.z);
}

M4 M4::Perspective(const f32 aspectRatio, const f32 fov, const f32 nearZ, const f32 farZ)
{
	return M4(
		V4(1.0f / (tan(fov * 0.5f) * aspectRatio), 0.0f, 0.0f, 0.0f),
			V4(0.0f, 1.0f / tan(fov * 0.5f), 0.0f, 0.0f),
			V4(0.0f, 0.0f, -farZ / (nearZ - farZ), 1.0f),
			V4(0.0f, 0.0f, nearZ * farZ / (nearZ - farZ), 0.0f)
		);
}

V4 M4::operator*(const V4& B) const
{
	return v[0] * B.x + v[1] * B.y + v[2] * B.z + v[3] * B.w;
}

M4 M4::operator*(const M4& B) const
{
	return M4(
		*this * B.v[0],
		*this * B.v[1],
		*this * B.v[2],
		*this * B.v[3]
	);
}
