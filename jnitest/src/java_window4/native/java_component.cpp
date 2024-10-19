#include <jni.h>

#include "java_component.h"

JNIEXPORT void JNICALL Java_java_1window4_java_Component_reshape
    (JNIEnv *env, jobject thisObj, jint x, jint y, jint width, jint height, jint operation)
{
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID hwndFieldID = env->GetFieldID(thisClass, "hwnd", "J");
    HWND hwnd = reinterpret_cast<HWND>(
        static_cast<LONG_PTR>(
            env->GetLongField(thisObj, hwndFieldID)
            )
        );
    
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