cbuffer RenderUniforms : register(b0)
{
    float4x4 WVPTransform;
    float4x4 WTransform;
    float4x4 NormalWTransform;
    float Shininess;
    float SpecularStrength;
};

cbuffer LightUniforms : register(b1)
{
    float3 LightColor;
    float LightAmbientIntensity;
    float3 LightDirection;
    float3 CameraPos;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : NORMAL;
};
