#pragma once
#include "DXBuffer.h"

#include "directx/descriptor_heap/DescriptorHeapSegment.h"
#include "directx/descriptor_heap/DescriptorHeapSegmentManager.h"
#include "directx/descriptor_heap/GlobalDescriptorHeapManager.h"

class DepthStencilBuffer :
    public DXBuffer
{
public:
    HRESULT CreateBuffer() override;
    void CreateView() override;

    DepthStencilBuffer(RECT wr)
        :
        m_dsvManager(),
        m_segment(),
        m_width(wr.right - wr.left),
        m_height(wr.bottom - wr.top)
    {
        m_dsvManager = &GlobalDescriptorHeapManager::GetCPUHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    DepthStencilBuffer(UINT64 width, UINT64 height)
        :
        m_dsvManager(),
        m_segment(),
        m_width(width),
        m_height(height)
    {
        m_dsvManager = &GlobalDescriptorHeapManager::GetCPUHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const
    {
        return m_segment.GetCPUHandle();
    }

private:
    DescriptorHeapSegmentManager* m_dsvManager;
    DescriptorHeapSegment m_segment;

    UINT64 m_width;
    UINT64 m_height;
};

