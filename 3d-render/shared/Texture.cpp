#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture() : width(0), height(0) {}

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

Texture Texture::generateCheckerboardTexture(u32 width, u32 height, u32 squareSize)
{
	Texture result(width, height);

	for (u32 y = 0; y < height; ++y)
	{
		for (u32 x = 0; x < width; ++x)
		{
			if ((x / squareSize) % 2 == (y / squareSize) % 2)
			{
				result.texels[y * width + x] = Colors::Black;
			}
			else
			{
				result.texels[y * width + x] = Colors::White;
			}
		}
	}
	return result;
}

Texture Texture::CreateSolidColor(u32 width, u32 height, u32 color)
{
	Texture result(width, height);
	for (u32 i = 0; i < width * height; ++i)
	{
		result.texels[i] = color;
	}
	return result;
}

Texture Texture::CreateRedTexture(u32 width, u32 height)
{
	return CreateSolidColor(width, height, 0xFFFF0000);
}

Texture Texture::CreateGreenTexture(u32 width, u32 height)
{
	return CreateSolidColor(width, height, 0xFF00FF00);
}

Texture Texture::CreateBlueTexture(u32 width, u32 height)
{
	return CreateSolidColor(width, height, 0xFF0000FF);
}

Texture Texture::LoadFromFile(const std::string& path)
{
	i32 x, y, channels;
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &channels, 4);

	if (!data)
	{
		AssertMsg("Failed to load texture");
	}

	Texture result(static_cast<u32>(x), static_cast<u32>(y));
	for (int i = 0; i < x * y; ++i)
	{
		stbi_uc r = data[i * 4 + 0];
		stbi_uc g = data[i * 4 + 1];
		stbi_uc b = data[i * 4 + 2];
		stbi_uc a = data[i * 4 + 3];

		result.texels[i] = ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | (u32)b;
	}

	stbi_image_free(data);
	return result;
}
