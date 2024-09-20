#include "RadianceCascades.h"
#include "Scene.h"

RadianceCascades::RadianceCascades(Device& device, const CascadeResultion& resolution, const CascadeExtends& extends, const CascadeOffset& offset, uint32_t cascadeCount)
    : m_resolution(resolution)
    , m_extends(extends)
    , m_offset(offset)
    , m_count(cascadeCount)
{
    // TODO compute cascade count from level 0
    constexpr auto cascadePixelsX = 4;
    constexpr auto cascadePixelsY = 2;

    const auto probeCount = resolution.x * resolution.y * resolution.z;
    const auto probesPerLine = (uint32_t)ceil(sqrt(probeCount));
    m_cascadePixelsX = cascadePixelsX * probesPerLine;
    m_cascadePixelsY = cascadePixelsY * probesPerLine;

    m_cascades = device.CreateTexture(DXGI_FORMAT_R16G16B16A16_FLOAT, m_cascadePixelsX, m_cascadePixelsY, cascadeCount, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    uavDesc.Texture2DArray.ArraySize = cascadeCount;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.MipSlice = 0;
    uavDesc.Texture2DArray.PlaneSlice = 0;

    m_cascadesUav = device.CreateUnorderedAccessView(m_cascades, uavDesc);

    m_cascadeSrvs.resize(m_count);
    m_cascadeUavs.resize(m_count);
    for (auto i = 0u; i < m_count; ++i)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        uavDesc.Texture2DArray.ArraySize = 1;
        uavDesc.Texture2DArray.FirstArraySlice = i;
        uavDesc.Texture2DArray.MipSlice = 0;
        uavDesc.Texture2DArray.PlaneSlice = 0;
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2DArray.ArraySize = 1;
        srvDesc.Texture2DArray.FirstArraySlice = i;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.PlaneSlice = 0;
        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.f;
        m_cascadeUavs[i] = device.CreateUnorderedAccessView(m_cascades, uavDesc);
        m_cascadeSrvs[i] = device.CreateShaderResourceView(m_cascades, srvDesc);
    }

    m_cascadeGenerationPipeline = device.CreateCascadeTracingPipeline();
    m_cascadeAccumulationPipeline = device.CreateCascadeAccumulationPipeline();

    m_tracingConstants = device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
}

D3D12_GPU_DESCRIPTOR_HANDLE RadianceCascades::Generate(const ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_GPU_DESCRIPTOR_HANDLE accelerationStructure, D3D12_GPU_DESCRIPTOR_HANDLE instanceData)
{
    Device::PipelineBarrierTransition(commandList, m_cascades, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    struct
    {
        CascadeResultion resolution; DUMMY_CBV_ENTRY;
        CascadeExtends extends; DUMMY_CBV_ENTRY;
        CascadeOffset offset; DUMMY_CBV_ENTRY;
        std::array<uint32_t, 2> size;
    } CascadeConstants;
    CascadeConstants.resolution = m_resolution;
    CascadeConstants.extends = m_extends;
    CascadeConstants.offset = m_offset;
    CascadeConstants.size = {m_cascadePixelsX, m_cascadePixelsY};

    Device::SetResourceData(m_tracingConstants, CascadeConstants);

    ComPtr<ID3D12GraphicsCommandList4> commandList4;
    commandList.As(&commandList4);
    assert(commandList4);
    commandList4->SetPipelineState1(m_cascadeGenerationPipeline.Object.Get());

    commandList->SetComputeRootSignature(m_cascadeGenerationPipeline.RootSignature.Get());
    commandList->SetComputeRootConstantBufferView(1, m_tracingConstants->GetGPUVirtualAddress());
    commandList->SetComputeRootDescriptorTable(2, accelerationStructure);
    commandList->SetComputeRootDescriptorTable(3, instanceData);
    commandList->SetComputeRootDescriptorTable(4, m_cascadesUav);

    D3D12_DISPATCH_RAYS_DESC rays = {};
    rays.Width = m_cascadePixelsX;
    rays.Height = m_cascadePixelsY;
    rays.Depth = m_count;
    rays.RayGenerationShaderRecord = m_cascadeGenerationPipeline.RayGenRange;
    rays.MissShaderTable = m_cascadeGenerationPipeline.RayMissRange;
    rays.HitGroupTable = m_cascadeGenerationPipeline.RayHitRange;

    commandList4->DispatchRays(&rays);

    // TODO compute shader for cascade accumulation

    commandList->SetPipelineState(m_cascadeAccumulationPipeline.State.Get());
    commandList->SetComputeRootSignature(m_cascadeAccumulationPipeline.RootSignature.Get());
    commandList->SetComputeRootConstantBufferView(1, m_tracingConstants->GetGPUVirtualAddress());

    assert(m_count > 1);
    for (int i = m_count - 2; i >= 0; --i)
    {
        Device::PipelineBarrierTransition(commandList, m_cascades, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, i + 1);
        commandList->SetComputeRoot32BitConstant(0, i, 0);
        commandList->SetComputeRootDescriptorTable(2, m_cascadeSrvs[i + 1]);
        commandList->SetComputeRootDescriptorTable(3, m_cascadeUavs[i]);

        constexpr auto groupSize = 8;
        const uint32_t x = (m_cascadePixelsX + groupSize - 1) / groupSize;
        const uint32_t y = (m_cascadePixelsY + groupSize - 1) / groupSize;
        //commandList->Dispatch(x, y, 1);
    }
    Device::PipelineBarrierTransition(commandList, m_cascades, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, 0);

    return m_cascadeSrvs[0];
}