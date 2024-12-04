#include "Display.h"

#include <tchar.h>

#include "DXCommand.h"
#include "DXDevice.h"
#include "DXFactory.h"

HRESULT Display::Init()
{
    HRESULT hr = CreateSwapChain();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create swap chain\n"));
        return hr;
    }

    hr = SetRenderTargetView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to set render target view\n"));
        return hr;
    }

    hr = SetDepthStencilView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to set depth stencil view\n"));
        return hr;
    }

    hr = CreateViewPort();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create view port\n"));
        return hr;
    }

    m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    m_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    return S_OK;
}

void Display::SetBeginBarrier()
{
    bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    m_barrier.Transition.pResource = back_buffers_[bbIdx];

    m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    DXCommand::GetCommandList()->ResourceBarrier(1, &m_barrier);
}

void Display::RenderToBackBuffer()
{
    auto rtvHandle = m_rtvHeap.GetCPUHandle();
    rtvHandle.ptr += static_cast<ULONG_PTR>(bbIdx) * m_rtvHeap.GetIncrementSize();

    auto dsvHandle = m_dsvHeap.GetCPUHandle();
    DXCommand::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    DXCommand::GetCommandList()->RSSetViewports(1, &m_viewport);
    DXCommand::GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
}

void Display::SetEndBarrier()
{
    m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    DXCommand::GetCommandList()->ResourceBarrier(1, &m_barrier);
}

void Display::Present() const
{
   HRESULT hr = m_swapChain->Present(1, 0);
   if (FAILED(hr))
   {
       OutputDebugString(_T("Failed to present\n"));
   }
}


void Display::SetHWND(HWND hwnd)
{
    this->hwnd = hwnd;
}

HRESULT Display::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};

    desc.Width = wr.right - wr.left;
    desc.Height = wr.bottom - wr.top;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = DXFactory::GetDXGIFactory()->CreateSwapChainForHwnd(
        DXCommand::GetCommandQueue(),
        hwnd,
        &desc,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(&m_swapChain)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("failed to create swap chain\n"));
    }

    return hr;
}


HRESULT Display::SetRenderTargetView()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    HRESULT hr = m_rtvHeap.CreateDescriptorHeap(&rtvHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create RTV descriptor heap\n"));
        return hr;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to get swap chain description\n"));
        return hr;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    back_buffers_.resize(swapChainDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.GetCPUHandle();
    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        hr = m_swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&back_buffers_[i]));
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to get buffer from swap chain\n"));
            return hr;
        }

        rtvDesc.Format = back_buffers_[i]->GetDesc().Format;

        DXDevice::GetDevice()->CreateRenderTargetView(
            back_buffers_[i],
            &rtvDesc,
            rtvHandle
        );

        rtvHandle.ptr += m_rtvHeap.GetIncrementSize();
    }

    return S_OK;
}

HRESULT Display::SetDepthStencilView()
{
    HRESULT hr = m_depthStencilBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create depth buffer\n"));
        return hr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};

    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    hr = m_dsvHeap.CreateDescriptorHeap(&dsvHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create DSV descriptor heap\n"));
        return hr;
    }

    m_depthStencilBuffer.SetDescriptorHeap(&m_dsvHeap);
    m_depthStencilBuffer.CreateView();

    return S_OK;
}

HRESULT Display::CreateViewPort()
{
    m_viewport.Width = static_cast<float>(wr.right - wr.left); 
    m_viewport.Height = static_cast<float>(wr.bottom - wr.top); 
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = m_scissorRect.left + (wr.right - wr.left);
    m_scissorRect.bottom = m_scissorRect.top + (wr.bottom - wr.top);

    return S_OK;
}
