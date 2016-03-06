/* ---------------------------------------------------------------
   g_main.c
   Game code
   ---------------------------------------------------------------
   PAC MAN sprite sheet from: http://www.javaonthebrain.com
   http://www.javaonthebrain.com/java/msmidpman/spritecloseup.gif
   ---------------------------------------------------------------
   FONT from http://www.bigmessowires.com/
   http://www.bigmessowires.com/atarifont.png
   ---------------------------------------------------------------
   PAC MAN maze based on this image
   http://www-inst.eecs.berkeley.edu/~cs188/pacman/projects/multiagent/pacman_multi_agent.png
   ---------------------------------------------------------------
   PAC MAN sounds taken from this site
   http://www.classicgaming.cc/classics/pacman/sounds.php
   ---------------------------------------------------------------

   Copyright (c) 2014 by Marco Giorgini <marco.giorgini@gmail.com>
   This file is part of the New G@ame: Pac Man sample code.

   ---------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use,
   copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following
   conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   --------------------------------------------------------------
*/

#include "g_main.h"

// --------------------------------------------------------------------

WORLD    g_W;
GAMELOOP g_G;

int      g_textID;
int      g_fontID;

int      g_wav_intro;
int      g_wav_eat,g_wav_eatG;
int      g_wav_chomp;
int      g_wav_die;

int      g_intoMUSIC,g_chomp;

// --------------------------------------------------------------------

float  l_dirX[5]= {0,0,0,-1,1};
float  l_dirY[5]= {0,-1,1,0,0};

AREA2D l_startBOX;

int    l_grabX=-1,l_grabY=-1;

int    g_nActors=5;
ACTOR  g_actors[5];
ACTOR* g_player;

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

void INGAME_set(WORLD*w);
void HOME_set(WORLD*w);

// --------------------------------------------------------------------

int PLAYER_act(MAZE*M,struct tagACTOR*m);
int GHOST_act(MAZE*M,struct tagACTOR*m);

// --------------------------------------------------------------------

void EVENT_set(WORLD*w,int event)
{
    w->event=event;
    w->event_timer=os_getMilliseconds()+1000;
}

// --------------------------------------------------------------------

