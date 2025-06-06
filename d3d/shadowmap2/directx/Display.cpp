#include "Display.h"

#include <d3dcompiler.h>
#include <tchar.h>
#include <ranges>

#include "Util.h"
#include "descriptor_heap/GlobalDescriptorHeapManager.h"
#include "resources/DXCommand.h"
#include "resources/DXDevice.h"
#include "resources/DXFactory.h"


HRESULT Display::Init(DescriptorHeapSegmentManager& base_poly_manager, DescriptorHeapSegmentManager& model_manager)
{
    m_basePolyManager = &base_poly_manager;
    m_modelManager = &model_manager;
    m_rtvManager = &GlobalDescriptorHeapManager::GetCPUHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvManager = &GlobalDescriptorHeapManager::GetCPUHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    HRESULT hr = CreateSwapChain();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create swap chain\n"));
        return hr;
    }

    hr = CreateBackBuffers();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create back buffers\n"));
        return hr;
    }

    hr = CreateRenderResource();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create render resource\n"));
        return hr;
    }

    hr = SetRenderTargetView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to set render target view\n"));
        return hr;
    }

    hr = SetBaseRenderTargetView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to set render target view\n"));
        return hr;
    }

    hr = CreateShaderResourceView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create shader resource view\n"));
        return hr;
    }

    hr = SetDepthStencilView();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to set depth stencil view\n"));
        return hr;
    }

    hr = CreateShadowMapBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create shadow map buffer\n"));
        return hr;
    }



    hr = CreateViewPort();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create view port\n"));
        return hr;
    }

    hr = CreateBasePolygon();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create base polygon\n"));
        return hr;
    }

    hr = CreateBasePipeline();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create base pipeline\n"));
        return hr;
    }

    m_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    m_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    m_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    return S_OK;
}

void Display::RenderToBackBuffer() const
{
    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);

    // base polygon1枚目をテクスチャとして使う
    m_basePolyManager->SetGraphicsRootDescriptorTable(m_baseSRVsSegment.GetID());

    DXCommand::GetCommandList()->SetPipelineState(m_pipelineState);

    DXCommand::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    DXCommand::GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    DXCommand::GetCommandList()->DrawInstanced(4, 1, 0, 0);
}

void Display::SetRenderToBase1Begin()
{
    Barrier(
        m_renderResource,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    m_modelManager->SetGraphicsRootDescriptorTable(m_shadowMapSRVSegment.GetID());

    auto baseRtvHandle = m_baseRTVsSegment.GetCPUHandle();
    auto dsvHandle = m_depthStencilBuffer.GetCPUHandle();

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
       OutputDebugString(_T("[Display.cpp] Failed to present\n"));
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

    Barrier(
        m_shadowMapBuffer.GetBuffer(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );
}

void Display::SetRenderToBackBuffer()
{
    UINT bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    Barrier(
        back_buffers_[bbIdx],
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_backBufferSegment.GetCPUHandle(bbIdx);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthStencilBuffer.GetCPUHandle();

    DXCommand::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    DXCommand::GetCommandList()->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
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

void Display::SetRenderToShadowMapBegin()
{
    auto dsvHandle = m_shadowMapBuffer.GetCPUHandle();

    DXCommand::GetCommandList()->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Display::SetRenderToShadowMapEnd()
{
    Barrier(
        m_shadowMapBuffer.GetBuffer(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
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
    DXGI_SWAP_CHAIN_DESC1 desc;

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
        OutputDebugString(_T("[Display.cpp] failed to create swap chain\n"));
    }

    return hr;
}

HRESULT Display::CreateBackBuffers()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    HRESULT hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to get swap chain description\n"));
        return hr;
    }

    back_buffers_.resize(swapChainDesc.BufferCount);

    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&back_buffers_[i]));
        if (FAILED(hr))
        {
            OutputDebugString(_T("[Display.cpp] Failed to get buffer from swap chain\n"));
            return hr;
        }
    }

    return S_OK;
}


HRESULT Display::SetRenderTargetView()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    HRESULT hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to get swap chain description\n"));
        return hr;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    m_backBufferSegment = m_rtvManager->Allocate(swapChainDesc.BufferCount);

    for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        rtvDesc.Format = back_buffers_[i]->GetDesc().Format;

        DXDevice::GetDevice()->CreateRenderTargetView(
            back_buffers_[i],
            &rtvDesc,
            m_backBufferSegment.GetCPUHandle(i)
        );
    }

    return S_OK;
}

