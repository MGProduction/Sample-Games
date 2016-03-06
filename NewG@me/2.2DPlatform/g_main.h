#ifndef G_MAIN_H_INCLUDED
#define G_MAIN_H_INCLUDED

#include "l_module.h"

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagMAP{
 char      *map;
 int        w,rw,h;
 float      sx,sy;
 float      ox,oy;
 float      tx,ty;
}MAP;

typedef struct tagINGAMESCENE{
 MAP       map;
 int       maptiles[256];
 int       maptilesmask[256];
 TEXATL    textID;
 Texture   backID;
}INGAMESCENE;

struct tagACTOR;
typedef int (*actionproc)(struct tagWORLD*w,INGAMESCENE*M,struct tagACTOR*m);

typedef struct tagACTOR
{
    float      x,y;
    float      vy;
    float      nx,ny;
    float      dir_x,dir_y;
    int        baseFrame,nFrames,curFrame;
    int        tick,animtick,animtickcnt;
    char       anim,layer;
    int        dir,moving,blink;
    int        movementmask;
    int        status,statustimer;
    actionproc act;
} ACTOR;

typedef struct tagWORLD{
    // i/o
    int  keys;
    int  event,event_timer;
    // match vars
    int  tempscore,score;
    int  lifes;
    int  world,area;
    // global var
    int  hiscore;
}WORLD;

typedef int (*gameloopproc)(WORLD*W);

typedef struct tagGAMELOOP{
    gameloopproc enter;
    gameloopproc leave;
    gameloopproc handleUI;
    gameloopproc action;
    gameloopproc postaction;
    gameloopproc draw;
    gameloopproc backaction;
}GAMELOOP;

// ---------------------------------------------
// - DEFINES
// ---------------------------------------------

#define dirUP     1
#define dirDOWN   2
#define dirLEFT   4
#define dirRIGHT  8
#define dirJUMP   16

#define event_none     0
#define event_GETREADY 1
#define event_NEWLEVEL 2
#define event_LIVELEFT 3
#define event_GAMEOVER 4
#define event_LEVELCOMPLETED 5

#if defined(OS_MAC) || defined(OS_LINUX)
#define GLFW_KEY_RIGHT              262
#define GLFW_KEY_LEFT               263
#define GLFW_KEY_DOWN               264
#define GLFW_KEY_UP                 265
#define GLFW_KEY_SPACE              32
#endif

// ---------------------------------------------
// - GLOBALS
// ---------------------------------------------

extern WORLD g_W;


// ---------------------------------------------
// - MAIN FUNCTIONS
// ---------------------------------------------

int  GAME_init(WORLD*w,const char*res_basepath,float width,float height);
void GAME_loop(WORLD*W);
int  GAME_reset(WORLD*w);

// ---------------------------------------------

int    DIAMOND_action(WORLD*w,INGAMESCENE*S,ACTOR*a);
int    WORM_action(WORLD*w,INGAMESCENE*S,ACTOR*a);
int    PLAYER_action(WORLD*w,INGAMESCENE*S,ACTOR*a);

// ---------------------------------------------


#endif // G_MAIN_H_INCLUDED
