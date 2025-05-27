#pragma once

#include <Windows.h>
#include <stdexcept>
#include <algorithm>
#include <dxgiformat.h>
#include "V3.h"
#include "Constants.h"

#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define AssertMsg(Msg) { OutputDebugStringA(Msg); Assert(false);}
#define ArraySize(Array) (sizeof(Array) / sizeof((Array)[0]))

void ThrowIfFailed(HRESULT result);

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

namespace Utils
{
	V3 u32ColorToV3Rgb(const u32 color);
	u32 V3BgrToU32Color(V3 color);
	u32 V3RgbToU32Color(V3 color);
	V3 Lerp(const V3& A, const V3& B, f32 t);
	u64 Align(const u64 location, const u64 alignment);
	u32 Dx12GetBytesPerPixel(DXGI_FORMAT format);
	f32 Determinant3x3(V3 columnA, V3 columnB, V3 columnC);

	constexpr float DegreesToRadians(float degrees)
	{
		return degrees * Constants::PI / 180.0f;
	}
}
