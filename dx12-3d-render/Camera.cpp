#include "Camera.h"

M4 Camera::getTranlationMatrix() const
{
	return M4::Translation(-position);
}

void Camera::move(const V3& direction)
{
	position += direction;
}

void Camera::moveReverse(const V3& direction)
{
	position -= direction;
}
