#pragma once

#include "Device.h"

class Scene;

struct CascadeResultion
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

class RadianceCascades
{
public:
    RadianceCascades(Device& device, const CascadeResultion& resolution, uint32_t cascadeCount, float spacing = 1.f);

    D3D12_GPU_DESCRIPTOR_HANDLE Generate(const ComPtr<ID3D12GraphicsCommandList>& commandList, D3D12_GPU_DESCRIPTOR_HANDLE accelerationStructure, D3D12_GPU_DESCRIPTOR_HANDLE instanceData);

private:
    State m_cascadeGenerationPipeline;
    ComPtr<ID3D12Resource> m_cascadesTrace;
    ComPtr<ID3D12Resource> m_cascadesAccum;
    ComPtr<ID3D12Resource> m_tracingConstants;
    D3D12_GPU_DESCRIPTOR_HANDLE m_cascadesTraceUav;
    D3D12_GPU_DESCRIPTOR_HANDLE m_cascadesTraceSrv;
    D3D12_GPU_DESCRIPTOR_HANDLE m_cascadesAccumUav;
    D3D12_GPU_DESCRIPTOR_HANDLE m_cascadesAccumSrv;
    CascadeResultion m_resolution;
    uint32_t m_cascadePixelsX = 0;
    uint32_t m_cascadePixelsY = 0;
    uint32_t m_cascadeCount = 0;
    float m_cascadeSpacing = 0;
};