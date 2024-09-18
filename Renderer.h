#pragma once

#include "Device.h"
#include "RadianceCascades.h"

class Camera;
class Scene;

class Renderer
{
public:
    Renderer(HWND window, uint32_t width, uint32_t height);

    void Render(const Camera& camera, Scene& scene);

    inline auto& GetDevice() { return m_device; }

private:
    static constexpr auto c_backBufferCount = 2;
    struct ViewedResource
    {
        ComPtr<ID3D12Resource> Resource;
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle; 
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle; 
    };

    Device m_device;
    RadianceCascades m_radianceCascades;

    ComPtr<IDXGISwapChain> m_swapChain;
    std::array<ViewedResource, c_backBufferCount> m_swapChainTargets;
    ViewedResource m_depthStencil;

    Pipeline m_drawingPipeline;
    ComPtr<ID3D12Resource> m_cameraConstants;

    State m_raytracingPipeline;
    ComPtr<ID3D12Resource> m_raytracingConstants;
    ViewedResource m_raytracingTarget;
    std::array<D3D12_GPU_DESCRIPTOR_HANDLE, c_backBufferCount> m_accelHandles = {};

    uint64_t m_frameCounter = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};