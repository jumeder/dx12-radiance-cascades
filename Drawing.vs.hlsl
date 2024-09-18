#include "Common.hlsl"

cbuffer CameraConstants : register(b0)
{
    float4x4 ViewProjection;
};

cbuffer ObjectConstants : register(b1)
{
    uint InstanceId;
}

StructuredBuffer<Instance> instances : register(t0);

struct VertexIn
{
    float3 Position : Position;
    float3 Normal : Normal;
};

struct VertexOut
{
    float4 Albedo : Albedo;
    float4 Emission : Emission;
    float3 Normal : Normal;
    float3 WorldPosition : WorldPosition;
    float4 Position : SV_Position;
};

VertexOut main(in VertexIn input)
{
    Instance instance = instances[InstanceId];

    VertexOut output;
    output.Albedo = instance.Albedo;
    output.Emission = instance.Emission;
    output.Normal = input.Normal;
    output.WorldPosition = mul(instance.Transform, float4(input.Position, 1)).xyz;
    output.Position = mul(ViewProjection, float4(output.WorldPosition , 1));
    return output;
}