#pragma once

#include <d2d1.h>
#include <dwrite.h>

class HelloText
{
public:
    HelloText();
    ~HelloText();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    HRESULT DrawTextHello();

    void SetText(const wchar_t* text);
    HRESULT DrawD2DContent();

private:
    HWND hwnd;

    float dpiScaleX = 1.0f;
    float dpiScaleY = 1.0f;

    ID2D1Factory* pD2DFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBlackBrush;

    IDWriteFactory* pDWriteFactory;
    IDWriteTextFormat* pTextFormat;

    const wchar_t* text;
    UINT32 cTextLength;
};

