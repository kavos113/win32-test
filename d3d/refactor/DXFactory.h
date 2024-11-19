#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "DXUtil.h"

#pragma comment(lib, "d3d12.lib")

class DXFactory
{
public:
    static IDXGIFactory6* GetDXGIFactory()
    {
        if (m_dxgiFactory == nullptr)
        {
            if (CreateDXGIFactory() != 0)
            {
                return nullptr;
            }
        }
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
            return -1;
        }
        return 0;
    }
};

