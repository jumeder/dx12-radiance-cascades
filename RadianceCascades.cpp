#include "RadianceCascades.h"
#include "Scene.h"

RadianceCascades::RadianceCascades(Device& device, const CascadeResultion& resolution, uint32_t cascadeCount, float spacing)
    : m_resolution(resolution)
    , m_cascadeCount(cascadeCount)
    , m_cascadeSpacing(spacing)
{
    // TODO compute cascade count from level 0
    constexpr auto cascadePixelsX = 4;
    constexpr auto cascadePixelsY = 2;

    const auto probeCount = resolution.x * resolution.y * resolution.z;
    const auto probesPerLine = (uint32_t)ceil(sqrt(probeCount));
    m_cascadePixelsX = cascadePixelsX * probesPerLine;
    m_cascadePixelsY = cascadePixelsY * probesPerLine;

    m_cascadesTrace = device.CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, m_cascadePixelsX, m_cascadePixelsY, cascadeCount, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_cascadesAccum = device.CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, m_cascadePixelsX, m_cascadePixelsY, cascadeCount, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    uavDesc.Texture2DArray.ArraySize = cascadeCount;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.MipSlice = 0;
    uavDesc.Texture2DArray.PlaneSlice = 0;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2DArray.ArraySize = cascadeCount;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.PlaneSlice = 0;
    srvDesc.Texture2DArray.ResourceMinLODClamp = 0.f;
    m_cascadesTraceUav = device.CreateUnorderedAccessView(m_cascadesTrace, uavDesc);
    m_cascadesAccumUav = device.CreateUnorderedAccessView(m_cascadesAccum, uavDesc);
    m_cascadesTraceSrv = device.CreateShaderResourceView(m_cascadesTrace, srvDesc);
    m_cascadesAccumSrv = device.CreateShaderResourceView(m_cascadesAccum, srvDesc);

    m_cascadeGenerationPipeline = device.CreateCascadeTracingPipeline();

    m_tracingConstants = device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
}

D3D12_GPU_DESCRIPTOR_HANDLE RadianceCascades::Generate(const ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_GPU_DESCRIPTOR_HANDLE accelerationStructure, D3D12_GPU_DESCRIPTOR_HANDLE instanceData)
{
    Device::PipelineBarrierTransition(commandList, m_cascadesAccum, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    struct
    {
        CascadeResultion cascadeResultion;
        float cascadeSpacing;
    } CascadeConstants;
    CascadeConstants.cascadeResultion = m_resolution;
    CascadeConstants.cascadeSpacing = m_cascadeSpacing;

    Device::SetResourceData(m_tracingConstants, CascadeConstants);

    ComPtr<ID3D12GraphicsCommandList4> commandList4;
    commandList.As(&commandList4);
    assert(commandList4);
    commandList4->SetPipelineState1(m_cascadeGenerationPipeline.Object.Get());

    commandList->SetComputeRootSignature(m_cascadeGenerationPipeline.RootSignature.Get());
    commandList->SetComputeRootConstantBufferView(1, m_tracingConstants->GetGPUVirtualAddress());
    commandList->SetComputeRootDescriptorTable(2, accelerationStructure);
    commandList->SetComputeRootDescriptorTable(3, instanceData);
    commandList->SetComputeRootDescriptorTable(4, m_cascadesTraceUav);

    D3D12_DISPATCH_RAYS_DESC rays = {};
    rays.Width = m_cascadePixelsX;
    rays.Height = m_cascadePixelsY;
    rays.Depth = 1;
    rays.RayGenerationShaderRecord = m_cascadeGenerationPipeline.RayGenRange;
    rays.MissShaderTable = m_cascadeGenerationPipeline.RayMissRange;
    rays.HitGroupTable = m_cascadeGenerationPipeline.RayHitRange;

    for (auto i = 0u; i < 1; ++i)
    {
        commandList->SetComputeRoot32BitConstant(0, i, 0);
        commandList4->DispatchRays(&rays);
    }

    // TODO compute shader for cascade accumulation

    //Device::PipelineBarrierTransition(commandList, m_cascadesTrace, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Device::PipelineBarrierTransition(commandList, m_cascadesAccum, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    return m_cascadesAccumSrv;
}