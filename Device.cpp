#include "Device.h"

#include "Drawing.vs.h"
#include "Drawing.ps.h"

Device::Device()
{
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

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

    m_rtvPos = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    m_dsvPos = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    // TODO srvs
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

ComPtr<ID3D12Resource> Device::CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, bool staging)
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
    m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
    return buffer;
}

VertexBuffer Device::CreateVertexBuffer(const std::vector<float>& data)
{
    const auto dataSize = data.size() * sizeof(data[0]);
    const auto stagingBuffer = CreateBuffer(dataSize, D3D12_RESOURCE_FLAG_NONE, true);
    const auto gpuBuffer = CreateBuffer(dataSize);

    SetResourceData(stagingBuffer, data.data(), dataSize);

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
    const auto stagingBuffer = CreateBuffer(dataSize, D3D12_RESOURCE_FLAG_NONE, true);
    const auto gpuBuffer = CreateBuffer(dataSize);

    SetResourceData(stagingBuffer, data.data(), dataSize);

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

D3D12_CPU_DESCRIPTOR_HANDLE Device::CreateRenderTargetView(const ComPtr<ID3D12Resource> resource, DXGI_FORMAT format)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvPos;
    m_rtvPos = m_rtvPos + m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    m_device->CreateRenderTargetView(resource.Get(), &rtvDesc, handle);

    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::CreateDepthStencilView(const ComPtr<ID3D12Resource> resource, DXGI_FORMAT format)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_dsvPos;
    m_dsvPos = m_dsvPos + m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = format;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    m_device->CreateDepthStencilView(resource.Get(), &dsvDesc, handle);
    
    return handle;
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
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pParameters = &cameraConstants;
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

    return ret;
}

void Device::SetResourceData(const ComPtr<ID3D12Resource>& resource, const void* data, uint64_t size)
{
    void* resourcePtr = nullptr;
    resource->Map(0, nullptr, &resourcePtr);
    assert(resourcePtr);

    std::memcpy(resourcePtr, data, size);

    D3D12_RANGE writeRange = {0, size};
    resource->Unmap(0, &writeRange);
}

void Device::WaitIdle()
{
    m_submissionFence->SetEventOnCompletion(m_submissionCounter, m_submissionEvent);
    WaitForSingleObject(m_submissionEvent, INFINITE);
}
