#include "Device.h"

#include "Drawing.vs.h"
#include "Drawing.ps.h"
#include "Raytracing.h"
#include "CascadeTracing.h"
#include "CascadeAccumulation.h"
#include "DebugCascades.vs.h"
#include "DebugCascades.ps.h"

namespace
{
    template<typename T, typename U>
    T roundUp(T value, U align)
    {
        return ((value + align - 1) / align) * align;
    }
}

Device::Device()
{
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};
    assert(SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
        && featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);

    D3D12_COMMAND_QUEUE_DESC queueDesc;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0b1;
    queueDesc.Priority = 0;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue));

    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_submissionFence));
    m_submissionEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    m_srvHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_rtvHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128);
    m_dsvHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128);
    m_samplerHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 128);


    //m_linearSampler = CreateSampler(samplerDesc);
}

Device::~Device()
{
    CloseHandle(m_submissionEvent);
}

ComPtr<IDXGISwapChain> Device::CreateSwapChain(HWND window, uint32_t width, uint32_t height, uint32_t bufferCount)
{
    ComPtr<IDXGIFactory> factory;
    ComPtr<IDXGISwapChain> swapChain;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Flags = 0;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.SampleDesc = {1, 0};
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.RefreshRate = {60, 1};
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Width = width;
    CreateDXGIFactory(IID_PPV_ARGS(&factory));
    factory->CreateSwapChain(m_queue.Get(), &swapChainDesc, &swapChain);
    return swapChain;
}

ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t size)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
    descriptorHeapDesc.Flags = type == 
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : 
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    descriptorHeapDesc.NodeMask = 0b1;
    descriptorHeapDesc.NumDescriptors = size;
    descriptorHeapDesc.Type = type;
    m_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    return descriptorHeap;
}

ComPtr<ID3D12Resource> Device::CreateTexture(DXGI_FORMAT format, uint16_t width, uint16_t height, uint16_t arraySize, D3D12_RESOURCE_STATES defaultState, D3D12_RESOURCE_FLAGS flags)
{
    ComPtr<ID3D12Resource> texture;
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.CreationNodeMask = 0b1;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.VisibleNodeMask = 0b1;
    D3D12_RESOURCE_DESC textureDesc;
    textureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    textureDesc.DepthOrArraySize = arraySize;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Flags = flags;
    textureDesc.Format = format;
    textureDesc.Height = height;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.MipLevels = 1;
    textureDesc.SampleDesc = {1, 0};
    textureDesc.Width = width;
    m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &textureDesc, defaultState, nullptr, IID_PPV_ARGS(&texture));
    return texture;
}

ComPtr<ID3D12Resource> Device::CreateBuffer(uint64_t size, D3D12_RESOURCE_STATES defaultState, D3D12_RESOURCE_FLAGS flags, bool staging)
{
    ComPtr<ID3D12Resource> buffer;
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.CreationNodeMask = 0b1;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.Type = staging ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
    heapProps.VisibleNodeMask = 0b1;
    D3D12_RESOURCE_DESC bufferDesc;
    bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Flags = flags;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.Height = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc = {1, 0};
    bufferDesc.Width = size;
    m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &bufferDesc, defaultState, nullptr, IID_PPV_ARGS(&buffer));
    return buffer;
}

