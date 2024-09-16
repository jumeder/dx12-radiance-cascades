#pragma once

#include "Shared.h"

/*
struct DescriptorHandle
{
    D3D12_CPU_DESCRIPTOR_HANDLE Cpu;
    D3D12_GPU_DESCRIPTOR_HANDLE Gpu;
};
*/

struct Commands
{
    ComPtr<ID3D12CommandAllocator> Allocator;
    ComPtr<ID3D12GraphicsCommandList> List;
};

struct VertexBuffer
{
    ComPtr<ID3D12Resource> Resource;
    D3D12_VERTEX_BUFFER_VIEW View;
};

struct IndexBuffer
{
    ComPtr<ID3D12Resource> Resource;
    D3D12_INDEX_BUFFER_VIEW View;
};

struct Pipeline
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> State;
};

class Device
{
public:
    Device();
    ~Device();

    ComPtr<IDXGISwapChain> CreateSwapChain(HWND window, uint32_t width, uint32_t height, uint32_t bufferCount);
    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t size = 65536);
    ComPtr<ID3D12Resource> CreateTexture(DXGI_FORMAT format, uint16_t width, uint16_t height, uint16_t arraySize, D3D12_RESOURCE_STATES defaultState = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
    ComPtr<ID3D12Resource> CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS = D3D12_RESOURCE_FLAG_NONE, bool staging = false);
    VertexBuffer CreateVertexBuffer(const std::vector<float>& data);
    IndexBuffer CreateIndexBuffer(const std::vector<uint32_t>& data);

    Commands CreateGraphicsCommands();
    void SubmitGraphicsCommands(Commands&& commands);

    D3D12_CPU_DESCRIPTOR_HANDLE CreateRenderTargetView(const ComPtr<ID3D12Resource> resource, DXGI_FORMAT format);
    D3D12_CPU_DESCRIPTOR_HANDLE CreateDepthStencilView(const ComPtr<ID3D12Resource> resource, DXGI_FORMAT format);
    D3D12_CPU_DESCRIPTOR_HANDLE CreateShaderResourceView(const ComPtr<ID3D12Resource> resource);

    Pipeline CreateDrawingPipeline();

    void SetResourceData(const ComPtr<ID3D12Resource>& resource, const void* data, uint64_t size);

    void WaitIdle();

    operator ID3D12Device*() const { return m_device.Get(); }
    operator ID3D12CommandQueue*() const { return m_queue.Get(); }

private:
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<ID3D12Fence> m_submissionFence;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvPos;
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsvPos;

    HANDLE m_submissionEvent;

    struct PendingCommands
    {
        ComPtr<ID3D12CommandAllocator> Allocator;
        uint64_t Submission;
    };

    std::deque<PendingCommands> m_pendingCommands;
    std::vector<ComPtr<ID3D12CommandAllocator>> m_freeAllocators;
    uint64_t m_submissionCounter = 0;
};