HRESULT Display::SetDepthStencilView()
{
    HRESULT hr = m_depthStencilBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create depth buffer\n"));
        return hr;
    }

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

    D3D12_CLEAR_VALUE clearValue = {};

    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    for (int i = 0; i < 4; ++i)
    {
        clearValue.Color[i] = m_clearColor[i];
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
        OutputDebugString(_T("[Display.cpp] Failed to create render resource\n"));
        return hr;
    }

    return S_OK;
}

HRESULT Display::SetBaseRenderTargetView()
{
    m_baseRTVsSegment = m_rtvManager->Allocate(1);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    DXDevice::GetDevice()->CreateRenderTargetView(
        m_renderResource,
        &rtvDesc,
        m_baseRTVsSegment.GetCPUHandle(0)
    );

    return S_OK;
}

HRESULT Display::CreateShaderResourceView()
{
    m_baseSRVsSegment = m_basePolyManager->Allocate(1);
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    DXDevice::GetDevice()->CreateShaderResourceView(
        m_renderResource,
        &srvDesc,
        m_baseSRVsSegment.GetCPUHandle(0)
    );

    m_renderResource->SetName(L"RenderResource");

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->NumDescriptors = 1;
    range->BaseShaderRegister = 0;

    m_basePolyManager->SetRootParameter(
        m_baseSRVsSegment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_PIXEL,
        range,
        1
    );

    return S_OK;
}

HRESULT Display::CreateBasePolygon()
{
    BaseVertex vertices[4] = { {{-1, -1, 0.1f}, {0, 1}},
                                {{-1,  1, 0.1f}, {0, 0}},
                                {{ 1, -1, 0.1f}, {1, 1}},
                                {{ 1,  1, 0.1f}, {1, 0}} };

    m_vertexBuffer.SetResourceWidth(sizeof(vertices));
    HRESULT hr = m_vertexBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create vertex buffer\n"));
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
        OutputDebugString(_T("[Display.cpp] Failed to compile vertex shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("[Display.cpp] File not found\n"));
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
        "ps",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to compile pixel shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("[Display.cpp] File not found\n"));
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

    std::pair<D3D12_ROOT_PARAMETER*, size_t> params = m_basePolyManager->GetRootParameters();

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

    rootSignatureDesc.NumParameters = static_cast<UINT>(params.second);
    rootSignatureDesc.pParameters = params.first;
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
        OutputDebugString(_T("[Display.cpp] Failed to serialize root signature\n"));

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
        OutputDebugString(_T("[Display.cpp] Failed to create root signature\n"));
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
        OutputDebugString(_T("[Display.cpp] Failed to create pipeline state\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Display::CreateShadowMapBuffer()
{
    HRESULT hr = m_shadowMapBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[Display.cpp] Failed to create shadow map buffer\n"));
        return hr;
    }

    m_shadowMapBuffer.CreateView();

    // srv
    m_shadowMapSRVSegment = m_modelManager->Allocate(1);

    D3D12_SHADER_RESOURCE_VIEW_DESC desc;

    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Texture2D.MipLevels = 1;
    desc.Texture2D.PlaneSlice = 0;
    desc.Texture2D.MostDetailedMip = 0;

    DXDevice::GetDevice()->CreateShaderResourceView(
        m_shadowMapBuffer.GetBuffer(),
        &desc,
        m_shadowMapSRVSegment.GetCPUHandle(0)
    );

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->NumDescriptors = 1;
    range->BaseShaderRegister = 4;
    range->RegisterSpace = 0;

    m_modelManager->SetRootParameter(
        m_shadowMapSRVSegment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_ALL,
        range,
        1
    );

    m_shadowMapBuffer.GetBuffer()->SetName(L"ShadowMapBuffer");

    return S_OK;
}
