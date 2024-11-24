#pragma once
#include <d3d12.h>
#include <Windows.h>
class DXFence
{
public:
    DXFence()
        :
        m_fence(nullptr),
        m_fenceValue(0)
    {
    }

    HRESULT Init();
    HRESULT Signal();
    HRESULT Wait();
    HRESULT Flush();

private:
    ID3D12Fence* m_fence;
    UINT64 m_fenceValue;
};