VertexBuffer Device::CreateVertexBuffer(const std::vector<float>& data)
{
    const auto dataSize = data.size() * sizeof(data[0]);
    const auto stagingBuffer = CreateBuffer(dataSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    const auto gpuBuffer = CreateBuffer(dataSize);

    SetResourceData(stagingBuffer, *data.data(), data.size());

    auto commands = CreateGraphicsCommands();
    commands.List->CopyResource(gpuBuffer.Get(), stagingBuffer.Get());
    SubmitGraphicsCommands(std::move(commands));

    VertexBuffer ret;
    ret.Resource = std::move(gpuBuffer);
    ret.View.BufferLocation = ret.Resource->GetGPUVirtualAddress();
    ret.View.SizeInBytes = (UINT)dataSize;
    ret.View.StrideInBytes = 3 * sizeof(data[0]);

    WaitIdle();

    return ret;
}

IndexBuffer Device::CreateIndexBuffer(const std::vector<uint32_t>& data)
{
    const auto dataSize = data.size() * sizeof(data[0]);
    const auto stagingBuffer = CreateBuffer(dataSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    const auto gpuBuffer = CreateBuffer(dataSize);

    SetResourceData(stagingBuffer, *data.data(), data.size());

    auto commands = CreateGraphicsCommands();
    commands.List->CopyResource(gpuBuffer.Get(), stagingBuffer.Get());
    SubmitGraphicsCommands(std::move(commands));

    IndexBuffer ret;
    ret.Resource = std::move(gpuBuffer);
    ret.View.BufferLocation = ret.Resource->GetGPUVirtualAddress();
    ret.View.SizeInBytes = (UINT)dataSize;
    ret.View.Format = DXGI_FORMAT_R32_UINT;

    WaitIdle();

    return ret;
}

ComPtr<ID3D12Resource> Device::CreateBottomLevelAccelerationStructure(const VertexBuffer& vertices, const IndexBuffer& indices, uint32_t vertexCount, uint32_t indexCount)
{
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometryDesc.Triangles.IndexBuffer = indices.Resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = indexCount;
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexBuffer = {vertices.Resource->GetGPUVirtualAddress(), sizeof(float) * 3};
    geometryDesc.Triangles.VertexCount = vertexCount;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;

    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.NumDescs = 1;
    inputs.pGeometryDescs = &geometryDesc;
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    const auto scratch = CreateBuffer(roundUp(info.ScratchDataSizeInBytes, 256), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    const auto ret = CreateBuffer(roundUp(info.ResultDataMaxSizeInBytes, 256), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
    buildDesc.Inputs = inputs;
    buildDesc.DestAccelerationStructureData = ret->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
    buildDesc.SourceAccelerationStructureData = 0;

    auto commands = CreateGraphicsCommands();
    ComPtr<ID3D12GraphicsCommandList4> commandList;
    commands.List.As(&commandList);
    assert(commandList);

    commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    // TODO this barrier is unnecessary as every submit is fenced
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.UAV.pResource = ret.Get();
    commandList->ResourceBarrier(1, &barr);

    SubmitGraphicsCommands(std::move(commands));

    WaitIdle();

    return ret;

}

ComPtr<ID3D12Resource> Device::CreateTopLevelAccelerationStructure(const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& instanceBuffer, uint32_t instanceCount)
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);
    
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.NumDescs = instanceCount;
    inputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

    const auto scratchSize = roundUp(info.ScratchDataSizeInBytes, 256);
    if(scratchSize > m_tlasScratchSize)
    {
        m_tlasScratch = CreateBuffer(scratchSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        m_tlasScratchSize = scratchSize;
    }

    // TODO can we optimize this allocation?
    const auto ret = CreateBuffer(roundUp(info.ResultDataMaxSizeInBytes, 256), D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS); 

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
    buildDesc.Inputs = inputs;
    buildDesc.DestAccelerationStructureData = ret->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = m_tlasScratch->GetGPUVirtualAddress();
    buildDesc.SourceAccelerationStructureData = 0;

    ComPtr<ID3D12GraphicsCommandList4> commandList4;
    commandList.As(&commandList4);
    assert(commandList4);

    commandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

    // TODO this barrier is unnecessary if our submit is fenced
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.UAV.pResource = ret.Get();
    commandList->ResourceBarrier(1, &barr);

    return ret;
}

Commands Device::CreateGraphicsCommands()
{
    Commands commands;
    if (m_freeAllocators.empty())
    {
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commands.Allocator));
    }
    else
    {
        commands.Allocator = std::move(m_freeAllocators.back());
        m_freeAllocators.pop_back();
    }
    commands.Allocator->Reset();
    m_device->CreateCommandList(0b1, D3D12_COMMAND_LIST_TYPE_DIRECT, commands.Allocator.Get(), nullptr, IID_PPV_ARGS(&commands.List));
    SetDescriptorHeaps(commands.List);
    return commands;
}

void Device::SubmitGraphicsCommands(Commands&& commands)
{
    commands.List->Close();
    ID3D12CommandList* list = commands.List.Get();
    m_queue->ExecuteCommandLists(1, &list);
    m_queue->Signal(m_submissionFence.Get(), ++m_submissionCounter);

    m_pendingCommands.push_back({std::move(commands.Allocator), m_submissionCounter});

    const auto fenceValue = m_submissionFence->GetCompletedValue();

    while(!m_pendingCommands.empty() && m_pendingCommands.front().Submission <= fenceValue)
    {
        auto& front = m_pendingCommands.front();
        m_freeAllocators.push_back(std::move(front.Allocator));
        m_pendingCommands.pop_front();
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::CreateRenderTargetView(const ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart() + m_rtvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    ++m_rtvPos;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    m_device->CreateRenderTargetView(resource.Get(), &rtvDesc, handle);

    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::CreateDepthStencilView(const ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart() + m_dsvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    ++m_dsvPos;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = format;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    m_device->CreateDepthStencilView(resource.Get(), &dsvDesc, handle);
    
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Device::CreateShaderResourceView(const ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, D3D12_GPU_DESCRIPTOR_HANDLE oldHandle)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle; 

    if (oldHandle.ptr)
    {
        const auto pos = (oldHandle.ptr - m_srvHeap->GetGPUDescriptorHandleForHeapStart().ptr) / m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart() + (uint32_t)pos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        gpuHandle = oldHandle;
    }
    else
    {
        cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart() + m_srvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart() + m_srvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        ++m_srvPos;
    }

    m_device->CreateShaderResourceView(desc.ViewDimension == D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE ? nullptr : resource.Get(), &desc, cpuHandle);

    return gpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Device::CreateUnorderedAccessView(const ComPtr<ID3D12Resource>& resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart() + m_srvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart() + m_srvPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ++m_srvPos;

    m_device->CreateUnorderedAccessView(resource.Get(), nullptr, &desc, cpuHandle);

    return gpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::CreateSampler(const D3D12_SAMPLER_DESC& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_samplerHeap->GetCPUDescriptorHandleForHeapStart() + m_samplerPos * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    m_device->CreateSampler(&desc, cpuHandle);

    return cpuHandle;
}

Pipeline Device::CreateDrawingPipeline()
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    Pipeline ret;

    ComPtr<ID3DBlob> blob;
    D3D12_ROOT_PARAMETER cameraConstants;
    cameraConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    cameraConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    cameraConstants.Descriptor.RegisterSpace = 0;
    cameraConstants.Descriptor.ShaderRegister = 0;
    D3D12_DESCRIPTOR_RANGE instancesRange;
    instancesRange.BaseShaderRegister = 0;
    instancesRange.NumDescriptors = 1;
    instancesRange.OffsetInDescriptorsFromTableStart = 0;
    instancesRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instancesRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER instances;
    instances.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    instances.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    instances.DescriptorTable.NumDescriptorRanges = 1;
    instances.DescriptorTable.pDescriptorRanges = &instancesRange;
    D3D12_ROOT_PARAMETER objectConstants;
    objectConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    objectConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    objectConstants.Constants.Num32BitValues = 1;
    objectConstants.Constants.RegisterSpace = 0;
    objectConstants.Constants.ShaderRegister = 1;
    D3D12_ROOT_PARAMETER cascadeConstants;
    cascadeConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    cascadeConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    cascadeConstants.Descriptor.RegisterSpace = 0;
    cascadeConstants.Descriptor.ShaderRegister = 2;
    D3D12_DESCRIPTOR_RANGE radianceCascadeRange;
    radianceCascadeRange.BaseShaderRegister = 1;
    radianceCascadeRange.NumDescriptors = 1;
    radianceCascadeRange.OffsetInDescriptorsFromTableStart = 0;
    radianceCascadeRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    radianceCascadeRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER radianceCascade;
    radianceCascade.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    radianceCascade.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    radianceCascade.DescriptorTable.NumDescriptorRanges = 1;
    radianceCascade.DescriptorTable.pDescriptorRanges = &radianceCascadeRange;
    std::array parameters = {cameraConstants, instances, objectConstants, cascadeConstants, radianceCascade};

    D3D12_STATIC_SAMPLER_DESC linearSampler;
    linearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearSampler.MaxAnisotropy = 0;
    linearSampler.MaxLOD = 0.f;
    linearSampler.MinLOD = 0.f;
    linearSampler.MipLODBias = 0.f;
    linearSampler.RegisterSpace = 0;
    linearSampler.ShaderRegister = 0;
    linearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = (UINT)parameters.size();
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pParameters = parameters.data();
    rootSignatureDesc.pStaticSamplers = &linearSampler;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
    assert(blob);

    device->CreateRootSignature(0b1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ret.RootSignature));

    struct
    {
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT;
            D3D12_INPUT_LAYOUT_DESC desc;
        } InputLayout;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
            D3D12_SHADER_BYTECODE desc;
        } VS;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
            D3D12_SHADER_BYTECODE desc;
        } PS;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
            ID3D12RootSignature* desc;
        } RootSignature;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
            D3D12_RT_FORMAT_ARRAY desc;
        } RenderTargets;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
            D3D12_DEPTH_STENCIL_DESC desc;
        } DepthStencil;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
            DXGI_FORMAT desc;
        } DepthFormat;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
            D3D12_RASTERIZER_DESC desc;
        } Rasterizer;
    } streamDesc;

    D3D12_INPUT_ELEMENT_DESC position;
    position.AlignedByteOffset = 0;
    position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
    position.InputSlot = 0;
    position.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    position.InstanceDataStepRate = 0;
    position.SemanticIndex = 0;
    position.SemanticName = "Position";
    D3D12_INPUT_ELEMENT_DESC normal = position;
    normal.InputSlot = 1;
    normal.SemanticIndex = 0;
    normal.SemanticName = "Normal";
    std::array vertexInputs = {position, normal};
    streamDesc.InputLayout.desc = {vertexInputs.data(), (UINT)vertexInputs.size()};
    streamDesc.VS.desc = { DrawingVS, sizeof(DrawingVS) };
    streamDesc.PS.desc = { DrawingPS, sizeof(DrawingPS) };
    streamDesc.RootSignature.desc = ret.RootSignature.Get();
    streamDesc.RenderTargets.desc = {};
    streamDesc.RenderTargets.desc.NumRenderTargets = 1;
    streamDesc.RenderTargets.desc.RTFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    streamDesc.DepthStencil.desc.DepthEnable = TRUE;
    streamDesc.DepthStencil.desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
    streamDesc.DepthStencil.desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    streamDesc.DepthStencil.desc.StencilEnable = FALSE;
    streamDesc.DepthFormat.desc = DXGI_FORMAT_D32_FLOAT;
    streamDesc.Rasterizer.desc = {};
    streamDesc.Rasterizer.desc.FillMode = D3D12_FILL_MODE_SOLID;
    streamDesc.Rasterizer.desc.CullMode = D3D12_CULL_MODE_NONE;
    D3D12_PIPELINE_STATE_STREAM_DESC stateDesc;
    stateDesc.pPipelineStateSubobjectStream = &streamDesc;
    stateDesc.SizeInBytes = sizeof(streamDesc);
    device->CreatePipelineState(&stateDesc, IID_PPV_ARGS(&ret.State));
    assert(ret.State);

    return ret;
}

