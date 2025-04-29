#pragma once
#include <algorithm>
#include "V3.h"

#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define AssertMsg(Msg) { OutputDebugStringA(Msg); Assert(false);}

namespace Utils
{
	V3 u32ColorToV3Rgb(const u32 color);
	u32 V3RgbToU32Color(V3 color);
	V3 Lerp(const V3& A, const V3& B, f32 t);
}
