#ifndef WIN32_TEST_HELLO_TEXT_H
#define WIN32_TEST_HELLO_TEXT_H

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

class HelloText
{
public:
    HelloText();
    ~HelloText();
    
    HRESULT Initialize();
    
    HWND Window() const
    {
        return hwnd;
    }
    
private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    
    HRESULT DrawD2DContent();
    HRESULT DrawTextHello();
    
    void OnResize(UINT width, UINT height);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    HWND hwnd;
    
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
