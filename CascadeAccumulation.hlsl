#include "Common.hlsl"

cbuffer Constants : register(b0)
{
    uint cascade;
};

cbuffer CascadeConstants : register(b1)
{
    uint3 probeCount;
    float3 extends;
    float3 offset;
    uint2 size;
};

Texture2DArray<float4> higherCascade : register(t0);
RWTexture2DArray<float4> currentCascade : register(u0);
SamplerState linearSampler : register(s0);

// TODO move this code to inclusion file
float4 SingleSample(float3 pixelCoord)
{
    return higherCascade.SampleLevel(linearSampler, float3(pixelCoord.xy / size.xy, pixelCoord.z), 0);
}

float4 SampleHigherCascade(float2 uv, float3 pos)
{
    uint3 nextCascadeProbeCount = probeCount >> (cascade + 1);
    uint2 hpixelCount = GetPixelCount(cascade + 1);

    // TODO is this correct?
    float3 higherPos = pos * nextCascadeProbeCount;
    higherPos = clamp(higherPos, 0.51f, nextCascadeProbeCount - 0.51f);
    
    float3 t = frac(higherPos) - 0.5;
    float3 interp = t < 0 ? 1 + t : t;
    float3 ll = t < 0.f ? floor(higherPos) - 1 : floor(higherPos); // TODO clamping
    // float3 ll = floor(higherPos);

    float3 pixelCoordll = float3(ll.xy * hpixelCount + uv * hpixelCount, ll.z);

    float4 samples[8] = {
        SingleSample(pixelCoordll + float3(0, 0, 0)),
        SingleSample(pixelCoordll + float3(hpixelCount.x, 0, 0)),
        SingleSample(pixelCoordll + float3(0, hpixelCount.y, 0)),
        SingleSample(pixelCoordll + float3(hpixelCount.xy, 0)),
        SingleSample(pixelCoordll + float3(0, 0, 1)),
        SingleSample(pixelCoordll + float3(hpixelCount.x, 0, 1)),
        SingleSample(pixelCoordll + float3(0, hpixelCount.y, 1)),
        SingleSample(pixelCoordll + float3(hpixelCount.xy, 1))
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

[numthreads(4, 4, 4)]
void main(in uint3 index : SV_DispatchThreadId)
{
    uint2 pixelCount = GetPixelCount(cascade);
    uint3 index3d = uint3(index.xy / pixelCount, index.z);
    uint3 levelResolution = probeCount >> cascade;

    float2 uv = (float2(index.xy) - float2(index3d.xy * pixelCount) + 0.5) / float2(pixelCount);
    float3 pos = float3(index3d + 0.5) / float3(levelResolution);
/*
    float gauss[] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 };

    int i = 0;
    float4 nextLevelRad = 0.f;
    for (float x = -1.f; x <= 1.f; ++x)
        for (float y = -1.f; y <= 1.f; ++y)
        {
            nextLevelRad += gauss[i++] * SampleHigherCascade(uv + float2(x, y) / pixelCount, pos);
        }

    nextLevelRad /= 16.f;*/

    float4 nextLevelRad = SampleHigherCascade(uv, pos);
    float4 currLevelRad = currentCascade[index];

    currentCascade[index] = float4(currLevelRad.rgb + currLevelRad.a * nextLevelRad.rgb, currLevelRad.a * nextLevelRad.a);
}