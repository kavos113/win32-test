#ifndef WIN32_TEST_HELLO_TEXT_H
#define WIN32_TEST_HELLO_TEXT_H


#include "base_window.h"

class HelloText : public BaseWindow<HelloText>
{
public:
    HelloText() :
    pD2DFactory(nullptr), pRenderTarget(nullptr), pBlackBrush(nullptr),
        pDWriteFactory(nullptr), pTextFormat(nullptr), text(L"Hello, World!"), cTextLength(0)
    {
        CreateDeviceIndependentResources();
    }
    
    PCWSTR ClassName() const
    {
        return L"Hello Text";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    HRESULT DrawTextHello();
    
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


#endif //WIN32_TEST_HELLO_TEXT_H
