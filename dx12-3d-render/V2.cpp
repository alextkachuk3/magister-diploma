#include "V2.h"

V2::V2() : x(0.0f), y(0.0f) {}

V2::V2(f32 value) : x(value), y(value) {}

V2::V2(f32 X, f32 Y) : x(X), y(Y) {}

V2 V2::operator+(const V2& other) const
{
	return V2(x + other.x, y + other.y);
}

V2 V2::operator-(const V2& other) const
{
	return V2(x - other.x, y - other.y);
}

V2 V2::operator*(const f32 scalar) const
{
	return V2(x * scalar, y * scalar);
}

V2 V2::operator*(const V2& other) const
{
	return V2(x * other.x, y * other.y);
}

V2 V2::operator/(const f32 scalar) const
{
	return V2(x / scalar, y / scalar);
}

V2 operator*(const f32 scalar, const V2& v2)
{
	return V2(scalar * v2.x, scalar * v2.y);
}

f32 V2::CrossProduct(const V2& A, const V2& B)
{
	return A.x * B.y - A.y * B.x;
}