Pipeline Device::CreateCascadeDebugPipeline()
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    Pipeline ret;

    ComPtr<ID3DBlob> blob;
    D3D12_ROOT_PARAMETER constants;
    constants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    constants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    constants.Descriptor.RegisterSpace = 0;
    constants.Descriptor.ShaderRegister = 1;
    std::array parameters = {constants};

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = (UINT)parameters.size();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pParameters = parameters.data();
    rootSignatureDesc.pStaticSamplers = nullptr;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
    assert(blob);

    device->CreateRootSignature(0b1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ret.RootSignature));

    struct
    {
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT;
            D3D12_INPUT_LAYOUT_DESC desc;
        } InputLayout;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
            D3D12_SHADER_BYTECODE desc;
        } VS;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
            D3D12_SHADER_BYTECODE desc;
        } PS;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
            ID3D12RootSignature* desc;
        } RootSignature;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
            D3D12_RT_FORMAT_ARRAY desc;
        } RenderTargets;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
            D3D12_DEPTH_STENCIL_DESC desc;
        } DepthStencil;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
            DXGI_FORMAT desc;
        } DepthFormat;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
            D3D12_RASTERIZER_DESC desc;
        } Rasterizer;
    } streamDesc;

    D3D12_INPUT_ELEMENT_DESC position;
    position.AlignedByteOffset = 0;
    position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
    position.InputSlot = 0;
    position.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    position.InstanceDataStepRate = 0;
    position.SemanticIndex = 0;
    position.SemanticName = "Position";
    std::array vertexInputs = {position};
    streamDesc.InputLayout.desc = {vertexInputs.data(), (UINT)vertexInputs.size()};
    streamDesc.VS.desc = { DebugCascadesVS, sizeof(DebugCascadesVS) };
    streamDesc.PS.desc = { DebugCascadesPS, sizeof(DebugCascadesPS) };
    streamDesc.RootSignature.desc = ret.RootSignature.Get();
    streamDesc.RenderTargets.desc = {};
    streamDesc.RenderTargets.desc.NumRenderTargets = 1;
    streamDesc.RenderTargets.desc.RTFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    streamDesc.DepthStencil.desc.DepthEnable = TRUE;
    streamDesc.DepthStencil.desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
    streamDesc.DepthStencil.desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    streamDesc.DepthStencil.desc.StencilEnable = FALSE;
    streamDesc.DepthFormat.desc = DXGI_FORMAT_D32_FLOAT;
    streamDesc.Rasterizer.desc = {};
    streamDesc.Rasterizer.desc.FillMode = D3D12_FILL_MODE_SOLID;
    streamDesc.Rasterizer.desc.CullMode = D3D12_CULL_MODE_NONE;
    D3D12_PIPELINE_STATE_STREAM_DESC stateDesc;
    stateDesc.pPipelineStateSubobjectStream = &streamDesc;
    stateDesc.SizeInBytes = sizeof(streamDesc);
    device->CreatePipelineState(&stateDesc, IID_PPV_ARGS(&ret.State));
    assert(ret.State);

    return ret;
}

