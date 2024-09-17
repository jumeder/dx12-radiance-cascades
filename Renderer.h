#pragma once

#include "Device.h"
#include "Camera.h"
#include "Model.h"

class Renderer
{
public:
    Renderer(HWND window, uint32_t width, uint32_t height);

    void Render(const Camera& camera);

private:
    static constexpr auto c_backBufferCount = 2;
    struct ViewedResource
    {
        ComPtr<ID3D12Resource> Resource;
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle; 
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle; 
    };

    Device m_device;

    ComPtr<IDXGISwapChain> m_swapChain;
    std::array<ViewedResource, c_backBufferCount> m_swapChainTargets;
    ViewedResource m_depthStencil;

    Pipeline m_drawingPipeline;
    ComPtr<ID3D12Resource> m_cameraConstants;

    State m_raytracingPipeline;
    ComPtr<ID3D12Resource> m_raytracingConstants;
    ViewedResource m_raytracingTarget;

    uint64_t m_frameCounter = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    std::unique_ptr<Model> m_model;
    ComPtr<ID3D12Resource> m_tlas;
    D3D12_GPU_DESCRIPTOR_HANDLE m_tlasHandle;
};