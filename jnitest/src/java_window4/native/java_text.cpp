#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>
#include <iostream>
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "java_text.h"
#include "dx_factory.h"
#include "util.h"
#include "jniutil.h"

std::once_flag JavaText::flag;

JavaText::~JavaText()
{
    SafeRelease(&pTextFormat);
    SafeRelease(&pColorBrush);
    SafeRelease(&pRenderTarget);
}

HRESULT JavaText::Initialize()
{
    HRESULT hr = S_OK;
    
    std::call_once(flag, [this]() {
        HDC screen = GetDC(nullptr);
        dpiScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
        dpiScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
        ReleaseDC(nullptr, screen);
        
        ATOM atom = RegisterNewClass();
    });

    return hr;
}

HRESULT JavaText::CreateDeviceIndependentResources()
{
    HRESULT hr = DXFactory::GetDWriteFactory()->CreateTextFormat(
        L"Consolas",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        24.0f,
        L"en-us",
        &pTextFormat
    );

    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    if (SUCCEEDED(hr))
    {
        hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    return hr;
}

HRESULT JavaText::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    if (pRenderTarget == nullptr)
    {
        hr = DXFactory::GetFactory()->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                m_hwnd,
                size
            ),
            &pRenderTarget
        );

        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &pColorBrush
            );
        }
    }

    return hr;
}

void JavaText::DiscardDeviceResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pColorBrush);
}

void JavaText::OnPaint()
{
    PAINTSTRUCT ps;
    printf("[Native] OnPaint in JavaText, text: %ls\n", text.c_str());
    BeginPaint(m_hwnd, &ps);
    DrawD2DContent();
    EndPaint(m_hwnd, &ps);
}

HRESULT JavaText::DrawD2DContent()
{
    HRESULT hr = CreateDeviceResources();
    
    if (!(pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        pRenderTarget->BeginDraw();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        pRenderTarget->Clear(m_backgroundColor);
        
        hr = DrawTextHello();
        
        if (SUCCEEDED(hr))
        {
            hr = pRenderTarget->EndDraw();
        }
    }
    
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
    
    return hr;
}

HRESULT JavaText::DrawTextHello()
{
    RECT rc;

    GetClientRect(m_hwnd, &rc);

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<FLOAT>(rc.left) / dpiScaleX,
        static_cast<FLOAT>(rc.top) / dpiScaleY,
        static_cast<FLOAT>(rc.right) / dpiScaleX,
        static_cast<FLOAT>(rc.bottom) / dpiScaleY
    );

    printf("[Native] Text in settext: %ls\n", text.c_str());

    pRenderTarget->DrawText(
        text.c_str(),
        text.size(),
        pTextFormat,
        layoutRect,
        pColorBrush
    );

    return S_OK;
}

void JavaText::OnResize(UINT width, UINT height)
{
    if (pRenderTarget != nullptr)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;

        pRenderTarget->Resize(size);
    }
}

LRESULT JavaText::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
            std::cout << "[Native] WM_PAINT in JavaText: " << wParam << std::endl;
            OnPaint();
            return 0;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    }
}

void JavaText::SetText(std::wstring newText)
{
    text = newText;
}

std::wstring JavaText::GetText()
{
    return text;
}

void JavaText::SetTextColor(int color)
{
    pColorBrush->SetColor(D2D1::ColorF(
        (float) ((color >> 16) & 0xFF) / 255.0f,
        (float) ((color >> 8) & 0xFF) / 255.0f,
        (float) (color & 0xFF) / 255.0f,
        (float) ((color >> 24) & 0xFF) / 255.0f
    ));
}

void JavaText::SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT alignment)
{
    pTextFormat->SetTextAlignment(alignment);
}

void JavaText::SetTextVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT alignment)
{
    pTextFormat->SetParagraphAlignment(alignment);
}

JNIEXPORT void JNICALL Java_java_1window4_java_Text_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring text)
{
    JavaComponent *parentComponent = nullptr;

    if (parent != nullptr)
    {
        jclass clazz = env->GetObjectClass(parent);
        jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
        parentComponent = reinterpret_cast<JavaComponent*>(
            static_cast<LONG_PTR >(
                env->GetLongField(parent, nativeWindowFieldID)
            )
        );
    }
    
    std::wstring textStr = JstringToWstring(env, text);

    printf("[Native] Text: %ls\n", textStr.c_str());
    
    auto *javaText = new JavaText();

    HRESULT hr = javaText->Initialize();
    
    javaText->Create(
        textStr.c_str(),
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        300,
        100,
        parentComponent != nullptr ? parentComponent->Window() : nullptr,
        nullptr
        );
    
    javaText->SetText(textStr);
    
    if (SUCCEEDED(hr))
    {
        hr = javaText->CreateDeviceIndependentResources();
    }
    
    if (SUCCEEDED(hr))
    {
        hr = javaText->DrawD2DContent();
    }
    
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    env->SetLongField(thisObj, nativeWindowFieldID, static_cast<jlong>(
        reinterpret_cast<LONG_PTR>(javaText)
        )
    );
}

