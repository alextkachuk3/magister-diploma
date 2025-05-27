#include "M4.h"

M4::M4() : v{ V4(), V4(), V4(), V4() } {}

M4::M4(const V4& row0, const V4& row1, const V4& row2, const V4& row3)
{
	v[0] = row0;
	v[1] = row1;
	v[2] = row2;
	v[3] = row3;
}

M4 M4::Transpose()
{
	M4 result;
	for (u32 i = 0; i < 4; ++i)
	{
		for (u32 j = 0; j < 4; ++j)
		{
			result[i][j] = (*this)[j][i];
		}
	}
	return result;
}

f32 M4::Determinant()
{
	return
		v[0][0] * Utils::Determinant3x3(v[1].yzw, v[2].yzw, v[3].yzw) -
		v[0][1] * Utils::Determinant3x3(V3(v[1].x, v[2].x, v[3].x), V3(v[1].z, v[2].z, v[3].z), V3(v[1].w, v[2].w, v[3].w)) +
		v[0][2] * Utils::Determinant3x3(V3(v[1].x, v[2].x, v[3].x), V3(v[1].y, v[2].y, v[3].y), V3(v[1].w, v[2].w, v[3].w)) -
		v[0][3] * Utils::Determinant3x3(v[1].xyz, v[2].xyz, v[3].xyz);
}

M4 M4::Inverse()
{
	f32 det = Determinant();
	Assert(det != 0.0f);
	f32 invDet = 1.0f / det;

	M4 result;

	result[0][0] = +Utils::Determinant3x3(v[1].yzw, v[2].yzw, v[3].yzw) * invDet;
	result[0][1] = -Utils::Determinant3x3(v[0].yzw, v[2].yzw, v[3].yzw) * invDet;
	result[0][2] = +Utils::Determinant3x3(v[0].yzw, v[1].yzw, v[3].yzw) * invDet;
	result[0][3] = -Utils::Determinant3x3(v[0].yzw, v[1].yzw, v[2].yzw) * invDet;

	result[1][0] = -Utils::Determinant3x3(V3(v[1].x, v[2].x, v[3].x), V3(v[1].z, v[2].z, v[3].z), V3(v[1].w, v[2].w, v[3].w)) * invDet;
	result[1][1] = +Utils::Determinant3x3(V3(v[0].x, v[2].x, v[3].x), V3(v[0].z, v[2].z, v[3].z), V3(v[0].w, v[2].w, v[3].w)) * invDet;
	result[1][2] = -Utils::Determinant3x3(V3(v[0].x, v[1].x, v[3].x), V3(v[0].z, v[1].z, v[3].z), V3(v[0].w, v[1].w, v[3].w)) * invDet;
	result[1][3] = +Utils::Determinant3x3(V3(v[0].x, v[1].x, v[2].x), V3(v[0].z, v[1].z, v[2].z), V3(v[0].w, v[1].w, v[2].w)) * invDet;

	result[2][0] = +Utils::Determinant3x3(V3(v[1].x, v[2].x, v[3].x), V3(v[1].y, v[2].y, v[3].y), V3(v[1].w, v[2].w, v[3].w)) * invDet;
	result[2][1] = -Utils::Determinant3x3(V3(v[0].x, v[2].x, v[3].x), V3(v[0].y, v[2].y, v[3].y), V3(v[0].w, v[2].w, v[3].w)) * invDet;
	result[2][2] = +Utils::Determinant3x3(V3(v[0].x, v[1].x, v[3].x), V3(v[0].y, v[1].y, v[3].y), V3(v[0].w, v[1].w, v[3].w)) * invDet;
	result[2][3] = -Utils::Determinant3x3(V3(v[0].x, v[1].x, v[2].x), V3(v[0].y, v[1].y, v[2].y), V3(v[0].w, v[1].w, v[2].w)) * invDet;

	result[3][0] = -Utils::Determinant3x3(v[1].xyz, v[2].xyz, v[3].xyz) * invDet;
	result[3][1] = +Utils::Determinant3x3(v[0].xyz, v[2].xyz, v[3].xyz) * invDet;
	result[3][2] = -Utils::Determinant3x3(v[0].xyz, v[1].xyz, v[3].xyz) * invDet;
	result[3][3] = +Utils::Determinant3x3(v[0].xyz, v[1].xyz, v[2].xyz) * invDet;

	return result.Transpose();
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

V4& M4::operator[](const u32 index)
{
	return v[index];
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
