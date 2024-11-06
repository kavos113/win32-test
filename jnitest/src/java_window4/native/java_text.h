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
        pTextFormat(nullptr),
        pColorBrush(nullptr),
        pRenderTarget(nullptr),
        dpiScaleX(1.0f),
        dpiScaleY(1.0f),
        pTextLayout(nullptr)
    {

    }

    ~JavaText();

    HRESULT Initialize() override;

    PCWSTR ClassName() const override
    {
        return L"Java Text";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
    HRESULT CreateDeviceIndependentResources();
    
    HRESULT DrawD2DContent();

    void SetText(std::wstring newText);
    std::wstring GetText();
    
    void SetTextColor(int color);
    void SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT alignment);
    void SetTextVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT alignment);
    
    void SetFontFamily(const std::wstring& fontFamily, DWRITE_TEXT_RANGE range);
    void SetFontSize(float fontSize, DWRITE_TEXT_RANGE range);
    void SetFontStretch(float stretch, DWRITE_TEXT_RANGE range);
    void SetFontStyle(DWRITE_FONT_STYLE style, DWRITE_TEXT_RANGE range);
    void SetFontWeight(int weight, DWRITE_TEXT_RANGE range);
    void SetFontLineHeight(float lineHeight);

private:
    HRESULT CreateDeviceResources() override;
    void DiscardDeviceResources() override;

    HRESULT DrawTextContent();

protected:
    void OnPaint() override;
    void OnResize(UINT width, UINT height) override;

private:
    float dpiScaleX;
    float dpiScaleY;

    ID2D1HwndRenderTarget *pRenderTarget;
    ID2D1SolidColorBrush *pColorBrush;

    IDWriteTextFormat *pTextFormat;
    IDWriteTextLayout *pTextLayout;

    std::wstring text;
    
    static std::once_flag flag;
};


#endif //WIN32_TEST_JAVA_TEXT_H
