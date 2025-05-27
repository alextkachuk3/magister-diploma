#include "Common.hlsl"

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.Position = mul(WVPTransform, float4(input.Position, 1.0f));
    result.WorldPos = mul(WTransform, float4(input.Position, 1.0f)).xyz;
    result.UV = input.UV;
    result.Normal = normalize(mul(NormalWTransform, float4(input.Normal, 0.0f)).xyz);
    return result;
}
