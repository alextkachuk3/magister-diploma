#pragma once

#include "Typedefs.h"

template<typename T>
union V2
{
	struct
	{
		T x;
		T y;
	};

	T e[2];

	V2();
	V2(const T value);
	V2(const T X, const T Y);

	V2<T> operator+(const V2<T>& other) const;
	V2<T> operator-(const V2<T>& other) const;
	V2<T> operator*(const T scalar) const;
	V2<T> operator*(const V2& other) const;
	V2<T> operator/(const T scalar) const;
	V2<T>& operator/=(const T scalar);

	friend V2 operator*(const T scalar, const V2<T>& v2)
	{
		return V2<T>(scalar * v2.x, scalar * v2.y);
	};

	static T CrossProduct(const V2<T>& A, const V2<T>& B);
};

extern template union V2<f32>;
extern template union V2<i32>;

using V2f = V2<f32>;
using V2i = V2<i32>;

V2f operator*(const f32 scalar, const V2f& v2);
V2i operator*(const i32 scalar, const V2i& v2);
