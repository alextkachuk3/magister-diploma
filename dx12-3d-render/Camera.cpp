#include "Camera.h"

V2 Camera::getPreviousMousePosition() const
{
	return previousMousePosition;
}

M4 Camera::getCameraTransformMatrix() const
{
	return cameraViewTransform * M4::Translation(-position);
}

M4 Camera::getCameraViewTransorm() const
{
	return cameraViewTransform;
}

bool Camera::getPreviousMousePressed() const
{
	return previousMousePressed;
}

f32 Camera::getYaw() const
{
	return yaw;
}

f32 Camera::getPitch() const
{
	return pitch;
}

void Camera::setPreviousMousePosition(const V2& mousePosition)
{
	previousMousePosition = mousePosition;
}

void Camera::moveYaw(const f32 delta)
{
	yaw += delta;
}

void Camera::movePitch(const f32 delta)
{
	pitch += delta;
}

void Camera::setCameraViewTransform(const M4& cameraViewTransform)
{
	this->cameraViewTransform = cameraViewTransform;
}

void Camera::setPreviousMousePressed(const bool mousePressed)
{
	previousMousePressed = mousePressed;
}

void Camera::move(const V3& direction)
{
	position += direction;
}

void Camera::moveReverse(const V3& direction)
{
	position -= direction;
}
