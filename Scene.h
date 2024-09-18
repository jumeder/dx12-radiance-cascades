#pragma once

#include "Device.h"
#include "Model.h"

class Scene
{
public:
    Scene(Device& device);

    void Update(const ComPtr<ID3D12GraphicsCommandList>& commandList);

    void Draw(const ComPtr<ID3D12GraphicsCommandList>& commandList);

    inline auto& GetAccelerationStructure() const { return m_tlas; }
    inline auto GetInstanceDataHandle() const { return m_instanceDataHandle; }
    
    uint32_t AddInstance(const Model& model, const DirectX::XMMATRIX& transform, const DirectX::XMVECTOR& albedo, const DirectX::XMVECTOR& emission);

    void SetInstanceTransform(uint32_t instance, const DirectX::XMMATRIX& transform);
    void SetInstanceAlbedo(uint32_t instance, const DirectX::XMVECTOR& albedo);
    void SetInstanceEmission(uint32_t instance, const DirectX::XMVECTOR& emission);

private:
    struct Instance
    {
        DirectX::XMMATRIX Transform;
        DirectX::XMVECTOR Albedo;
        DirectX::XMVECTOR Emission;
    };

    static constexpr auto c_instanceCount = 65536;
    static constexpr auto c_instanceDataSize = c_instanceCount * sizeof(Instance);

    Device& m_device;
    std::vector<const Model*> m_modelRefs;
    ComPtr<ID3D12Resource> m_tlas;
    ComPtr<ID3D12Resource> m_instanceDataCpu;
    ComPtr<ID3D12Resource> m_instanceDataGpu;
    ComPtr<ID3D12Resource> m_tlasBuildData;
    D3D12_GPU_DESCRIPTOR_HANDLE m_instanceDataHandle = {};
    Instance* m_instanceDataPtr = nullptr;
    bool m_transformsDirty = false;
    bool m_instanceDataDirty = false;
};