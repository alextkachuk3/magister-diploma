#include "Camera.h"

M4 Camera::getCameraTransformMatrix() const
{
	return cameraViewTransform * M4::Translation(-position);
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

void Camera::UpdateMouseControl(HWND windowHandle)
{
	if (GetActiveWindow() != windowHandle)
		return;

	POINT win32MousePos = {};
	if (!GetCursorPos(&win32MousePos)) return;
	if (!ScreenToClient(windowHandle, &win32MousePos)) return;

	RECT clientRect = {};
	if (!GetClientRect(windowHandle, &clientRect)) return;

	win32MousePos.y = clientRect.bottom - win32MousePos.y;

	V2f currentMousePos(
		f32(win32MousePos.x) / f32(clientRect.right - clientRect.left),
		f32(win32MousePos.y) / f32(clientRect.bottom - clientRect.top)
	);

	bool mousePressed = (GetKeyState(VK_LBUTTON) & 0x80) != 0;

	if (mousePressed)
	{
		if (!previousMousePressed)
		{
			previousMousePosition = currentMousePos;
		}

		V2f mouseDelta = currentMousePos - previousMousePosition;
		movePitch(mouseDelta.y);
		moveYaw(mouseDelta.x);
		previousMousePosition = currentMousePos;
	}

	previousMousePressed = mousePressed;
}

void Camera::UpdateViewMatrix(f32 frameTime, bool wPressed, bool aPressed, bool sPressed, bool dPressed)
{
	M4 yawTransform = M4::Rotation(0.0f, yaw, 0.0f);
	M4 pitchTransform = M4::Rotation(pitch, 0.0f, 0.0f);
	M4 axisTransform = yawTransform * pitchTransform;

	V3 right = V3::Normalize((axisTransform * V4(1.0f, 0.0f, 0.0f, 0.0f)).xyz);
	V3 up = V3::Normalize((axisTransform * V4(0.0f, 1.0f, 0.0f, 0.0f)).xyz);
	V3 lookAt = V3::Normalize((axisTransform * V4(0.0f, 0.0f, 1.0f, 0.0f)).xyz);

	f32 speed = frameTime * 50.0f;

	if (wPressed)
		move(lookAt * speed);
	if (aPressed)
		moveReverse(right * speed);
	if (sPressed)
		moveReverse(lookAt * speed);
	if (dPressed)
		move(right * speed);

	cameraViewTransform = M4::Identity();
	cameraViewTransform.v[0].x = right.x;
	cameraViewTransform.v[1].x = right.y;
	cameraViewTransform.v[2].x = right.z;

	cameraViewTransform.v[0].y = up.x;
	cameraViewTransform.v[1].y = up.y;
	cameraViewTransform.v[2].y = up.z;

	cameraViewTransform.v[0].z = lookAt.x;
	cameraViewTransform.v[1].z = lookAt.y;
	cameraViewTransform.v[2].z = lookAt.z;
}

void Camera::setPreviousMousePosition(const V2f& mousePosition)
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

void Camera::move(const V3& direction)
{
	position += direction;
}

void Camera::moveReverse(const V3& direction)
{
	position -= direction;
}
