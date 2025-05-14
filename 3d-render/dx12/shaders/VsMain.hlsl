#include "Common.hlsl"

cbuffer RenderUniforms : register(b0)
{
    float4x4 WVPTransform;
};

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.Position = mul(WVPTransform, float4(input.Position, 1.0f));
    result.UV = input.UV;
    return result;
}
