#include "Common.hlsl"

cbuffer Constants : register(b0)
{
    float4x4 viewProjection;
    float4x4 model;
    uint cascade;
};

cbuffer CascadeConstants : register(b1)
{
    uint3 probeCount;
    float3 extends;
    float3 offset;
    uint2 size;
};

Texture2DArray<float4> RadianceCascade : register(t0);
SamplerState linearSampler : register(s0);

float4 SampleCascade(uint cascade, uint3 index3d, float2 uv)
{
    uint2 pixelCount = GetPixelCount(cascade);

    float2 pixelCoord = clamp(uv * pixelCount, 0.5f, pixelCount - 0.5f);
    float2 coord = (index3d.xy * pixelCount + pixelCoord) / size.xy;

    return RadianceCascade.SampleLevel(linearSampler, float3(coord, index3d.z), 0);
}

struct PixelIn
{
    uint3 Index : Indexs;
    float3 Dir : Direction;
    float4 Position : SV_Position;
};

float4 main(in PixelIn input) : SV_Target
{
    return SampleCascade(cascade, input.Index, toSpherical(input.Dir));
}