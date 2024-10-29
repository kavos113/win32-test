#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>
#include <iostream>
#include <vector>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "java_window.h"
#include "dx_factory.h"
#include "util.h"
#include "jniutil.h"

std::once_flag JavaWindow::flag;

HRESULT JavaWindow::Initialize()
{
    HRESULT hr = S_OK;

    std::call_once(flag, [this]() {
        RegisterNewClass();
        m_backgroundColor = D2D1::ColorF(D2D1::ColorF::White);
    });
    
    return hr;
}

HRESULT JavaWindow::CreateDeviceResources()
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

void JavaWindow::DiscardDeviceResources()
{
    SafeRelease(&pRenderTarget);
}

void JavaWindow::OnPaint()
{
    HRESULT hr = CreateDeviceResources();
    
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(m_backgroundColor);
        
        hr = pRenderTarget->EndDraw();
        
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardDeviceResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void JavaWindow::OnResize(UINT width, UINT height)
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
        DiscardDeviceResources();
        DXFactory::ReleaseFactory();
        DXFactory::GetDWriteFactory();
        PostQuitMessage(0);
        std::cout << "[Native] WM_DESTROY: " << this << std::endl;
        return 0;
    
    case WM_PAINT:
        OnPaint();
        return 0;
        
    case WM_SIZE:
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        OnResize(width, height);
        return 0;
    }
    
    return lr;
}

JNIEXPORT void JNICALL Java_java_1window4_java_Window_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring windowName)
{
    // std::cout << "[Native] Java_java_1window4_java_Window_create" << std::endl;
    
    JavaComponent *parentHwnd = nullptr;
    
    if (parent != nullptr)
    {
        parentHwnd = reinterpret_cast<JavaComponent *>(GetJavaWindowPtr(env, parent));
    }
    
    auto *window = new JavaWindow();
    
    std::wstring windowNameW = JstringToWstring(env, windowName);

    HRESULT res = window->Initialize();
    if (FAILED(res))
    {
        MessageBox(
            nullptr,
            L"RegisterClass failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        
        return;
    }
    
    if (!window->Create(
        windowNameW.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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
    
    SetJavaWindowPtr(env, thisObj, reinterpret_cast<LONG_PTR>(window));
    
    // std::cout << "[Native] Window Handle Set window: " << &window << std::endl;
}

JNIEXPORT void JNICALL Java_java_1window4_java_Window_showWindow
    (JNIEnv *env, jobject thisObj)
{
    auto *pThis = reinterpret_cast<JavaWindow *>(GetJavaWindowPtr(env, thisObj));
    
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