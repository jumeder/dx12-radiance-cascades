cbuffer Constants : register(b1)
{
    float4x4 viewProjection;
    float4x4 model;
    uint3 cascadeResolution;
    float3 cascadeExtends;
    float3 cascadeOffset;
};

float4 main(in float3 position : Position, in uint cascadeIndexLinear : SV_InstanceID) : SV_Position
{
    uint3 cascadeIndex;
    cascadeIndex.z = cascadeIndexLinear / (cascadeResolution.y * cascadeResolution.x);
    cascadeIndex.y = (cascadeIndexLinear % (cascadeResolution.y * cascadeResolution.x)) / cascadeResolution.x;
    cascadeIndex.x = cascadeIndexLinear - cascadeIndex.y * cascadeResolution.x - cascadeIndex.z * (cascadeResolution.y * cascadeResolution.x);

    float3 cascadePosition = float3(cascadeIndex + 0.5) / float3(cascadeResolution) * 2 - 1;
    cascadePosition *= cascadeExtends;
    cascadePosition += cascadeOffset;

    float3 gridRes = cascadeExtends / cascadeResolution;

    float3 worldPos = mul(model, float4(position * gridRes, 1)).xyz;
    return mul(viewProjection, float4(worldPos + cascadePosition, 1.f));
}