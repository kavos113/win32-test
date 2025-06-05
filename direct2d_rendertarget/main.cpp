#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h> // For Microsoft::WRL::ComPtr
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#include <iostream>

#include "base_window.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
    ID2D1Factory1           *pFactory;
    ID2D1SolidColorBrush    *pBrush;
    ID2D1DeviceContext      *pRenderTarget;
    ID2D1Device           *pD2DDevice;
    ID2D1Bitmap1           *pBitmap;
    IDXGISwapChain1         *pSwapChain;
    D2D1_ELLIPSE            ellipse;
    D2D1_POINT_2F           ptMouse;
    
    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
    void    OnLButtonUp();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);

public:
    
    MainWindow() : pFactory(nullptr), pRenderTarget(nullptr), pBrush(nullptr),
        ellipse(D2D1::Ellipse(D2D1::Point2F(), 10.0f, 10.0f)),
        ptMouse(D2D1::Point2F())
    {
    
    }
    
    [[nodiscard]] PCWSTR  ClassName() const override
    {
        return L"Direct2D sample class";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

void MainWindow::CalculateLayout()
{

}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        Microsoft::WRL::ComPtr<ID3D11Device> pD3DDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> pD3DContext;
        hr = D3D11CreateDevice(
            nullptr, // Use default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr, // No software device
            creationFlags,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &pD3DDevice,
            nullptr, // Feature level not needed
            &pD3DContext
        );
        if (FAILED(hr))
        {
            std::cerr << "Failed to create D3D11 device: " << std::hex << hr << std::endl;
            return hr;
        }

        IDXGIDevice *pDXGIDevice = nullptr;
        hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
        if (FAILED(hr))
        {
            std::cerr << "Failed to get IDXGIDevice: " << std::hex << hr << std::endl;
            return hr;
        }

        hr = pFactory->CreateDevice(pDXGIDevice, &pD2DDevice);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create D2D device: " << std::hex << hr << std::endl;
            return hr;
        }

        hr = pD2DDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pRenderTarget);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create D2D device context: " << std::hex << hr << std::endl;
            return hr;
        }

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = size.width;
        swapChainDesc.Height = size.height;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = 0;

        Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
        hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
        if (FAILED(hr))
        {
            std::cerr << "Failed to get DXGI adapter: " << std::hex << hr << std::endl;
            return hr;
        }

        Microsoft::WRL::ComPtr<IDXGIFactory2> pDXGIFactory;
        hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));
        if (FAILED(hr))
        {
            std::cerr << "Failed to get DXGI factory: " << std::hex << hr << std::endl;
            return hr;
        }

        hr = pDXGIFactory->CreateSwapChainForHwnd(
            pD3DDevice.Get(),
            m_hwnd,
            &swapChainDesc,
            nullptr, // No additional swap chain options
            nullptr, // No restrict to output
            &pSwapChain
        );
        if (FAILED(hr))
        {
            std::cerr << "Failed to create swap chain: " << std::hex << hr << std::endl;
            return hr;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            std::cerr << "Failed to get back buffer: " << std::hex << hr << std::endl;
            return hr;
        }

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f, // Default DPI
            96.0f  // Default DPI
        );

        Microsoft::WRL::ComPtr<IDXGISurface> pDXGISurface;
        hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDXGISurface));
        if (FAILED(hr))
        {
            std::cerr << "Failed to get DXGI surface: " << std::hex << hr << std::endl;
            return hr;
        }

        hr = pRenderTarget->CreateBitmapFromDxgiSurface(
            pDXGISurface.Get(),
            &bitmapProperties,
            &pBitmap
        );
        if (FAILED(hr))
        {
            std::cerr << "Failed to create D2D bitmap from DXGI surface: " << std::hex << hr << std::endl;
            return hr;
        }

        pRenderTarget->SetTarget(pBitmap);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
            
            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
    SafeRelease(&pBitmap);
    SafeRelease(&pD2DDevice);
    SafeRelease(&pSwapChain);
    SafeRelease(&pFactory);
}

void MainWindow::OnPaint()
{
    std::cout << "OnPaint called" << std::endl;
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        pRenderTarget->FillEllipse(ellipse, pBrush);
        
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }

        pSwapChain->Present(1, 0); // Present with vsync

        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != nullptr)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        
        CalculateLayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow window;
    
    if (!window.Create(L"Direct2D sample", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }
    
    ShowWindow(window.Window(), nCmdShow);
    
    // run message loop
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;
        }
        return 0;
    
    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;
    
    case WM_PAINT:
        OnPaint();
        return 0;
    
    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}