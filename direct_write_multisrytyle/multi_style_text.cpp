#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <iostream>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include "multi_style_text.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

MultiStyleText::~MultiStyleText()
{
    SafeRelease(&pD2DFactory);
    SafeRelease(&pDWriteFactory);
    SafeRelease(&pTextFormat);
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBlackBrush);
}

HRESULT MultiStyleText::Initialize()
{
    HDC screen = GetDC(0);
    dpiScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
    dpiScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
    ReleaseDC(0, screen);
    
    HRESULT hr = S_OK;
    
    ATOM atom = RegisterNewClassEx();
    
    hr = atom ? S_OK : E_FAIL;
    
    Create(
        L"MultiStyleText",
        WS_OVERLAPPEDWINDOW,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(640.0f / dpiScaleX),
        static_cast<int>(480.0f / dpiScaleY),
        nullptr,
        nullptr
    );
    
    if (SUCCEEDED(hr))
    {
        hr = m_hwnd ? S_OK : E_FAIL;
    }
    
    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceIndependentResources();
    }
    
    if (SUCCEEDED(hr))
    {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);
    }
    
    if (SUCCEEDED(hr))
    {
        DrawD2DContent();
    }
    
    return hr;
}

HRESULT MultiStyleText::CreateDeviceIndependentResources()
{
    HRESULT hr;
    
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory
    );
    
    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory)
        );
    }
    
    text = L"Hello, World DirectWrite!";
    cTextLength = (UINT32) wcslen(text);
    
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTextFormat(
            L"Gabriola",
            nullptr,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            72.0f,
            L"en-us",
            &pTextFormat
        );
    }
    
    // horizontal center
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    
    // vertical center
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    
    // +
    if (SUCCEEDED(hr))
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        
        float width = (rc.right - rc.left) / dpiScaleX;
        float height = (rc.bottom - rc.top) / dpiScaleY;
        
        hr = pDWriteFactory->CreateTextLayout(
            text,
            cTextLength,
            pTextFormat,
            width,
            height,
            &pTextLayout
        );
    }
    
    if (SUCCEEDED(hr))
    {
        DWRITE_TEXT_RANGE textRange = {13, 6};
        hr = pTextLayout->SetFontSize(100.0f, textRange);
    }
    
    if (SUCCEEDED(hr))
    {
        DWRITE_TEXT_RANGE textRange = {13, 11};
        hr = pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, textRange);
    }
    
    IDWriteTypography* pTypography = nullptr;
    
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTypography(&pTypography);
    }
    
    DWRITE_FONT_FEATURE fontFeature = {DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7, 1};
    
    if (SUCCEEDED(hr))
    {
        hr = pTypography->AddFontFeature(fontFeature);
    }
    
    if (SUCCEEDED(hr))
    {
        DWRITE_TEXT_RANGE textRange = {0, cTextLength};
        hr = pTextLayout->SetTypography(pTypography, textRange);
    }
    
    SafeRelease(&pTypography);
    // +
    
    return hr;
}

HRESULT MultiStyleText::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    
    if (!pRenderTarget)
    {
        
        hr = pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget
        );
        
        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &pBlackBrush
            );
        }
    }
    
    return hr;
}

void MultiStyleText::DiscardDeviceResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBlackBrush);
}

HRESULT MultiStyleText::DrawTextHello()
{
    RECT rc;
    
    GetClientRect(m_hwnd, &rc);
    
    // -
//    D2D1_RECT_F layoutRect = D2D1::RectF(
//        static_cast<FLOAT>(rc.left) / dpiScaleX,
//        static_cast<FLOAT>(rc.top) / dpiScaleY,
//        static_cast<FLOAT>(rc.right) / dpiScaleX,
//        static_cast<FLOAT>(rc.bottom) / dpiScaleY
//    );
//
//    pRenderTarget->DrawText(
//        text,
//        cTextLength,
//        pTextFormat,
//        layoutRect,
//        pBlackBrush
//    );

    // +
    D2D1_POINT_2F origin = D2D1::Point2F(
        static_cast<FLOAT>(rc.left / dpiScaleX),
        static_cast<FLOAT>(rc.top / dpiScaleY)
    );
    
    pRenderTarget->DrawTextLayout(
        origin,
        pTextLayout,
        pBlackBrush
    );
    
    return S_OK;
}

HRESULT MultiStyleText::DrawD2DContent()
{
    HRESULT hr = CreateDeviceResources();
    
    if (!(pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        pRenderTarget->BeginDraw();
        
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
        
        hr = DrawTextHello();
        
        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->EndDraw();
        }
    }
    
    if (FAILED(hr))
    {
        DiscardDeviceResources();
    }
    
    return hr;
}

void MultiStyleText::OnResize(UINT width, UINT height)
{
    // +
    if (pTextLayout)
    {
        pTextLayout->SetMaxWidth(static_cast<FLOAT>(width / dpiScaleX));
        pTextLayout->SetMaxHeight(static_cast<FLOAT>(height / dpiScaleY));
    }
    // +
    
    if (pRenderTarget)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;
        pRenderTarget->Resize(size);
    }
}

LRESULT MultiStyleText::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        OnResize(width, height);
    }
        return 0;
    
    case WM_DISPLAYCHANGE:
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        DrawD2DContent();
        EndPaint(m_hwnd, &ps);
    }
        return 0;
    
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
        return 1;
    }
    
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

