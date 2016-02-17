#include "g_main.3.h"

WORLD    g_W;

int      g_textID;

char orig_maze[11][20]= { "11111111111111111111",
                          "1.o..1........1..o.1",
                          "1.11.1.111111.1.11.1",
                          "1.1..............1.1",
                          "1.1.11.11  11.11.1.1",
                          "1......1ABCD1......1",
                          "1.1.11.111111.11.1.1",
                          "1.1.......*......1.1",
                          "1.11.1.111111.1.11.1",
                          "1.o..1........1..o.1",
                          "11111111111111111111"
                        };

// --------------------------------------------------------------------
//
//   DRAW FUNCTIONS
//
// --------------------------------------------------------------------

void drawMAZE(WORLD*w)
{
    int   msx=20,msy=11;
    float x,y;
    float sx=os_video_w/msx;
    float sy=os_video_h/msy;
    for(y=0; y<sy; y++)
        for(x=0; x<sx; x++)
        {
            float px=x*sx,py=y*sy;
            switch(orig_maze[(int)y][(int)x])
            {
            case '1':
                gfx_drawBOX(px,py,sx,sy,0,0,0.5f,1);
                break;
            case '.':
                gfx_drawBOX(px+sx/2-sx/10,py+sy/2-sx/10,sx/5,sx/5,1,1,1,1);
                break;
            case 'o':
                gfx_drawBOX(px+sx/2-sx/4,py+sy/2-sx/4,sx/2,sx/2,1,1,0,1);
                break;
            }

        }
}

int INGAME_handleUI(WORLD*w)
{
 return 1;
}

int INGAME_action(WORLD*w)
{

 return 1;
}

int INGAME_draw(WORLD*w)
{
    gfx_cleanSCREEN();

    drawMAZE(w);

    return 1;
}

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
 INGAME_handleUI(w);
 INGAME_action(w);
 INGAME_draw(w);
}
