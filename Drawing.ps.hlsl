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
    uint3 probeCount;
    float3 extends;
    float3 offset;
    uint2 size;
};

Texture2DArray<float4> RadianceCascade : register(t1);

SamplerState linearSampler : register(s0);

// TODO move this code to inclusion file
float4 SingleSample(float3 pixelCoord)
{
    return RadianceCascade.SampleLevel(linearSampler, float3(pixelCoord.xy / size.xy, pixelCoord.z), 0);
}

float4 SampleCascade(float2 uv, float3 pos)
{
    uint2 hpixelCount = GetPixelCount(0);

    float3 higherPos = pos * probeCount;
    higherPos = clamp(higherPos, 0.51f, probeCount - 0.51f);

    float3 t = frac(higherPos) - 0.5;
    float3 interp = t < 0 ? 1 + t : t;
    float3 ll = t < 0.f ? floor(higherPos) - 1 : floor(higherPos);

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

float4 integrateCascades(float3 n, float3 pos)
{
    float4 accum = 0.f;
    uint2 pixelCount = GetPixelCount(0);
    for (float y = 0.5; y < pixelCount.y; ++y)
        for (float x = 0.5; x < pixelCount.x; ++x)
        {
            float2 uv = float2(x, y) / pixelCount;
            accum += SampleCascade(uv, pos) * max(0, dot(n, fromSpherical(uv)));
        }

    return accum / (pixelCount.x * pixelCount.y);
}

float4 main(in PixelIn input) : SV_Target
{
    float3 n = normalize(input.Normal);
    float3 cascadePos = (input.WorldPosition + 0.1 * n - offset) / extends * 0.5 + 0.5;

    //float3 v = normalize(CameraPosition - input.WorldPosition);
    float3 l = float3(1.f, 1.f, 1.f);
    
    //return float4(input.Normal, 1);// input.Albedo * max(0.05, dot(n, l) / M_PI) + input.Emission;
    return integrateCascades(n, cascadePos) + input.Emission;
}