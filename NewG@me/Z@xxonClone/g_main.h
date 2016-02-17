#ifndef G_MAIN_H_INCLUDED
#define G_MAIN_H_INCLUDED

#include "l_module.h"

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagTOWERS{
 float x,y,h,w;
}TOWERS;

typedef struct tagACTOR3D
{
    coord3D    obj;
    quickMESH* mesh;
    AABB       aabb;
    Texture    textID;
    int        status;
} ACTOR3D;

typedef struct tagWORLD{
    // i/o
    int       keys;
    int       event,event_timer;
    // 3D vars
    camera3D  camera;
    frustum3D frustum;
    float     hview;
    ACTOR3D   ship;    
    TOWERS    towers[8];
    int       itowers;
    // match vars
    int       score;
    int       game_start;
    float     speed;
    // global var
    int       hiscore;
}WORLD;


typedef int (*gameloopproc)(WORLD*W);

typedef struct tagGAMELOOP{
    gameloopproc handleUI;
    gameloopproc action;
    gameloopproc postaction;
    gameloopproc draw;
}GAMELOOP;

// ---------------------------------------------
// - DEFINES
// ---------------------------------------------

#define dirUP     1
#define dirDOWN   2
#define dirLEFT   3
#define dirRIGHT  4

#define event_none      0
#define event_GETREADY  1
#define event_EXPLOSION 2
#define event_GAMEOVER  3

#define status_collision         1
#define status_collision_handled 2

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

int  GAME_restore_textures(WORLD*w);

#endif // G_MAIN_H_INCLUDED
