#include "DXEngine.h"

HRESULT DXEngine::Init()
{
    HRESULT hr = EnableDebugLayer();
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_display.Init();
    if (FAILED(hr))
    {
        return hr;
    }

    

    
    return S_OK;
}