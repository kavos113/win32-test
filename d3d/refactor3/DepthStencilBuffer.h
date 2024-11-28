#pragma once
#include "DXBuffer.h"
#include "DXDescriptorHeap.h"

class DepthStencilBuffer :
    public DXBuffer
{
public:
    HRESULT CreateBuffer() override;
    void CreateView() override;
    void SetDescriptorHeap(DXDescriptorHeap* heap);

    DepthStencilBuffer(RECT wr)
        :
        m_heap(nullptr),
        wr(wr)
    {
    }

private:
    DXDescriptorHeap* m_heap;

    RECT wr;
};

