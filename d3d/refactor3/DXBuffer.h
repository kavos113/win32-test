#pragma once

#include <Windows.h>

#include "DXDescriptorHeap.h"

class DXBuffer
{
public:
    virtual HRESULT CreateBuffer() = 0;
    virtual void CreateView() = 0;
    void SetDescriptorHeap(DXDescriptorHeap* heap)
    {
        m_descriptorHeap = heap;
    }

    DXBuffer()
        :
        m_descriptorHeap(nullptr),
        m_buffer(nullptr)
    {
    }

    virtual ~DXBuffer() = default;

protected:
    DXDescriptorHeap* m_descriptorHeap;

    ID3D12Resource* m_buffer;
};

