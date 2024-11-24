#pragma once
#include <d3d12.h>
#include <dxgi1_2.h>
#include <vector>

#include "DepthStencilBuffer.h"
#include "DXDescriptorHeap.h"
#include "DXFence.h"

class Display
{
public:
    Display(RECT wr, HWND hwnd)
        :
        wr(wr),
        m_hwnd(hwnd),
        m_swapChain(nullptr),
        m_viewport(),
        m_scissorRect(),
        m_depthStencilBuffer(wr)
    {
    }

    HRESULT Init();

private:
    RECT wr;
    HWND m_hwnd;

    IDXGISwapChain1* m_swapChain;
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    std::vector<ID3D12Resource*> m_backBuffers;

    DXDescriptorHeap m_rtvHeap;
    DepthStencilBuffer m_depthStencilBuffer;
    DXFence m_fence;

    HRESULT CreateSwapChain();
    HRESULT CreateRTV();
    void SetViewport();
};

