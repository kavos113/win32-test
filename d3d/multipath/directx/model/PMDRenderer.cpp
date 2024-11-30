#include "PMDRenderer.h"

#include <d3dcompiler.h>
#include <string>
#include <tchar.h>

#include "directx/resources/DXCommand.h"
#include "directx/resources/DXDevice.h"


HRESULT PMDRenderer::Init()
{
    HRESULT hr = CompileShaders();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile shaders\n"));
        return hr;
    }

    hr = CreateGraphicsPipeline();
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to set graphics pipeline\n"));
        return hr;
    }

    return S_OK;
}

HRESULT PMDRenderer::CompileShaders()
{
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"shaders/BasicVertexShader.hlsl",
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
        L"shaders/BasicPixelShader.hlsl",
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

HRESULT PMDRenderer::CreateGraphicsPipeline()
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
        // {
        //     "EDGE_FLAG",
        //     0,
        //     DXGI_FORMAT_R8_UINT,
        //     0,
        //     D3D12_APPEND_ALIGNED_ELEMENT,
        //     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        //     0
        // }
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
        return hr;
    }

    return S_OK;
}

HRESULT PMDRenderer::CreateRootSignature()
{
    ID3DBlob* errorBlob = nullptr;
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    std::pair<D3D12_ROOT_PARAMETER*, size_t> rootParams = globalHeap->GetRootParameters();

    rootSignatureDesc.NumParameters = static_cast<UINT>(rootParams.second);
    rootSignatureDesc.pParameters = rootParams.first;

    D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};

    samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc[0].MinLOD = 0.0f;
    samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc[0].ShaderRegister = 0;
    samplerDesc[1] = samplerDesc[0];
    samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc[1].ShaderRegister = 1;

    rootSignatureDesc.NumStaticSamplers = 2;
    rootSignatureDesc.pStaticSamplers = samplerDesc;

    ID3DBlob* signatureBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
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

void PMDRenderer::SetPipelineState() const
{
    DXCommand::GetCommandList()->SetPipelineState(m_pipelineState);
}

void PMDRenderer::SetRootSignature() const
{
    DXCommand::GetCommandList()->SetGraphicsRootSignature(m_rootSignature);
}