int MATCH_init(WORLD*w,int way)
{
    int x,y;
    memset(&g_actors,0,sizeof(g_actors));
    g_player=&g_actors[0];
    if(way==1)
    {
        w->score=0;
        w->lifes=3;
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
                a->x=a->nx=(float)x;
                a->y=a->ny=(float)y;
                a->baseFrame=0;
                a->nFrames=3;
                a->dir=dirLEFT;
                a->act=PLAYER_act;
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
                a->x=a->nx=(float)x;
                a->y=a->ny=(float)y;
                a->baseFrame=8+wh*8;
                a->nFrames=2;
                a->dir=wh;
                a->act=GHOST_act;
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
    g_fontID=tex_readTGA("font.tga",0);
    return (g_textID!=-1)&&(g_fontID!=-1);
}

int GFXRES_reset()
{
    tex_delete(&g_textID);
    tex_delete(&g_fontID);
    return 1;
}

// --------------------------------------------------------------------
//
//   SND RESOURCE LOAD/UNLOAD
//
// --------------------------------------------------------------------

int SNDRES_init()
{
    snd_init();
    g_wav_intro=wav_read("pacman_beginning.wav",0);
    g_wav_eat=wav_read("pacman_eatfruit.wav",0);
    g_wav_eatG=wav_read("pacman_eatghost",0);
    g_wav_die=wav_read("pacman_death.wav",0);
    g_wav_chomp=wav_read("pacman_chomp.wav",0);
    return (g_wav_intro!=-1)&&(g_wav_eat!=-1)&&(g_wav_eatG!=-1)&&(g_wav_die!=-1)&&(g_wav_chomp!=-1);
}

int SNDRES_reset()
{
    wav_deleteall();
    snd_reset();
    return 1;
}

// --------------------------------------------------------------------
//
//   DRAW FUNCTIONS
//
// --------------------------------------------------------------------

int getWALLMASK(WORLD*w,int x,int y)
{
    int mask=0;
    if((x==0)||(w->M.maze[y][x-1]!='1')) mask|=1;
    if((y==0)||(w->M.maze[y-1][x]!='1')) mask|=2;
    if((x==w->M.sx-1)||(w->M.maze[y][x+1]!='1')) mask|=4;
    if((y==w->M.sy-1)||(w->M.maze[y+1][x]!='1')) mask|=8;
    return mask;
}

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
                {
                    int mask=getWALLMASK(w,(int)x,(int)y);
                    gfx_drawBOX(px,py,sx,sy,0,0,0.05f,1);
                    if(mask&1) {gfx_drawBOX(px,py,sx/4,sy,0,0,0.25f,1);}
                    if(mask&2) {gfx_drawBOX(px,py,sx,sy/4,0,0,0.25f,1);}
                    if(mask&4) {gfx_drawBOX(px+sx*3/4,py,sx/4,sy,0,0,0.25f,1);}
                    if(mask&8) {gfx_drawBOX(px,py+sy*3/4,sx,sy/4,0,0,0.25f,1);}
                    if(mask&1) {gfx_drawBOX(px,py,sx/8,sy,0,0,0.5f,1);}
                    if(mask&2) {gfx_drawBOX(px,py,sx,sy/8,0,0,0.5f,1);}
                    if(mask&4) {gfx_drawBOX(px+sx*7/8,py,sx/8,sy,0,0,0.5f,1);}
                    if(mask&8) {gfx_drawBOX(px,py+sy*7/8,sx,sy/8,0,0,0.5f,1);}
                }
                break;
            case '.':
                gfx_drawBOX(px+sx/2-sx/10,py+sy/2-sx/10,sx/5,sx/5,1,1,1,1);
                break;
            case 'o':
                {
                 float border=(float)ceil(sx/16);
                 gfx_drawBOX(px+sx/2-sx/4-border,py+sy/2-sx/4,sx/2+border*2,sx/2,1,1,0,0.5f);
                 gfx_drawBOX(px+sx/2-sx/4,py+sy/2-sx/4-border,sx/2,sx/2+border*2,1,1,0,0.5f);
                 gfx_drawBOX(px+sx/2-sx/4,py+sy/2-sx/4,sx/2,sx/2,1,1,0,1);
                }
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
        if(a->status&3)
        {
            float px=a->x*sx,py=a->y*sy;
            float fx,fy;
            int   curframe;
            if((a->status&3)==1)
            {
                curframe=12+a->curFrame;
                a->blink=0;
            }
            else
            {
                curframe=14+a->curFrame;
                a->blink++;
            }
            fy=(curframe/8)*32.0f,fx=(curframe%8)*32.0f;
            if((a->blink/5)%2)
                ;
            else
                gfx_drawSPRITE(px,py,sx,sy,g_textID,fx,fy,32,32,1,1,0);
            a->curFrame++;
            if(a->curFrame>=a->nFrames)
                a->curFrame=0;
        }
        else
        {
            float px=a->x*sx,py=a->y*sy;
            int   curframe=a->baseFrame+(a->dir-1)*a->nFrames+a->curFrame;
            float fy=(curframe/8)*32.0f,fx=(curframe%8)*32.0f;
            gfx_drawSPRITE(px,py,sx,sy,g_textID,fx,fy,32,32,1,1,0);
            if(a->dir_x||a->dir_y)
            {
                a->curFrame++;
                if(a->curFrame>=a->nFrames)
                    a->curFrame=0;
            }
        }
    }
}

void drawTEXT(float px,float py,float sx,float sy,const char*sz)
{
    int i=0;
    while(sz[i])
    {
        int curframe=sz[i];
        float fy=(curframe/16)*32.0f,fx=(curframe%16)*16.0f;
        gfx_drawSPRITE(px,py+sy/4,sx/2,sy/2,g_fontID,fx,fy,16,32,1,1,0);
        px+=sx/2;
        i++;
    }
}

