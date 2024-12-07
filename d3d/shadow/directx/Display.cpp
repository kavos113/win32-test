#include "Display.h"

#include <d3dcompiler.h>
#include <tchar.h>

#include "Util.h"
#include "resources/DXCommand.h"
#include "resources/DXDevice.h"
#include "resources/DXFactory.h"


HRESULT Display::Init(const std::shared_ptr<GlobalDescriptorHeap1>& globalHeap)
{
    this->globalHeap = globalHeap;

    HRESULT hr = CreateSwapChain();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create swap chain\n"));
        return hr;
    }

    hr = CreateBackBuffers();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create back buffers\n"));
        return hr;
    }

    hr = CreateRenderResource();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create render resource\n"));
        return hr;
    }

    hr = SetRenderTargetView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to set render target view\n"));
        return hr;
    }

    hr = SetBaseRenderTargetView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to set render target view\n"));
        return hr;
    }

    hr = CreateShaderResourceView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create shader resource view\n"));
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

    hr = CreateBasePolygon();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create base polygon\n"));
        return hr;
    }

    hr = CreateBlurBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create blur buffer\n"));
        return hr;
    }

    hr = CreateBasePipeline();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create base pipeline\n"));
        return hr;
    }

    m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    m_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    return S_OK;
}

// base polygon2���� + blur(by pso2)��back buffer�ɕ`��
void Display::RenderToBackBuffer() const
{
    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);

    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = globalHeap->GetGPUHandle(m_srvHeapId);
    srvHandle.ptr += globalHeap->GetIncrementSize();
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle);
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(1, globalHeap->GetGPUHandle(blur_weight_heap_id_));

    DXCommand::GetCommandList()->SetPipelineState(m_pipelineState2);

    DXCommand::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    DXCommand::GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DXCommand::GetCommandList()->DrawInstanced(4, 1, 0, 0);
}

