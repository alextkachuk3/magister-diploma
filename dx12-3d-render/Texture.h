#pragma once
#include <memory>
#include <string>
#include "Typedefs.h"

class Texture
{
public:
	Texture(u32 width, u32 height);

	u32 getWidth() const;
	u32 getHeight() const;

	u32 operator[](const u32 index) const;

	void generateCheckerboardTexture(u32 squareSize);

private:
	u32 width;
	u32 height;
	std::unique_ptr<u32[]> texels;
};
