/* ---------------------------------------------------------------
   g_main.c
   Game code
   ---------------------------------------------------------------
   ---------------------------------------------------------------

   Copyright (c) 2014 by Marco Giorgini <marco.giorgini@gmail.com>
   This file is part of the New G@ame: 2Dworld sample code.

   ---------------------------------------------------------------
   background, tiles and sprites by KENNEY.NL (CC0)
   http://kenney.nl/post/platformer-art-assets-deluxe
   ---------------------------------------------------------------
   FONT from http://www.bigmessowires.com/
   http://www.bigmessowires.com/atarifont.png
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

/*
int         g_show=1|  // background / viewport '1'
                   2|  // collision             '2'
                   4|  // diamands              '3'
                   8|  // worms                 '4'
                   16| // worms collisions      '5'
                   32| // hud                   '6'
                   64| // home                  '7'
                   0
                   ;
*/

// set bSHOWTIME to 1 when you want to select the elements one by one to show
// how the game has been made, piece after piece

int         bSHOWTIME=0;
int         g_show=1|2|4|8|16|32|64;

WORLD       g_W;
GAMELOOP    g_G;

Texture     g_fontID;

INGAMESCENE g_ingamescene;

// --------------------------------------------------------------------

AREA2D      l_startBOX;
AREA2D      l_padBOX,l_jumpBOX,l_pauseBOX;

int         g_nActors=0;
ACTOR       g_actors[256];
ACTOR*      g_player;

// --------------------------------------------------------------------

void   INGAME_set(WORLD*w);

void   HOME_set(WORLD*w);

int    LEVEL_init(WORLD*w);
int    LEVEL_reset(WORLD*w);

// --------------------------------------------------------------------

// --------------------------------------------------------------------


// --------------------------------------------------------------------



// --------------------------------------------------------------------
//
//   GFX RESOURCE LOAD/UNLOAD
//
// --------------------------------------------------------------------

int GFXRES_init()
{
    #if defined(HAVE_STBIMAGE)
    g_fontID=tex_read("font.png",4);
    #else
    g_fontID=tex_readTGA("font.tga",4);
    #endif
    return (g_fontID.textID!=-1);
}

int GFXRES_reset()
{
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
    return 1;
}

int SNDRES_reset()
{
    wav_deleteall();
    snd_reset();
    return 1;
}

// --------------------------------------------------------------------

char MAP_get(INGAMESCENE*s,int x,int y)
{
 return s->map.map[x+y*s->map.rw];
}

void MAP_set(INGAMESCENE*s,int x,int y,char c)
{
 s->map.map[x+y*s->map.rw]=c;
}

// --------------------------------------------------------------------

void EVENT_set(WORLD*w,int event)
{
    w->event=event;
    w->event_timer=os_getMilliseconds()+2000;
}

// --------------------------------------------------------------------
//
//   DRAW FUNCTIONS
//
// --------------------------------------------------------------------

void drawWORLD(WORLD*w,INGAMESCENE*s)
{
 float vx=0,vy=0,sx,sy,ll=1;
 int   x,y;
 MAP*  g_map=&s->map;

 sx=g_map->tx;
 sy=g_map->ty;

 if(g_show&1)
  {
   // background
   gfx_drawSPRITE(-g_map->ox/100,-g_map->oy/100,os_video_w+g_map->sx/100,os_video_h+g_map->sx/100,&s->backID,0,0,256,128,1,1,0);
  }
 else
  {
   // gradient
   for(vy=0;vy<os_video_h;vy+=sy,ll-=0.05f)
    gfx_drawBOX(0,vy,os_video_w,sy,0,0,ll,1);
  }

 for(vx=(float)floor(-g_map->ox),x=0;x<g_map->w;x++,vx+=sx)
  if(vx>os_video_w)
   break;
  else
  if(vx+sx<0)
   ;
  else
   for(vy=-g_map->oy,y=0;y<g_map->h;y++,vy+=sy)
    if(vy>os_video_h)
     break;
    else
    if(vy+sy<0)
     ;
    else
     {
      char c=MAP_get(s,x,y);
      if(c!=' ')
       {
        int id=s->maptiles[c];
        if(id!=-1)
         gfx_drawSPRITETEXATL(vx,vy,&s->textID,id,os_gfxratio,1,0);
       }
     }
}