// base polygon1���ڂɏ������ނ悤�ݒ�
void Display::SetRenderToBase1Begin()
{
    Barrier(
        m_renderResource,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    auto baseRtvHandle = m_baseRtvHeap.GetCPUHandle();
    auto dsvHandle = m_dsvHeap.GetCPUHandle();

    DXCommand::GetCommandList()->OMSetRenderTargets(
        1,
        &baseRtvHandle,
        false,
        &dsvHandle
    );

    float clearColor[] = { 0.7f, 0.8f, 0.6f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(baseRtvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Display::Present() const
{
   HRESULT hr = m_swapChain->Present(1, 0);
   if (FAILED(hr))
   {
       OutputDebugString(_T("Failed to present\n"));
   }
}

void Display::SetViewports() const
{
    DXCommand::GetCommandList()->RSSetViewports(1, &m_viewport);
    DXCommand::GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
}

void Display::SetRenderToBase1End()
{
    Barrier(
        m_renderResource,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
}

// base polygon2���ڂɁC1���ڂ�base polygon + blur(by pso1)��`��
void Display::RenderToBase2()
{
    Barrier(
        m_renderResource2,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    D3D12_CPU_DESCRIPTOR_HANDLE baseRtvHandle = m_baseRtvHeap.GetCPUHandle();
    baseRtvHandle.ptr += m_baseRtvHeap.GetIncrementSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap.GetCPUHandle();

    DXCommand::GetCommandList()->OMSetRenderTargets(1, &baseRtvHandle, false, &dsvHandle);

    float clearColor[] = { 0.7f, 0.8f, 0.6f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(baseRtvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);

    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0,
        globalHeap->GetGPUHandle(m_srvHeapId)
    );
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        1,
        globalHeap->GetGPUHandle(blur_weight_heap_id_)
    );

    DXCommand::GetCommandList()->SetPipelineState(m_pipelineState);

    DXCommand::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    DXCommand::GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DXCommand::GetCommandList()->DrawInstanced(4, 1, 0, 0);

    Barrier(
        m_renderResource2,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
}

// back buffer�ɕ`�悷��悤�ݒ�
void Display::SetRenderToBackBuffer()
{
    UINT bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    Barrier(
        back_buffers_[bbIdx],
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.GetCPUHandle();
    rtvHandle.ptr += bbIdx * m_rtvHeap.GetIncrementSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap.GetCPUHandle();

    DXCommand::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    float clearColor[] = { 0.7f, 0.8f, 0.6f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Display::EndRender()
{
    UINT bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    Barrier(
        back_buffers_[bbIdx],
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
}

void Display::Barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    m_barrier.Transition.pResource = resource;
    m_barrier.Transition.StateBefore = before;
    m_barrier.Transition.StateAfter = after;

    DXCommand::GetCommandList()->ResourceBarrier(1, &m_barrier);
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

HRESULT Display::CreateBackBuffers()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    HRESULT hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to get swap chain description\n"));
        return hr;
    }

    back_buffers_.resize(swapChainDesc.BufferCount);

    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&back_buffers_[i]));
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to get buffer from swap chain\n"));
            return hr;
        }
    }

    return S_OK;
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

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.GetCPUHandle();
    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
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

HRESULT Display::CreateRenderResource()
{
    ID3D12Resource*& back_buffer = back_buffers_[0];
    D3D12_RESOURCE_DESC desc = back_buffer->GetDesc();

    D3D12_HEAP_PROPERTIES heapProp = {};

    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProp.CreationNodeMask = 0;

    float clearColor[] = { 0.7f, 0.8f, 0.6f, 1.0f };

    D3D12_CLEAR_VALUE clearValue = {};

    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    for (int i = 0; i < 4; ++i)
    {
        clearValue.Color[i] = clearColor[i];
    }

    HRESULT hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue,
        IID_PPV_ARGS(&m_renderResource)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create render resource\n"));
        return hr;
    }

    hr = DXDevice::GetDevice()->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue,
        IID_PPV_ARGS(&m_renderResource2)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create render resource\n"));
        return hr;
    }

    return S_OK;
}

HRESULT Display::SetBaseRenderTargetView()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    HRESULT hr = m_baseRtvHeap.CreateDescriptorHeap(&rtvHeapDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create RTV descriptor heap\n"));
        return hr;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_baseRtvHeap.GetCPUHandle();

    DXDevice::GetDevice()->CreateRenderTargetView(
        m_renderResource,
        &rtvDesc,
        rtvHandle
    );

    rtvHandle.ptr += m_baseRtvHeap.GetIncrementSize();
    DXDevice::GetDevice()->CreateRenderTargetView(
        m_renderResource2,
        &rtvDesc,
        rtvHandle
    );

    return S_OK;
}

HRESULT Display::CreateShaderResourceView()
{
    m_srvHeapId = globalHeap->Allocate(2);
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = globalHeap->GetCPUHandle(m_srvHeapId);
    
    DXDevice::GetDevice()->CreateShaderResourceView(
        m_renderResource,
        &srvDesc,
        srvHandle
    );

    srvHandle.ptr += globalHeap->GetIncrementSize();

    DXDevice::GetDevice()->CreateShaderResourceView(
        m_renderResource2,
        &srvDesc,
        srvHandle
    );

    return S_OK;
}

HRESULT Display::CreateBasePolygon()
{
    BaseVertex vertices[4] = { {{-1, -1, 0.1}, {0, 1}},
                                {{-1,  1, 0.1}, {0, 0}},
                                {{ 1, -1, 0.1}, {1, 1}},
                                {{ 1,  1, 0.1}, {1, 0}} };

    m_vertexBuffer.SetResourceWidth(sizeof(vertices));
    HRESULT hr = m_vertexBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create vertex buffer\n"));
        return hr;
    }

    std::ranges::copy(vertices, m_vertexBuffer.GetMappedBuffer());

    m_vertexBuffer.UmmapBuffer();

    m_vertexBufferView.BufferLocation = m_vertexBuffer.GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(vertices);
    m_vertexBufferView.StrideInBytes = sizeof(BaseVertex);

    return S_OK;
}

HRESULT Display::CreateBasePipeline()
{
    D3D12_INPUT_ELEMENT_DESC layout[2] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"shaders/BaseVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile vertex shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File not found\n"));
            return E_FAIL;
        }

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return E_FAIL;
    }

    hr = D3DCompileFromFile(
        L"shaders/BasePixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        //"ps",
        "normal",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile pixel shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File not found\n"));
            return E_FAIL;
        }

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return E_FAIL;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.InputLayout.NumElements = _countof(layout);
    psoDesc.InputLayout.pInputElementDescs = layout;

    psoDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
    psoDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
    psoDesc.PS.BytecodeLength = psBlob->GetBufferSize();

    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};

    rtvBlendDesc.BlendEnable = FALSE;
    rtvBlendDesc.LogicOpEnable = FALSE;
    rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    psoDesc.BlendState.RenderTarget[0] = rtvBlendDesc;

    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    D3D12_DESCRIPTOR_RANGE range[2] = {};

    range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range[0].NumDescriptors = 1;
    range[0].BaseShaderRegister = 0;
    range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range[1].NumDescriptors = 1;
    range[1].BaseShaderRegister = 0;

    D3D12_ROOT_PARAMETER rootParameter[2] = {};

    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameter[0].DescriptorTable.pDescriptorRanges = &range[0];
    rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameter[1].DescriptorTable.pDescriptorRanges = &range[1];

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    rootSignatureDesc.NumParameters = 2;
    rootSignatureDesc.pParameters = rootParameter;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* signatureBlob = nullptr;
    hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        &signatureBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to serialize root signature\n"));

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return E_FAIL;
    }

    hr = DXDevice::GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create root signature\n"));
        return E_FAIL;
    }

    signatureBlob->Release();

    psoDesc.pRootSignature = m_rootSignature;

    hr = DXDevice::GetDevice()->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create pipeline state\n"));
        return E_FAIL;
    }

    hr = D3DCompileFromFile(
        L"shaders/BlurPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        //"vertical",
        "normal",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );

    psoDesc.PS.BytecodeLength = psBlob->GetBufferSize();
    psoDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();

    hr = DXDevice::GetDevice()->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState2)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create pipeline state\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Display::CreateBlurBuffer()
{
    std::vector<float> weights = GetGaussianWeights(8, 5.0f);

    m_blurWeightBuffer.SetResourceWidth((sizeof(weights[0]) * weights.size() + 0xff) & ~0xff);
    HRESULT hr = m_blurWeightBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create blur weight buffer\n"));
        return hr;
    }

    std::ranges::copy(weights, m_blurWeightBuffer.GetMappedBuffer());

    m_blurWeightBuffer.UmmapBuffer();

    blur_weight_heap_id_ = globalHeap->Allocate(1);

    m_blurWeightBuffer.SetGlobalHeap(globalHeap);
    m_blurWeightBuffer.SetSegment(blur_weight_heap_id_);
    m_blurWeightBuffer.CreateView();

    return S_OK;
}