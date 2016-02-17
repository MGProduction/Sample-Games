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
    int  hunt_timer,hunt_score;
} MAZE;

struct tagACTOR;
typedef int (*actionproc)(MAZE*M,struct tagACTOR*m);

typedef struct tagACTOR
{
    float      x,y;
    float      nx,ny;
    float      dir_x,dir_y;
    int        baseFrame,nFrames,curFrame;
    int        dir,moving,blink;
    int        movementmask;
    int        status;
    actionproc act;
} ACTOR;

typedef struct tagWORLD{
    // i/o
    int  keys;
    int  event,event_timer;
    // match vars
    int  score;
    int  lifes;
    MAZE M;
    // global var
    int  hiscore;
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

#define event_none     0
#define event_GETREADY 1
#define event_NEWLEVEL 2
#define event_LIVELEFT 3
#define event_GAMEOVER 4

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