void drawGUI(WORLD*w)
{
    float sx=os_video_w/w->M.sx;
    float sy=os_video_h/w->M.sy;
    int   cur_time=os_getMilliseconds();
    if(w->event)
    {
        float ww=10*sx,h=2*sy;
        float brd=sx/4;
        float x=(os_video_w-ww)/2,y=(os_video_h-h)/2;
        char  sz[32];

        gfx_drawBOX(x-brd,y-brd,ww+brd*2,h+brd*2,0,0,1,1);
        gfx_drawBOX(x,y,ww,h,0,0,0,1);
        switch(w->event)
        {
        case event_GETREADY:
            strcpy(sz,"GET READY");
            break;
        case event_GAMEOVER:
            strcpy(sz,"GAME OVER");
            break;
        case event_NEWLEVEL:
            strcpy(sz,"NEW LEVEL");
            break;
        case event_LIVELEFT:
            sprintf(sz,"%d LIFES LEFT",w->lifes);
            break;
        }

        drawTEXT(x+(ww-strlen(sz)*sx/2)/2,y+(h-sy)/2,sx,sy,sz);

        if(cur_time>w->event_timer)
        {
            w->event_timer=0;
            switch(w->event)
            {
            case event_GETREADY:
                w->event=0;
                break;
            case event_GAMEOVER:
                w->event=0;
                HOME_set(w);
                break;
            case event_LIVELEFT:
            case event_NEWLEVEL:
                w->event=event_GETREADY;
                w->event_timer=os_getMilliseconds()+1000;
                break;
            }

        }
    }
    else
    {
        char sz[32];
        int  i;
        float px=0,py=0;

        if(w->hiscore<w->score)
            w->hiscore=w->score;

        sprintf(sz,"HIGH SCORE %05d",w->hiscore);
        px=os_video_w-sx-strlen(sz)*sx/2;
        py=0;
        drawTEXT(px,py,sx,sy,sz);

        sprintf(sz,"SCORE %05d",w->score);
        px=sx;
        py=0;
        drawTEXT(px,py,sx,sy,sz);

        if(w->lifes)
        {
            i=w->lifes-1;
            px=sx;
            py=os_video_h-sy;
            while(i--)
            {
                int   curframe=6;
                float fy=(curframe/8)*32.0f,fx=(curframe%8)*32.0f;
                gfx_drawSPRITE(px,py+sy/4,sx/2,sy/2,g_textID,fx,fy,32,32,1,1,0);
                px+=sx/2;
            }
        }
    }
    if(w->event==event_none)
        if((l_grabX!=-1)&&(l_grabY!=-1))
        {
            gfx_drawBOX(l_grabX-sx*1.5f,l_grabY-sy/2,sx*3,sy,1,1,1,0.25f);
            gfx_drawBOX(l_grabX-sx/2,l_grabY-sy*1.5f,sx,sy*1.0f,1,1,1,0.25f);
            gfx_drawBOX(l_grabX-sx/2,l_grabY+sy*0.5f,sx,sy*1.0f,1,1,1,0.25f);
        }
}

