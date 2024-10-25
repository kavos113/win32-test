#ifndef WIN32_TEST_UTIL_H
#define WIN32_TEST_UTIL_H

#include <string>

std::wstring JstringToWstring(JNIEnv *env, jstring jstr);

#endif //WIN32_TEST_UTIL_H
