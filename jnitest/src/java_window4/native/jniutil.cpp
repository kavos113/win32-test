#include "jniutil.h"

std::wstring JstringToWstring(JNIEnv *env, jstring jstr) {
    std::wstring wstr;
    
    const jchar *jchars = env->GetStringChars(jstr, nullptr);
    jsize length = env->GetStringLength(jstr);
    
    wstr.assign(jchars, jchars + length);
    
    env->ReleaseStringChars(jstr, jchars);
    
    return wstr;
}

LONG_PTR GetJavaWindowPtr(JNIEnv *env, jobject thisObj, const char *fieldName) {
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(thisClass, fieldName, "J");
    return reinterpret_cast<LONG_PTR>(
        env->GetLongField(thisObj, nativeWindowFieldID)
        );
}

void SetJavaWindowPtr(JNIEnv *env, jobject thisObj, LONG_PTR ptr, const char *fieldName) {
    jclass thisClass = env->GetObjectClass(thisObj);
    jfieldID nativeWindowFieldID = env->GetFieldID(thisClass, fieldName, "J");
    env->SetLongField(thisObj, nativeWindowFieldID, static_cast<jlong>(ptr));
}