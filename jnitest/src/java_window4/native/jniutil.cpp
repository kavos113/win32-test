#include "jniutil.h"

std::wstring JstringToWstring(JNIEnv *env, jstring jstr) {
    std::wstring wstr;
    
    const jchar *jchars = env->GetStringChars(jstr, nullptr);
    jsize length = env->GetStringLength(jstr);
    
    wstr.assign(jchars, jchars + length);
    
    env->ReleaseStringChars(jstr, jchars);
    
    return wstr;
}