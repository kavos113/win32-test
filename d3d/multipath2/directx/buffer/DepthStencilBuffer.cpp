#include "DepthStencilBuffer.h"

#include <tchar.h>

#include "directx/resources/DXDevice.h"

HRESULT DepthStencilBuffer::CreateBuffer()
{
    D3D12_RESOURCE_DESC resource_desc;

    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Width = wr.right - wr.left;
    resource_desc.Height = wr.bottom - wr.top;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.Format = DXGI_FORMAT_D32_FLOAT;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.MipLevels = 1;
    resource_desc.Alignment = 0;

    D3D12_HEAP_PROPERTIES heap_properties = {};

    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clear_value = {};

    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil.Depth = 1.0f;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clear_value,
        IID_PPV_ARGS(&m_buffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create depth buffer\n"));
        return hr;
    }

    return S_OK;
}

void DepthStencilBuffer::CreateView()
{
    m_segment = m_dsvManager.Allocate(1);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    DXDevice::GetDevice()->CreateDepthStencilView(
        m_buffer,
        &dsvDesc,
        m_segment.GetCPUHandle()
    );
}