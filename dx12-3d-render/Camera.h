#pragma once
#include "V3.h"
#include "M4.h"

class Camera
{
public:
	M4 getTranlationMatrix() const;
	
	void move(const V3& direction);
	void moveReverse(const V3& direction);
private:
	V3 position;
};