void drawACTORS(WORLD*w,INGAMESCENE*s)
{
    int l;
    for(l=0;l<2;l++)
    {
      int i=g_nActors;
      while(i--)
      {
          ACTOR*a=&g_actors[i];
          if(a->layer==l)
           if(a->status&1)
            ;
           else
            {
             int   id=a->curFrame;
             float sx=s->textID.tai[id].w*os_gfxratio;
             float sy=s->textID.tai[id].h*os_gfxratio;

             if(a->x+sx/2-s->map.ox<0)
              continue;
             if(a->x-sx/2-s->map.ox>os_video_w)
              continue;

             if(a->status&2)
              {
               a->tick++;
               if((a->tick/4)%2)
                gfx_drawSPRITETEXATL(a->x-sx/2-s->map.ox,a->y-sy-s->map.oy,&s->textID,id,os_gfxratio,1,a->dir);
              }
             else
              gfx_drawSPRITETEXATL(a->x-sx/2-s->map.ox,a->y-sy-s->map.oy,&s->textID,id,os_gfxratio,1,a->dir);
             if(a->animtickcnt)
              {
               a->animtick++;
               if(a->animtick>=a->animtickcnt)
                {
                 a->curFrame++;
                 a->animtick=0;
                }
              }
             else
              a->curFrame++;
             if(a->curFrame>=a->baseFrame+a->nFrames)
              a->curFrame=a->baseFrame;
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
        gfx_drawSPRITE(px,py+sy/4,sx/2,sy/2,&g_fontID,fx,fy,16,32,1,1,0);
        px+=sx/2;
        i++;
    }
}

void drawGUI(WORLD*w,INGAMESCENE*s)
{
 float sx=os_video_w/20;
 float sy=os_video_h/11;
 int   cur_time=os_getMilliseconds();
 if(w->event)
 {
     float ww=10*sx,h=2*sy;
     float brd=sx/4;
     float x=(os_video_w-ww)/2,y=(os_video_h-h)/2;
     char  sz[32];

     gfx_drawBOX(0,y-brd,os_video_w,h+brd*2,0,0,0,0.5f);
     gfx_drawBOX(0,y,os_video_w,h,0,0,0,0.5f);
     switch(w->event)
     {
     case event_GETREADY:
         strcpy(sz,"GET READY");
         break;
     case event_LEVELCOMPLETED:
         strcpy(sz,"LEVEL COMPLETED");
         break;
     case event_GAMEOVER:
         strcpy(sz,"GAME OVER");
         break;
     case event_NEWLEVEL:
         sprintf(sz,"WORLD %d - AREA %d",w->world,w->area);
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
         case event_LEVELCOMPLETED:
             w->score+=w->tempscore;w->tempscore=0;
             w->event=0;
             LEVEL_reset(w);
             // gestione del passaggio di livello
             w->area++;
             if(w->area==10)
              {
               if(w->world==3)
                {
                 // qui va inserito anche il controllo di fine gioco
                }
               else
                {
                 w->world++;
                 w->area=1;
                }
              }
             if(LEVEL_init(w))
              ;
             else
              {
               if(w->score>w->hiscore) w->hiscore=w->score;
               if(g_show&64)
                HOME_set(w);
               else
                INGAME_set(w);
              }
         break;
         case event_GAMEOVER:
             w->score+=w->tempscore;w->tempscore=0;
             if(w->score>w->hiscore) w->hiscore=w->score;
             w->event=0;
             if(g_show&64)
              HOME_set(w);
             else
              INGAME_set(w);
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
   int   id,i;
   char  sz[256];
   float sx,sy,x,y,ww;
   float ratio=os_gfxratio*0.75f;
   id=TEXATL_find(&s->textID,"flatDark03");
   sx=s->textID.tai[id].w*ratio;
   sy=s->textID.tai[id].h*ratio;
   x=32*os_gfxratio;
   y=os_video_h-sy-32*os_gfxratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,0.5f,0);
   l_padBOX.x=x-sx/2;l_padBOX.y=y-sy/2;
   l_padBOX.w=sx*2;l_padBOX.h=sy*2;

   id=TEXATL_find(&s->textID,"flatDark25");
   sx=s->textID.tai[id].w*ratio;
   sy=s->textID.tai[id].h*ratio;
   x=os_video_w-sx-32*os_gfxratio;
   y=os_video_h-sy-32*os_gfxratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,0.5f,0);
   l_jumpBOX.x=x;l_jumpBOX.y=y;
   l_jumpBOX.w=sx;l_jumpBOX.h=sy;

   y=4*os_gfxratio;
   /*
   id=TEXATL_find(&s->textID,"flatDark13");
   sx=s->textID.tai[id].w*ratio;
   sy=s->textID.tai[id].h*ratio;
   x=(os_video_w-sx)/2;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
   */

   x=4*os_gfxratio;
   sprintf(sz,"%06d",w->score);
   id=TEXATL_find(&s->textID,"hud_coins");
   sx=s->textID.tai[id].w*ratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
   x+=sx;
   id=TEXATL_find(&s->textID,"hud_x");
   sx=s->textID.tai[id].w*ratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
   x+=sx;
   i=0;
   while(sz[i])
    {
     char ss[16];
     strcpy(ss,"hud_0");ss[4]=sz[i];
     id=TEXATL_find(&s->textID,ss);
     sx=s->textID.tai[id].w*ratio;
     sy=s->textID.tai[id].h*ratio;
     gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
     x+=sx;
     i++;
     if(i==1)
      ww=x-4*os_gfxratio;
    }

   x=os_video_w-ww-4*os_gfxratio;
   sprintf(sz,"%01d",w->lifes);
   id=TEXATL_find(&s->textID,"hud_p2");
   sx=s->textID.tai[id].w*ratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
   x+=sx;
   id=TEXATL_find(&s->textID,"hud_x");
   sx=s->textID.tai[id].w*ratio;
   gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
   x+=sx;
   i=0;
   while(sz[i])
    {
     char ss[16];
     strcpy(ss,"hud_0");ss[4]=sz[i];
     id=TEXATL_find(&s->textID,ss);
     sx=s->textID.tai[id].w*ratio;
     sy=s->textID.tai[id].h*ratio;
     gfx_drawSPRITETEXATL(x,y,&s->textID,id,ratio,1,0);
     x+=sx;
     i++;
    }
  }
}

void drawHOME(WORLD*w)
{
    float sx=os_video_w/20;
    float sy=os_video_h/11;
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
    strcpy(sz,"2Dworld");
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


// --------------------------------------------------------------------
//
//   ACTORS ACTIONS
//
// --------------------------------------------------------------------


// --------------------------------------------------------------------
//
//   HOME CALLBACKS
//
// --------------------------------------------------------------------

int HOME_handleUI(WORLD*w)
{
    int   i;
    float sx=os_video_w/20;
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
    gfx_cleanSCREEN();

    drawHOME(w);
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
    float sx=os_video_w/20;
    int i;
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
    if(GetKeyState(VK_SPACE) & 0x80 )
        w->keys|=dirJUMP;
    if(GetKeyState(VK_F3)&0x80)
     EVENT_set(w,event_LEVELCOMPLETED);
#endif

    for(i=0; i<os_np; i++)
    {
        if((os_status[i]==touchDOWN)&&AREA2D_isinbound((float)os_x[i],(float)os_y[i],&l_jumpBOX,sx))
        {
            w->keys=dirJUMP;
        }
        else
        if((os_status[i]!=touchNONE)&&AREA2D_isinbound((float)os_x[i],(float)os_y[i],&l_padBOX,sx))
        {
            if(os_y[i]>l_padBOX.y+l_padBOX.h*2/3)
             w->keys=dirDOWN;
            else
            if(os_x[i]<l_padBOX.x+l_padBOX.w/2)
             w->keys=dirLEFT;
            else
            if(os_x[i]>l_padBOX.x+l_padBOX.w/2)
             w->keys=dirRIGHT;
        }
        if(os_status[i]==touchUP)
            os_status[i]=touchNONE;
    }

 return 1;
}

int INGAME_postaction(WORLD*w)
{
 if(w->tempscore)
  {
   w->tempscore--;
   w->score++;
  }
 return 1;
}

int checkGROUND(INGAMESCENE*s,ACTOR*a,float dx)
{
 AREA2D b;
 int    id=a->curFrame,x,y;
 float  vx,vy,sx,sy;
 b.w=s->textID.tai[id].w*os_gfxratio;
 b.h=s->textID.tai[id].h*os_gfxratio;
 b.x=a->x-b.w/3;b.y=a->y-b.h;
 b.w=2*b.w/3;

 sx=s->map.tx;
 sy=s->map.ty;

 for(vx=0,x=0;x<s->map.w;x++,vx+=sx)
  if(vx>b.x+b.w+100)
   break;
  else
  if(vx+sx<b.x-100)
   ;
  else
   for(vy=0,y=0;y<s->map.h;y++,vy+=sy)
    if(vy>b.y+b.h+100)
     break;
    else
    if(vy+sy<b.y-100)
     ;
    else
     {
      char c=MAP_get(s,x,y);
      if(c!=' ')
       {
        int mask=s->maptilesmask[c];
        if(mask&(1|2))
         {
          int bottom=(int)(b.y+b.h);
          int left=(int)(b.x+dx);
          int right=(int)(b.x+b.w+dx);
          if((right<=vx)||(left>=vx+sx))
           ;
          else
           if(floor(bottom)==floor(vy))
            return 1;
         }
       }
    }

 return 0;
}

int checkCOLLISIONS(INGAMESCENE*s,ACTOR*a,float*dx,float*dy)
{
 AREA2D b;
 int    id=a->curFrame,x,y,coll=0;
 float  vx,vy,sx,sy;
 b.w=s->textID.tai[id].w*os_gfxratio;
 b.h=s->textID.tai[id].h*os_gfxratio;
 b.x=a->x-b.w/2;b.y=a->y-b.h;

 sx=s->map.tx;
 sy=s->map.ty;

 for(vx=0,x=0;x<s->map.w;x++,vx+=sx)
  if(vx>b.x+b.w+100)
   break;
  else
  if(vx+sx<b.x-100)
   ;
  else
   for(vy=0,y=0;y<s->map.h;y++,vy+=sy)
    if(vy>b.y+b.h+100)
     break;
    else
    if(vy+sy<b.y-100)
     ;
    else
     {
      char c=MAP_get(s,x,y);
      if(c!=' ')
       {
        int mask=s->maptilesmask[c];
        if(mask&(4|8))
         {
          int top=(int)(b.y+*dy);
          int bottom=(int)(b.y+b.h+*dy);
          int left=(int)(b.x+*dx);
          int right=(int)(b.x+b.w+*dx);
          if((right<=vx)||(left>=vx+sx))
           ;
          else
           if((bottom<=vy)||(top>=vy+sx))
            ;
           else
            {*dx=0;coll=1;}
         }
        if(mask&(1|2))
         {
          int top=(int)(b.y+*dy);
          int bottom=(int)(b.y+b.h+*dy);
          int left=(int)(b.x+*dx);
          int right=(int)(b.x+b.w+*dx);
          if((right<=vx)||(left>=vx+sx))
           ;
          else
           if((bottom<=vy)||(top>=vy+sy))
            ;
           else
           if(*dy<0)
            {
             if(mask&2)
              {
               *dy=0;
               coll=1;
               a->y=vy+sy+b.h;
               a->vy=-a->vy;
              }
            }
           else
           if(*dy>0)
            {
             if(a->y<=vy)
              {
               *dy=0;
               coll=1;
               a->y=vy;
               if(a->vy!=0.0f)
                {
                 a->vy=0;
                 a->anim=0;
                }
              }
            }
         }
       }
    }

 return coll;
}

void checkMAPBOUNDS(INGAMESCENE*s,ACTOR*a)
{
 if(a->x<0)
  a->x=0;
 else
 if(a->x>s->map.sx)
  a->x=s->map.sx;
 if(a->y<0)
  a->y=0;
 else
 if(a->y>s->map.sy)
  a->y=s->map.sy;
}

void adjustVIEWPORT(MAP*g_map,ACTOR*a)
{
 g_map->ox=max(a->x,os_video_w/2);
 g_map->oy=max(a->y,os_video_h/2);
 g_map->ox=min(g_map->ox,g_map->sx-os_video_w/2);
 g_map->oy=min(g_map->oy,g_map->sy-os_video_h/2);
 g_map->ox-=os_video_w/2;
 g_map->oy-=os_video_h*2/3;
 g_map->ox=(float)floor(g_map->ox);
 g_map->oy=(float)floor(g_map->oy);
}

void getACTORBOUND(WORLD*w,INGAMESCENE*s,ACTOR*a,AREA2D*b)
{
 int id=a->curFrame;
 b->w=s->textID.tai[id].w*os_gfxratio;
 b->h=s->textID.tai[id].h*os_gfxratio;
 b->x=a->x-b->w/2;b->y=a->y-b->h;
}

int DIAMOND_action(WORLD*w,INGAMESCENE*s,ACTOR*a)
{
 AREA2D bD,bP;

 getACTORBOUND(w,s,a,&bD);
 getACTORBOUND(w,s,g_player,&bP);

 if(AREA2D_intersect(&bD,&bP))
  {
   w->tempscore+=20;
   return 0;
  }
 else
  return 1;
}

int COIN_action(WORLD*w,INGAMESCENE*s,ACTOR*a)
{
 AREA2D bD,bP;

 getACTORBOUND(w,s,a,&bD);
 getACTORBOUND(w,s,g_player,&bP);

 if(AREA2D_intersect(&bD,&bP))
  {
   w->tempscore+=5;
   return 0;
  }
 else
  return 1;
}

int SNAKE_action(WORLD*w,INGAMESCENE*s,ACTOR*a)
{
 return 1;
}

int WORM_action(WORLD*w,INGAMESCENE*s,ACTOR*a)
{
 AREA2D bW,bP;
 float  dx=0,dy=0;
 if(a->dir==0)
  {
   dx=-12.0f*0.33f*os_gfxratio;
  }
 else
  {
   dx=12.0f*0.33f*os_gfxratio;
  }
 if(a->x<0)
  {
   dx=0;
   a->x=0;
   a->dir=1;
  }
 else
 if(a->x>s->map.sx)
  {
   dx=0;
   a->x=s->map.sx;
   a->dir=0;
  }
 else
 if(checkCOLLISIONS(s,a,&dx,&dy))
  a->dir=!a->dir;

 a->x+=dx;
 a->y+=dy;

 if(g_show&16)
  {
   getACTORBOUND(w,s,a,&bW);
   getACTORBOUND(w,s,g_player,&bP);

   if(AREA2D_intersect(&bW,&bP))
    if((g_player->status&2)==0)
     {
      g_player->statustimer=os_getMilliseconds()+2000;
      g_player->status|=2;
      w->lifes--;
      if(w->lifes==0)
       EVENT_set(w,event_GAMEOVER);
      else
       {
        g_player->anim='j';
        g_player->curFrame=g_player->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_jump",&g_player->nFrames);
        g_player->vy=-10*os_gfxratio;
       }
     }
  }
 return 1;
}

int PLAYER_action(WORLD*w,INGAMESCENE*s,ACTOR*a)
{
 float dx=0,dy=0;
 if(a->status&2)
  {
   if(os_getMilliseconds()>a->statustimer)
    a->status-=2;
  }
 if((g_show&1)&&((w->keys&dirJUMP)||(a->anim=='j')))
  {
   if(a->anim!='j')
    {
     a->anim='j';
     a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_jump",&a->nFrames);
     a->vy=-15*os_gfxratio;
    }
   dy+=a->vy;
   a->vy+=0.75f*os_gfxratio;
   if(a->vy>15*os_gfxratio)
    a->vy=15*os_gfxratio;
   if(w->keys&dirLEFT)
    {
     dx=-15.0f*0.33f*os_gfxratio;
     a->dir=1;
    }
   else
   if(w->keys&dirRIGHT)
    {
     dx=15.0f*0.33f*os_gfxratio;
     a->dir=0;
    }
  }
 else
 if(w->keys&dirLEFT)
  {
   dx=-25.0f*0.33f*os_gfxratio;
   a->dir=1;
   if(a->anim!='w')
    {
     a->anim='w';
     a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_walk",&a->nFrames);
    }
  }
 else
 if(w->keys&dirRIGHT)
  {
   dx=25.0f*0.33f*os_gfxratio;
   a->dir=0;
   if(a->anim!='w')
    {
     a->anim='w';
     a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_walk",&a->nFrames);
    }
  }
 else
 if(w->keys&dirDOWN)
  {
   if(a->anim!='d')
    {
     a->anim='d';
     a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_duck",&a->nFrames);
    }
  }
 else
 if(a->anim!='s')
  {
   a->anim='s';
   a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_stand",&a->nFrames);
  }

 if((checkGROUND(s,a,dx)==0)&&(a->anim!='j'))
  {
   a->anim='j';
   a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_jump",&a->nFrames);
   a->vy=5;
  }

 if(g_show&2)
  if(checkCOLLISIONS(s,a,&dx,&dy)==0)
   {
   }

 a->x+=dx;
 a->y+=dy;

 if(g_show&1)
  checkMAPBOUNDS(s,a);

 if(a->x==s->map.sx)
  EVENT_set(w,event_LEVELCOMPLETED);

 if(g_show&1)
  adjustVIEWPORT(&s->map,a);

 return 1;
}

int INGAME_action(WORLD*w)
{
 if((w->event)&&(g_show&32))
  {
  }
 else
  {
   int i=g_nActors;
   while(i--)
   {
       ACTOR*a=&g_actors[i];
       if(a->act)
        if(a->act(w,&g_ingamescene,a)==0)
         {
          a->act=NULL;
          a->status|=1;
         }
   }
  }
 return 1;
}

void resyncRESIZE(INGAMESCENE*s)
{
 int   id;
 os_gfxratio=os_video_h/320.0f;
 id=TEXATL_find(&s->textID,"sandCenter");
 s->map.tx=s->map.ty=s->textID.tai[id].w*os_gfxratio;
 s->map.sx=s->map.w*s->map.tx;
 s->map.sy=s->map.h*s->map.ty;
}

int INGAME_draw(WORLD*w)
{
    resyncRESIZE(&g_ingamescene);

    gfx_cleanSCREEN();

    drawWORLD(w,&g_ingamescene);
    drawACTORS(w,&g_ingamescene);
    if(g_show&32) drawGUI(w,&g_ingamescene);

    return 1;
}

// --------------------------------------------------------------------
//
//   GAME SCENE SELECTORS
//
// --------------------------------------------------------------------

int MAP_read(INGAMESCENE*s,int world,int area,float ww)
{
 int  size;
 char nm[256];
 MAP*g_map=&s->map;
 sprintf(nm,"map.%d.%d.txt",world,area);
 g_map->map=os_readFILE(nm,&size);
 if(size>0)
  {
   int x=0,y,mx=0;
   while((x<size)&&(g_map->map[x]>=' ')) x++;
   while((x+mx<size)&&(g_map->map[x+mx]<' ')) mx++;
   g_map->w=x;
   g_map->rw=x+mx;
   g_map->h=size/(x+mx);
   g_map->tx=g_map->ty=ww;
   g_map->sx=g_map->w*g_map->tx;
   g_map->sy=g_map->h*g_map->ty;
   memset(&s->maptiles,0xFF,sizeof(s->maptiles));
   memset(&s->maptilesmask,0,sizeof(s->maptilesmask));
   for(y=0;y<g_map->h;y++)
    for(x=0;x<g_map->w;x++)
     {
      char c=MAP_get(s,x,y);
      if((c==' ')||((c>='A')&&(c<='Z')))
       ;
      else
       {
        switch(c)
         {
          case '*':
           {
            ACTOR*a=&g_actors[g_nActors++];
            a->x=(float)(x*s->map.tx)+s->map.tx/2;
            a->y=(float)((y+1)*s->map.ty);
            a->anim='s';
            a->act=PLAYER_action;
            a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"alienBlue_stand",&a->nFrames);
            a->layer=1;
            g_player=a;
           }
          break;
          case '$':
           if(g_show&8)
            {
             ACTOR*a=&g_actors[g_nActors++];
             a->x=(float)(x*s->map.tx)+s->map.tx/2;
             a->y=(float)((y+1)*s->map.ty);
             a->anim='s';
             a->act=SNAKE_action;
             a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"snakeSlime",&a->nFrames);
             a->animtickcnt=8+rand()%4;
            }
          break;
          case '!':
           if(g_show&8)
            {
             ACTOR*a=&g_actors[g_nActors++];
             a->x=(float)(x*s->map.tx)+s->map.tx/2;
             a->y=(float)((y+1)*s->map.ty);
             a->anim='w';
             a->act=WORM_action;
             a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"worm_walk",&a->nFrames);
             a->layer=1;
            }
          break;
          case ':':
           if(g_show&4)
            {
             ACTOR*a=&g_actors[g_nActors++];
             a->x=(float)(x*s->map.tx)+s->map.tx/2;
             a->y=(float)((y+1)*s->map.ty);
             a->anim='s';
             a->act=DIAMOND_action;
             a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"gemYellow",&a->nFrames);
            }
          break;
          case '.':
           if(g_show&4)
            {
             ACTOR*a=&g_actors[g_nActors++];
             a->x=(float)(x*s->map.tx)+s->map.tx/2;
             a->y=(float)((y+1)*s->map.ty);
             a->anim='s';
             a->act=COIN_action;
             a->curFrame=a->baseFrame=TEXATL_findanim(&s->textID,"coinGold",&a->nFrames);
            }
          break;
         }
        MAP_set(s,x,y,' ');
       }
     }
   return 1;
  }
 else
  return 0;
}

