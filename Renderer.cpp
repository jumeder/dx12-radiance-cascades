#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height)
    : m_radianceCascades(m_device, {16, 16, 16}, {10.f, 10.f, 10.f}, {0.f, 6.f, 0.f}, 5)
    , m_width(width)
    , m_height(height)
{
    m_swapChain = m_device.CreateSwapChain(hwnd, width, height, c_backBufferCount);
    for (auto i = 0u; i < m_swapChainTargets.size(); ++i)
    {
        auto& swapChainTarget = m_swapChainTargets[i];
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainTarget.Resource));
        swapChainTarget.CpuHandle = m_device.CreateRenderTargetView(swapChainTarget.Resource, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    }
    m_depthStencil.Resource = m_device.CreateTexture(DXGI_FORMAT_D32_FLOAT, width, height, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    m_depthStencil.CpuHandle = m_device.CreateDepthStencilView(m_depthStencil.Resource, DXGI_FORMAT_D32_FLOAT);
    
    m_drawingPipeline = m_device.CreateDrawingPipeline();
    m_cameraConstants = m_device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);

    m_debugSphere = std::make_unique<Model>("d:\\Scenes\\Test\\Sphere.glb", m_device);
    m_debugCascadesPipeline = m_device.CreateCascadeDebugPipeline();
    m_debugCascadesConstants = m_device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);

#if 0
    m_raytracingPipeline = m_device.CreateRayTracingPipeline();
    m_raytracingConstants = m_device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    m_raytracingTarget.Resource = m_device.CreateTexture(DXGI_FORMAT_B8G8R8A8_UNORM, m_width, m_height, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_raytracingTarget.GpuHandle = m_device.CreateUnorderedAccessView(m_raytracingTarget.Resource, DXGI_FORMAT_B8G8R8A8_UNORM);
#endif
}

