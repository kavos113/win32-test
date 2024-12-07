#include "DXDescriptorHeap.h"

#include "resources/DXCommand.h"
#include "resources/DXDevice.h"

HRESULT DXDescriptorHeap::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* desc)
{
    HRESULT hr = DXDevice::GetDevice()->CreateDescriptorHeap(
        desc, 
        IID_PPV_ARGS(&m_descriptorHeap)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    return S_OK;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXDescriptorHeap::GetCPUHandle() const
{
    return m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

}

D3D12_GPU_DESCRIPTOR_HANDLE DXDescriptorHeap::GetGPUHandle() const
{
    return m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

UINT DXDescriptorHeap::GetIncrementSize() const
{
    return DXDevice::GetDevice()->GetDescriptorHandleIncrementSize(m_descriptorHeap->GetDesc().Type);
}

void DXDescriptorHeap::SetToCommand() const
{
    DXCommand::GetCommandList()->SetDescriptorHeaps(1, &m_descriptorHeap);
}
