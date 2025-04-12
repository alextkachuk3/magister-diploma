#include "V3.h"

V3::V3() : x(0), y(0), z(0) {}

V3::V3(f32 X, f32 Y, f32 Z) : x(X), y(Y), z(Z) {}

V3 V3::operator+(const V3& other) const
{
	return V3(x + other.x, y + other.y, z + other.z);
}

V2 V3::getXY() const
{
	return V2(x, y);
}

V2 V3::getYZ() const
{
	return V2(y, z);
}

V3 V3::operator*(f32 scalar) const
{
	return V3(x * scalar, y * scalar, z * scalar);
}

V3 operator*(f32 scalar, const V3& v3)
{
	return V3(v3.x * scalar, v3.y * scalar, v3.z * scalar);
}