void Renderer::Render(const Camera& camera, Scene& scene)
{
    auto commands = m_device.CreateGraphicsCommands();

    const auto frameIndex = m_frameCounter % c_backBufferCount;
    auto& frameTarget = m_swapChainTargets[frameIndex];
    D3D12_RESOURCE_BARRIER barr;
    barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr.Transition.pResource = frameTarget.Resource.Get();
    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commands.List->ResourceBarrier(1, &barr);

    FLOAT color[] = {0.f, 0.f, 0.f, 0.f};
    D3D12_RECT rect = {0, 0, (LONG)m_width, (LONG)m_height};
    commands.List->ClearRenderTargetView(frameTarget.CpuHandle, color, 1, &rect);
    commands.List->ClearDepthStencilView(m_depthStencil.CpuHandle, D3D12_CLEAR_FLAG_DEPTH, 0.f, 0, 1, &rect);

    scene.Update(commands.List);

#if 0
    auto& accelHandle = m_accelHandles[frameIndex];
    auto& accelStruct = scene.GetAccelerationStructure();

    D3D12_SHADER_RESOURCE_VIEW_DESC accelViewDesc;
    accelViewDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    accelViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    accelViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    accelViewDesc.RaytracingAccelerationStructure.Location = accelStruct->GetGPUVirtualAddress();
    accelHandle = m_device.CreateShaderResourceView(accelStruct, accelViewDesc, accelHandle);

    struct
    {
        DirectX::XMMATRIX viewTransform;
        float aspect;
    } RayTracingConstants;
    RayTracingConstants.viewTransform = camera.GetTransform();
    RayTracingConstants.aspect = (float)m_height / m_width;
    
    m_device.SetResourceData(m_raytracingConstants, RayTracingConstants);

    ComPtr<ID3D12GraphicsCommandList4> commandList;
    commands.List.As(&commandList);
    assert(commandList);
    commandList->SetPipelineState1(m_raytracingPipeline.Object.Get());

    commands.List->SetComputeRootSignature(m_raytracingPipeline.RootSignature.Get());
    commands.List->SetComputeRootConstantBufferView(0, m_raytracingConstants->GetGPUVirtualAddress());
    commands.List->SetComputeRootDescriptorTable(1, accelHandle);
    commands.List->SetComputeRootDescriptorTable(2, scene.GetInstanceDataHandle());
    commands.List->SetComputeRootDescriptorTable(3, m_raytracingTarget.GpuHandle);

    D3D12_DISPATCH_RAYS_DESC rays = {};
    rays.Width = m_width;
    rays.Height = m_height;
    rays.Depth = 1;
    rays.RayGenerationShaderRecord = m_raytracingPipeline.RayGenRange;
    rays.MissShaderTable = m_raytracingPipeline.RayMissRange;
    rays.HitGroupTable = m_raytracingPipeline.RayHitRange;
    commandList->DispatchRays(&rays);

    Device::PipelineBarrierTransition(commands.List, m_raytracingTarget.Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    Device::PipelineBarrierTransition(commands.List, frameTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

    commands.List->CopyResource(frameTarget.Resource.Get(), m_raytracingTarget.Resource.Get());

    Device::PipelineBarrierTransition(commands.List, frameTarget.Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    Device::PipelineBarrierTransition(commands.List, m_raytracingTarget.Resource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
#else

    auto accelStruct = scene.GetAccelerationStructure();

    D3D12_GPU_DESCRIPTOR_HANDLE accelHandle = {};
    if(!m_freeRaytracingHandles.empty())
    {
        accelHandle = m_freeRaytracingHandles.back();
        m_freeRaytracingHandles.pop_back();
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC accelViewDesc;
    accelViewDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    accelViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    accelViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    accelViewDesc.RaytracingAccelerationStructure.Location = accelStruct->GetGPUVirtualAddress();
    accelHandle = m_device.CreateShaderResourceView(accelStruct, accelViewDesc, accelHandle);

    auto cascadesHandle = m_radianceCascades.Generate(commands.List, accelHandle, scene.GetInstanceDataHandle());
    
    commands.List->SetPipelineState(m_drawingPipeline.State.Get());
    commands.List->SetGraphicsRootSignature(m_drawingPipeline.RootSignature.Get());
    
    struct 
    {
        DirectX::XMMATRIX viewProjection;
    } cameraConstants;  
    cameraConstants.viewProjection = camera.GetViewProjection();
    m_device.SetResourceData(m_cameraConstants, cameraConstants);
    commands.List->SetGraphicsRootConstantBufferView(0, m_cameraConstants->GetGPUVirtualAddress());
    commands.List->SetGraphicsRootDescriptorTable(1, scene.GetInstanceDataHandle());
    commands.List->SetGraphicsRootConstantBufferView(3, m_radianceCascades.GetConstants()->GetGPUVirtualAddress());
    commands.List->SetGraphicsRootDescriptorTable(4, cascadesHandle);
    
    D3D12_VIEWPORT viewport = {0.f, 0.f, (float)m_width, (float)m_height, 0.f, 1.f};
    commands.List->RSSetScissorRects(1, &rect);
    commands.List->RSSetViewports(1, &viewport);
    commands.List->OMSetRenderTargets(1, &frameTarget.CpuHandle, FALSE, &m_depthStencil.CpuHandle);
    commands.List->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    scene.Draw(commands.List);
#if 1
    struct
    {
        DirectX::XMMATRIX vp;
        DirectX::XMMATRIX model;
        uint32_t cascade;
    } debugConstants;
    debugConstants.vp = cameraConstants.viewProjection;
    debugConstants.model = DirectX::XMMatrixScaling(0.005f, 0.005f, 0.005f);
    debugConstants.cascade = 0;

    Device::SetResourceData(m_debugCascadesConstants, debugConstants);

    commands.List->SetPipelineState(m_debugCascadesPipeline.State.Get());
    commands.List->SetGraphicsRootSignature(m_debugCascadesPipeline.RootSignature.Get());
    commands.List->SetGraphicsRootConstantBufferView(0, m_debugCascadesConstants->GetGPUVirtualAddress());
    commands.List->SetGraphicsRootConstantBufferView(1, m_radianceCascades.GetConstants()->GetGPUVirtualAddress());
    commands.List->SetGraphicsRootDescriptorTable(2, cascadesHandle);

    auto& res = m_radianceCascades.GetResolution();
    const auto div = 1 << debugConstants.cascade;
    m_debugSphere->DrawInstanced(commands.List, res.x * res.y * res.z / (div * div * div));
#endif

    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commands.List->ResourceBarrier(1, &barr);
    #endif

    const auto submission = m_device.SubmitGraphicsCommands(std::move(commands));

    m_pendingRaytracingResources.push_back({accelStruct, accelHandle, submission});
    const auto finishedSubmission = m_device.GetCompletedSubmission();
    while (!m_pendingRaytracingResources.empty() && m_pendingRaytracingResources.front().Submission <= finishedSubmission)
    {
        m_freeRaytracingHandles.push_back(m_pendingRaytracingResources.front().Handle);
        m_pendingRaytracingResources.pop_front();
    }

    m_swapChain->Present(1, 0);

    ++m_frameCounter;
}