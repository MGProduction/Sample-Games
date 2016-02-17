#ifndef G_MAIN_H_INCLUDED
#define G_MAIN_H_INCLUDED

#include "l_module.h"

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagMAZE
{
    char maze[11][20];
    int  sx,sy;
    int  home_x,home_y;
    int  coins;
} MAZE;

typedef struct tagACTOR
{
    float      x,y;
    float      nx,ny;
    float      dir_x,dir_y,moving;
    int        baseFrame,nFrames,curFrame;
    int        dir;
} ACTOR;


typedef struct tagWORLD{
    int  keys;
    int  score;
    MAZE M;
}WORLD;

// ---------------------------------------------
// - DEFINES
// ---------------------------------------------

#define dirUP     1
#define dirDOWN   2
#define dirLEFT   3
#define dirRIGHT  4

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

#endif // G_MAIN_H_INCLUDED
