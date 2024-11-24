#include "DXFence.h"

#include "resources/DXDevice.h"

HRESULT DXFence::Init()
{
    HRESULT hr = S_OK;

    hr = DXDevice::GetDevice()->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}

HRESULT DXFence::Signal()
{
}

HRESULT DXFence::Wait()
{
}

HRESULT DXFence::Flush()
{
}
