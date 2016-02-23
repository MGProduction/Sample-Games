#ifndef _ANDROID_JNI_H_
#define _ANDROID_JNI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* JNI Includes */
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_initGame(JNIEnv*, jclass, jstring, jstring, jint, jint, jint);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_setOrientationGame(JNIEnv*, jclass, jint, jint, jint, jint);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_drawGame(JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_destroyGame(JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_pauseGame(JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_resumeGame(JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_restoreGameTextures(JNIEnv*, jclass);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_keyEvent(JNIEnv*, jclass, jint, jint);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_touchEvent(JNIEnv*, jclass, jint, jint, jint, jint);
JNIEXPORT void JNICALL Java_com_marcogiorgini_World2D_GameJNI_accelEvent(JNIEnv *, jclass, jfloat, jfloat, jfloat);

#ifdef __cplusplus
}
#endif

void jni_exit_app(int code);
void jni_open_url(const char* url);

#endif // _ANDROID_JNI_H_
