#pragma once

#include <memory>
#include <string>
#include "Utils.h"
#include "Typedefs.h"
#include "Colors.h"

class Texture
{
public:
	Texture();
	Texture(u32 width, u32 height);

	u32 getWidth() const;
	u32 getHeight() const;

	u32 operator[](const u32 index) const;
	u32* getTexels() const;

	static Texture generateCheckerboardTexture(u32 width, u32 height, u32 squareSize, u32 color1, u32 color2);
	static Texture CreateSolidColor(u32 width, u32 height, u32 color);
	static Texture CreateRedTexture(u32 width, u32 height);
	static Texture CreateGreenTexture(u32 width, u32 height);
	static Texture CreateBlueTexture(u32 width, u32 height);
	static Texture LoadFromFile(const std::string& path);

private:
	u32 width;
	u32 height;
	std::shared_ptr<u32[]> texels;
};
