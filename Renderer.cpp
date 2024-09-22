#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height)
    : m_radianceCascades(m_device, {32, 32, 32}, {1.f, 1.f, 1.f}, {0.f, 1.f, 0.f}, 4)
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

    auto& cascadesHandles = m_radianceCascades.Generate(commands.List, accelHandle, scene.GetInstanceDataHandle());
    
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
    commands.List->SetGraphicsRootDescriptorTable(4, cascadesHandles[0]);
    
    D3D12_VIEWPORT viewport = {0.f, 0.f, (float)m_width, (float)m_height, 0.f, 1.f};
    commands.List->RSSetScissorRects(1, &rect);
    commands.List->RSSetViewports(1, &viewport);
    commands.List->OMSetRenderTargets(1, &frameTarget.CpuHandle, FALSE, &m_depthStencil.CpuHandle);
    commands.List->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    scene.Draw(commands.List);

    if(m_debugCascade >= 0)
    {
        struct
        {
            DirectX::XMMATRIX vp;
            DirectX::XMMATRIX model;
            uint32_t cascade;
        } debugConstants;
        debugConstants.vp = cameraConstants.viewProjection;
        debugConstants.model = DirectX::XMMatrixScaling(0.005f, 0.005f, 0.005f);
        debugConstants.cascade = m_debugCascade;

        Device::SetResourceData(m_debugCascadesConstants, debugConstants);

        commands.List->SetPipelineState(m_debugCascadesPipeline.State.Get());
        commands.List->SetGraphicsRootSignature(m_debugCascadesPipeline.RootSignature.Get());
        commands.List->SetGraphicsRootConstantBufferView(0, m_debugCascadesConstants->GetGPUVirtualAddress());
        commands.List->SetGraphicsRootConstantBufferView(1, m_radianceCascades.GetConstants()->GetGPUVirtualAddress());
        commands.List->SetGraphicsRootDescriptorTable(2, cascadesHandles[debugConstants.cascade]);

        auto& res = m_radianceCascades.GetResolution();
        const auto div = 1 << debugConstants.cascade;
        m_debugSphere->DrawInstanced(commands.List, res.x * res.y * res.z / (div * div * div));
    }

    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commands.List->ResourceBarrier(1, &barr);

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

void Renderer::Finish()
{
    m_device.Finish();
}