#include "Utils.h"

V3 Utils::u32ColorToV3Rgb(const u32 color)
{
	return V3(
		static_cast<f32>((color >> 16) & 0xFF),
		static_cast<f32>((color >> 8) & 0xFF),
		static_cast<f32>((color >> 0) & 0xFF)) / 255.0f;
}

u32 Utils::V3BgrToU32Color(V3 color)
{
	color *= 255.0f;
	return (static_cast<u32>(0xFF) << 24) | (static_cast<u32>(color.b) << 16) | (static_cast<u32>(color.g) << 8) | static_cast<u32>(color.r);
}

u32 Utils::V3RgbToU32Color(V3 color)
{
	color *= 255.0f;
	return (static_cast<u32>(0xFF) << 24) | (static_cast<u32>(color.r) << 16) | (static_cast<u32>(color.g) << 8) | static_cast<u32>(color.b);
}

V3 Utils::Lerp(const V3& A, const V3& B, f32 t)
{
	return (1.0f - t) * A + t * B;
}

u64 Utils::Align(const u64 location, const u64 alignment)
{
	return (location + (alignment - 1)) & (~(alignment - 1));
}

u32 Utils::Dx12GetBytesPerPixel(DXGI_FORMAT format)
{
	u32 result = 0;

	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
	{
		result = 16;
	} break;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	{
		result = 8;
	} break;

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	{
		result = 4;
	} break;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	{
		result = 2;
	} break;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	{
		result = 1;
	} break;
	}

	return result;
}

f32 Utils::Determinant3x3(V3 columnA, V3 columnB, V3 columnC)
{
	return (columnA[0] * columnB[1] * columnC[2] + columnB[0] * columnC[1] * columnA[2] + columnC[0] * columnA[1] * columnB[2] -
		columnC[0] * columnB[1] * columnA[2] - columnB[0] * columnA[1] * columnC[2] - columnA[0] * columnC[1] * columnB[2]);
}

void ThrowIfFailed(HRESULT result)
{
	if (result != S_OK)
	{
		throw std::runtime_error("HRESULT failed");
	}
}
