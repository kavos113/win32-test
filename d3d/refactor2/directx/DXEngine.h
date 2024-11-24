#pragma once
#include <utility>
#include <Windows.h>

#include "Display.h"
#include "DisplayMatrix.h"
#include "DXFence.h"
#include "model/Model.h"
#include "model/ModelRenderer.h"
#include "model/PMDModel.h"
#include "model/PMDModelRenderer.h"
#include "resources/DXCommand.h"

class DXEngine
{
public:
    HRESULT Init();
    HRESULT Render();
private:
    // DXCommand m_command;  // «—ˆ“I‚É”ñstatic‚É‚·‚é
    DXFence m_fence;
    Display m_display;
    DisplayMatrix m_displayMatrix;
    PMDModel m_model;
    PMDModelRenderer m_modelRenderer;

    HWND m_hwnd;


    static HRESULT EnableDebugLayer()
    {
        HRESULT hr = S_OK;
        ID3D12Debug* debugController;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
        if (FAILED(hr))
        {
            return hr;
        }
        debugController->EnableDebugLayer();
        return hr;
    }
};

