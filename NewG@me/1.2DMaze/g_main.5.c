#include "g_main.5.h"

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

float  l_dirX[5]= {0,0,0,-1,1};
float  l_dirY[5]= {0,-1,1,0,0};

int    g_nActors=5;
ACTOR  g_actors[5];
ACTOR* g_player;

int MATCH_init(WORLD*w,int way)
{
    int x,y;
    memset(&g_actors,0,sizeof(g_actors));
    g_player=&g_actors[0];
    if(way==1)
    {
        w->score=0;
    }
    if(way)
    {
        memset(&w->M,0,sizeof(w->M));
        w->M.coins=0;
        w->M.sx=20;
        w->M.sy=11;
        memcpy(w->M.maze,orig_maze,sizeof(orig_maze));
    }
    for(y=0; y<w->M.sy; y++)
        for(x=0; x<w->M.sx; x++)
        {
            char ch=w->M.maze[y][x];
            if((ch=='.')||(ch=='o'))
            {
                if(way)
                    w->M.coins++;
            }
            else if(ch=='*')
            {
                int   wh=0;
                ACTOR*a=&g_actors[wh];
                a->x=a->nx=x;
                a->y=a->ny=y;
                a->baseFrame=0;
                a->nFrames=3;
                a->dir=dirLEFT;
            }
            else if((ch>='A')&&(ch<='D'))
            {
                int   wh=1+(ch-'A');
                ACTOR*a=&g_actors[wh];
                if(ch=='B')
                {
                    w->M.home_x=x;
                    w->M.home_y=y;
                }
                a->x=a->nx=x;
                a->y=a->ny=y;
                a->baseFrame=8+wh*8;
                a->nFrames=2;
                a->dir=wh;
            }
        }
    return 1;
}

// --------------------------------------------------------------------
//
//   GFX RESOURCE LOAD/UNLOAD
//
// --------------------------------------------------------------------

int GFXRES_init()
{
    g_textID=tex_readTGA("sprite.tga",0);
    return (g_textID!=-1);
}

int GFXRES_reset()
{
    tex_delete(&g_textID);
    return 1;
}

// --------------------------------------------------------------------
//
//   ACTOR ACTION HELPERS
//
// --------------------------------------------------------------------

int getavailableDIRS(MAZE*M,ACTOR*m,int*dir,int*mask,int check)
{
    int x=(int)m->x,y=(int)m->y,ndirs=0;
    if(mask) *mask=0;
    if(x&&M->maze[y][x-1]!='1')
    {
        if((check==0)||(m->dir!=dirRIGHT))
        {
            if(dir) dir[ndirs++]=dirLEFT;
            if(mask) *mask|=1;
        }
    }
    if((x+1<M->sx)&&(M->maze[y][x+1]!='1'))
    {
        if((check==0)||(m->dir!=dirLEFT))
        {
            if(dir) dir[ndirs++]=dirRIGHT;
            if(mask) *mask|=2;
        }
    }
    if(y&&M->maze[y-1][x]!='1')
    {
        if((check==0)||(m->dir!=dirDOWN))
        {
            if(dir) dir[ndirs++]=dirUP;
            if(mask) *mask|=4;
        }
    }
    if((y+1<M->sy)&&(M->maze[y+1][x]!='1'))
    {
        if((check==0)||(m->dir!=dirUP))
        {
            if(dir) dir[ndirs++]=dirDOWN;
            if(mask) *mask|=8;
        }
    }
    return ndirs;
}

int chooseDIR(MAZE*M,ACTOR*m)
{
    int dirs[4],ndirs=getavailableDIRS(M,m,dirs,NULL,1);
    if(ndirs)
        return dirs[rand()%ndirs];
    else
        return 0;
}

// --------------------------------------------------------------------
//
//   DRAW FUNCTIONS
//
// --------------------------------------------------------------------

void drawMAZE(WORLD*w)
{
    float x,y;
    float sx=os_video_w/w->M.sx;
    float sy=os_video_h/w->M.sy;
    for(y=0; y<w->M.sy; y++)
        for(x=0; x<w->M.sx; x++)
        {
            float px=x*sx,py=y*sy;
            switch(w->M.maze[(int)y][(int)x])
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

void drawACTORS(WORLD*w)
{
    int i;
    float sx=os_video_w/w->M.sx;
    float sy=os_video_h/w->M.sy;
    for(i=0; i<g_nActors; i++)
    {
        ACTOR*a=&g_actors[i];

        float px=a->x*sx,py=a->y*sy;
        int   curframe=a->baseFrame+(a->dir-1)*a->nFrames+a->curFrame;
        float fy=(curframe/8)*32.0f,fx=(curframe%8)*32.0f;
        gfx_drawSPRITE(px,py,sx,sy,g_textID,fx,fy,32,32,1,1,0);
        a->curFrame++;
        if(a->curFrame>=a->nFrames)
         a->curFrame=0;
    }
}


int INGAME_handleUI(WORLD*w)
{
    w->keys=0;
#if defined(OS_WIN32)
    if(GetKeyState(VK_UP) & 0x80 )
        w->keys=dirUP;
    else if(GetKeyState(VK_DOWN) & 0x80 )
        w->keys=dirDOWN;
    else if(GetKeyState(VK_LEFT) & 0x80 )
        w->keys=dirLEFT;
    else if(GetKeyState(VK_RIGHT) & 0x80 )
        w->keys=dirRIGHT;
#endif
 return 1;
}

int INGAME_action(WORLD*w)
{
    int i;

    if(w->keys)
     if(g_player->moving==0)
    {
        int dirs[4],ndirs=getavailableDIRS(&w->M,g_player,dirs,NULL,0);
        int i;
        for(i=0; i<ndirs; i++)
            if(dirs[i]==w->keys)
            {
                g_player->dir_x=l_dirX[w->keys];
                g_player->dir_y=l_dirY[w->keys];
                g_player->dir=w->keys;
                break;
            }
    }


    for(i=0; i<g_nActors; i++)
    {
        ACTOR*g_actor=&g_actors[i];

        if(g_actor->moving==0)
        {
            int   x,y;
            x=(int)g_actor->x+g_actor->dir_x;
            y=(int)g_actor->y+g_actor->dir_y;
            if(w->M.maze[y][x]=='1')
                g_actor->dir_x=g_actor->dir_y=0;
            else
            {
                g_actor->nx=(int)g_actor->x+g_actor->dir_x;
                g_actor->ny=(int)g_actor->y+g_actor->dir_y;
                if(i==0)
                {
                    x=(int)g_actor->x;
                    y=(int)g_actor->y;
                    if(w->M.maze[y][x]=='.')
                    {
                        w->M.maze[y][x]=' ';
                        w->M.coins--;
                        w->score+=10;
                    }
                }
            }
        }

        if(g_actor->dir_x||g_actor->dir_y)
        {
            g_actor->x+=g_actor->dir_x*0.20f;
            g_actor->y+=g_actor->dir_y*0.20f;
            g_actor->moving++;
            if((fabs(g_actor->x-g_actor->nx)<0.11f)&&(fabs(g_actor->y-g_actor->ny)<0.11f))
            {
                g_actor->moving=0;
                g_actor->x=g_actor->nx;
                g_actor->y=g_actor->ny;
            }
        }
        else
            g_actor->moving=0;
    }


    return 1;
}

int INGAME_draw(WORLD*w)
{
    gfx_cleanSCREEN();

    drawMAZE(w);
    drawACTORS(w);

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

    GFXRES_init();

    MATCH_init(w,1);

    return 1;
}

int GAME_reset(WORLD*w)
{
    GFXRES_reset();

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
