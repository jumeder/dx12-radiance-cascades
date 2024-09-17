#include "Model.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

Model::Model(const std::string& filepath, Device& device)
{
    Assimp::Importer importer;
    importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenNormals);
    const auto scene = importer.GetScene();

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<uint32_t> indices;
    uint32_t indexOffset = 0;
    for(auto meshIndex = 0u; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        const auto& mesh = scene->mMeshes[meshIndex];
        vertices.insert(vertices.end(), (float*)mesh->mVertices, (float*)(mesh->mVertices + mesh->mNumVertices));
        normals.insert(normals.end(), (float*)mesh->mNormals, (float*)(mesh->mNormals + mesh->mNumVertices));

        for(auto faceIndex = 0u; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            const auto& face = mesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3);

            indices.push_back(face.mIndices[0] + indexOffset);
            indices.push_back(face.mIndices[1] + indexOffset);
            indices.push_back(face.mIndices[2] + indexOffset);
        }

        indexOffset = (uint32_t)vertices.size() / 3;
    }

    m_vertexBuffer = device.CreateVertexBuffer(vertices);
    m_normalBuffer = device.CreateVertexBuffer(normals);
    m_indexBuffer = device.CreateIndexBuffer(indices);
    m_vertexCount = (uint32_t)vertices.size() / 3;
    m_indexCount = (uint32_t)indices.size();

    m_blas = device.CreateBottomLevelAccelerationStructure(m_vertexBuffer, m_indexBuffer, m_vertexCount, m_indexCount);
}

void Model::Draw(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    std::array vertexBufferViews = {m_vertexBuffer.View, m_normalBuffer.View};
    commandList->IASetVertexBuffers(0, (UINT)vertexBufferViews.size(), vertexBufferViews.data());
    commandList->IASetIndexBuffer(&m_indexBuffer.View);
    commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}