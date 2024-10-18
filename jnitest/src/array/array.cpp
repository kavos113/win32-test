#include <iostream>
#include "array_Main.h"

JNIEXPORT jobjectArray JNICALL Java_array_Main_newArray
    (JNIEnv *env, jclass thisCls, jint size)
{
    char buf[16];
    
    jclass clazz = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(size, clazz, NULL);
    std::cout << "Creating String Array in native code" << std::endl;
    
    for (int i = 0; i < size; i++)
    {
        sprintf(buf, "String %d", i);
        jstring str = env->NewStringUTF(buf);
        env->SetObjectArrayElement(array, i, str);
        env->DeleteLocalRef(str);
    }
    
    return array;
}

JNIEXPORT void JNICALL Java_array_Main_printArray
    (JNIEnv *env, jclass thisCls, jobjectArray array)
{
    jint size = env->GetArrayLength(array);
    std::cout << "Printing String Array in native code" << std::endl;
    
    for (int i = 0; i < size; i++)
    {
        jstring str = (jstring) env->GetObjectArrayElement(array, i);
        const char *cstr = env->GetStringUTFChars(str, 0);
        std::cout << "The " << i << "th element is: " << cstr << std::endl;
        env->ReleaseStringUTFChars(str, cstr);
    }
}