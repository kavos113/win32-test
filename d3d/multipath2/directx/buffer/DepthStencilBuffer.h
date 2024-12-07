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
        m_dsvManager(GlobalDescriptorHeapManager::GetCpuHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)),
        m_segment(),
        wr(wr)
    {
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const
    {
        return m_segment.GetCPUHandle();
    }

private:
    DescriptorHeapSegmentManager& m_dsvManager;
    DescriptorHeapSegment m_segment;

    RECT wr;
};

