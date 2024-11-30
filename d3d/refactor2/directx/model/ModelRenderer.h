#pragma once
#include <d3d12.h>
#include <d3dcommon.h>

class ModelRenderer
{
public:
    HRESULT CompileShaders();

    virtual HRESULT CreateGraphicsPipeline();
    virtual HRESULT SetRootSignature();
protected:
    ID3D10Blob* vsblob;
    ID3D10Blob* psblob;

    ID3D12RootSignature* root_signature;
    ID3D12PipelineState* pipeline_state;
};

