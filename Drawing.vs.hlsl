cbuffer CameraConstants : register(b0)
{
    float4x4 viewProjection;
};

struct VertexIn
{
    float3 position : Position;
    float3 normal : Normal;
};

struct VertexOut
{
    float3 Normal : Normal;
    float3 WorldPosition : WorldPosition;
    float4 Position : SV_Position;
};

VertexOut main(in VertexIn input)
{
    VertexOut output;
    output.Normal = input.normal;
    output.WorldPosition = input.position;
    output.Position = mul(viewProjection, float4(input.position, 1));
    return output;
}