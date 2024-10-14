#ifndef UNICODE
#define UNICODE
#endif

#include <jni.h>
#include <windows.h>
#include <string>

#include "windowjava_Component.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

HINSTANCE hInstance = GetModuleHandle(nullptr);

std::wstring jstringToWstring(JNIEnv *env, jstring jStr)
{
    std::wstring wStr;
    
    const jchar *jChars = env->GetStringChars(jStr, nullptr);
    jsize length = env->GetStringLength(jStr);
    
    wStr.assign(jChars, jChars + length);
    
    env->ReleaseStringChars(jStr, jChars);
    
    return wStr;
}

JNIEXPORT void JNICALL Java_windowjava_Component_CreateWindowClass
  (JNIEnv *env, jclass cls, jstring className)
{
    const std::wstring CLASS_NAME = jstringToWstring(env, className);
    
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
    wc.lpszClassName = CLASS_NAME.c_str();
    
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
}

JNIEXPORT void JNICALL Java_windowjava_Component_CreateComponent
  (JNIEnv *env, jclass cls, jstring className, jstring windowName)
{
    hwnd = CreateWindowEx(
        0,
        jstringToWstring(env, className).c_str(),
        jstringToWstring(env, windowName).c_str(),
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
            L"CreateWindowEx failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        
        return;
    }
    
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
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
            
            EndPaint(hwnd, &ps);
            
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}