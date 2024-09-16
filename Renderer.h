#pragma once

#include "Device.h"
#include "Camera.h"

class Renderer
{
public:
    Renderer(HWND window, uint32_t width, uint32_t height);

    void Render(const Camera& camera);

private:
    static constexpr auto c_backBufferCount = 2;
    struct TargetedResource
    {
        ComPtr<ID3D12Resource> Resource;
        D3D12_CPU_DESCRIPTOR_HANDLE Target; 
    };

    Device m_device;

    ComPtr<IDXGISwapChain> m_swapChain;
    std::array<TargetedResource, c_backBufferCount> m_swapChainTargets;
    TargetedResource m_depthStencil;

    Pipeline m_drawingPipeline;
    ComPtr<ID3D12Resource> m_cameraConstants;

    uint64_t m_frameCounter = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};