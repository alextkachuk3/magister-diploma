#pragma once
#include <string>
#include "Typedefs.h"

class Texture
{
public:
	Texture(u32 width, u32 height);
	virtual ~Texture();

	u32 getWidth() const;
	u32 getHeight() const;
	u32* getTexels() const;

	u32 operator[](const u32 index) const;

	void generateCheckerboardTexture(u32 squareSize);
	void uploadFromFile(const std::string& filePath);

private:
	u32 width;
	u32 height;
	u32* texels;
};
