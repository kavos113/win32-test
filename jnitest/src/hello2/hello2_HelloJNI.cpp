#include <jni.h>
#include <iostream>

#include "hello2_HelloJNI.h"

JNIEXPORT void JNICALL Java_hello2_HelloJNI_sayHello
  (JNIEnv *env, jobject thisObj, jstring name)
{
    const char *nameChar = env->GetStringUTFChars(name, nullptr);
    std::cout << "Hello " << nameChar << "!" << std::endl;
    env->ReleaseStringUTFChars(name, nameChar);
}