#pragma once

#include "V3.h"

struct PhongBuffer
{
    V3 LightColor;
    f32 LightAmbientIntensity;
    V3 LightDirection;
    u32 NumPointLights;
    V3 CameraPos;
};
