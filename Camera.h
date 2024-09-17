#pragma once

#include "Shared.h"

class Camera
{
public:
    inline Camera(float fov, float aspect, float n)
        : m_pos(DirectX::XMVectorZero())
        , m_pitch(0.f)
        , m_yaw(0.f)
        , m_roll(0.f)
    {
        DirectX::XMMATRIX mat = {
            fov, 0.f, 0.f, 0.f,
            0.f, fov * aspect, 0.f, 0.f,
            0.f, 0.f, 0.f, n,
            0.f, 0.f, -1.f, 0.f
        };
        m_projection = DirectX::XMMatrixTranspose(mat);
    }

    inline void SetPosition(float x, float y, float z)
    {
        m_pos = {x, y, z, 0.f};
    }

    inline void MoveLocal(float x, float y, float z)
    {
        DirectX::XMVECTOR dir = {x, y, z, 0.f};
        dir = DirectX::XMVector4Transform(dir, GetRotation());
        m_pos = DirectX::XMVectorAdd(m_pos, dir);
    }

    inline void Pitch(float angle)
    {
        m_pitch += angle;
    }

    inline void Yaw(float angle)
    {
        m_yaw += angle;
    }

    inline void Roll(float angle)
    {
        m_roll += angle;
    }

    inline DirectX::XMMATRIX GetRotation() const
    {
        return DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);
    }

    inline DirectX::XMMATRIX GetTranslation() const
    {
        return DirectX::XMMatrixTranslation(m_pos.m128_f32[0], m_pos.m128_f32[1], m_pos.m128_f32[2]);
    }

    inline DirectX::XMMATRIX GetTransform() const
    {
        const auto rotation = GetRotation();
        const auto translation = GetTranslation();
        return DirectX::XMMatrixMultiply(rotation, translation);
    }

    inline DirectX::XMMATRIX GetViewProjection() const
    {       
        return DirectX::XMMatrixMultiply(DirectX::XMMatrixInverse(nullptr, GetTransform()), m_projection);
    }

private:
    DirectX::XMMATRIX m_projection;
    DirectX::XMVECTOR m_pos;
    float m_pitch;
    float m_yaw;
    float m_roll;
};