#pragma once
#include <dxgi1_6.h>
#include <vector>

#include "DepthStencilBuffer.h"
#include "DXDescriptorHeap.h"

class Display
{
public:
    HRESULT Init();
    void SetBeginBarrier();
    void RenderToBackBuffer();
    void SetEndBarrier();
    void Present() const;

    void SetHWND(HWND hwnd);

    Display(HWND hwnd, RECT wr)
        :
        m_swapChain(nullptr),
        m_depthStencilBuffer(wr),
        m_viewport(),
        m_scissorRect(),
        wr(wr),
        hwnd(hwnd),
        m_barrier()
    {

    }
private:
    HRESULT CreateSwapChain();
    HRESULT SetRenderTargetView();
    HRESULT SetDepthStencilView();

    HRESULT CreateViewPort();

    IDXGISwapChain4* m_swapChain;
    DXDescriptorHeap m_rtvHeap;
    std::vector<ID3D12Resource*> back_buffers_;

    DepthStencilBuffer m_depthStencilBuffer;
    DXDescriptorHeap m_dsvHeap;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    RECT wr;
    HWND hwnd;

    D3D12_RESOURCE_BARRIER m_barrier;
    UINT bbIdx;
};

