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

struct VertexOut
{
    uint3 Index : Indexs;
    float3 Dir : Direction;
    float4 Position : SV_Position;
};

VertexOut main(in float3 position : Position, in float3 normal : Normal, in uint cascadeIndexLinear : SV_InstanceID)
{
    uint3 levelResolution = probeCount >> cascade;

    uint3 cascadeIndex;
    cascadeIndex.z = cascadeIndexLinear / (levelResolution.y * levelResolution.x);
    cascadeIndex.y = (cascadeIndexLinear % (levelResolution.y * levelResolution.x)) / levelResolution.x;
    cascadeIndex.x = cascadeIndexLinear - cascadeIndex.y * levelResolution.x - cascadeIndex.z * (levelResolution.y * levelResolution.x);

    float3 cascadePosition = float3(cascadeIndex + 0.5) / float3(levelResolution) * 2 - 1;
    cascadePosition *= extends;
    cascadePosition += offset;

    float3 gridRes = extends / levelResolution;

    float3 worldPos = mul(model, float4(position * gridRes, 1)).xyz;

    VertexOut output;
    output.Index = cascadeIndex;
    output.Dir = normalize(normal);
    output.Position = mul(viewProjection, float4(worldPos + cascadePosition, 1.f));
    return output;
}