#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "android_jni.h"
#include "android_main.h"

#define JNI_GAME_CLASS "com/marcogiorgini/PacMan/GameJNI"

/* Callbacks to Android */
jclass jNativesCls = NULL;
jmethodID jexit_app = NULL;
jmethodID jopen_url = NULL;

/* Containts the path to /data/data/(package_name)/libs */
//static char* lib_dir=NULL;

static JavaVM *jVM = NULL;
static char* data_dir = NULL;
static char* save_dir = NULL;

#define TRACE_DEBUG

static void android_log_error(const char* err_string)
{
	__android_log_print(ANDROID_LOG_ERROR, "android_jni", err_string);
}

static int neon_support()
{
    char buf[80];
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if(!fp)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "android_jni", "Unable to open /proc/cpuinfo\n");
        return 0;
    }

    while(fgets(buf, 80, fp) != NULL)
    {
        char *features = strstr(buf, "Features");

        if(features)
        {
            char *feature;
            features += strlen("Features");
            feature = strtok(features, ": ");
            while(feature)
            {
                if(!strcmp(feature, "neon"))
                    return 1;

                feature = strtok(NULL, ": ");
            }
            return 0;
        }
    }
    return 0;
}

int JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv *env;
	jVM = vm;

#ifdef TRACE_DEBUG
	__android_log_print(ANDROID_LOG_DEBUG, "android_jni", "JNI_OnLoad called");
#endif

	if((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		__android_log_print(ANDROID_LOG_ERROR, "android_jni", "Failed to get the environment using GetEnv()");
		return -1;
	}
	return JNI_VERSION_1_4;
}

//////////////////////////////////////////////
JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_initGame(JNIEnv* env, jclass c, jstring data_path, jstring save_path, jint width, jint height, jint mode)
{
	jboolean iscopy;
	const jbyte* path1;
	const jbyte* path2;
		
	path1 = (*env)->GetStringUTFChars(env, data_path, &iscopy);
	data_dir = strdup(path1);
	(*env)->ReleaseStringUTFChars(env, data_path, path1);
	
	path2 = (*env)->GetStringUTFChars(env, save_path, &iscopy);
	save_dir = strdup(path2);
	(*env)->ReleaseStringUTFChars(env, save_path, path2);
	
	
	jNativesCls = (*env)->FindClass(env, JNI_GAME_CLASS);
	if (!jNativesCls) 
	{
		android_log_error("Unable to find jni class");
	  return;
	}
	
	jexit_app = (*env)->GetStaticMethodID(env, jNativesCls, "exit_app", "(I)V");
	if (!jexit_app)
	{
		android_log_error("Unable to find method exit_app");
		return;
	}

	jopen_url = (*env)->GetStaticMethodID(env, jNativesCls, "open_url", "(Ljava/lang/String;)V");
	if (!jopen_url)
	{
		android_log_error("Unable to find method open_url");
		return;
	}

	game_init(data_dir, save_dir, width, height, mode);	
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_setOrientationGame(JNIEnv* env, jclass c, jint portrait, jint width, jint height,  jint flip)
{
	game_set_orientation(portrait, width, height, flip);	
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_drawGame(JNIEnv* env, jclass c)
{
	game_draw();
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_destroyGame(JNIEnv* env, jclass c)
{
	game_destroy();
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_pauseGame(JNIEnv* env, jclass c)
{
	game_pause();
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_resumeGame(JNIEnv* env, jclass c)
{
	game_resume();
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_restoreGameTextures(JNIEnv* env, jclass c)
{
	game_restore_textures();
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_keyEvent(JNIEnv* env, jclass c, jint key, jint state)
{
	game_key_event(key, state);
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_touchEvent(JNIEnv* env, jclass c, jint id, jint action, jint x, jint y)
{
	game_touch_event(id, action, x, y);
}

JNIEXPORT void JNICALL Java_com_marcogiorgini_PacMan_GameJNI_accelEvent(JNIEnv* env, jclass c, jfloat x, jfloat y, jfloat z)
{
	game_accel_event(x, y, z);
}

/////////////////////////////////////////////////////////////
// system callbacks
/////////////////////////////////////////////////////////////
void jni_exit_app(int code)
{
	JNIEnv* env;
	(*jVM)->AttachCurrentThread(jVM, (JNIEnv**)&env, NULL);
	(*env)->CallStaticVoidMethod(env, jNativesCls, jexit_app, code);
}

void jni_open_url(const char* url)
{
	JNIEnv* env;
	(*jVM)->AttachCurrentThread(jVM, (JNIEnv**)&env, NULL);
	(*env)->CallStaticVoidMethod(env, jNativesCls, jopen_url, (*env)->NewStringUTF(env, url));
}

