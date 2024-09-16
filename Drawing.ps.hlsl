struct PixelIn
{
    float3 Normal : Normal;
    float3 WorldPosition : WorldPosition;
    float4 Position : SV_Position;
};

float4 main(in PixelIn input) : SV_Target
{
    return float4(input.Normal, 1.f);
}