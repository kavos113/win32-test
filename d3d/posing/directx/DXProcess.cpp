#include "DXProcess.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
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

    OutputDebugString(_T("Debug layer is enabled\n"));

    HRESULT hr = CreateSwapChain();
    if (FAILED(hr)) return E_FAIL;

    hr = SetRenderTargetView();
    if (FAILED(hr)) return E_FAIL;

    hr = SetDepthStencilView();
    if (FAILED(hr)) return E_FAIL;

    hr = CompileShaders();
    if (FAILED(hr)) return E_FAIL;

    hr = SetGraphicsPipeline();
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

    model = std::make_unique<PMDModel>("model/�����~�Nmetal.pmd", DXDevice::GetDevice());
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
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
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

    HRESULT hr = DXDevice::GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
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
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (int idx = 0; idx < swapChainDesc.BufferCount; ++idx)
    {
        hr = m_swapChain->GetBuffer(idx, IID_PPV_ARGS(&back_buffers_[idx]));
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to get buffer from swap chain\n"));
            return hr;
        }

        DXDevice::GetDevice()->CreateRenderTargetView(
            back_buffers_[idx],
            &rtvDesc,
            rtvHandle
        );

        rtvHandle.ptr += DXDevice::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

    hr = DXDevice::GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
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
        m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    return S_OK;
}

HRESULT DXProcess::CompileShaders()
{
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"BasicVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicVS",
        "vs_5_0",
        D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
        0,
        &m_vsBlob,
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
        std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return E_FAIL;
    }

    hr = D3DCompileFromFile(
        L"BasicPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicPS",
        "ps_5_0",
        D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
        0,
        &m_psBlob,
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
        std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return E_FAIL;
    }

    return S_OK;
}

HRESULT DXProcess::SetGraphicsPipeline()
{
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "BONE_NUMBER",
            0,
            DXGI_FORMAT_R16G16_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "WEIGHT",
            0,
            DXGI_FORMAT_R8_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "EDGE_FLAG",
            0,
            DXGI_FORMAT_R8_UINT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics_pipeline = {};

    graphics_pipeline.pRootSignature = nullptr;

    graphics_pipeline.VS.pShaderBytecode = m_vsBlob->GetBufferPointer();
    graphics_pipeline.VS.BytecodeLength = m_vsBlob->GetBufferSize();

    graphics_pipeline.PS.pShaderBytecode = m_psBlob->GetBufferPointer();
    graphics_pipeline.PS.BytecodeLength = m_psBlob->GetBufferSize();

    graphics_pipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    graphics_pipeline.BlendState.AlphaToCoverageEnable = FALSE;
    graphics_pipeline.BlendState.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};

    rtvBlendDesc.BlendEnable = FALSE;
    rtvBlendDesc.LogicOpEnable = FALSE;
    rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    graphics_pipeline.BlendState.RenderTarget[0] = rtvBlendDesc;

    graphics_pipeline.RasterizerState.MultisampleEnable = FALSE;
    graphics_pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    graphics_pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    graphics_pipeline.RasterizerState.DepthClipEnable = TRUE;

    graphics_pipeline.RasterizerState.FrontCounterClockwise = FALSE;
    graphics_pipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    graphics_pipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    graphics_pipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    graphics_pipeline.RasterizerState.AntialiasedLineEnable = FALSE;
    graphics_pipeline.RasterizerState.ForcedSampleCount = 0;
    graphics_pipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    graphics_pipeline.DepthStencilState.DepthEnable = TRUE;
    graphics_pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    graphics_pipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    graphics_pipeline.DepthStencilState.StencilEnable = FALSE;

    graphics_pipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    graphics_pipeline.InputLayout.pInputElementDescs = inputElementDescs;
    graphics_pipeline.InputLayout.NumElements = _countof(inputElementDescs);

    graphics_pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    graphics_pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphics_pipeline.NumRenderTargets = 1;
    graphics_pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    graphics_pipeline.SampleDesc.Count = 1;
    graphics_pipeline.SampleDesc.Quality = 0;

    HRESULT hr = CreateRootSignature();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create root signature\n"));
        return hr;
    }

    graphics_pipeline.pRootSignature = m_rootSignature;

    hr = DXDevice::GetDevice()->CreateGraphicsPipelineState(
        &graphics_pipeline,
        IID_PPV_ARGS(&m_pipelineState)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create pipeline state\n"));
        return 1;
    }
}

