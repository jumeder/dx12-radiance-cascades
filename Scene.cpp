#include "Scene.h"

Scene::Scene(Device& device)
    : m_device(device)
{
    m_instanceDataCpu = device.CreateBuffer(c_instanceDataSize, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);
    m_instanceDataGpu = device.CreateBuffer(c_instanceDataSize);
    m_tlasBuildData = device.CreateBuffer(c_instanceDataSize * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE, true);

    D3D12_SHADER_RESOURCE_VIEW_DESC instanceDataViewDesc;
    instanceDataViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    instanceDataViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    instanceDataViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    instanceDataViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    instanceDataViewDesc.Buffer.FirstElement = 0;
    instanceDataViewDesc.Buffer.NumElements = c_instanceCount;
    instanceDataViewDesc.Buffer.StructureByteStride = sizeof(Instance);
    m_instanceDataHandle = device.CreateShaderResourceView(m_instanceDataGpu, instanceDataViewDesc);

    m_modelRefs.reserve(c_instanceCount);

    D3D12_RANGE range = {0, c_instanceDataSize};
    m_instanceDataCpu->Map(0, &range, (void**)&m_instanceDataPtr);
}

void Scene::Update(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    if(m_transformsDirty || m_instanceDataDirty)
    {
        // TODO is this really necessary?
        D3D12_RANGE writeRange = {0, c_instanceDataSize};
        m_instanceDataCpu->Unmap(0, &writeRange);

        // TODO barriers?
        commandList->CopyResource(m_instanceDataGpu.Get(), m_instanceDataCpu.Get());
        
        m_instanceDataCpu->Map(0, nullptr, (void**)&m_instanceDataPtr);
    }

    // TODO update tlas
    if(m_transformsDirty)
    {
        m_tlas = m_device.CreateTopLevelAccelerationStructure(commandList.Get(), m_tlasBuildData.Get(), (uint32_t)m_modelRefs.size());
    }

    m_transformsDirty = false;
    m_instanceDataDirty = false;
}

void Scene::Draw(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    for (auto i = 0u; i < m_modelRefs.size(); ++i)
    {
        m_modelRefs[i]->Draw(commandList, i);
    }
}

uint32_t Scene::AddInstance(const Model& model, const DirectX::XMMATRIX& transform, const DirectX::XMVECTOR& albedo, const DirectX::XMVECTOR& emission)
{
    assert(m_modelRefs.size() < c_instanceCount);

    // TODO support deletion
    const uint32_t instanceId = (uint32_t)m_modelRefs.size();
    m_modelRefs.push_back(&model);

    auto& instance = m_instanceDataPtr[instanceId];
    instance.Transform = transform;
    instance.Albedo = albedo;
    instance.Emission = emission;

    D3D12_RAYTRACING_INSTANCE_DESC* buildData;
    m_tlasBuildData->Map(0, nullptr, (void**)&buildData);

    const auto transposedTransform = DirectX::XMMatrixTranspose(transform);

    auto& instanceBuildData = buildData[instanceId];
    instanceBuildData.AccelerationStructure = model.GetBLAS()->GetGPUVirtualAddress();
    instanceBuildData.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
    instanceBuildData.InstanceContributionToHitGroupIndex = 0;
    instanceBuildData.InstanceID = instanceId;
    instanceBuildData.InstanceMask = 0xFF;
    std::memcpy(instanceBuildData.Transform, &transposedTransform, sizeof(instanceBuildData.Transform));

    D3D12_RANGE writeRange = {instanceId * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), (instanceId + 1) * sizeof(D3D12_RAYTRACING_INSTANCE_DESC) };
    m_tlasBuildData->Unmap(0, &writeRange);

    m_transformsDirty = true;
    m_instanceDataDirty = true;

    return instanceId;
}