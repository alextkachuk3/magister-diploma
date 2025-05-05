#pragma once

#include <Windows.h>
#include "V3.h"
#include "M4.h"

class Camera
{
public:
	M4 getCameraTransformMatrix() const;
	bool getPreviousMousePressed() const;

	f32 getYaw() const;
	f32 getPitch() const;

	void UpdateMouseControl(HWND windowHandle);
	void UpdateViewMatrix(f32 frameTime, bool wPressed, bool aPressed, bool sPressed, bool dPressed);

	void setPreviousMousePosition(const V2f& mousePosition);
	void moveYaw(const f32 delta);
	void movePitch(const f32 delta);

	void move(const V3& direction);
	void moveReverse(const V3& direction);
	
private:
	f32 yaw = 0.0f;
	f32 pitch = 0.0f;

	bool previousMousePressed;

	V3 position;
	V2f previousMousePosition;

	M4 cameraViewTransform;
};
