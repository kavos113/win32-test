#include <iostream>
#include <d2d1.h>
#include <dwrite.h>

#include "hello_text.h"

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
        (*ppT)->Release();
        *ppT = NULL;
	}
}

HRESULT HelloText::CreateDeviceIndependentResources()
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
    cTextLength = (UINT32)wcslen(text);

    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTextFormat(
            L"Consolas",
            nullptr,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            72.0f,
            L"en-US",
            &pTextFormat
        );
    }

    // ’†‰›
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    // ã‰º’†‰›‘µ‚¦
    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
}

HRESULT HelloText::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    if (pRenderTarget == nullptr)
    {
        hr = pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
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
}

void HelloText::DiscardDeviceResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBlackBrush);
}

HRESULT HelloText::DrawTextHello()
{
    RECT rc;

    GetClientRect(hwnd, &rc);

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left) / dpiScaleX,
        static_cast<FLOAT>(rc.top) / dpiScaleY,
        static_cast<FLOAT>(rc.right - rc.left) / dpiScaleX,
        static_cast<FLOAT>(rc.bottom - rc.top) / dpiScaleY
    );

    pRenderTarget->DrawText(
        text,
        cTextLength,
        pTextFormat,
        layoutRect,
        pBlackBrush
    );
}

HRESULT HelloText::DrawD2DContent()
{
    HRESULT hr;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        pRenderTarget->BeginDraw();

        pRenderTarget->SetTransform(D2D1::IdentityMatrix());

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
}
