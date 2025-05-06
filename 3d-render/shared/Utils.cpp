#include "Utils.h"

V3 Utils::u32ColorToV3Rgb(const u32 color)
{
	return V3(
		static_cast<f32>((color >> 16) & 0xFF),
		static_cast<f32>((color >> 8) & 0xFF),
		static_cast<f32>((color >> 0) & 0xFF)) / 255.0f;
}

u32 Utils::V3RgbToU32Color(V3 color)
{
	color *= 255.0f;
	return ((u32)0xFF << 24) | ((u32)color.r << 16) | ((u32)color.g << 8) | (u32)color.b;
}

V3 Utils::Lerp(const V3& A, const V3& B, f32 t)
{
	return (1.0f - t) * A + t * B;
}

void ThrowIfFailed(HRESULT result)
{
	if (result != S_OK)
	{
		throw std::runtime_error("HRESULT failed");
	}
}
