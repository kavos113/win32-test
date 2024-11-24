#include "Display.h"

#include "resources/DXCommand.h"
#include "resources/DXDevice.h"
#include "resources/DXFactory.h"

HRESULT Display::Init()
{
    HRESULT hr = CreateSwapChain();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CreateRTV();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_depthStencilBuffer.Create();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_fence.Init();
    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}

HRESULT Display::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

    swapchainDesc.Width = wr.right - wr.left;
    swapchainDesc.Height = wr.bottom - wr.top;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = FALSE;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = DXFactory::GetDXGIFactory()->CreateSwapChainForHwnd(
        DXCommand::GetCommandQueue(),
        m_hwnd,
        &swapchainDesc,
        nullptr,
        nullptr,
        &m_swapChain
    );
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create swap chain\n");
        return hr;
    }

    return S_OK;    
}

HRESULT Display::CreateRTV()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    HRESULT hr = m_rtvHeap.CreateDescriptorHeap(rtvHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create RTV descriptor heap\n");
        return hr;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to get swap chain description\n");
        return hr;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    m_backBuffers.resize(swapChainDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.GetCPUHandle();
    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
        if (FAILED(hr))
        {
            OutputDebugString(L"Failed to get back buffer\n");
            return hr;
        }

        DXDevice::GetDevice()->CreateRenderTargetView(
            m_backBuffers[i],
            &rtvDesc,
            rtvHandle
        );

        rtvHandle.ptr += m_rtvHeap.GetIncrementSize();
    }

    return S_OK;
}

void Display::SetViewport()
{
    m_viewport.Width = wr.right - wr.left;
    m_viewport.Height = wr.bottom - wr.top;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = m_scissorRect.left + (wr.right - wr.left);
    m_scissorRect.bottom = m_scissorRect.top + (wr.bottom - wr.top);
}
