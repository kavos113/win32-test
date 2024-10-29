#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <iostream>

#pragma comment(lib, "d2d1")

#include "main_window.h"
#include "hello_text.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget
        );
        
        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
        }
    }
    
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
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
        
        pRenderTarget->Resize(size);
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow window;
    
    window.RegisterNewClassEx();
    
    if (!window.Create(L"Direct2D sample", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN))
    {
        return 0;
    }
    
    HelloText helloText;
    HRESULT hr = helloText.Initialize();
    
    helloText.Create(
        L"Hello, DirectWrite!",
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        300,
        170,
        window.Window(),
        nullptr
        );
    
    
    
    if (SUCCEEDED(hr))
    {
        hr = helloText.CreateDeviceIndependentResources();
    }
    
    if (SUCCEEDED(hr))
    {
        hr = helloText.DrawD2DContent();
    }
    
    ShowWindow(window.Window(), nCmdShow);
    ShowWindow(helloText.Window(), nCmdShow);
    
    SetWindowPos(
        helloText.Window(),
        nullptr,
        10,
        10,
        600,
        370,
        SWP_NOZORDER
    );
    
    
    UpdateWindow(window.Window());
    
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
        OutputDebugString(L"WM_PAINT in MainWindow\n");
        return 0;
    
    case WM_SIZE:
        Resize();
        OutputDebugString(L"WM_SIZE in MainWindow\n");
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}