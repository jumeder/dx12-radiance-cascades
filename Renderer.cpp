#include "Renderer.h"

#include "Model.h"

#include <DirectXMath.h>

Model* model; 

Renderer::Renderer(HWND hwnd, uint32_t width, uint32_t height)
    : m_width(width)
    , m_height(height)
{
    m_swapChain = m_device.CreateSwapChain(hwnd, width, height, c_backBufferCount);
    for (auto i = 0u; i < m_swapChainTargets.size(); ++i)
    {
        auto& swapChainTarget = m_swapChainTargets[i];
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainTarget.Resource));
        swapChainTarget.Target = m_device.CreateRenderTargetView(swapChainTarget.Resource, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    }
    m_depthStencil.Resource = m_device.CreateTexture(DXGI_FORMAT_D32_FLOAT, width, height, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    m_depthStencil.Target = m_device.CreateDepthStencilView(m_depthStencil.Resource, DXGI_FORMAT_D32_FLOAT);
    model = new Model("d:\\Scenes\\Test\\Sponza.fbx", m_device);
    m_drawingPipeline = m_device.CreateDrawingPipeline();
    m_cameraConstants = m_device.CreateBuffer(256, D3D12_RESOURCE_FLAG_NONE, true);
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
    commands.List->ClearRenderTargetView(frameTarget.Target, color, 1, &rect);
    commands.List->ClearDepthStencilView(m_depthStencil.Target, D3D12_CLEAR_FLAG_DEPTH, 0.f, 0, 1, &rect);

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
    commands.List->OMSetRenderTargets(1, &frameTarget.Target, FALSE, &m_depthStencil.Target);
    commands.List->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    model->Draw(commands.List);

    barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commands.List->ResourceBarrier(1, &barr);

    m_device.SubmitGraphicsCommands(std::move(commands));

    m_swapChain->Present(1, 0);

    ++m_frameCounter;
}