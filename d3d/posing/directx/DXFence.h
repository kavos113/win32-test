#pragma once
#include <d3d12.h>

#include "DXDevice.h"

#pragma comment(lib, "d3d12.lib")

class DXFence
{
public:
    static ID3D12Fence* GetFence()
    {
        if (m_fence == nullptr)
        {
            if (CreateFence() != 0)
            {
                return nullptr;
            }
        }
        return m_fence;
    }

    static int GetFenceValue()
    {
        return m_fenceValue;
    }

    static int GetIncrementedFenceValue()
    {
        return ++m_fenceValue;
    }

    static void SetFenceValue(int value)
    {
        m_fenceValue = value;
    }

    static void ReleaseFence()
    {
        SafeRelease(&m_fence);
    }
private:
    static ID3D12Fence* m_fence;
    static int m_fenceValue;

    static int CreateFence()
    {
        HRESULT hr = DXDevice::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr))
        {
            return -1;
        }
        return 0;
    }
};

