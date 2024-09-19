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
}

RaytracingAccelerationStructure Scene : register(t0);
StructuredBuffer<Instance> Instances : register(t1);

RWTexture2DArray<unorm float4> Cascades : register(u0);

struct RayPayload
{
    float4 color;
};

// TODO function to compute direction from spherical texcoords
// TODO function to compute spherical coords from ray coords and cascade index

float3 fromSpherical(float2 spherical)
{
    return float3(
        sin(spherical.y * M_PI) * cos(spherical.x * 2 * M_PI),
        sin(spherical.y * M_PI) * sin(spherical.x * 2 * M_PI),
        cos(spherical.y * M_PI)
    );
}

[shader("raygeneration")]
void RayGen()
{
    uint2 index = DispatchRaysIndex().xy;

    // TODO verify this
    uint2 resolutionShift = cascade + uint2(cascade >> 1, (cascade + 1) >> 1);

    uint2 pixelCount = uint2(4, 2) << resolutionShift;
    uint3 levelResolution = resolution >> cascade;

    uint2 index2d = index / pixelCount;
    uint cascadeIndexLinear = index2d.y * DispatchRaysDimensions().x / pixelCount.x + index2d.x;

    uint3 index3d;
    index3d.z = cascadeIndexLinear / (levelResolution.y * levelResolution.x);
    index3d.y = (cascadeIndexLinear % (levelResolution.y * levelResolution.x)) / levelResolution.x;
    index3d.x = cascadeIndexLinear - index3d.y * levelResolution.x - index3d.z * (levelResolution.y * levelResolution.x);

    float3 cascadePosition = float3(index3d + 0.5) / float3(levelResolution) * 2 - 1;
    cascadePosition *= extends;
    cascadePosition += offset;

    // TODO generate ray direction from cascade index and coord
    float2 uv = (float2(index) - float2(index2d * pixelCount) + 0.5) / float2(pixelCount);
    float3 rayStart = cascadePosition;
    float3 rayDir = fromSpherical(uv);

    float start = cascade == 0 ? 0.001f : (2u << cascade);
    float end = 2u << (cascade + 1);

    RayDesc ray;
    ray.Origin = rayStart;
    ray.Direction = rayDir;
    ray.TMin = start; // TODO these need to be scaled by the true distances between the probes
    ray.TMax = end;

    RayPayload payload = { float4(0, 0, 0, 0) };

    TraceRay(Scene, 0, ~0, 0, 0, 0, ray, payload);

    Cascades[uint3(index, cascade)] = payload.color;
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
