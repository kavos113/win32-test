#include "DXDescriptorHeap.h"

#include "resources/DXDevice.h"

HRESULT DXDescriptorHeap::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC desc)
{
    HRESULT hr = DXDevice::GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap));
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create descriptor heap\n");
        return hr;
    }

    return S_OK;
}
