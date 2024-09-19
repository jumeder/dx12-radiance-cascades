#define M_PI 3.1415926f

struct Instance
{
    float4x4 Transform;
    float4 Albedo;
    float4 Emission;
};

float3 fromSpherical(float2 spherical)
{
    return float3(
        sin(spherical.y * M_PI) * cos(spherical.x * 2 * M_PI),
        sin(spherical.y * M_PI) * sin(spherical.x * 2 * M_PI),
        cos(spherical.y * M_PI)
    );
}

float2 toSpherical(float3 dir)
{
    float2 spherical = {atan2(dir.y, dir.x), acos(dir.z)};
    return spherical / float2(2 * M_PI, M_PI);
}

uint2 GetPixelCount(uint cascade)
{
    // TODO verify this
    uint2 resolutionShift = cascade + uint2(cascade >> 1, (cascade + 1) >> 1);
    uint2 pixelCount = uint2(4, 2) << resolutionShift;

    return pixelCount;
}
