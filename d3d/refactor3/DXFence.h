#pragma once
#include <d3d12.h>

#include "DXDevice.h"

#pragma comment(lib, "d3d12.lib")

class DXFence
{
public:
    static void Init()
    {
        CreateFence();
    }

    static ID3D12Fence* GetFence()
    {
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

    static HRESULT WaitFence();
private:
    static ID3D12Fence* m_fence;
    static int m_fenceValue;

    static int CreateFence()
    {
        HRESULT hr = DXDevice::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to create fence\n"));
            return -1;
        }
        return 0;
    }
};

