#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>
#include <iostream>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "java_window.h"
#include "dx_factory.h"
#include "util.h"
#include "jniutil.h"

HRESULT JavaWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    
    if (pRenderTarget == nullptr)
    {
        RECT rect;
        GetClientRect(m_hwnd, &rect);
        
        D2D1_SIZE_U size = D2D1::SizeU(
            rect.right - rect.left,
            rect.bottom - rect.top
        );
        
        hr = DXFactory::GetFactory()->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget
        );
    }
    
    return hr;
}

void JavaWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
}

void JavaWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(m_backgroundColor));
        
        hr = pRenderTarget->EndDraw();
        
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void JavaWindow::Resize()
{
    if (pRenderTarget != nullptr)
    {
        RECT rect;
        GetClientRect(m_hwnd, &rect);
        
        D2D1_SIZE_U size = D2D1::SizeU(
            rect.right - rect.left,
            rect.bottom - rect.top
        );
        
        pRenderTarget->Resize(size);
    }
}


LRESULT JavaWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lr = JavaComponent::ComponentHandleMessage(uMsg, wParam, lParam);
    
    switch (uMsg)
    {
    case WM_DESTROY:
        DiscardGraphicsResources();
        DXFactory::ReleaseFactory();
        PostQuitMessage(0);
        std::cout << "[Native] WM_DESTROY" << std::endl;
        return 0;
    
    case WM_PAINT:
        OnPaint();
        return 0;
        
    case WM_SIZE:
        Resize();
        return 0;
    }
    
    return lr;
}

JavaWindow window;

JNIEXPORT void JNICALL Java_java_1window4_java_Window_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring windowName)
{
    // std::cout << "[Native] Java_java_1window4_java_Window_create" << std::endl;
    
    JavaComponent *parentHwnd = nullptr;
    
    if (parent != nullptr)
    {
        jclass clazz = env->GetObjectClass(parent);
        jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
        parentHwnd = reinterpret_cast<JavaComponent*>(
            static_cast<LONG_PTR>(
                env->GetLongField(parent, nativeWindowFieldID)
            )
        );
    }
    
    std::wstring windowNameW = jstringToWstring(env, windowName);
    
    ATOM res = window.RegisterNewClass();
    if (res == 0)
    {
        MessageBox(
            nullptr,
            L"RegisterClass failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        
        return;
    }
    
    if (!window.Create(
        windowNameW.c_str(),
        WS_OVERLAPPEDWINDOW,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        parentHwnd ? parentHwnd->Window() : nullptr,
        nullptr
        )
        )
    {
        return;
    }
    
    std::cout << "[Native] Window Created" << std::endl;
    
    jclass thisClazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID2 = env->GetFieldID(thisClazz, "nativeWindow", "J");
    env->SetLongField(thisObj, nativeWindowFieldID2, static_cast<jlong>(
        reinterpret_cast<LONG_PTR>(&window)
    ));
    
    // std::cout << "[Native] Window Handle Set window: " << &window << std::endl;
}

JNIEXPORT void JNICALL Java_java_1window4_java_Window_showWindow
    (JNIEnv *env, jobject thisObj)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(clazz, "nativeWindow", "J");
    JavaWindow *pThis = reinterpret_cast<JavaWindow*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
        )
    );
    
    HWND hwnd = pThis->Window();
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}