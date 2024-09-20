#include "Common.hlsl"

cbuffer Constants : register(b0)
{
    uint cascade;
};

cbuffer CascadeConstants : register(b1)
{
    uint3 resolution;
    float3 extends;
    float3 offset;
    uint2 size;
};

Texture2D<float4> higherCascade : register(t0);
RWTexture2D<float4> currentCascade : register(u0);
SamplerState linearSampler : register(s0);

uint2 GetIndex2d(uint3 index3d, uint3 resolution, uint probeCountX)
{
    uint indexLinear = index3d.z * resolution.y * resolution.x + index3d.y * resolution.x + index3d.x;
    return uint2(indexLinear % probeCountX, indexLinear / probeCountX); 
}

float4 SingleSample(float2 localCoord, uint3 resolution, uint probeCountX, uint2 pixelCount, uint3 index3d)
{
    uint2 index2d = GetIndex2d(index3d, resolution, probeCountX);
    float2 coord = index2d * pixelCount + localCoord;
    return higherCascade.SampleLevel(linearSampler, coord / size, 0); // TODO optimize
}

float4 SampleHigherCascade(float2 uv, float3 pos)
{
    // TODO fetch 8 samples
    // TODO need to clamp to the inside of the cascade

    uint2 higherPixelCount = GetPixelCount(cascade + 1);
    uint3 higherResolution = resolution >> (cascade + 1);
    uint probeCountX = size.x / higherPixelCount.x;

     // TODO is this correct?
    float3 higherPos = pos * higherResolution;
    float3 t = frac(higherPos) - 0.5;
    float3 interp = t < 0 ? 1 + t : t;
    float3 ll = t < 0 ? floor(higherPos) - 1 : floor(higherPos); // TODO clamping

    float2 localPixelCoord = uv * higherPixelCount;
    float4 samples[8] = {
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(0, 0, 0), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(1, 0, 0), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(0, 1, 0), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(1, 1, 0), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(0, 0, 1), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(1, 0, 1), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(0, 1, 1), 0.f, higherResolution)),
        SingleSample(localPixelCoord, higherResolution, probeCountX, higherPixelCount, clamp(ll + float3(1, 1, 1), 0.f, higherResolution)),
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

[numthreads(8, 8, 1)]
void main(in uint2 index : SV_DispatchThreadId)
{
    uint2 pixelCount = GetPixelCount(cascade);
    uint3 levelResolution = resolution >> cascade;

    uint2 index2d = index / pixelCount;
    uint cascadeIndexLinear = index2d.y * size.x / pixelCount.x + index2d.x;

    uint3 index3d;
    index3d.z = cascadeIndexLinear / (levelResolution.y * levelResolution.x);
    index3d.y = (cascadeIndexLinear % (levelResolution.y * levelResolution.x)) / levelResolution.x;
    index3d.x = cascadeIndexLinear - index3d.y * levelResolution.x - index3d.z * (levelResolution.y * levelResolution.x);

    float2 uv = (float2(index) - float2(index2d * pixelCount) + 0.5) / float2(pixelCount);
    float3 pos = float3(index3d + 0.5) / float3(levelResolution);

    float4 nextLevelRad = SampleHigherCascade(uv, pos);
    float4 currLevelRad = currentCascade[index];

    currentCascade[index] = float4(currLevelRad.rgb + currLevelRad.a * nextLevelRad.rgb, currLevelRad.a * nextLevelRad.a);
}