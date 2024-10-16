#include "access_Java.h"

JNIEXPORT void JNICALL Java_access_Java_setPrivate
    (JNIEnv *env, jobject thisObj, jint value)
{
    jclass cls = env->GetObjectClass(thisObj);
    jfieldID fid = env->GetFieldID(cls, "intPrivate", "I");
    env->SetIntField(thisObj, fid, value);
}

JNIEXPORT void JNICALL Java_access_Java_setStaticPrivate
    (JNIEnv *env, jobject thisObj, jint value)
{
    jclass cls = env->GetObjectClass(thisObj);
    jfieldID fid = env->GetStaticFieldID(cls, "intStaticPrivate", "I");
    env->SetStaticIntField(cls, fid, value);
}