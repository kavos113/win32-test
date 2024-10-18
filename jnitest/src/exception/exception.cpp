#include <iostream>
#include "exception_Main.h"

JNIEXPORT void JNICALL Java_exception_Main_nativeThrowException
    (JNIEnv *env, jclass cls, jstring message)
{
    std::cerr << "Throwing exception in native code" << std::endl;
    
    jclass newExcCls = env->FindClass("java/lang/IllegalArgumentException");
    jmethodID mid = env->GetMethodID(newExcCls, "<init>", "()V");
    
    jthrowable obj = (jthrowable) env->NewObject(newExcCls, mid);
    
    env->Throw(obj);
    
    std::cerr << "Exception thrown in native code" << std::endl;
}

JNIEXPORT void JNICALL Java_exception_Main_nativeThrowNewException
    (JNIEnv *env, jclass cls)
{
    std::cerr << "Throwing new exception in native code" << std::endl;
    
    jclass newExcCls = env->FindClass("java/lang/IllegalArgumentException");
    
    env->ThrowNew(newExcCls, "New exception thrown in native code");
    
    std::cerr << "New exception thrown in native code" << std::endl;
}