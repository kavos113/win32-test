#pragma once
#include <d3d12.h>

#include "directx/DXUtil.h"

class DXCommand
{
public:
    static ID3D12CommandAllocator* GetCommandAllocator()
    {
        if (m_commandAllocator == nullptr)
        {
            if (CreateCommandAllocator() != 0)
            {
                return nullptr;
            }
        }
        return m_commandAllocator;
    }

    static ID3D12CommandQueue* GetCommandQueue()
    {
        if (m_commandQueue == nullptr)
        {
            if (CreateCommandQueue() != 0)
            {
                return nullptr;
            }
        }
        return m_commandQueue;
    }

    static ID3D12GraphicsCommandList* GetCommandList()
    {
        if (m_commandList == nullptr)
        {
            if (m_commandAllocator == nullptr)
            {
                if (CreateCommandAllocator() != 0)
                {
                    return nullptr;
                }
            }

            if (CreateCommandList() != 0)
            {
                return nullptr;
            }
        }
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

private:
    static ID3D12CommandQueue* m_commandQueue;
    static ID3D12CommandAllocator* m_commandAllocator;
    static ID3D12GraphicsCommandList* m_commandList;

    static int CreateCommandQueue();
    static int CreateCommandAllocator();
    static int CreateCommandList();
};

