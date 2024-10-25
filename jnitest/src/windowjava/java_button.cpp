#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>

#include "java_button.h"
#include "util.h"

LRESULT JavaButton::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
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

JNIEXPORT void JNICALL Java_windowjava_Button_createButton
    (JNIEnv *env, jobject thisObj, jstring buttonText)
{
    JavaButton button;
    
    std::wstring buttonTextW = JstringToWstring(env, buttonText);
    
    if (!button.Create(
        buttonTextW.c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        100,
        100,
        nullptr,
        nullptr))
    {
        return;
    }
    
    ShowWindow(button.Window(), SW_SHOW);
    UpdateWindow(button.Window());
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}