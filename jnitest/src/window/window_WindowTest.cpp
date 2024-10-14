#ifndef UNICODE
#define UNICODE
#endif

#include <jni.h>
#include <windows.h>
#include <string>

#include "window_WindowTest.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

std::wstring jstringToWstring(JNIEnv *env, jstring jStr)
{
    std::wstring wStr;
    
    const jchar *jChars = env->GetStringChars(jStr, nullptr);
    jsize length = env->GetStringLength(jStr);
    
    wStr.assign(jChars, jChars + length);
    
    env->ReleaseStringChars(jStr, jChars);
    
    return wStr;
}

JNIEXPORT void JNICALL Java_window_WindowTest_CreateWindow
    (JNIEnv *env, jobject jObj, jstring jStr)
{
    const std::wstring TITLE = jstringToWstring(env, jStr);
    const wchar_t CLASS_NAME[] = L"sample window class from jni";
    
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    WNDCLASS wc = {};
    
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = CLASS_NAME;
    
    ATOM res = RegisterClass(&wc);
    if (res == 0)
    {
        MessageBox(
            nullptr,
            L"RegisterClass failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        
        return;
    }
    
    hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        TITLE.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInstance,
        nullptr);
    if (hwnd == nullptr)
    {
        MessageBox(
            nullptr,
            L"CreateWindow failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        
        return;
    }
}

JNIEXPORT void JNICALL Java_window_WindowTest_ShowWindow
    (JNIEnv *env, jobject jObj)
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        
        EndPaint(hwnd, &ps);
        
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}