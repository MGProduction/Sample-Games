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
extern float os_gfxratio;
extern int   os_BACK_pressed;
extern int   os_PAUSE;


extern int   os_tris_3d,os_tris_2d;

#define                            GFX3D_CACHE 2048

extern float           gfx_tex[2*3*GFX3D_CACHE];
extern float           gfx_texM[2*3*GFX3D_CACHE];
extern float           gfx_ver[3*3*GFX3D_CACHE];
extern unsigned char   gfx_light[4*3*GFX3D_CACHE];

// ---------------------------------------------

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagAREA2D{
    float x,y;
    float w,h;
}AREA2D;

typedef struct tagTexture{
 int textID;
 int sx,sy;
}Texture;

typedef struct TEXATLI{
 float x,y,w,h;
 float bx,by,rw,rh;
 char  nm[64];
}TEXATLI;

typedef struct TEXATL{
 Texture texID;
 int     cnt;
 TEXATLI*tai;
}TEXATL;

typedef struct{
    float xm,ym,zm;
    float xM,yM,zM;
}AABB;

// ---------------------------------------------

typedef struct
{
 float x,y,z;
}vector3;

typedef struct
{
 vector3 m_vPosition;					
	vector3 m_vView;						
	vector3 m_vUpVector;			
	vector3 m_vStrafe;						
}camera3D;

typedef struct tagcoord3D{
 vector3 pos;
 vector3 rot;
 float   scale;
}coord3D;

// ---------------------------------------------

typedef struct {
    float vm[16];             
    float vmi[16];            
    float vx,vy,vz;           
    float frust_planes[6][4]; 
    float frust_points[8][3]; 
    float q_pix;              
    int   setcnt;             
}frustum3D;

// ---------------------------------------------

typedef struct{
 float*vert;
 float*tex;
 int   tris;
}quickMESH;

typedef struct{
 char*  name;
 float**frame;
 int    nframes;
 int    ms;
}anims;

typedef struct{
 float*   tex;
 int      tris;
 // -------------
 anims*   anims;
 int      nanims;
}simpleMESH;


// ---------------------------------------------

// ---------------------------------------------
// - DEFINES
// ---------------------------------------------

#define touchNONE 0
#define touchDOWN 1
#define touchMOVE 2
#define touchUP   3

#define isbetween(a,b,c) (((a)>=(b))&&((a)<=(c)))
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
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
int    AREA2D_intersect(AREA2D*a,AREA2D*b);

// ---------------------------------------------

// ---------------------------------------------
// - TEXTURE
// ---------------------------------------------

Texture tex_core_create(int width,int height,void*data,int mode);

// ---------------------------------------------

Texture tex_readTGA(const char*s,int mode);
void    tex_delete(Texture*textID);

// ---------------------------------------------

int     TEXATL_read(const char*name,TEXATL*t);
int     TEXATL_delete(TEXATL*t);
int     TEXATL_find(TEXATL*t,const char*name);
int     TEXATL_findanim(TEXATL*t,const char*name,int*nframes);

// ---------------------------------------------



// ---------------------------------------------
// - QUICKMESH
// ---------------------------------------------

int    quickmesh_readMD2(quickMESH*mesh,const char*s,Texture*textID/*NULL*/);
int    quickmesh_delete(quickMESH*mesh);

int    quickmesh_draw(quickMESH*mesh,Texture*textID,coord3D*obj);

void   quickmesh_getAABB(quickMESH*mesh,AABB*aabb);

// ---------------------------------------------

int    AABB_interset(AABB*a1,coord3D*c1,AABB*a2,coord3D*c2);

// ---------------------------------------------

void   vector3_new(vector3*th,float X, float Y, float Z);
void   vector3_add(vector3*th,vector3*v);
void   vector3_sub(vector3*th,vector3*v);
void   vector3_mul(vector3*th,float num);
void   vector3_div(vector3*th,float num);
void   vector3_Cross(vector3*vNormal,vector3*vVector1,vector3*vVector2);
float  vector3_Magnitude(vector3*vNormal);
void   vector3_Normalize(vector3*vVector);
// ---------------------------------------------


// ---------------------------------------------
// - CAMERA
// ---------------------------------------------

void   gfx_LookAt(float eyex, float eyey, float eyez,
                float centerx, float centery, float centerz,
                float upx, float upy, GLfloat upz);

void   camera3D_new(camera3D*th);
void   camera3D_setPOSITION(camera3D*th,float positionX, float positionY, float positionZ,
				  		                                float viewX,     float viewY,     float viewZ,
							                                 float upVectorX, float upVectorY, float upVectorZ);
void   camera3D_lookAT(camera3D*th);

// ---------------------------------------------

// ---------------------------------------------
// - FRUSTUM
// ---------------------------------------------

void   frustum3D_setVIEW(frustum3D*fr,float pxsize,float pysize);
int    frustum3D_checkAABB(frustum3D*fr,AABB*aabb);

// ---------------------------------------------

// ---------------------------------------------
// - GRAPHIC
// ---------------------------------------------

int    gfx_init();
int    gfx_reset();

void   gfx_enable2D(int enable);
void   gfx_enable3D(int enable);

void   gfx_cleanSCREEN();

void   gfx_drawBOX(float x,float y,float w,float h,float r,float g,float b,float alpha);
void   gfx_drawSPRITE(float x,float y,float w,float h,Texture*textID,float tx,float ty,float tw,float th,float alpha,float scale,int dir);
int    gfx_drawSPRITETEXATL(float x,float y,TEXATL*t,int id,float ratio,float alpha,int dir);

// ---------------------------------------------

void   gfx_enablefog(int enable,float r,float g,float b,float start,float end);

// ---------------------------------------------

void   gfx_getPROJECTIONMATRIX(float*proj);
void   gfx_getMODELVIEWMATRIX(float*modl);

// ---------------------------------------------

void   gfx_test3D(quickMESH*mesh,int textID);

// ---------------------------------------------

// ---------------------------------------------
// - SOUND
// ---------------------------------------------

int    wav_read(const char*s,int mode);
int    wav_delete(int*waveID);

// ---------------------------------------------

int    snd_init();
int    snd_reset();

int    snd_play(int waveID,int loop,int mode);
int    snd_stop(int playwaveID);
int    snd_pause(int playwaveID);

// ---------------------------------------------


#endif // L_MODULE_H_INCLUDED
