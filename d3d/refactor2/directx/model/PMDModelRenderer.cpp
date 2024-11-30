#include "PMDModelRenderer.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <tchar.h>

#include "directx/resources/DXDevice.h"

HRESULT PMDModelRenderer::CreateGraphicsPipeline()
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

    graphics_pipeline.VS.pShaderBytecode = vsblob->GetBufferPointer();
    graphics_pipeline.VS.BytecodeLength = vsblob->GetBufferSize();

    graphics_pipeline.PS.pShaderBytecode = psblob->GetBufferPointer();
    graphics_pipeline.PS.BytecodeLength = psblob->GetBufferSize();

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

    graphics_pipeline.pRootSignature = root_signature;

    HRESULT hr = DXDevice::GetDevice()->CreateGraphicsPipelineState(
        &graphics_pipeline,
        IID_PPV_ARGS(&pipeline_state)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create pipeline state\n"));
        return 1;
    }

    return S_OK;
}

HRESULT PMDModelRenderer::SetRootSignature()
{
    ID3D10Blob* errorBlob = nullptr;

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
        return 1;
    }

    hr = DXDevice::GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to create root signature\n"));
        return 1;
    }

    signatureBlob->Release();
    return S_OK;
}