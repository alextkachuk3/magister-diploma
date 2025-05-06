#include "V2.h"

template<typename T>
V2<T>::V2() : x(0), y(0) {}

template<typename T>
V2<T>::V2(const T value) : x(value), y(value) {}

template<typename T>
V2<T>::V2(const T X, const T Y) : x(X), y(Y) {}

template<typename T>
V2<T> V2<T>::operator+(const V2<T>& other) const
{
    return V2<T>(x + other.x, y + other.y);
}

template<typename T>
V2<T> V2<T>::operator-(const V2<T>& other) const
{
    return V2<T>(x - other.x, y - other.y);
}

template<typename T>
V2<T> V2<T>::operator*(const T scalar) const
{
    return V2<T>(x * scalar, y * scalar);
}

template<typename T>
V2<T> V2<T>::operator*(const V2<T>& other) const
{
    return V2<T>(x * other.x, y * other.y);
}

template<typename T>
V2<T> V2<T>::operator/(const T scalar) const
{
    return V2<T>(x / scalar, y / scalar);
}

template<typename T>
V2<T>& V2<T>::operator*=(const T scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

template<typename T>
V2<T>& V2<T>::operator/=(const T scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

template<typename T>
T V2<T>::CrossProduct(const V2<T>& A, const V2<T>& B)
{
    return A.x * B.y - A.y * B.x;
}

template union V2<f32>;
template union V2<i32>;
