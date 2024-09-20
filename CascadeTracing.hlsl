#include "Common.hlsl"

// TODO refactor
cbuffer Constants : register(b0)
{
    uint UNUSED_cascade;
};

cbuffer CascadeConstants : register(b1)
{
    uint3 resolution;
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

// TODO function to compute spherical coords from ray coords and cascade index

[shader("raygeneration")]
void RayGen()
{
    uint2 index = DispatchRaysIndex().xy;
    uint cascade = DispatchRaysIndex().z;
    uint2 pixelCount = GetPixelCount(cascade);
    uint3 levelResolution = resolution >> cascade;

    uint2 index2d = index / pixelCount;
    uint cascadeIndexLinear = index2d.y * size.x / pixelCount.x + index2d.x;

    uint3 index3d;
    index3d.z = cascadeIndexLinear / (levelResolution.y * levelResolution.x);
    index3d.y = (cascadeIndexLinear % (levelResolution.y * levelResolution.x)) / levelResolution.x;
    index3d.x = cascadeIndexLinear - index3d.y * levelResolution.x - index3d.z * (levelResolution.y * levelResolution.x);

    float3 cascadePosition = float3(index3d + 0.5) / float3(levelResolution) * 2 - 1;
    cascadePosition *= extends;
    cascadePosition += offset;

    float2 uv = (float2(index) - float2(index2d * pixelCount) + 0.5) / float2(pixelCount);
    float3 rayStart = cascadePosition;
    float3 rayDir = fromSpherical(uv);

    float start = cascade == 0.f ? 0.001f : float(1u << (cascade  - 1));
    float end = float(1u << cascade);

    RayDesc ray;
    ray.Origin = rayStart;
    ray.Direction = rayDir;
    ray.TMin = start; // TODO these need to be scaled by the true distances between the probes
    ray.TMax = end;

    RayPayload payload = { float4(0, 0, 0, 0) };

    TraceRay(Scene, 0, ~0, 0, 0, 0, ray, payload);

    Cascades[DispatchRaysIndex()] = payload.color;
    //Cascades[uint3(index, cascade)] = float4(rayDir, 1);
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
