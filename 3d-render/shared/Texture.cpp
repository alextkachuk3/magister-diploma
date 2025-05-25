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

u32* Texture::getTexels() const
{
	return texels.get();
}

Texture Texture::generateCheckerboardTexture(u32 width, u32 height, u32 squareSize, u32 color1, u32 color2)
{
    Texture result(width, height);

    for (u32 y = 0; y < height; ++y)
    {
        for (u32 x = 0; x < width; ++x)
        {
            if ((x / squareSize) % 2 == (y / squareSize) % 2)
            {
                result.texels[y * width + x] = color1;
            }
            else
            {
                result.texels[y * width + x] = color2;
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
	i32 x;
	i32 y;
	i32 channels;
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &channels, 4);

	if (!data)
	{
		return Texture::generateCheckerboardTexture(64, 64, 8, Colors::Black, Colors::Purple);
	}

	Texture result(static_cast<u32>(x), static_cast<u32>(y));
	for (int i = 0; i < x * y; ++i)
	{
		stbi_uc r = data[i * 4 + 0];
		stbi_uc g = data[i * 4 + 1];
		stbi_uc b = data[i * 4 + 2];
		stbi_uc a = data[i * 4 + 3];

		result.texels[i] = ((u32)r << 0) | ((u32)g << 8) | ((u32)b << 16) | ((u32)a << 24);
	}

	stbi_image_free(data);
	return result;
}
