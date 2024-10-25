#ifndef WIN32_TEST_JNIUTIL_H
#define WIN32_TEST_JNIUTIL_H

#include <jni.h>
#include <string>

std::wstring JstringToWstring(JNIEnv *env, jstring jstr);

#endif //WIN32_TEST_JNIUTIL_H
