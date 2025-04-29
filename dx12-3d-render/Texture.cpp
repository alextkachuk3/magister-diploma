#include "Texture.h"

Texture::Texture(u32 width, u32 height)
	: width(width), height(height)
{
	texels = std::make_unique<u32[]>(width * height);
}

u32 Texture::getWidth() const
{
	return width;
}

u32 Texture::getHeight() const  
{  
   return height;  
}

u32 Texture::operator[](const u32 index) const
{
	return texels[index];
}

void Texture::generateCheckerboardTexture(u32 squareSize)
{
	const u32 black = 0xFF000000;
	const u32 white = 0xFFFFFFFF;

	for (u32 y = 0; y < height; ++y)
	{
		for (u32 x = 0; x < width; ++x)
		{
			if ((x / squareSize) % 2 == (y / squareSize) % 2)
			{
				texels[y * width + x] = black;
			}
			else
			{
				texels[y * width + x] = white;
			}
		}
	}
}

