#pragma once
#include <d3d12.h>

#include "directx/DXUtil.h"


class DXCommand
{
public:
    static void Init()
    {
        CreateCommandAllocator();
        CreateCommandQueue();
        CreateCommandList();
    }

    static ID3D12CommandAllocator* GetCommandAllocator()
    {
        return m_commandAllocator;
    }

    static ID3D12CommandQueue* GetCommandQueue()
    {
        return m_commandQueue;
    }

    static ID3D12GraphicsCommandList* GetCommandList()
    {
        return m_commandList;
    }

    static void ReleaseCommandAllocator()
    {
        SafeRelease(&m_commandAllocator);
    }

    static void ReleaseCommandQueue()
    {
        SafeRelease(&m_commandQueue);
    }

    static void ReleaseCommandList()
    {
        SafeRelease(&m_commandList);
    }

    static HRESULT ExecuteCommands();

private:
    static ID3D12CommandQueue* m_commandQueue;
    static ID3D12CommandAllocator* m_commandAllocator;
    static ID3D12GraphicsCommandList* m_commandList;

    static int CreateCommandQueue();
    static int CreateCommandAllocator();
    static int CreateCommandList();
};

