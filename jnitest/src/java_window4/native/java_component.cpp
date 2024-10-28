#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <jni.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "java_component.h"

LRESULT JavaComponent::ComponentHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

// color: 0xAARRGGBB
void JavaComponent::SetBackgroundColor(int color)
{
    std::cout << "R, G, B, A: " << ((color >> 16) & 0xFF) << ", " << ((color >> 8) & 0xFF) << ", " << (color & 0xFF) << ", " << ((color >> 24) & 0xFF) << std::endl;
    
    m_backgroundColor = D2D1::ColorF(
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        (color & 0xFF) / 255.0f,
        ((color >> 24) & 0xFF) / 255.0f
        );
}

JNIEXPORT void JNICALL Java_java_1window4_java_Component_reshape
    (JNIEnv *env, jobject thisObj, jint x, jint y, jint width, jint height, jint operation)
{
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(thisClass, "nativeWindow", "J");
    JavaComponent *pThis = reinterpret_cast<JavaComponent*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
            )
        );
    
    HWND hwnd = pThis->Window();
    
    switch (operation)
    {
    case java_window4_java_Component_SET_POSITION:
        SetWindowPos(
            hwnd,
            nullptr,
            x,
            y,
            0,
            0,
            SWP_NOSIZE | SWP_NOZORDER
            );
        break;
        
    case java_window4_java_Component_SET_SIZE:
        SetWindowPos(
            hwnd,
            nullptr,
            0,
            0,
            width,
            height,
            SWP_NOMOVE | SWP_NOZORDER
            );
        break;
        
    case java_window4_java_Component_SET_POSITION_AND_SIZE:
        SetWindowPos(
            hwnd,
            nullptr,
            x,
            y,
            width,
            height,
            SWP_NOZORDER
            );
        break;
    }
}

JNIEXPORT void JNICALL Java_java_1window4_java_Component_setBackgroundColor
    (JNIEnv *env, jobject thisObj, jint color)
{
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(thisClass, "nativeWindow", "J");
    JavaComponent *pThis = reinterpret_cast<JavaComponent*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
            )
        );
    
    pThis->SetBackgroundColor(color);
    SendMessage(pThis->Window(), WM_PAINT, 0, 0);
}

JNIEXPORT void JNICALL Java_java_1window4_java_Component_destroy
    (JNIEnv *env, jobject thisObj)
{
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(thisClass, "nativeWindow", "J");
    JavaComponent *pThis = reinterpret_cast<JavaComponent*>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, nativeWindowFieldID)
            )
        );
    
    if (pThis != nullptr)
    {
        delete pThis;
    }
}