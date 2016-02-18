#ifndef CONST_H_INCLUDED
#define CONST_H_INCLUDED

#if defined(WIN32)
#define OS_WIN32
#elif defined(MAC)
#define OS_MAC
#elif defined(ANDROID_NDK)
#define OS_ANDROID
#else
#define OS_IPHONE
#endif

#define GFX_OPENGL
#define SND_OPENAL

#define HAVE_STBIMAGE

#endif // CONST_H_INCLUDED
