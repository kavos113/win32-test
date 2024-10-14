#include <jni.h>
#include <iostream>

#include "hello_Hello.h"

JNIEXPORT void JNICALL Java_hello_Hello_sayHello
  (JNIEnv *env, jobject obj) 
{
    std::cout << "Hello, World from C++" << std::endl;
}