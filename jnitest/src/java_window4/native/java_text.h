#ifndef WIN32_TEST_JAVA_TEXT_H
#define WIN32_TEST_JAVA_TEXT_H

#include <windows.h>
#include <jni.h>
#include <d2d1.h>
#include <dwrite.h>
#include <iostream>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "java_window4_java_Text.h"
#include "java_component.h"

class JavaText : public JavaComponent
{
public:
    JavaText() :
        pDWriteFactory(nullptr),
        pTextFormat(nullptr),
        pColorBrush(nullptr),
        pRenderTarget(nullptr),
        dpiScaleX(1.0f),
        dpiScaleY(1.0f)
    {

    }

    ~JavaText();

    HRESULT Initialize();

    PCWSTR ClassName() const override
    {
        return L"Java Text";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
    HRESULT CreateDeviceIndependentResources();
    
    HRESULT DrawD2DContent();

    void SetText(std::wstring newText);
    std::wstring GetText();

private:
    HRESULT CreateDeviceResources() override;
    void DiscardDeviceResources() override;

    HRESULT DrawTextHello();

protected:
    void OnPaint() override;
    void OnResize(UINT width, UINT height) override;

private:
    float dpiScaleX;
    float dpiScaleY;

    ID2D1HwndRenderTarget *pRenderTarget;
    ID2D1SolidColorBrush *pColorBrush;

    IDWriteFactory *pDWriteFactory;
    IDWriteTextFormat *pTextFormat;

    std::wstring text;
};


#endif //WIN32_TEST_JAVA_TEXT_H
