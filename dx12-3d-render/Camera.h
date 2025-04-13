#pragma once
#include "V3.h"
#include "M4.h"

class Camera
{
public:
	V2 getPreviousMousePosition() const;
	M4 getCameraTransformMatrix() const;
	M4 getCameraViewTransorm() const;
	bool getPreviousMousePressed() const;

	f32 getYaw() const;
	f32 getPitch() const;	
	
	void setPreviousMousePosition(const V2& mousePosition);
	void moveYaw(const f32 delta);
	void movePitch(const f32 delta);
	void setCameraViewTransform(const M4& cameraViewTransform);
	void setPreviousMousePressed(const bool mousePressed);

	void move(const V3& direction);
	void moveReverse(const V3& direction);
private:
	f32 yaw = 0.0f;
	f32 pitch = 0.0f;

	bool previousMousePressed;

	V3 position;
	V2 previousMousePosition;

	M4 cameraViewTransform;
};
