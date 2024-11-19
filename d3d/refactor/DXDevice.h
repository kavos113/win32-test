#pragma once
#include <d3d12.h>
#include <string>
#include <tchar.h>
#include <vector>

#include "DXFactory.h"
#include "DXUtil.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DXDevice
{
public:
    static ID3D12Device* GetDevice()
    {
        if (m_device == nullptr)
        {
            if (CreateDevice() != 0)
            {
                return nullptr;
            }
        }
        return m_device;
    }

    static void ReleaseDevice()
    {
        SafeRelease(&m_device);
    }
private:
    static ID3D12Device* m_device;

    static int CreateDevice()
    {
        // Selecting a GPU
        std::vector<IDXGIAdapter*> adapters;

        IDXGIAdapter* adapter = nullptr;

        for (int i = 0;
            DXFactory::GetDXGIFactory()->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;
            ++i)
        {
            adapters.push_back(adapter);
        }

        for (auto adp : adapters)
        {
            DXGI_ADAPTER_DESC desc;
            adp->GetDesc(&desc);

            std::wstring str(desc.Description);

            if (str.find(L"NVIDIA") != std::string::npos)
            {
                adapter = adp;
                break;
            }
        }

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        D3D_FEATURE_LEVEL featureLevel;
        for (auto level : featureLevels)
        {
            if (D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(&m_device)) == S_OK)
            {
                featureLevel = level;
                break;
            }
        }
        if (m_device == nullptr)
        {
            OutputDebugString(_T("Failed to create D3D12 device\n"));
            return E_FAIL;
        }

        return S_OK;
    }
};

