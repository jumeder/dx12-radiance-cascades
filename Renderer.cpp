#include "Renderer.h"

#include <DirectXMath.h>

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height)
    : m_width(width)
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

    m_raytracingPipeline = m_device.CreateRayTracingPipeline();
    m_raytracingConstants = m_device.CreateBuffer(256, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    m_raytracingTarget.Resource = m_device.CreateTexture(DXGI_FORMAT_B8G8R8A8_UNORM, m_width, m_height, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    m_raytracingTarget.GpuHandle = m_device.CreateUnorderedAccessView(m_raytracingTarget.Resource, DXGI_FORMAT_B8G8R8A8_UNORM);

    m_model = std::make_unique<Model>("d:\\Scenes\\Test\\Sponza.fbx", m_device);
    std::array blass = {m_model->GetBLAS()};
    m_tlas = m_device.CreateTopLevelAccelerationStructure(blass.data(), blass.size());

    D3D12_SHADER_RESOURCE_VIEW_DESC tlasDesc;
    tlasDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    tlasDesc.Format = DXGI_FORMAT_UNKNOWN;
    tlasDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    tlasDesc.RaytracingAccelerationStructure.Location = m_tlas->GetGPUVirtualAddress();
    m_tlasHandle = m_device.CreateShaderResourceView(m_tlas, tlasDesc);
}

void Renderer::Render(const Camera& camera)
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
#if 1
    struct
    {
        DirectX::XMMATRIX viewTransform;
        float aspect;
    } RayTracingConstants;
    RayTracingConstants.viewTransform = camera.GetTransform();
    RayTracingConstants.aspect = (float)m_height / m_width;
    
    m_device.SetResourceData(m_raytracingConstants, &RayTracingConstants, sizeof(RayTracingConstants));

    ComPtr<ID3D12GraphicsCommandList4> commandList;
    commands.List.As(&commandList);
    assert(commandList);
    commandList->SetPipelineState1(m_raytracingPipeline.Object.Get());

    m_device.SetDescriptorHeaps(commands.List);

    commands.List->SetComputeRootSignature(m_raytracingPipeline.RootSignature.Get());
    commands.List->SetComputeRootConstantBufferView(0, m_raytracingConstants->GetGPUVirtualAddress());
    commands.List->SetComputeRootDescriptorTable(1, m_tlasHandle);
    commands.List->SetComputeRootDescriptorTable(2, m_raytracingTarget.GpuHandle);

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
    
    commands.List->SetPipelineState(m_drawingPipeline.State.Get());
    commands.List->SetGraphicsRootSignature(m_drawingPipeline.RootSignature.Get());
    
    struct 
    {
        DirectX::XMMATRIX viewProjection;
    } cameraConstants;  
    cameraConstants.viewProjection = camera.GetViewProjection();
    m_device.SetResourceData(m_cameraConstants, &cameraConstants, sizeof(cameraConstants));
    commands.List->SetGraphicsRootConstantBufferView(0, m_cameraConstants->GetGPUVirtualAddress());
    
    D3D12_VIEWPORT viewport = {0.f, 0.f, (float)m_width, (float)m_height, 0.f, 1.f};
    commands.List->RSSetScissorRects(1, &rect);
    commands.List->RSSetViewports(1, &viewport);
    commands.List->OMSetRenderTargets(1, &frameTarget.CpuHandle, FALSE, &m_depthStencil.CpuHandle);
    commands.List->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_model->Draw(commands.List);

    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commands.List->ResourceBarrier(1, &barr);
    #endif

    m_device.SubmitGraphicsCommands(std::move(commands));

    m_swapChain->Present(1, 0);

    ++m_frameCounter;
}