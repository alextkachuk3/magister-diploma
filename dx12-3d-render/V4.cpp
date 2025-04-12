#include "V4.h"

V4::V4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

V4::V4(f32 X, f32 Y, f32 Z, f32 W) : x(X), y(Y), z(Z), w(W) {}

V4::V4(V3 v3, f32 W) : x(v3.x), y(v3.y), z(v3.z), w(W) {}

V4 V4::operator+(const V4& B) const
{
	return V4(x + B.x, y + B.y, z + B.z, w + B.w);
}

V4 V4::operator*(const f32 B) const
{
	return V4(x * B, y * B, z * B, w * B);
}
