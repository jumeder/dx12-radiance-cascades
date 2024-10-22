#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <dxgi.h>
#include <d3d12.h>
#include <DirectXMath.h>

#define EXPR(a, b) a ## b
#define EXPR_EXPAND(a,b) EXPR(a, b)
#define DUMMY_CBV_ENTRY uint32_t EXPR_EXPAND(dummy, __LINE__)

using Microsoft::WRL::ComPtr;

inline D3D12_CPU_DESCRIPTOR_HANDLE operator+(D3D12_CPU_DESCRIPTOR_HANDLE left, uint32_t right)
{
    D3D12_CPU_DESCRIPTOR_HANDLE ret;
    ret.ptr = left.ptr + right;
    return ret;
}

inline D3D12_GPU_DESCRIPTOR_HANDLE operator+(D3D12_GPU_DESCRIPTOR_HANDLE left, uint32_t right)
{
    D3D12_GPU_DESCRIPTOR_HANDLE ret;
    ret.ptr = left.ptr + right;
    return ret;
}