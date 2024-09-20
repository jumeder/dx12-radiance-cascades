#include "Common.hlsl"

struct PixelIn
{
    float4 Albedo : Albedo;
    float4 Emission : Emission;
    float3 Normal : Normal;
    float3 WorldPosition : WorldPosition;
    float4 Position : SV_Position;
};

cbuffer CascadeConstants : register(b2)
{
    uint3 resolution;
    float3 extends;
    float3 offset;
    uint2 size;
};

Texture2D<float4> RadianceCascade : register(t1);

SamplerState linearSampler : register(s0);

uint2 GetIndex2d(uint3 index3d, uint3 resolution, uint probeCountX)
{
    uint indexLinear = index3d.z * resolution.y * resolution.x + index3d.y * resolution.x + index3d.x;
    return uint2(indexLinear % probeCountX, indexLinear / probeCountX); 
}

float4 SingleSample(float2 uv, uint3 resolution, uint probeCountX, uint2 pixelCount, uint3 index3d)
{
    uint2 index2d = GetIndex2d(index3d, resolution, probeCountX);
    float2 uvSize = float2(pixelCount) / size; // TODO optimize
    float2 coord = index2d + uv;
    return RadianceCascade.SampleLevel(linearSampler, coord * uvSize, 0); // TODO optimize
}

float4 SampleCascade(float2 uv, float3 pos)
{
    // TODO fetch 8 samples
    // TODO need to clamp to the inside of the cascade

    uint2 pixelCount = GetPixelCount(0);
    uint probeCountX = size.x / pixelCount.x;

    // TODO is this correct?
    float3 t = frac(pos) - 0.5;
    float3 interp = t < 0 ? 1 + t : t;
    float3 ll = t < 0 ? floor(pos) - 1 : floor(pos);

    float4 samples[8] = {
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(0, 0, 0), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(1, 0, 0), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(0, 1, 0), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(1, 1, 0), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(0, 0, 1), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(1, 0, 1), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(0, 1, 1), 0.f, resolution)),
        SingleSample(uv, resolution, probeCountX, pixelCount, clamp(ll + float3(1, 1, 1), 0.f, resolution)),
    };

    float4 lerpX[4];
    lerpX[0] = lerp(samples[0], samples[1], interp.x);
    lerpX[1] = lerp(samples[2], samples[3], interp.x);
    lerpX[2] = lerp(samples[4], samples[5], interp.x);
    lerpX[3] = lerp(samples[6], samples[7], interp.x);

    float4 lerpY[2];
    lerpY[0] = lerp(lerpX[0], lerpX[1], interp.y);
    lerpY[1] = lerp(lerpX[2], lerpX[3], interp.y);

    return lerp(lerpY[0], lerpY[1], interp.z);
}

float4 main(in PixelIn input) : SV_Target
{
    float3 cascadePos = ((input.WorldPosition - offset) / extends * 0.5 + 0.5) * resolution;

    // TODO need to transform input normal

    float3 v = normalize(CameraPosition - input.WorldPosition);
    float3 l = float3(1.f, 1.f, 1.f);
    float3 n = normalize(input.Normal);
    //return input.Albedo * max(0.05, dot(n, l) / M_PI) + input.Emission;
    //return float4(n, 1);
    return input.Emission 
    + input.Albedo * SampleCascade(float2(0.25, 0.25), cascadePos) * max(0, dot(fromSpherical(float2(0.25, 0.25)), n))
    + input.Albedo * SampleCascade(float2(0.25, 0.75), cascadePos) * max(0, dot(fromSpherical(float2(0.25, 0.75)), n))
    + input.Albedo * SampleCascade(float2(0.75, 0.25), cascadePos) * max(0, dot(fromSpherical(float2(0.75, 0.25)), n))
    + input.Albedo * SampleCascade(float2(0.75, 0.75), cascadePos) * max(0, dot(fromSpherical(float2(0.75, 0.75)), n))
    + input.Albedo * SampleCascade(toSpherical(n), cascadePos);
}