HRESULT DXProcess::CreateRootSignature()
{
    ID3DBlob* errorBlob = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descRange[3] = {};

    // descriptor range for constant buffer
    descRange[0].NumDescriptors = 1;
    descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange[0].BaseShaderRegister = 0;
    descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // descriptor range for material buffer
    descRange[1].NumDescriptors = 1;
    descRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange[1].BaseShaderRegister = 1;
    descRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // descriptor range for texture
    descRange[2].NumDescriptors = 4;
    descRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRange[2].BaseShaderRegister = 0;
    descRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParam[2] = {};

    // root parameter for constant buffer
    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descRange[0];
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

    // root parameter for material buffer
    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descRange[1];
    rootParam[1].DescriptorTable.NumDescriptorRanges = 2;

    rootSignatureDesc.NumParameters = 2;
    rootSignatureDesc.pParameters = rootParam;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};

    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;

    ID3DBlob* signatureBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to serialize root signature\n"));

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return hr;
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
        return hr;
    }

    signatureBlob->Release();

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
    struct SceneMatrix
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;

        DirectX::XMFLOAT3 eye;
    };

    float angle = 0;
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
    constantBufferResourceDesc.Width = (sizeof(SceneMatrix) + 0xff) & ~0xff;  // ~0xff: 256�o�C�g�ȉ���0
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

    SceneMatrix* constantBufferMap = nullptr;
    hr = constantBuffer->Map(0, nullptr, (void**)&constantBufferMap);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to map constant buffer\n"));
        return hr;
    }

    constantBufferMap->world = worldMatrix;
    constantBufferMap->view = viewMatrix;
    constantBufferMap->proj = projectionMatrix;
    constantBufferMap->eye = eye;

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};

    descriptorHeapDesc.NumDescriptors = 1; // texture(SRV) and constant(CBV) buffer
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;

    hr = DXDevice::GetDevice()->CreateDescriptorHeap(
        &descriptorHeapDesc,
        IID_PPV_ARGS(&m_cbvHeap)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create texture descriptor heap\n"));
        return hr;
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

    cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBuffer->GetDesc().Width;

    D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();

    DXDevice::GetDevice()->CreateConstantBufferView(
        &cbvDesc,
        cbvHandle
    );

    return S_OK;
}

HRESULT DXProcess::OnRender()
{
    auto bbIdx = m_swapChain->GetCurrentBackBufferIndex();

    D3D12_RESOURCE_BARRIER barrier = {};

    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = back_buffers_[bbIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    DXCommand::GetCommandList()->ResourceBarrier(1, &barrier);

    DXCommand::GetCommandList()->SetPipelineState(m_pipelineState);

    auto rendertvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rendertvHandle.ptr += bbIdx * DXDevice::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    DXCommand::GetCommandList()->OMSetRenderTargets(1, &rendertvHandle, TRUE, &dsvHandle);

    float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    DXCommand::GetCommandList()->ClearRenderTargetView(rendertvHandle, clearColor, 0, nullptr);
    DXCommand::GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // draw polygon
    DXCommand::GetCommandList()->RSSetViewports(1, &m_viewport);
    DXCommand::GetCommandList()->RSSetScissorRects(1, &m_scissorRect);
    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);

    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
    DXCommand::GetCommandList()->SetDescriptorHeaps(1, &m_cbvHeap);
    DXCommand::GetCommandList()->SetGraphicsRootDescriptorTable(
        0,
        m_cbvHeap->GetGPUDescriptorHandleForHeapStart()
    );

    model->Render();

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    DXCommand::GetCommandList()->ResourceBarrier(1, &barrier);

    HRESULT hr = DXCommand::GetCommandList()->Close();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to close command list\n"));
        return hr;
    }

    ID3D12CommandList* cmdLists[] = { DXCommand::GetCommandList() };
    DXCommand::GetCommandQueue()->ExecuteCommandLists(1, cmdLists);

    hr = DXCommand::GetCommandQueue()->Signal(DXFence::GetFence(), DXFence::GetIncrementedFenceValue());
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to signal fence\n"));
        return hr;
    }

    if (DXFence::GetFence()->GetCompletedValue() != DXFence::GetFenceValue())
    {
        HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        hr = DXFence::GetFence()->SetEventOnCompletion(DXFence::GetFenceValue(), event);
        if (FAILED(hr))
        {
            OutputDebugString(_T("Failed to set event on completion\n"));
            return hr;
        }
        WaitForSingleObject(event, INFINITE);
        CloseHandle(event);
    }

    hr = DXCommand::GetCommandAllocator()->Reset();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to reset command allocator\n"));
        return hr;
    }
    hr = DXCommand::GetCommandList()->Reset(DXCommand::GetCommandAllocator(), nullptr);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to reset command list\n"));
        return hr;
    }

    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to present\n"));
        return hr;
    }

    return S_OK;
}
