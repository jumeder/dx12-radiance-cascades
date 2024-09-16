#pragma once

#include "Device.h"

class Model
{
public:
    Model(const std::string& filepath, Device& device);

    void Draw(const ComPtr<ID3D12GraphicsCommandList>& commandList);

private:
    VertexBuffer m_vertexBuffer;
    VertexBuffer m_normalBuffer;
    IndexBuffer m_indexBuffer;
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;
};