int MAP_delete(INGAMESCENE*s)
{
 free(s->map.map);
 s->map.map=NULL;
 return 1;
}

void LEVEL_gfxinit(WORLD*w)
{
 char nm[256];
 TEXATL_read("ken_new",&g_ingamescene.textID);
 #if defined(HAVE_STBIMAGE)
 sprintf(nm,"background.%d.jpg",w->world);
 g_ingamescene.backID=tex_read(nm,0);
 #else
 sprintf(nm,"background.%d.tga",w->world);
 g_ingamescene.backID=tex_readTGA(nm,0);
 #endif
}

void LEVEL_gfxreset(WORLD*w)
{
 tex_delete(&g_ingamescene.backID);
 TEXATL_delete(&g_ingamescene.textID);
}

int LEVEL_init(WORLD*w)
{
 int   id;
 float ww;

 g_nActors=0;
 memset(&g_actors,0,sizeof(g_actors));
 g_player=NULL;

 LEVEL_gfxinit(w);

 os_gfxratio=os_video_h/320.0f;
 id=TEXATL_find(&g_ingamescene.textID,"sandCenter");
 ww=g_ingamescene.textID.tai[id].w*os_gfxratio;

 if(MAP_read(&g_ingamescene,w->world,w->area,ww))
  {
   TEXATL*t=&g_ingamescene.textID;
   g_ingamescene.maptiles['A']=TEXATL_find(t,"sandCenter");
   g_ingamescene.maptilesmask['A']=1|2|4|8;
   g_ingamescene.maptiles['B']=TEXATL_find(t,"grassMid");
   g_ingamescene.maptilesmask['B']=1;
   g_ingamescene.maptiles['C']=TEXATL_find(t,"grassHalfLeft");
   g_ingamescene.maptilesmask['C']=1;
   g_ingamescene.maptiles['D']=TEXATL_find(t,"grassHalfMid");
   g_ingamescene.maptilesmask['D']=1;
   g_ingamescene.maptiles['E']=TEXATL_find(t,"grassHalfRight");
   g_ingamescene.maptilesmask['E']=1;
   g_ingamescene.maptiles['F']=TEXATL_find(t,"boxEmpty");
   g_ingamescene.maptilesmask['F']=1|2|4|8;
   g_ingamescene.maptiles['G']=TEXATL_find(t,"brickWall");
   g_ingamescene.maptilesmask['G']=1|2|4|8;
   g_ingamescene.maptiles['H']=TEXATL_find(t,"cactus");
   g_ingamescene.maptiles['I']=TEXATL_find(t,"signRight");
   g_ingamescene.maptiles['J']=TEXATL_find(t,"plant");
   adjustVIEWPORT(&g_ingamescene.map,g_player);
   EVENT_set(w,event_NEWLEVEL);
   return 1;
  }
 else
  return 0;
}

