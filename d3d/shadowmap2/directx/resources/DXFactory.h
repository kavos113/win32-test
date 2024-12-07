#pragma once

#include <dxgi1_6.h>

#include "directx/DXUtil.h"

#pragma comment(lib, "dxgi.lib")

class DXFactory
{
public:
    static void Init()
    {
        CreateDXGIFactory();
    }
    static IDXGIFactory6* GetDXGIFactory()
    {
        return m_dxgiFactory;
    }

    static void ReleaseDXGIFactory()
    {
        SafeRelease(&m_dxgiFactory);
    }
private:
    static IDXGIFactory6* m_dxgiFactory;

    static int CreateDXGIFactory()
    {
        HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory));
        if (FAILED(hr))
        {
            OutputDebugString(L"Failed to create DXGI Factory\n");
            return -1;
        }
        return 0;
    }
};

