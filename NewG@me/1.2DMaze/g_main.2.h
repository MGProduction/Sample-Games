#ifndef G_MAIN_H_INCLUDED
#define G_MAIN_H_INCLUDED

#include "l_module.h"

// ---------------------------------------------
// - STRUCTURES
// ---------------------------------------------

typedef struct tagWORLD{

}WORLD;

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
