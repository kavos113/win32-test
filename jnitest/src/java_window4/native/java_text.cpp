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

JavaText::~JavaText()
{
    SafeRelease(&pDWriteFactory);
    SafeRelease(&pTextFormat);
    SafeRelease(&pColorBrush);
}

HRESULT JavaText::Initialize()
{
    HRESULT hr = S_OK;

    HDC screen = GetDC(nullptr);
    dpiScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
    dpiScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
    ReleaseDC(nullptr, screen);

    WNDCLASSEX wcex = {0};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = ClassName();

    ATOM atom = RegisterClassEx(&wcex);

    hr = atom ? S_OK : E_FAIL;

    Create(
        L"JavaText",
        WS_OVERLAPPEDWINDOW,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(640.0f / dpiScaleX),
        static_cast<int>(480.0f / dpiScaleY),
        nullptr,
        nullptr
    );

    if (SUCCEEDED(hr))
    {
        hr = m_hwnd ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceIndependentResources();
    }

    if (SUCCEEDED(hr))
    {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);
    }

    if (SUCCEEDED(hr))
    {
        DrawD2DContent();
    }

    return hr;
}

HRESULT JavaText::CreateDeviceIndependentResources()
{
    HRESULT hr;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&pDWriteFactory)
    );

    text = L"Hello, World!";
    cTextLength = (UINT32) wcslen(text);

    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->CreateTextFormat(
            L"Verdana",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            24.0f,
            L"en-us",
            &pTextFormat
        );
    }

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
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
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

HRESULT JavaText::DrawD2DContent()
{
    HRESULT hr = CreateDeviceResources();

    if (!(pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        pRenderTarget->BeginDraw();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

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

    pRenderTarget->DrawText(
        text,
        cTextLength,
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
        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            OnResize(width, height);
        }
            return 0;

        case WM_DISPLAYCHANGE:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(m_hwnd, &ps);
            DrawD2DContent();
            EndPaint(m_hwnd, &ps);
        }
            return 0;

        case WM_DESTROY:
        {
            DiscardDeviceResources();
            PostQuitMessage(0);
        }
            return 1;

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    }
}

HRESULT JavaText::CreateGraphicsResources()
{
    return 0;
}

void JavaText::DiscardGraphicsResources()
{

}

void JavaText::OnPaint()
{

}

void JavaText::Resize()
{

}

JavaText javaText;

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

}