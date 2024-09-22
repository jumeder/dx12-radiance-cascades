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

RaytracingAccelerationStructure Scene : register(t0);
StructuredBuffer<Instance> Instances : register(t1);

RWTexture2DArray<float4> Cascades : register(u0);

struct RayPayload
{
    float4 color;
};

float GetEnd(int cascade)
{
    const float interval = 0.1f;  
    return (interval * (1 - pow(8, cascade + 1))) / (1 - 8);
}

// TODO function to compute spherical coords from ray coords and cascade index

[shader("raygeneration")]
void RayGen()
{
    uint3 pixelIndex = uint3(DispatchRaysIndex());

    uint2 pixelCount = GetPixelCount(cascade);
    uint3 index3d = uint3(DispatchRaysIndex().xy / pixelCount, DispatchRaysIndex().z);
    uint3 levelProbeCount = probeCount >> cascade;

    float3 cascadePosition = float3(index3d + 0.5) / float3(levelProbeCount) * 2 - 1;
    cascadePosition *= extends;
    cascadePosition += offset;

    float2 uv = (pixelIndex.xy - index3d.xy * pixelCount + 0.5) / float2(pixelCount);
    float3 rayStart = cascadePosition;
    float3 rayDir = fromSpherical(uv);

    float start = GetEnd((int)cascade - 1);
    float end = GetEnd(cascade);

    RayDesc ray;
    ray.Origin = rayStart;
    ray.Direction = rayDir;
    ray.TMin = 0.01f + start; // TODO these need to be scaled by the true distances between the probes
    ray.TMax = end;

    RayPayload payload = { float4(0, 0, 0, 0) };

    TraceRay(Scene, 0, ~0, 0, 0, 0, ray, payload);

    Cascades[DispatchRaysIndex()] = payload.color;
   // Cascades[DispatchRaysIndex()] = float4(rayStart, 1);
}

[shader("closesthit")]
void RayHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint instanceId = InstanceID();
    Instance instance = Instances[instanceId];
    payload.color = float4(instance.Emission.rgb, 0.f);
    
    //float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    //payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void RayMiss(inout RayPayload payload)
{
    payload.color = float4(0.f, 0.f, 0.f, 1.f);
}
