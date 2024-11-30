#pragma once
#include "ModelRenderer.h"
class PMDModelRenderer :
    public ModelRenderer
{
public:
    HRESULT CreateGraphicsPipeline() override;
    HRESULT SetRootSignature() override;
};

