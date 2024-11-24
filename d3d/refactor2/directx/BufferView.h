#pragma once
#include <d3d12.h>
#include <memory>

#include "DXDescriptorHeap.h"

class BufferView
{
public:
    BufferView()
        :
        m_buffer_(nullptr),
        m_descriptor_heap_(nullptr)
    {}

    virtual HRESULT Create() = 0;

    ID3D12Resource* GetBuffer() const
    {
        return m_buffer_;
    }

    ID3D12DescriptorHeap* GetDescriptorHeap() const
    {
        return m_descriptor_heap_->GetDescriptorHeap();
    }

    virtual ~BufferView()
    {
        if (m_buffer_)
        {
            m_buffer_->Release();
        }
    }

protected:
    ID3D12Resource* m_buffer_;
    std::shared_ptr<DXDescriptorHeap> m_descriptor_heap_;
};

