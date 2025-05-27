#pragma once

#include "M4.h"

struct TransformBuffer
{
    M4 WVPTransform;
    M4 WTransform;
    M4 NormalWTransform;
    f32 Shininess;
    f32 SpecularStrength;
};
