#pragma once

#include "DescriptorHeapSegmentManager.h"
#include "../DXDescriptorHeap.h"

// shader invisible
class GlobalDescriptorHeap
{
public:
    virtual void Init(D3D12_DESCRIPTOR_HEAP_TYPE type);
    GlobalDescriptorHeap();

    DescriptorHeapSegmentManager& GetHeapManager() { return m_heapManager; }

protected:
    DXDescriptorHeap m_heap;
    UINT m_incrementSize;

    D3D12_CPU_DESCRIPTOR_HANDLE m_lastCpuHandle;

private:
    DescriptorHeapSegmentManager m_heapManager;

    constexpr static unsigned int kMaxDescriptorHeapSize = 65536;
};
