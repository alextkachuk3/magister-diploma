#include "V3.h"

V3::V3() : x(0.0f), y(0.0f), z(0.0f) {}

V3::V3(f32 X, f32 Y, f32 Z) : x(X), y(Y), z(Z) {}

V3 V3::operator-() const
{
	return V3(-x, -y, -z);
}

V3 V3::operator+(const V3& other) const
{
	return V3(x + other.x, y + other.y, z + other.z);
}

V3 V3::operator-(const V3& other) const
{
	return V3(x - other.x, y - other.y, z - other.z);
}

V3 V3::operator*(const f32 scalar) const
{
	return V3(x * scalar, y * scalar, z * scalar);
}

V3 V3::operator/(const f32 scalar) const
{
	return V3(x / scalar, y / scalar, z / scalar);
}

V3& V3::operator+=(const V3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

V3& V3::operator-=(const V3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

V3& V3::operator/=(const f32 scalar)
{
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

V3 operator*(f32 scalar, const V3& v3)
{
	return V3(v3.x * scalar, v3.y * scalar, v3.z * scalar);
}

V2 V3::getXY() const
{
	return V2(x, y);
}

V2 V3::getYZ() const
{
	return V2(y, z);
}

V3 V3::Normalize(const V3& A)
{
	f32 length = sqrtf(A.x * A.x + A.y * A.y + A.z * A.z);
	return A / length;
}
