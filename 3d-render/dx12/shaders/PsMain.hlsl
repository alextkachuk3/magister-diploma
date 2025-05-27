#include "Common.hlsl"

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);

struct PSOutput
{
    float4 Color : SV_TARGET0;
};

PSOutput PsMain(PSInput input)
{
    PSOutput Result;
    float4 SurfaceColor = Texture.Sample(BilinearSampler, input.UV);
    float3 SurfaceNormal = normalize(input.Normal);
    if (SurfaceColor.a == 0.0f)
    {
        discard;
    }

    float3 NegativeLightDir = -LightDirection;
    float AccumIntensity = LightAmbientIntensity;
    {
        float DiffuseIntensity = max(0, dot(SurfaceNormal, NegativeLightDir));
        AccumIntensity += DiffuseIntensity;
    }

    {
        float3 CameraDirection = normalize(CameraPos - input.WorldPos);
        float3 ReflectionVector = -(NegativeLightDir - 2 * dot(NegativeLightDir, SurfaceNormal) * SurfaceNormal);
        float SpecularIntensity = SpecularStrength * pow(max(0, dot(ReflectionVector, CameraDirection)), Shininess);
        AccumIntensity += SpecularIntensity;
    }
    
    float3 MixedColor = LightColor * SurfaceColor.rgb;
    Result.Color = float4(AccumIntensity * MixedColor, SurfaceColor.a);
    
    return Result;
}
