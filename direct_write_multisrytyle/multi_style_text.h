#ifndef WIN32_TEST_MULTI_STYLE_TEXT_H
#define WIN32_TEST_MULTI_STYLE_TEXT_H

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include "base_window.h"

class MultiStyleText : public BaseWindow<MultiStyleText>
{
public:
    
    MultiStyleText() :
        text(nullptr),
        cTextLength(0),
        pD2DFactory(nullptr),
        pDWriteFactory(nullptr),
        pTextFormat(nullptr),
        pRenderTarget(nullptr),
        pBlackBrush(nullptr)
    {
    
    }
    
    ~MultiStyleText();
    
    HRESULT Initialize();
    
    PCWSTR ClassName() const
    {
        return L"Hello Text";
    }
    
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    
    HRESULT DrawD2DContent();
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
    IDWriteTextLayout *pTextLayout; //+
    
    const wchar_t *text;
    UINT32 cTextLength;
};


#endif //WIN32_TEST_MULTI_STYLE_TEXT_H
