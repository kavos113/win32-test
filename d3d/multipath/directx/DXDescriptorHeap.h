#pragma once

#include <d3d12.h>
#include <Windows.h>

class DXDescriptorHeap
{
public:
    HRESULT CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* desc);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;
    UINT GetIncrementSize() const;
    void SetToCommand() const;

    DXDescriptorHeap()
        :
        m_descriptorHeap(nullptr)
    {
    }

private:
    ID3D12DescriptorHeap* m_descriptorHeap;
};

