#pragma once
#include <d3d12.h>

#include "resources/DXDevice.h"

class DXDescriptorHeap
{
public:
    HRESULT CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC desc);

    ID3D12DescriptorHeap* GetDescriptorHeap() const
    {
        return m_descriptorHeap;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType() const
    {
        return m_descriptorHeap->GetDesc().Type;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const
    {
        return m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    UINT GetIncrementSize() const
    {
        return DXDevice::GetDevice()->GetDescriptorHandleIncrementSize(m_descriptorHeap->GetDesc().Type);
    }

    DXDescriptorHeap() : m_descriptorHeap(nullptr) {}

private:
    ID3D12DescriptorHeap* m_descriptorHeap;
};

