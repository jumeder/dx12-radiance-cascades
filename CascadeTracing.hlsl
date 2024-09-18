#include "Common.hlsl"

cbuffer Constants : register(b0)
{
    uint cascade;
};

cbuffer CascadeConstants : register(b1)
{
    uint3 cascadeResolution;
    float cascadeSpacing;
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

    uint2 pixelCount = uint2(4, 2) * pow(2, cascade);

    uint2 cascadeIndex2d = index / pixelCount;
    uint cascadeIndexLinear = cascadeIndex2d.y * DispatchRaysDimensions().x / pixelCount.x + cascadeIndex2d.x;

    uint3 cascadeIndex;
    cascadeIndex.z = cascadeIndexLinear / (cascadeResolution.y * cascadeResolution.x);
    cascadeIndex.y = (cascadeIndexLinear % (cascadeResolution.y * cascadeResolution.x)) / cascadeResolution.x;
    cascadeIndex.x = cascadeIndexLinear - cascadeIndex.y * cascadeResolution.x - cascadeIndex.z * (cascadeResolution.y * cascadeResolution.x);

    float3 cascadePosition = (float3(cascadeIndex) * 2 - float3(cascadeResolution)) * cascadeSpacing;

    // TODO generate ray direction from cascade index and coord
    float2 uv = (float2(index) - float2(cascadeIndex2d * pixelCount) + 0.5) / float2(pixelCount);
    float3 rayStart = cascadePosition;
    float3 rayDir = fromSpherical(uv);

    RayDesc ray;
    ray.Origin = rayStart;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0; // TODO set ray dist according to spacing and cascade level

    RayPayload payload = { float4(0, 0, 0, 0) };

    TraceRay(Scene, 0, ~0, 0, 0, 0, ray, payload);

    Cascades[uint3(index, cascade)] = payload.color;
    //Cascades[uint3(index, cascade)] = float4(cascadePosition, 1);
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
