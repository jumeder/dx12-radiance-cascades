#include "Common.hlsl"

struct PixelIn
{
    float4 Albedo : Albedo;
    float4 Emission : Emission;
    float3 Normal : Normal;
    float3 WorldPosition : WorldPosition;
    float4 Position : SV_Position;
};

float4 main(in PixelIn input) : SV_Target
{
    //float3 v = normalize(CameraPosition - input.WorldPosition);
    float3 l = float3(1, 1, 1);
    float3 n = normalize(input.Normal);
    return input.Albedo * max(0.05, dot(n, l) / M_PI) + input.Emission;
}