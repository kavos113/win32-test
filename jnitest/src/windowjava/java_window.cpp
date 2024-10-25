#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>

#include "java_window.h"
#include "util.h"

LRESULT JavaWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

JNIEXPORT void JNICALL Java_windowjava_Window_createWindow
    (JNIEnv *env, jobject thisObj, jstring windowName)
{
    JavaWindow window;
    
    std::wstring windowNameW = JstringToWstring(env, windowName);
    
    if (!window.Create(windowNameW.c_str(), WS_OVERLAPPEDWINDOW))
    {
        return;
    }
    
    ShowWindow(window.Window(), SW_SHOW);
    UpdateWindow(window.Window());
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}