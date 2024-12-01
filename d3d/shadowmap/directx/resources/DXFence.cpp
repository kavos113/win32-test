#include "DXFence.h"

#include "DXCommand.h"

ID3D12Fence* DXFence::m_fence = nullptr;
int DXFence::m_fenceValue = 0;

HRESULT DXFence::WaitFence()
{
    HRESULT hr = DXCommand::GetCommandQueue()->Signal(m_fence, ++m_fenceValue);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to signal fence\n"));
        return hr;
    }

    if (m_fence->GetCompletedValue() != static_cast<UINT64>(m_fenceValue))
    {
        HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        hr = m_fence->SetEventOnCompletion(m_fenceValue, event);
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to set event on completion\n"));
            return hr;
        }
        WaitForSingleObjectEx(event, INFINITE, false);
        CloseHandle(event);
    }

    return S_OK;
}