#include "DXCommand.h"

#include <tchar.h>

#include "DXDevice.h"

ID3D12CommandQueue* DXCommand::m_commandQueue = nullptr;
ID3D12CommandAllocator* DXCommand::m_commandAllocator = nullptr;
ID3D12GraphicsCommandList* DXCommand::m_commandList = nullptr;

int DXCommand::CreateCommandAllocator()
{
    HRESULT hr = DXDevice::GetDevice()->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_commandAllocator)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command allocator\n"));
        return -1;
    }

    return 0;
}

int DXCommand::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    HRESULT hr = DXDevice::GetDevice()->CreateCommandQueue(
        &desc,
        IID_PPV_ARGS(&m_commandQueue)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command queue\n"));
        return -1;
    }

    return 0;
}

int DXCommand::CreateCommandList()
{
    HRESULT hr = DXDevice::GetDevice()->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create command list\n"));
        return -1;
    }

    return 0;
}