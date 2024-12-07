#pragma once
#include <memory>

#include "DXBuffer.h"
#include "directx/GlobalDescriptorHeap1.h"
#include "directx/resources/DXDevice.h"

// vertexbuffer, indexbuffer などのサブクラスを作ってもいいかも?
template <typename T>
class ConstantBuffer :
    public DXBuffer
{
public:

    void SetResourceWidth(UINT64 width)
    {
        resource_width_ = width;
    }

    T* GetMappedBuffer()
    {
        return mapped_buffer_;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return m_buffer->GetGPUVirtualAddress();
    }

    void SetGlobalHeap(const std::shared_ptr<GlobalDescriptorHeap1>& globalHeap)
    {
        this->globalHeap = globalHeap;
    }

    void SetSegment(GLOBAL_HEAP_ID id)
    {
        heap_id_ = id;
    }

    ConstantBuffer()
        :
        mapped_buffer_(nullptr),
        resource_width_(sizeof(T)),
        heap_id_(-1)
    {
        
    }

    HRESULT CreateBuffer() override
    {
        D3D12_HEAP_PROPERTIES heap_properties = {};

        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resource_desc = {};

        resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Width = resource_width_;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.SampleDesc.Count = 1;
        resource_desc.SampleDesc.Quality = 0;
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_buffer)
        );
        if (FAILED(hr))
        {
            OutputDebugString(L"Failed to create constant buffer resource\n");
            return hr;
        }

        hr = m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped_buffer_));
        if (FAILED(hr))
        {
            OutputDebugString(L"Failed to map constant buffer resource\n");
            return hr;
        }

        return S_OK;
    }

    void CreateView() override
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};

        cbv_desc.BufferLocation = m_buffer->GetGPUVirtualAddress();
        cbv_desc.SizeInBytes = static_cast<UINT>(m_buffer->GetDesc().Width);

        DXDevice::GetDevice()->CreateConstantBufferView(
            &cbv_desc,
            globalHeap->GetCPUHandle(heap_id_)
        );
    }

    void UmmapBuffer() const
    {
        m_buffer->Unmap(0, nullptr);
    }

private:
    T* mapped_buffer_;

    UINT64 resource_width_;
    std::shared_ptr<GlobalDescriptorHeap1> globalHeap;
    GLOBAL_HEAP_ID heap_id_;
};