void drawHOME(WORLD*w)
{
    float sx=os_video_w/w->M.sx;
    float sy=os_video_h/w->M.sy;
    float brd=sx/4;
    float x,y,ww,h;
    float px,py;
    char  sz[64];

    x=sx;
    y=sy;
    ww=os_video_w-sx*2;
    h=os_video_h-sy*2;
    gfx_drawBOX(x-brd,y-brd,ww+brd*2,h+brd*2,0,0,1,1);
    gfx_drawBOX(x,y,ww,h,0,0,0,1);

    py=1*sy;
    strcpy(sz,"P@CM@N");
    px=(os_video_w-strlen(sz)*sx)/2;
    drawTEXT(px,py,sx*2,sy*2,sz);
    py+=2*sy;
    strcpy(sz,"New G@me: programming lessons");
    px=(os_video_w-strlen(sz)*sx/2)/2;
    drawTEXT(px,py,sx,sy,sz);

    py+=2*sy;
    strcpy(sz,"HIGH SCORE");
    px=(os_video_w-strlen(sz)*sx/2)/2;
    drawTEXT(px,py,sx,sy,sz);

    py+=sy;
    sprintf(sz,"%05d",w->hiscore);
    px=(os_video_w-strlen(sz)*sx/2)/2;
    drawTEXT(px,py,sx,sy,sz);

    py+=2*sy;
    strcpy(sz,"START");

    l_startBOX.w=strlen(sz)*sx/2;
    l_startBOX.h=sy;
    px=(os_video_w-l_startBOX.w)/2;
    l_startBOX.x=px;
    l_startBOX.y=py;

    drawTEXT(px,py,sx,sy,sz);
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
//   ACTORS ACTIONS
//
// --------------------------------------------------------------------

int PLAYER_act(MAZE*M,struct tagACTOR*m)
{
    if(g_W.keys)
    {
        if(m->moving==0)
        {
            int dirs[4],ndirs=getavailableDIRS(M,m,dirs,NULL,0);
            int i;
            for(i=0; i<ndirs; i++)
                if(dirs[i]==g_W.keys)
                {
                    m->dir_x=l_dirX[g_W.keys];
                    m->dir_y=l_dirY[g_W.keys];
                    m->dir=g_W.keys;
                    return 1;
                }
        }
    }
    if(m->dir_x||m->dir_y)
    {
        if(g_chomp==-1)
        {
            g_chomp=snd_play(g_wav_chomp,1,0);
        }
    }
    else
    {

        if(g_chomp!=-1)
        {
            snd_stop(g_chomp);
            g_chomp=-1;
        }
    }
    return 0;
}

int GHOST_act(MAZE*M,struct tagACTOR*m)
{
    if(m->moving==0)
        if(m->dir_x||m->dir_y)
        {
            int mask;
            getavailableDIRS(M,m,NULL,&mask,1);
            if(mask!=m->movementmask)
            {
                m->movementmask=mask;
                if((rand()%10)>=5)
                {
                    m->dir=chooseDIR(M,m);
                    m->dir_x=l_dirX[m->dir];
                    m->dir_y=l_dirY[m->dir];
                }
            }
        }
        else
        {
            m->dir=chooseDIR(M,m);
            m->dir_x=l_dirX[m->dir];
            m->dir_y=l_dirY[m->dir];
        }
    return 1;
}

// --------------------------------------------------------------------
//
//   HOME CALLBACKS
//
// --------------------------------------------------------------------

int HOME_handleUI(WORLD*w)
{
    int   i;
    float sx=os_video_w/w->M.sx;
    w->keys=0;
    for(i=0; i<os_np; i++)
    {
        if((os_status[i]!=touchNONE)&&AREA2D_isinbound((float)os_x[i],(float)os_y[i],&l_startBOX,sx))
        {
            w->keys=1;
        }
        if(os_status[i]==touchUP)
            os_status[i]=touchNONE;
    }
    return 1;
}

int HOME_action(WORLD*w)
{
    if(w->keys)
    {
        INGAME_set(w);
    }
    return 1;
}

int HOME_draw(WORLD*w)
{
    int err;
    gfx_cleanSCREEN();
    err=glGetError();

    drawHOME(w);
    err=glGetError();
    return 1;
}

int HOME_backaction(WORLD*w)
{
#if defined(OS_ANDROID)
	jni_exit_app(0);
#endif
	return 1;
}

// --------------------------------------------------------------------
//
//   INGAME CALLBACKS
//
// --------------------------------------------------------------------

int INGAME_handleUI(WORLD*w)
{
    static int bias=20;
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
#elif defined(OS_MAC)||defined(OS_LINUX)
    if(os_keys[GLFW_KEY_UP])
        w->keys=dirUP;
    else if(os_keys[GLFW_KEY_DOWN])
        w->keys=dirDOWN;
    else if(os_keys[GLFW_KEY_LEFT])
        w->keys=dirLEFT;
    else if(os_keys[GLFW_KEY_RIGHT])
        w->keys=dirRIGHT;
#endif
    if(os_np)
    {
        if(os_status[0]==touchDOWN)
        {
            l_grabX=os_x[0];
            l_grabY=os_y[0];
        }
        else if(os_status[0]==touchUP)
        {
            l_grabX=-1;
            l_grabY=-1;
            os_status[0]=touchNONE;
        }
        else if(os_status[0]==touchMOVE)
            if((l_grabX!=-1)&&(l_grabY!=-1))
            {
                int dirx=os_x[0]-l_grabX;
                int diry=os_y[0]-l_grabY;
                if((abs(dirx)>bias)&&(abs(dirx)>abs(diry)))
                {
                    if(dirx>0)
                        w->keys=dirRIGHT;
                    else if(dirx<0)
                        w->keys=dirLEFT;
                }
                else if((abs(diry)>bias)&&(abs(diry)>abs(dirx)))
                {
                    if(diry>0)
                        w->keys=dirDOWN;
                    else if(diry<0)
                        w->keys=dirUP;
                }
            }
    }
 return 1;
}

int INGAME_postaction(WORLD*w)
{
    if(w->event==event_none)
        if(w->M.coins==0)
        {
            MATCH_init(w,2); // WIN!
            EVENT_set(w,event_NEWLEVEL);
        }
        else
        {
            int i;
            for(i=1; i<g_nActors; i++)
            {
                ACTOR*g_actor=&g_actors[i];
                if((fabs(g_actor->x-g_player->x)<0.5f)&&(fabs(g_actor->y-g_player->y)<0.5f))
                    if(g_actor->status&3)
                    {
                        snd_play(g_wav_eatG,0,0);
                        g_actor->x=g_actor->nx=(float)w->M.home_x;
                        g_actor->y=g_actor->ny=(float)w->M.home_y;
                        g_actor->dir=dirUP;
                        g_actor->moving=0;
                        g_actor->movementmask=0;
                        g_actor->status=0;
                        w->score+=w->M.hunt_score;
                        w->M.hunt_score*=2;
                    }
                    else
                    {
                        snd_play(g_wav_die,0,0);
                        if(w->lifes>1)
                        {
                            w->lifes--;
                            MATCH_init(w,0);
                            EVENT_set(w,event_LIVELEFT);
                        }
                        else
                        {
                            EVENT_set(w,event_GAMEOVER);
                        }
                    }
            }
        }
 return 1;
}

int INGAME_action(WORLD*w)
{
    if(w->event)
    {
        if(g_chomp!=-1)
        {
            snd_stop(g_chomp);
            g_chomp=-1;
        }
    }
    else
    {
        int i;
        for(i=0; i<g_nActors; i++)
        {
            ACTOR*g_actor=&g_actors[i];
            if(g_actor->act)
                g_actor->act(&w->M,g_actor);

            if(g_actor->moving==0)
            {
                int   x,y;
                x=(int)g_actor->x+(int)g_actor->dir_x;
                y=(int)g_actor->y+(int)g_actor->dir_y;
                if(w->M.maze[y][x]=='1')
                    g_actor->dir_x=g_actor->dir_y=0;
                else
                {
                    g_actor->nx=g_actor->x+g_actor->dir_x;
                    g_actor->ny=g_actor->y+g_actor->dir_y;
                    if(i==0)
                    {
                        x=(int)g_actor->x;
                        y=(int)g_actor->y;
                        if(w->M.maze[y][x]=='.')
                        {
                            snd_play(g_wav_eat,0,0);
                            w->M.maze[y][x]=' ';
                            w->M.coins--;
                            w->score+=10;
                        }
                        else if(w->M.maze[y][x]=='o')
                        {
                            int j;
                            snd_play(g_wav_eat,0,0);
                            w->M.maze[y][x]=' ';
                            w->M.coins--;
                            w->score+=50;
                            w->M.hunt_timer=os_getMilliseconds()+5000;
                            w->M.hunt_score=200;
                            for(j=1; j<g_nActors; j++)
                                g_actor[j].status=1;
                        }
                    }
                }
            }
            if(g_actor->dir_x||g_actor->dir_y)
            {
                if(g_actor->status)
                {
                    g_actor->x+=g_actor->dir_x*0.10f;
                    g_actor->y+=g_actor->dir_y*0.10f;
                }
                else
                {
                    g_actor->x+=g_actor->dir_x*0.20f;
                    g_actor->y+=g_actor->dir_y*0.20f;
                }
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
        if(w->M.hunt_timer)
            if(os_getMilliseconds()>w->M.hunt_timer)
            {
                int j;
                w->M.hunt_timer=0;
                for(j=1; j<g_nActors; j++)
                    g_actors[j].status=0;
            }
            else if(os_getMilliseconds()+2500>w->M.hunt_timer)
            {
                int j;
                for(j=1; j<g_nActors; j++)
                    if(g_actors[j].status&1)
                        g_actors[j].status|=2;

            }
    }
 return 1;
}

int INGAME_draw(WORLD*w)
{
    gfx_cleanSCREEN();

    drawMAZE(w);
    drawACTORS(w);
    drawGUI(w);

    return 1;
}

int INGAME_backaction(WORLD*w)
{
#if defined(OS_ANDROID)
	jni_exit_app(0);
#endif
	return 1;
}

// --------------------------------------------------------------------
//
//   GAME SCENE SELECTORS
//
// --------------------------------------------------------------------


void INGAME_set(WORLD*w)
{
    if(g_intoMUSIC!=-1)
    {
        snd_stop(g_intoMUSIC);g_intoMUSIC=-1;
    }
    g_G.handleUI=INGAME_handleUI;
    g_G.action=INGAME_action;
    g_G.postaction=INGAME_postaction;
    g_G.draw=INGAME_draw;

    MATCH_init(w,1);
    EVENT_set(w,event_GETREADY);
}

void HOME_set(WORLD*w)
{
    g_intoMUSIC=snd_play(g_wav_intro,1,0);
    g_G.handleUI=HOME_handleUI;
    g_G.action=HOME_action;
    g_G.postaction=NULL;
    g_G.draw=HOME_draw;

    MATCH_init(w,1);
}

// --------------------------------------------------------------------
//
//   GAME
//
// --------------------------------------------------------------------

int GAME_init(WORLD*w,const char*res_basepath,float width,float height)
{
    os_init(res_basepath,width,height);

    gfx_init();
    snd_init();

    gfx_enable2D(1);

    GFXRES_init();
    SNDRES_init();

    HOME_set(w);

    return 1;
}

int GAME_reset(WORLD*w)
{
    SNDRES_reset();
    GFXRES_reset();

    gfx_enable2D(0);

    snd_reset();
    gfx_reset();
    os_reset();
    return 1;
}

void GAME_loop(WORLD*w)
{
    if(os_PAUSE)
        ;
    else
    {
        if(g_G.handleUI)
            g_G.handleUI(w);
        if(g_G.action)
            g_G.action(w);
        if(g_G.postaction)
            g_G.postaction(w);
        if(g_G.draw)
            g_G.draw(w);
        if((g_G.backaction)&&(os_BACK_pressed))
        {
            g_G.backaction(w);
            os_BACK_pressed = 0;
        }
    }
}

int GAME_restore_textures(WORLD*w)
{
	GFXRES_reset();
	return GFXRES_init();
}

void GAME_pause(WORLD*w)
{
    os_PAUSE=1;
}

void GAME_resume(WORLD*w)
{
    os_PAUSE=0;
}

int GAME_handle_back_pressed(WORLD*w)
{
#if defined(OS_ANDROID)
 jni_exit_app(0);
#endif
	return 0;
}

// --------------------------------------------------------------------
