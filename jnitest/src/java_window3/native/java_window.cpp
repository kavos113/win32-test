#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>
#include <iostream>

#include "java_window.h"
#include "util.h"

JavaWindow window;

LRESULT JavaWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        std::cout << "[Native] WM_DESTROY" << std::endl;
        return 0;
    
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
        
        EndPaint(m_hwnd, &ps);
    }
        return 0;
    
    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    
    return TRUE;
}

JNIEXPORT void JNICALL Java_java_1window3_java_Window_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring windowName)
{
    std::cout << "[Native] Java_java_1window3_java_Window_create" << std::endl;
    
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
    
    std::wstring windowNameW = JstringToWstring(env, windowName);
    
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
    
    std::cout << "[Native] Window hwnd: " << window.Window() << std::endl;
}

JNIEXPORT void JNICALL Java_java_1window3_java_Window_showWindow
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
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}