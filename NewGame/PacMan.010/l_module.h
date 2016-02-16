#ifndef L_MODULE_H_INCLUDED
#define L_MODULE_H_INCLUDED

#include "const.h"

#if defined(OS_WIN32)
#include <windows.h>
#endif

#if defined(OS_IPHONE)
typedef unsigned char BYTE;
#include <string.h>
#endif

#if defined(GFX_OPENGL)
#if defined(OS_WIN32)
#include <gl/gl.h>
#elif defined(OS_MAC)
#import <OpenGL/glu.h>
#elif defined(OS_ANDROID)
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef int            BOOL;
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
#include <GLES/gl.h>
#include <GLES/glext.h>
#elif defined(OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#endif
#endif

#if defined(SND_OPENAL)
#if defined(OS_WIN32)
#include "al/al.h"
#include "al/alc.h"
#include "al/alut.h"
#elif defined(OS_MAC)
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#elif defined(OS_ANDROID)
#elif defined(OS_IPHONE)
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ---------------------------------------------
// - GLOBALS
// ---------------------------------------------

extern float os_video_w,os_video_h;
extern float os_scale;
extern float os_roll,os_pitch,os_Z;
extern int   os_x[10],os_y[10],
             os_status[10],
             os_np;
extern char  os_szResPath[256];
extern int   os_portrait,
             os_flip,os_touch_flip;
extern int   os_BACK_pressed;
extern int   os_PAUSE;

// ---------------------------------------------

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagAREA2D{
    float x,y;
    float w,h;
}AREA2D;

// ---------------------------------------------

// ---------------------------------------------
// - DEFINES
// ---------------------------------------------

#define touchNONE 0
#define touchDOWN 1
#define touchMOVE 2
#define touchUP   3

// ---------------------------------------------

// ---------------------------------------------
// - TIMERS
// ---------------------------------------------

int    os_getMilliseconds(void);

// ---------------------------------------------
// - INIT/RESET
// ---------------------------------------------

void os_init(const char*szname,float width,float height);
void os_reset(void);

// ---------------------------------------------
// - I/O
// ---------------------------------------------

void*  os_readFILE(const char*szname,int*size);

// ---------------------------------------------

// ---------------------------------------------
// - 2D FUNCTIONS
// ---------------------------------------------

int    AREA2D_isinbound(float x,float y,AREA2D*area,float bound);

// ---------------------------------------------

// ---------------------------------------------
// - TEXTURE
// ---------------------------------------------

int    tex_core_create(int width,int height,void*data,int mode);

// ---------------------------------------------

int    tex_readTGA(const char*s,int mode);
void   tex_delete(int*textID);

// ---------------------------------------------


// ---------------------------------------------
// - GRAPHIC
// ---------------------------------------------

int   gfx_init();
int   gfx_reset();

void  gfx_enable2D(int enable);
void  gfx_cleanSCREEN();

void  gfx_drawBOX(float x,float y,float w,float h,float r,float g,float b,float alpha);
void  gfx_drawSPRITE(float x,float y,float w,float h,int textID,float tx,float ty,float tw,float th,float alpha,float scale,int dir);

// ---------------------------------------------

// ---------------------------------------------
// - SOUND
// ---------------------------------------------

int  wav_read(const char*s,int mode);
int  wav_delete(int*waveID);
void wav_deleteall();

// ---------------------------------------------

int  snd_init();
int  snd_reset();

int  snd_play(int waveID,int loop,int mode);
int  snd_stop(int playwaveID);
int  snd_pause(int playwaveID);

// ---------------------------------------------


#endif // L_MODULE_H_INCLUDED
