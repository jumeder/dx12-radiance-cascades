#define M_PI 3.1415926f

struct Instance
{
    float4x4 Transform;
    float4 Albedo;
    float4 Emission;
};

float3 fromSpherical(float2 spherical)
{
    float2 s = spherical * float2(2, 1) - float2(1, 0);
    return float3(
        sin(s.y * M_PI) * cos(s.x * M_PI),
        cos(s.y * M_PI),
        sin(s.y * M_PI) * sin(s.x * M_PI)
    );
}

float2 toSpherical(float3 dir)
{
    float2 spherical = {atan2(dir.z, dir.x), acos(dir.y)};
    return spherical / float2(M_PI, M_PI) * float2(0.5, 1.f) + float2(0.5, 0.f);
}

uint2 GetPixelCount(uint cascade)
{
    // TODO verify this
    uint2 pixelCount = uint2(4, 2) << cascade;

    return pixelCount;
}
