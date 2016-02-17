#include "g_main.2.h"

WORLD    g_W;

// --------------------------------------------------------------------
//
//   GAME
//
// --------------------------------------------------------------------

int GAME_init(WORLD*w,const char*res_basepath,float width,float height)
{
    strcpy(os_szResPath,res_basepath);
    os_video_w=width;
    os_video_h=height;

    gfx_init();
    snd_init();

    gfx_enable2D(1);

    return 1;
}

int GAME_reset(WORLD*w)
{

    gfx_enable2D(0);

    snd_reset();
    gfx_reset();
    return 1;
}

void GAME_loop(WORLD*w)
{

}
