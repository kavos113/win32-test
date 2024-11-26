#include "DXProcess.h"

#include <chrono>
#include <DirectXMath.h>
#include <memory>
#include <ratio>
#include <tchar.h>

#include "DXCommand.h"
#include "DXDevice.h"
#include "DXFactory.h"
#include "DXFence.h"
#include "PMDModel.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

HRESULT DXProcess::Init()
{
    EnableDebug();

    DXFactory::Init();
    DXDevice::Init();
    DXCommand::Init();
    DXFence::Init();

    HRESULT hr = CreateSwapChain();
    if (FAILED(hr)) return E_FAIL;

    hr = SetRenderTargetView();
    if (FAILED(hr)) return E_FAIL;

    hr = SetDepthStencilView();
    if (FAILED(hr)) return E_FAIL;

    renderer = std::make_unique<PMDRenderer>(hwnd, wr);
    hr = renderer->Init();
    if (FAILED(hr)) return E_FAIL;

    hr = CreateViewPort();
    if (FAILED(hr)) return E_FAIL;

    hr = SetMatrixBuffer();
    if (FAILED(hr)) return E_FAIL;

    if (DXFactory::GetDXGIFactory() != nullptr)
    {
        OutputDebugString(_T("DXGI Factory is created\n"));
    }
    else
    {
        OutputDebugString(_T("DXGI Factory is not created\n"));
    }

    model = std::make_unique<PMDModel>("model/初音ミクmetal.pmd");
    model->Read();

    return S_OK;
}

void DXProcess::Render()
{
    HRESULT hr = OnRender();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to render\n"));
        return;
    }
}

void DXProcess::SetHWND(HWND hwnd)
{
    this->hwnd = hwnd;
}

void DXProcess::EnableDebug()
{
    ID3D12Debug* debugController = nullptr;
    HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (hr == S_OK)
    {
        debugController->EnableDebugLayer();
    }
    debugController->EnableDebugLayer();
    debugController->Release();
    OutputDebugString(_T("Debug layer is enabled\n"));
}

HRESULT DXProcess::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

    swapchainDesc.Width = wr.right - wr.left;
    swapchainDesc.Height = wr.bottom - wr.top;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = FALSE;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = DXFactory::GetDXGIFactory()->CreateSwapChainForHwnd(
        DXCommand::GetCommandQueue(),
        hwnd,
        &swapchainDesc,
        nullptr,
        nullptr,
        (IDXGISwapChain1**)&m_swapChain
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("failed to create swap chain\n"));
    }

    return hr;
}

HRESULT DXProcess::SetRenderTargetView()
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
    for (int i = 0; i < swapChainDesc.BufferCount; ++i)
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

HRESULT DXProcess::SetDepthStencilView()
{
    D3D12_RESOURCE_DESC depthResourceDesc = {};

    depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResourceDesc.Width = wr.right - wr.left;
    depthResourceDesc.Height = wr.bottom - wr.top;
    depthResourceDesc.DepthOrArraySize = 1;
    depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthResourceDesc.SampleDesc.Count = 1;
    depthResourceDesc.SampleDesc.Quality = 0;
    depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthResourceDesc.MipLevels = 1;
    depthResourceDesc.Alignment = 0;

    D3D12_HEAP_PROPERTIES heapProperties = {};

    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clearValue = {};

    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    );
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

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    DXDevice::GetDevice()->CreateDepthStencilView(
        m_depthStencilBuffer,
        &dsvDesc,
        m_dsvHeap.GetCPUHandle()
    );

    return S_OK;
}

HRESULT DXProcess::CreateViewPort()
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

    return S_OK;
}

HRESULT DXProcess::SetMatrixBuffer()
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    DirectX::XMFLOAT3 eye(0.0f, 15.0f, -15.0f);
    DirectX::XMFLOAT3 target(0.0f, 10.0f, 0.0f);
    DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&eye),
        DirectX::XMLoadFloat3(&target),
        DirectX::XMLoadFloat3(&up)
    );

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV2,
        static_cast<float>(wr.right - wr.left) / static_cast<float>((wr.bottom - wr.top)),
        1.0f,
        100.0f
    );

    D3D12_HEAP_PROPERTIES constantBufferHeapProperties = {};

    constantBufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    constantBufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    constantBufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    constantBufferHeapProperties.CreationNodeMask = 0;
    constantBufferHeapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC constantBufferResourceDesc = {};

    constantBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    constantBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    constantBufferResourceDesc.Width = (sizeof(SceneMatrix) + 0xff) & ~0xff;  // ~0xff: 256バイト以下が0
    constantBufferResourceDesc.Height = 1;
    constantBufferResourceDesc.DepthOrArraySize = 1;
    constantBufferResourceDesc.MipLevels = 1;
    constantBufferResourceDesc.SampleDesc.Count = 1;
    constantBufferResourceDesc.SampleDesc.Quality = 0;
    constantBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* constantBuffer = nullptr;

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &constantBufferHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create constant buffer\n"));
        return hr;
    }

    hr = constantBuffer->Map(0, nullptr, (void**)&m_constantBufferMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return hr;
    }

    m_constantBufferMap->world = worldMatrix;
    m_constantBufferMap->view = viewMatrix;
    m_constantBufferMap->proj = projectionMatrix;
    m_constantBufferMap->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;

    hr = m_cbvHeap.CreateDescriptorHeap(&descriptorHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

    cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constantBuffer->GetDesc().Width);

    D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_cbvHeap.GetCPUHandle();

    DXDevice::GetDevice()->CreateConstantBufferView(
        &cbvDesc,
        cbvHandle
    );

    return S_OK;
}
 
// 重く見えていたのはWM_PAINTメッセージが呼ばれておらずペイントされていなかっただけだった T_T
HRESULT DXProcess::OnRender()
{
    auto start = std::chrono::high_resolution_clock::now();

    angle += 0.05f;
    m_constantBufferMap->world = DirectX::XMMatrixRotationY(angle);

    auto bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER barrier = {};

    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = back_buffers_[bbIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    DXCommand::GetCommandList()->ResourceBarrier(1, &barrier);

    renderer->SetPipelineState();

    auto rtvHandle = m_rtvHeap.GetCPUHandle();
    rtvHandle.ptr += static_cast<ULONG_PTR>(bbIdx) * m_rtvHeap.GetIncrementSize();

    auto dsvHandle = m_dsvHeap.GetCPUHandle();
    DXCommand::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // draw polygon
    DXCommand::GetCommandList()->RSSetViewports(1, &m_viewport);
    DXCommand::GetCommandList()->RSSetScissorRects(1, &m_scissorRect);

    model->SetIA();

    renderer->SetRootSignature();

    m_cbvHeap.SetToCommand();
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0,
        m_cbvHeap.GetGPUHandle()
    );

    model->Render();

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    DXCommand::GetCommandList()->ResourceBarrier(1, &barrier);

    HRESULT hr = DXCommand::ExecuteCommands();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to execute commands\n"));
        return hr;
    }

    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to present\n"));
        return hr;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    OutputDebugStringA(std::to_string(elapsed.count()).c_str());
    OutputDebugStringA("ms\n");

    return S_OK;
}
