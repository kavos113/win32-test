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
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        
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
    
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    
    return TRUE;
}

JavaWindow window;

JNIEXPORT void JNICALL Java_java_1window4_java_Window_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring windowName)
{
    std::cout << "[Native] Java_java_1window4_java_Window_create" << std::endl;
    
    HWND parentHwnd = nullptr;
    
    if (parent != nullptr)
    {
        jclass clazz = env->GetObjectClass(parent);
        jfieldID hwndFieldID = env->GetFieldID(clazz, "hwnd", "J");
        parentHwnd = reinterpret_cast<HWND>(
            static_cast<LONG_PTR>(
                env->GetLongField(parent, hwndFieldID)
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
        parentHwnd,
        nullptr))
    {
        return;
    }
    
    std::cout << "[Native] Window Created" << std::endl;
    
    jclass thisClazz = env->GetObjectClass(thisObj);
    jfieldID hwndFieldID2 = env->GetFieldID(thisClazz, "hwnd", "J");
    env->SetLongField(thisObj, hwndFieldID2, static_cast<jlong>(
        reinterpret_cast<LONG_PTR>(window.Window())
    ));
}

JNIEXPORT void JNICALL Java_java_1window4_java_Window_showWindow
    (JNIEnv *env, jobject thisObj)
{
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID hwndFieldID = env->GetFieldID(clazz, "hwnd", "J");
    HWND hwnd = reinterpret_cast<HWND>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, hwndFieldID)
        )
    );
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}