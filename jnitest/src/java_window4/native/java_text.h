#ifndef WIN32_TEST_JAVA_TEXT_H
#define WIN32_TEST_JAVA_TEXT_H

#include <windows.h>
#include <jni.h>
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "java_window4_java_Text.h"
#include "java_component.h"

class JavaText : public JavaComponent
{
public:
    JavaText() :
        text(nullptr),
        cTextLength(0),
        pDWriteFactory(nullptr),
        pTextFormat(nullptr),
        pColorBrush(nullptr)
    {

    }

    ~JavaText();

    HRESULT Initialize();

    PCWSTR ClassName() const override
    {
        return L"Java Text";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT DrawD2DContent();
    HRESULT DrawTextHello();

    void OnResize(UINT width, UINT height);

protected:
    HRESULT CreateGraphicsResources() override;

    void DiscardGraphicsResources() override;

    void OnPaint() override;

    void Resize() override;

private:
    float dpiScaleX{};
    float dpiScaleY{};

    ID2D1SolidColorBrush *pColorBrush;

    IDWriteFactory *pDWriteFactory;
    IDWriteTextFormat *pTextFormat;

    const wchar_t *text;
    UINT32 cTextLength;
};


#endif //WIN32_TEST_JAVA_TEXT_H
