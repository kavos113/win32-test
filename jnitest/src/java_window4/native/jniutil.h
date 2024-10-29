#ifndef WIN32_TEST_JNIUTIL_H
#define WIN32_TEST_JNIUTIL_H

#include <jni.h>
#include <string>
#include <windows.h>

std::wstring JstringToWstring(JNIEnv *env, jstring jstr);
LONG_PTR GetJavaWindowPtr(JNIEnv *env, jobject thisObj, const char *fieldName = "nativeWindow");
void SetJavaWindowPtr(JNIEnv *env, jobject thisObj, LONG_PTR ptr, const char *fieldName = "nativeWindow");

#endif //WIN32_TEST_JNIUTIL_H
