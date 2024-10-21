#ifndef WIN32_TEST_HELLO_TEXT_H
#define WIN32_TEST_HELLO_TEXT_H

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include "base_window.h"

class HelloText : public BaseWindow<HelloText>
{
public:
    
    HelloText() :
        text(nullptr),
        cTextLength(0),
        pDWriteFactory(nullptr),
        pTextFormat(nullptr),
        pBlackBrush(nullptr),
        pD2DFactory(nullptr),
        pRenderTarget(nullptr),
        dpiScaleX(1.0f),
        dpiScaleY(1.0f)
    {
    
    }
    
    ~HelloText();
    
    HRESULT Initialize();
    
    PCWSTR ClassName() const override
    {
        return L"Hello Text";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
    HRESULT CreateDeviceIndependentResources();
    
    HRESULT DrawD2DContent();

private:
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    
    HRESULT DrawTextHello();
    
    void OnResize(UINT width, UINT height);
    
private:
    float dpiScaleX;
    float dpiScaleY;
    
    ID2D1Factory *pD2DFactory;
    ID2D1HwndRenderTarget *pRenderTarget;
    ID2D1SolidColorBrush *pBlackBrush;
    
    IDWriteFactory *pDWriteFactory;
    IDWriteTextFormat *pTextFormat;
    
    const wchar_t *text;
    UINT32 cTextLength;
};


#endif //WIN32_TEST_HELLO_TEXT_H
