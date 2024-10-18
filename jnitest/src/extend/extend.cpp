#include "extend_Main.h"

JNIEXPORT void JNICALL Java_extend_Main_nativeHello
    (JNIEnv *env, jclass thisCls, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jmethodID mid = env->GetMethodID(cls, "hello", "()Ljava/lang/String;");
    jstring str = (jstring)env->CallObjectMethod(obj, mid, NULL);
    const char *cstr = env->GetStringUTFChars(str, 0);
    printf("Call hello: %s\n", cstr);
    
    jclass superCls = env->GetSuperclass(cls);
    jmethodID superMid = env->GetMethodID(superCls, "hello", "()Ljava/lang/String;");
    jstring superStr = (jstring)env->CallObjectMethod(obj, superMid, NULL);
    const char *superCstr = env->GetStringUTFChars(superStr, 0);
    printf("Call super hello: %s\n", superCstr);
    
    jstring jstr = (jstring) env->CallNonvirtualObjectMethod(obj, superCls, superMid, NULL);
    const char *jstrCstr = env->GetStringUTFChars(jstr, 0);
    printf("Call nonvirtual hello: %s\n", jstrCstr);
}