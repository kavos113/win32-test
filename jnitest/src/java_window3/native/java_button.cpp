#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <string>
#include <iostream>

#include "java_button.h"
#include "util.h"

JavaButton button;

LRESULT JavaButton::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::cout << "[Native] JavaButton::HandleMessage" << std::endl;
    std::cout << "[Native] uMsg: " << uMsg << std::endl;
    switch (uMsg)
    {
    case WM_COMMAND:
        MessageBox(
            nullptr,
            L"Button clicked",
            L"Button",
            MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }
    
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

JNIEXPORT void JNICALL Java_java_1window3_java_Button_create
    (JNIEnv *env, jobject thisObj, jobject parent, jstring text)
{
    std::cout << "[Native] Java_java_1window3_java_Button_create" << std::endl;
    
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
    
    std::cout << "[Native] Parent hwnd: " << parentHwnd << std::endl;
    
    std::wstring textW = JstringToWstring(env, text);
    
    if (!button.Create(
        textW.c_str(),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0,
        10,
        10,
        100,
        30,
        parentHwnd,
        nullptr
    ))
    {
        MessageBox(
            nullptr,
            L"Button creation failed",
            L"Error",
            MB_ICONEXCLAMATION | MB_OK);
        return;
    }
    
    std::cout << "[Native] Button created" << std::endl;
    
    jclass clazz = env->GetObjectClass(thisObj);
    jfieldID hwndFieldID = env->GetFieldID(clazz, "hwnd", "J");
    env->SetLongField(thisObj, hwndFieldID, static_cast<jlong>(
        reinterpret_cast<LONG_PTR>(button.Window())
        ));
    
    std::cout << "[Native] Button hwnd: " << button.Window() << std::endl;
}