JNIEXPORT void JNICALL Java_java_1window4_java_Text_setNativeText
    (JNIEnv *env, jobject thisObj, jstring text)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    JavaText *pThis = reinterpret_cast<JavaText*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
        )
    );

    std::wstring textStr = JstringToWstring(env, text);
    
    pThis->SetText(textStr);
    /*
     * if (isVisible)
     * {
     *   SendMessage(pThis->Window(), WM_PAINT, 0, 0);
     * }
     */
}

JNIEXPORT void JNICALL Java_java_1window4_java_Text_setTextColor
    (JNIEnv *env, jobject thisObj, jint color)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    JavaText *pThis = reinterpret_cast<JavaText*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
        )
    );

    pThis->SetTextColor(color);
    
    /*
     * if (isVisible)
     * {
     *   SendMessage(pThis->Window(), WM_PAINT, 0, 0);
     * }
     */
}

JNIEXPORT void JNICALL Java_java_1window4_java_Text_setTextHorizontalAlignment
    (JNIEnv *env, jobject thisObj, jobject alignment)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    JavaText *pThis = reinterpret_cast<JavaText*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
        )
    );

    jclass alignmentClazz = env->GetObjectClass(alignment);
    
    jfieldID alignmentCenterFieldID = env->GetStaticFieldID(alignmentClazz, "CENTER", "Ljava_window4/java/TextHorizontalAlignment;");
    jobject center = env->GetStaticObjectField(alignmentClazz, alignmentCenterFieldID);
    
    jfieldID alignmentLeadingFieldID = env->GetStaticFieldID(alignmentClazz, "LEADING", "Ljava_window4/java/TextHorizontalAlignment;");
    jobject leading = env->GetStaticObjectField(alignmentClazz, alignmentLeadingFieldID);
    
    jfieldID alignmentTrailingFieldID = env->GetStaticFieldID(alignmentClazz, "TRAILING", "Ljava_window4/java/TextHorizontalAlignment;");
    jobject trailing = env->GetStaticObjectField(alignmentClazz, alignmentTrailingFieldID);
    
    jfieldID alignmentJustifyFieldID = env->GetStaticFieldID(alignmentClazz, "JUSTIFY", "Ljava_window4/java/TextHorizontalAlignment;");
    jobject justify = env->GetStaticObjectField(alignmentClazz, alignmentJustifyFieldID);
    
    if (env->IsSameObject(alignment, center))
    {
        pThis->SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else if (env->IsSameObject(alignment, leading))
    {
        pThis->SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    }
    else if (env->IsSameObject(alignment, trailing))
    {
        pThis->SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    }
    else if (env->IsSameObject(alignment, justify))
    {
        pThis->SetTextHorizontalAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
    }
    
    /*
     * if (isVisible)
     * {
     *   SendMessage(pThis->Window(), WM_PAINT, 0, 0);
     * }
     */
}

JNIEXPORT void JNICALL Java_java_1window4_java_Text_setTextVerticalAlignment
    (JNIEnv *env, jobject thisObj, jobject alignment)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    JavaText *pThis = reinterpret_cast<JavaText*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
        )
    );

    jclass alignmentClazz = env->GetObjectClass(alignment);
    
    jfieldID alignmentCenterFieldID = env->GetStaticFieldID(alignmentClazz, "CENTER", "Ljava_window4/java/TextVerticalAlignment;");
    jobject center = env->GetStaticObjectField(alignmentClazz, alignmentCenterFieldID);
    
    jfieldID alignmentTopFieldID = env->GetStaticFieldID(alignmentClazz, "TOP", "Ljava_window4/java/TextVerticalAlignment;");
    jobject top = env->GetStaticObjectField(alignmentClazz, alignmentTopFieldID);
    
    jfieldID alignmentBottomFieldID = env->GetStaticFieldID(alignmentClazz, "BOTTOM", "Ljava_window4/java/TextVerticalAlignment;");
    jobject bottom = env->GetStaticObjectField(alignmentClazz, alignmentBottomFieldID);
    
    if (env->IsSameObject(alignment, center))
    {
        pThis->SetTextVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    else if (env->IsSameObject(alignment, top))
    {
        pThis->SetTextVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    }
    else if (env->IsSameObject(alignment, bottom))
    {
        pThis->SetTextVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
    }
    
    /*
     * if (isVisible)
     * {
     *   SendMessage(pThis->Window(), WM_PAINT, 0, 0);
     * }
     */
}