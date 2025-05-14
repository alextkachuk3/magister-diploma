#include "Common.hlsl"

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);

struct PS_OUTPUT
{
    float4 Color : SV_TARGET0;
};

PS_OUTPUT PsMain(PSInput input)
{
    PS_OUTPUT result;
    result.Color = Texture.Sample(BilinearSampler, input.UV);
    if (result.Color.a == 0.0f)
    {
        discard;
    }

    return result;
}
