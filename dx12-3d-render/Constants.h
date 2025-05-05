#pragma once
#include <numbers>

namespace Constants
{
	constexpr f32 W_CLIPPING_PLANE = 0.000001f;
	constexpr u32 CLIP_MAX_VERTICES = 384;
	constexpr f32 PI = std::numbers::pi_v<f32>;
}