State Device::CreateRayTracingPipeline()
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    State ret;

    ComPtr<ID3DBlob> blob;
    D3D12_ROOT_PARAMETER raytracingConstants;
    raytracingConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    raytracingConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    raytracingConstants.Descriptor.RegisterSpace = 0;
    raytracingConstants.Descriptor.ShaderRegister = 0;

    D3D12_DESCRIPTOR_RANGE as;
    as.BaseShaderRegister = 0;
    as.NumDescriptors = 1;
    as.OffsetInDescriptorsFromTableStart = 0;
    as.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    as.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER accelerationStructure;
    accelerationStructure.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    accelerationStructure.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    accelerationStructure.DescriptorTable.NumDescriptorRanges = 1;
    accelerationStructure.DescriptorTable.pDescriptorRanges = &as;

    D3D12_DESCRIPTOR_RANGE instanceRange;
    instanceRange.BaseShaderRegister = 1;
    instanceRange.NumDescriptors = 1;
    instanceRange.OffsetInDescriptorsFromTableStart = 0;
    instanceRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER instances;
    instances.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    instances.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    instances.DescriptorTable.NumDescriptorRanges = 1;
    instances.DescriptorTable.pDescriptorRanges = &instanceRange;

    D3D12_DESCRIPTOR_RANGE uav;
    uav.BaseShaderRegister = 0;
    uav.NumDescriptors = 1;
    uav.OffsetInDescriptorsFromTableStart = 0;
    uav.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uav.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER renderTarget;
    renderTarget.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    renderTarget.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    renderTarget.DescriptorTable.NumDescriptorRanges = 1;
    renderTarget.DescriptorTable.pDescriptorRanges = &uav;

    std::array parameters = {raytracingConstants, accelerationStructure, instances, renderTarget};
    
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    rootSignatureDesc.NumParameters = (UINT)parameters.size();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pParameters = parameters.data();
    rootSignatureDesc.pStaticSamplers = nullptr;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
    assert(blob);
    device->CreateRootSignature(0b1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ret.RootSignature));

    std::vector<D3D12_STATE_SUBOBJECT> subobjects;
    subobjects.reserve(10);

    D3D12_EXPORT_DESC rayGen = {L"RayGen", nullptr, D3D12_EXPORT_FLAG_NONE};
    D3D12_EXPORT_DESC rayHit = {L"RayHit", nullptr, D3D12_EXPORT_FLAG_NONE};
    D3D12_EXPORT_DESC rayMiss = {L"RayMiss", nullptr, D3D12_EXPORT_FLAG_NONE};
    std::array exports = {rayGen, rayHit, rayMiss};
    D3D12_DXIL_LIBRARY_DESC rayLibraryDesc;
    rayLibraryDesc.DXILLibrary = {Raytracing, sizeof(Raytracing)};
    rayLibraryDesc.NumExports = (UINT)exports.size();
    rayLibraryDesc.pExports = exports.data();
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &rayLibraryDesc});

    D3D12_HIT_GROUP_DESC hitGroup = {};
    hitGroup.ClosestHitShaderImport = L"RayHit";
    hitGroup.HitGroupExport = L"HitGroup";
    hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup});

    D3D12_RAYTRACING_SHADER_CONFIG sc;
    sc.MaxAttributeSizeInBytes = 2 * sizeof(float);
    sc.MaxPayloadSizeInBytes = 4 * sizeof(float);
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &sc});

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig;
    globalRootSig.pGlobalRootSignature = ret.RootSignature.Get();
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRootSig});

    D3D12_RAYTRACING_PIPELINE_CONFIG pc;
    pc.MaxTraceRecursionDepth = 1;
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pc});

    D3D12_STATE_OBJECT_DESC stateDesc;
    stateDesc.NumSubobjects = (UINT)subobjects.size();
    stateDesc.pSubobjects = subobjects.data();
    stateDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    auto r = device->CreateStateObject(&stateDesc, IID_PPV_ARGS(&ret.Object));
    assert(ret.Object);

    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ret.Object.As(&stateObjectProperties);
    assert(stateObjectProperties);

    const auto rayGenId = stateObjectProperties->GetShaderIdentifier(L"RayGen");
    const auto rayMissId = stateObjectProperties->GetShaderIdentifier(L"RayMiss");
    const auto rayHitId = stateObjectProperties->GetShaderIdentifier(L"HitGroup");

    const auto shaderTableSize = 3 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    ret.ShaderTable = CreateBuffer(shaderTableSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    ret.RayGenRange = {ret.ShaderTable->GetGPUVirtualAddress(), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};
    ret.RayMissRange = {ret.ShaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};
    ret.RayHitRange = {ret.ShaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * 2, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};

    std::vector<uint8_t> shaderIds(shaderTableSize);
    std::memcpy(shaderIds.data(), rayGenId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    std::memcpy(shaderIds.data() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, rayMissId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    std::memcpy(shaderIds.data() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * 2, rayHitId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    SetResourceData(ret.ShaderTable, *shaderIds.data(), shaderTableSize);

    return ret;
}

State Device::CreateCascadeTracingPipeline()
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    State ret;

    ComPtr<ID3DBlob> blob;
    D3D12_ROOT_PARAMETER constants;
    constants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    constants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    constants.Constants.Num32BitValues = 1;
    constants.Constants.RegisterSpace = 0;
    constants.Constants.ShaderRegister = 0;

    D3D12_ROOT_PARAMETER cascadeConstants;
    cascadeConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    cascadeConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    cascadeConstants.Descriptor.RegisterSpace = 0;
    cascadeConstants.Descriptor.ShaderRegister = 1;

    D3D12_DESCRIPTOR_RANGE as;
    as.BaseShaderRegister = 0;
    as.NumDescriptors = 1;
    as.OffsetInDescriptorsFromTableStart = 0;
    as.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    as.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER accelerationStructure;
    accelerationStructure.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    accelerationStructure.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    accelerationStructure.DescriptorTable.NumDescriptorRanges = 1;
    accelerationStructure.DescriptorTable.pDescriptorRanges = &as;

    D3D12_DESCRIPTOR_RANGE instanceRange;
    instanceRange.BaseShaderRegister = 1;
    instanceRange.NumDescriptors = 1;
    instanceRange.OffsetInDescriptorsFromTableStart = 0;
    instanceRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER instances;
    instances.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    instances.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    instances.DescriptorTable.NumDescriptorRanges = 1;
    instances.DescriptorTable.pDescriptorRanges = &instanceRange;

    D3D12_DESCRIPTOR_RANGE cascadesRange;
    cascadesRange.BaseShaderRegister = 0;
    cascadesRange.NumDescriptors = 1;
    cascadesRange.OffsetInDescriptorsFromTableStart = 0;
    cascadesRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    cascadesRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER cascades;
    cascades.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    cascades.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    cascades.DescriptorTable.NumDescriptorRanges = 1;
    cascades.DescriptorTable.pDescriptorRanges = &cascadesRange;

    std::array parameters = {constants, cascadeConstants, accelerationStructure, instances, cascades};
    
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    rootSignatureDesc.NumParameters = (UINT)parameters.size();
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pParameters = parameters.data();
    rootSignatureDesc.pStaticSamplers = nullptr;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
    assert(blob);
    device->CreateRootSignature(0b1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ret.RootSignature));

    std::vector<D3D12_STATE_SUBOBJECT> subobjects;
    subobjects.reserve(10);

    D3D12_EXPORT_DESC rayGen = {L"RayGen", nullptr, D3D12_EXPORT_FLAG_NONE};
    D3D12_EXPORT_DESC rayHit = {L"RayHit", nullptr, D3D12_EXPORT_FLAG_NONE};
    D3D12_EXPORT_DESC rayMiss = {L"RayMiss", nullptr, D3D12_EXPORT_FLAG_NONE};
    std::array exports = {rayGen, rayHit, rayMiss};
    D3D12_DXIL_LIBRARY_DESC rayLibraryDesc;
    rayLibraryDesc.DXILLibrary = {CascadeTracing, sizeof(CascadeTracing)};
    rayLibraryDesc.NumExports = (UINT)exports.size();
    rayLibraryDesc.pExports = exports.data();
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &rayLibraryDesc});

    D3D12_HIT_GROUP_DESC hitGroup = {};
    hitGroup.ClosestHitShaderImport = L"RayHit";
    hitGroup.HitGroupExport = L"HitGroup";
    hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup});

    D3D12_RAYTRACING_SHADER_CONFIG sc;
    sc.MaxAttributeSizeInBytes = 2 * sizeof(float);
    sc.MaxPayloadSizeInBytes = 4 * sizeof(float);
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &sc});

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSig;
    globalRootSig.pGlobalRootSignature = ret.RootSignature.Get();
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalRootSig});

    D3D12_RAYTRACING_PIPELINE_CONFIG pc;
    pc.MaxTraceRecursionDepth = 1;
    subobjects.push_back({D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pc});

    D3D12_STATE_OBJECT_DESC stateDesc;
    stateDesc.NumSubobjects = (UINT)subobjects.size();
    stateDesc.pSubobjects = subobjects.data();
    stateDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    auto r = device->CreateStateObject(&stateDesc, IID_PPV_ARGS(&ret.Object));
    assert(ret.Object);

    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ret.Object.As(&stateObjectProperties);
    assert(stateObjectProperties);

    const auto rayGenId = stateObjectProperties->GetShaderIdentifier(L"RayGen");
    const auto rayMissId = stateObjectProperties->GetShaderIdentifier(L"RayMiss");
    const auto rayHitId = stateObjectProperties->GetShaderIdentifier(L"HitGroup");

    const auto shaderTableSize = 3 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
    ret.ShaderTable = CreateBuffer(shaderTableSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    ret.RayGenRange = {ret.ShaderTable->GetGPUVirtualAddress(), D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};
    ret.RayMissRange = {ret.ShaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};
    ret.RayHitRange = {ret.ShaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * 2, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT};

    std::vector<uint8_t> shaderIds(shaderTableSize);
    std::memcpy(shaderIds.data(), rayGenId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    std::memcpy(shaderIds.data() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, rayMissId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    std::memcpy(shaderIds.data() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * 2, rayHitId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    SetResourceData(ret.ShaderTable, *shaderIds.data(), shaderTableSize);

    return ret;
}

Pipeline Device::CreateCascadeAccumulationPipeline()
{
    ComPtr<ID3D12Device5> device;
    m_device.As(&device);
    assert(device);

    Pipeline ret;

    ComPtr<ID3DBlob> blob;
    D3D12_ROOT_PARAMETER constants;
    constants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    constants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    constants.Constants.Num32BitValues = 1;
    constants.Constants.RegisterSpace = 0;
    constants.Constants.ShaderRegister = 0;
    D3D12_ROOT_PARAMETER cascadeConstants;
    cascadeConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    cascadeConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    cascadeConstants.Descriptor.RegisterSpace = 0;
    cascadeConstants.Descriptor.ShaderRegister = 1;
    D3D12_DESCRIPTOR_RANGE higherCascadeRange;
    higherCascadeRange.BaseShaderRegister = 0;
    higherCascadeRange.NumDescriptors = 1;
    higherCascadeRange.OffsetInDescriptorsFromTableStart = 0;
    higherCascadeRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    higherCascadeRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER higherCascade;
    higherCascade.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    higherCascade.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    higherCascade.DescriptorTable.NumDescriptorRanges = 1;
    higherCascade.DescriptorTable.pDescriptorRanges = &higherCascadeRange;
    D3D12_DESCRIPTOR_RANGE currentCascadeRange;
    currentCascadeRange.BaseShaderRegister = 0;
    currentCascadeRange.NumDescriptors = 1;
    currentCascadeRange.OffsetInDescriptorsFromTableStart = 0;
    currentCascadeRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    currentCascadeRange.RegisterSpace = 0;
    D3D12_ROOT_PARAMETER currentCascade;
    currentCascade.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    currentCascade.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    currentCascade.DescriptorTable.NumDescriptorRanges = 1;
    currentCascade.DescriptorTable.pDescriptorRanges = &currentCascadeRange;
    std::array parameters = {constants, cascadeConstants, higherCascade, currentCascade};

    D3D12_STATIC_SAMPLER_DESC linearSampler;
    linearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    linearSampler.MaxAnisotropy = 0;
    linearSampler.MaxLOD = 0.f;
    linearSampler.MinLOD = 0.f;
    linearSampler.MipLODBias = 0.f;
    linearSampler.RegisterSpace = 0;
    linearSampler.ShaderRegister = 0;
    linearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = (UINT)parameters.size();
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pParameters = parameters.data();
    rootSignatureDesc.pStaticSamplers = &linearSampler;
    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, nullptr);
    assert(blob);

    device->CreateRootSignature(0b1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ret.RootSignature));

    struct
    {
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
            D3D12_SHADER_BYTECODE desc;
        } CS;
        struct alignas(void*)
        {
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
            ID3D12RootSignature* desc;
        } RootSignature;
    } streamDesc;

    streamDesc.CS.desc = {CascadeAccumulation, sizeof(CascadeAccumulation)};
    streamDesc.RootSignature.desc = ret.RootSignature.Get();

    D3D12_PIPELINE_STATE_STREAM_DESC stateDesc;
    stateDesc.pPipelineStateSubobjectStream = &streamDesc;
    stateDesc.SizeInBytes = sizeof(streamDesc);
    device->CreatePipelineState(&stateDesc, IID_PPV_ARGS(&ret.State));
    assert(ret.State);

    return ret;
}

// TODO make this into a type dependend template
void Device::SetResourceDataInternal(const ComPtr<ID3D12Resource>& resource, const void* data, uint64_t size)
{
    void* resourcePtr = nullptr;
    resource->Map(0, nullptr, &resourcePtr);
    assert(resourcePtr);

    std::memcpy(resourcePtr, data, size);

    D3D12_RANGE writeRange = {0, size};
    resource->Unmap(0, &writeRange);
}

void Device::SetDescriptorHeaps(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto srvHeap = m_srvHeap.Get();
    commandList->SetDescriptorHeaps(1, &srvHeap);
}

void Device::WaitIdle()
{
    m_submissionFence->SetEventOnCompletion(m_submissionCounter, m_submissionEvent);
    WaitForSingleObject(m_submissionEvent, INFINITE);
}

void Device::PipelineBarrierUav(const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& resource)
{
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.UAV.pResource = resource.Get();
    commandList->ResourceBarrier(1, &barr);
}

void Device::PipelineBarrierTransition(const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, uint32_t subresource)
{
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = resource.Get();
    barr.Transition.StateBefore = before;
    barr.Transition.StateAfter = after;
    barr.Transition.Subresource = subresource;
    commandList->ResourceBarrier(1, &barr);
}