int LEVEL_reset(WORLD*w)
{
 MAP_delete(&g_ingamescene);
 LEVEL_gfxreset(w);
 return 1;
}

int INGAME_enter(WORLD*w)
{
    w->tempscore=w->score=0;
    w->lifes=3;
    w->world=1;
    w->area=1;

    LEVEL_init(w);

    return 1;
}

int INGAME_leave(WORLD*w)
{
    LEVEL_reset(w);
    return 1;
}

void INGAME_set(WORLD*w)
{
    if(g_G.leave) g_G.leave(w);
    g_G.enter=INGAME_enter;
    g_G.leave=INGAME_leave;
    g_G.handleUI=INGAME_handleUI;
    g_G.action=INGAME_action;
    g_G.postaction=INGAME_postaction;
    g_G.draw=INGAME_draw;
    if(g_G.enter) g_G.enter(w);
}

void HOME_set(WORLD*w)
{
    if(g_G.leave) g_G.leave(w);
    g_G.enter=NULL;
    g_G.leave=NULL;
    g_G.handleUI=HOME_handleUI;
    g_G.action=HOME_action;
    g_G.postaction=NULL;
    g_G.draw=HOME_draw;
    if(g_G.enter) g_G.enter(w);
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

    if(bSHOWTIME==0)
     g_show=-1;
    else
     g_show=0;

    if(g_show&64)
     HOME_set(w);
    else
     INGAME_set(w);

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
    #if defined(OS_WIN32)
        if(bSHOWTIME)
         {
          if(GetKeyState('0') & 0x80 )
          {
           if(g_G.draw==INGAME_draw)
             INGAME_set(w);
           else
             HOME_set(w);
          }
          if(GetKeyState('1') & 0x80 )
           g_show|=1;
          if(GetKeyState('2') & 0x80 )
           g_show|=2;
          if(GetKeyState('3') & 0x80 )
           g_show|=4;
          if(GetKeyState('4') & 0x80 )
           g_show|=8;
          if(GetKeyState('5') & 0x80 )
           g_show|=16;
          if(GetKeyState('6') & 0x80 )
           g_show|=32;
          if(GetKeyState('7') & 0x80 )
           g_show|=64;
          if(GetKeyState('8') & 0x80 )
           g_show|=128;
          if(GetKeyState('9') & 0x80 )
           g_show|=256;
         }
    #endif
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
 int ret;
 if(g_G.draw==INGAME_draw) LEVEL_gfxreset(w);
	GFXRES_reset();
	ret=GFXRES_init();
	if(g_G.draw==INGAME_draw) LEVEL_gfxinit(w);
	return ret;
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
