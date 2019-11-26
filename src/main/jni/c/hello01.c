
#include <jni.h>
#include <stdio.h>

JNIEXPORT void JNICALL
Java_HelloWorld01_print(JNIEnv *env, jobject obj)
{
printf("Hello01 World!\n");
return;
}