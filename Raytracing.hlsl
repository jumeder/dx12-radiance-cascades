#include "Common.hlsl"

cbuffer RayTracingConstants : register(b0)
{
    float4x4 viewTransform;
    float aspect;
}

RaytracingAccelerationStructure Scene : register(t0);
StructuredBuffer<Instance> Instances : register(t1);

RWTexture2D<unorm float4> RenderTarget : register(u0);

struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void RayGen()
{
    float2 lerpValues = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

    // TODO aspect ration (unnecessary for world space tracing with radiance cascades)
    float3 rayStart = float3(0, 0, 0);
    float3 rayDir = lerp(float3(-1, aspect, -1), float3(1, -aspect, -1), float3(lerpValues, 1));

    rayStart = mul(viewTransform, float4(rayStart, 1)).xyz;
    rayDir = mul(viewTransform, float4(rayDir, 0)).xyz;

    const uint numBounces = 8;
    //for(int i = 0; i < numBounces; ++i)
    //{
        RayDesc ray;
        ray.Origin = rayStart;
        ray.Direction = rayDir;
        ray.TMin = 0.001;
        ray.TMax = 10000.0;

        RayPayload payload = { float4(0, 0, 0, 0) };

        TraceRay(Scene, 0, ~0, 0, 0, 0, ray, payload);

        // TODO modify start and dir
    //}
    
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void RayHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint instanceId = InstanceID();
    Instance instance = Instances[instanceId];
    payload.color = instance.Albedo;
    
    //float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    //payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void RayMiss(inout RayPayload payload)
{
    payload.color = float4(1.f, 0.f, 0.f, 1.f);
}
