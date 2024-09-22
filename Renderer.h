#pragma once

#include "Device.h"
#include "RadianceCascades.h"

class Camera;
class Scene;
class Model;

class Renderer
{
public:
    Renderer(HWND window, uint32_t width, uint32_t height);

    void Render(const Camera& camera, Scene& scene);

    void Finish();

    inline auto& GetDevice() { return m_device; }

    inline void VisualizeCascade(int cascadeIndex) { m_debugCascade = cascadeIndex; }

private:
    static constexpr auto c_backBufferCount = 2;
    struct ViewedResource
    {
        ComPtr<ID3D12Resource> Resource;
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle; 
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle; 
    };

    struct PendingAccelStruct
    {
        ComPtr<ID3D12Resource> Resource;
        D3D12_GPU_DESCRIPTOR_HANDLE Handle;
        uint64_t Submission;
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
    std::deque<PendingAccelStruct> m_pendingRaytracingResources;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_freeRaytracingHandles;

    uint64_t m_frameCounter = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    std::unique_ptr<Model> m_debugSphere;
    Pipeline m_debugCascadesPipeline;
    ComPtr<ID3D12Resource> m_debugCascadesConstants;
    int m_debugCascade = -1;
};