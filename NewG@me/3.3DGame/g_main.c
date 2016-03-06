/* ---------------------------------------------------------------
   g_main.c
   Game code
   ---------------------------------------------------------------
   SHIP MODEL from: http://www.3drt.com [free models collection]
   http://www.3drt.com/3dm/free/Free-game-models-collection.zip
   License:
    Free models from this downloads section can not be used for any commercial purposes.
    We provide these free samples for testing and educational purposes only.
   ---------------------------------------------------------------
   FONT from http://www.bigmessowires.com/
   http://www.bigmessowires.com/atarifont.png
   ---------------------------------------------------------------
   TEXTURE from www.philipk.net | Philip Klevestav
   http://www.philipk.net/portfolio/textures/ancient_collection.zip
   Licence: Creative Commons Attribution 3.0 Unported License.
     <div xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/" about="http://www.philipk.net/"><span property="dct:title">PK01, PK02 and Ancient Collection Texture packages</span> (<a rel="cc:attributionURL" property="cc:attributionName" href="http://www.philipk.net">Philip Klevestav</a>) / <a rel="license" href="http://creativecommons.org/licenses/by/3.0/">CC BY 3.0</a></div>
   ---------------------------------------------------------------
   SKYMAP from Hipshot www.zfight.com
   ---------------------------------------------------------------

   Copyright (c) 2013-2014 by Marco Giorgini <marco.giorgini@gmail.com>
   This file is part of the New G@ame: Z@xxon Clone sample code.

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

Texture   g_textID[2];
Texture   g_fontID;
Texture   g_shipID;
Texture   g_towerID;

quickMESH g_meshSHIP;
quickMESH g_meshTOWER;
int       bUseTOWERMODEL=0;
int       bSHOWTIME=1;

int       screen_sx=20;
int       screen_sy=11;

#define show_drawroad      1
#define show_drawroadfull  2
#define show_drawsky       4
#define show_moveroad      8
#define show_drawtower    16
#define show_collision    32
#define show_drawgui      64
#define show_drawhome    128
#define show_drawhome3D  256

int       g_show=-1;

// --------------------------------------------------------------------

AREA2D l_startBOX;

// --------------------------------------------------------------------

void INGAME_set(WORLD*w);
void HOME_set(WORLD*w);

// --------------------------------------------------------------------

// --------------------------------------------------------------------

void EVENT_set(WORLD*w,int event,int timer)
{
    w->event=event;
    w->event_timer=os_getMilliseconds()+timer;
}

// --------------------------------------------------------------------


// --------------------------------------------------------------------
//
//   MATCH INIT/RESET
//
// --------------------------------------------------------------------

void MATCH_init(WORLD*w)
{
    float y;
    int   i;
    gfx_enable3D(1);

    camera3D_new(&w->camera);
    w->score=0;

    memset(&w->ship,0,sizeof(w->ship));
    w->ship.mesh=&g_meshSHIP;
    w->ship.textID=g_shipID;
    vector3_new(&w->ship.obj.pos,0,0,0);
    vector3_new(&w->ship.obj.rot,0,0,0);
    w->ship.obj.scale=1.0f;
    quickmesh_getAABB(w->ship.mesh,&w->ship.aabb);

    w->itowers=8;

    for(y=10,i=0; i<w->itowers; i++)
    {
        w->towers[i].w=(float)(1+rand()%2);
        w->towers[i].x=(float)(-4+rand()%(8-(int)w->towers[i].w));
        w->towers[i].h=(rand()%4)*0.25f;
        y=w->towers[i].y=y+6+rand()%12;
    }

    w->game_start=0;

    w->hview=0.4f;

    w->speed=0.25f;
}

void MATCH_reset(WORLD*w)
{
}


// --------------------------------------------------------------------
//
//   GFX RESOURCE LOAD/UNLOAD
//
// --------------------------------------------------------------------

int GFXRES_init()
{
    #if defined(HAVE_STBIMAGE)
    g_textID[0]=tex_read("texture.jpg",0); // http://www.art.eonworks.com/gallery/texture/metal_textures-200514.html
    g_textID[1]=tex_read("skymap.jpg",0); // http://www.art.eonworks.com/gallery/texture/metal_textures-200514.html
    g_fontID=tex_read("font.png",0);
    #else
    g_textID[0]=tex_readTGA("texture.tga",0); // http://www.art.eonworks.com/gallery/texture/metal_textures-200514.html
    g_textID[1]=tex_readTGA("skymap.tga",0); // http://www.art.eonworks.com/gallery/texture/metal_textures-200514.html
    g_fontID=tex_readTGA("font.tga",0);
    #endif

    quickmesh_readMD2(&g_meshSHIP,"ship.md2",&g_shipID);

    if(bUseTOWERMODEL)
     quickmesh_readMD2(&g_meshTOWER,"build03emp-.md2",&g_towerID);

    return (g_textID[0].textID!=-1)&&(g_textID[1].textID!=-1)&&(g_fontID.textID!=-1);
}

int GFXRES_reset()
{
    if(bUseTOWERMODEL)
     {
      quickmesh_delete(&g_meshTOWER);
      tex_delete(&g_towerID);
     }

    quickmesh_delete(&g_meshSHIP);
    tex_delete(&g_shipID);

    tex_delete(&g_textID[0]);
    tex_delete(&g_textID[1]);
    tex_delete(&g_fontID);
    return 1;
}

// --------------------------------------------------------------------
//
//   DRAW FUNCTIONS
//
// --------------------------------------------------------------------

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

void drawGUI(WORLD*w)
{
    float sx=os_video_w/screen_sx;
    float sy=os_video_h/screen_sy;
    int   cur_time=os_getMilliseconds();
    if(w->event)
    {

        float ww=10*sx,h=2*sy;
        float x=(os_video_w-ww)/2,y=(os_video_h-h)/2;
        char  sz[32];
        *sz=0;

        switch(w->event)
        {
        case event_GETREADY:
        {
            float value=(float)(w->event_timer-cur_time)/1000.0f;
            w->hview=2.0f-value*2.0f;
            strcpy(sz,"GET READY");
        }
        break;
        case event_EXPLOSION:
        {
            float alpha=(float)(w->event_timer-cur_time)/500.0f;
            gfx_drawBOX(0,0,os_video_w,os_video_h,1,1,1-alpha,alpha);
        }
        break;
        case event_GAMEOVER:
        {
            float value=(float)(w->event_timer-cur_time)/2500.0f;
            w->hview=value*2.0f;
            strcpy(sz,"GAME OVER");
            gfx_drawBOX(0,0,os_video_w,os_video_h,0,0,0,1.0f-value);
        }
        break;
        }

        if(*sz)
            drawTEXT(x+(ww-strlen(sz)*sx/2)/2,y+(h-sy)/2,sx,sy,sz);

        if(cur_time>w->event_timer)
        {
            w->event_timer=0;
            switch(w->event)
            {
            case event_GETREADY:
                w->event=0;
                w->game_start=os_getMilliseconds();
                break;
            case event_EXPLOSION:
                EVENT_set(w,event_GAMEOVER,2500);
                break;
            case event_GAMEOVER:
                w->event=0;
                MATCH_reset(w);
                if(g_show&show_drawhome)
                 HOME_set(w);
                else
                 INGAME_set(w);
                break;
            }

        }
    }
    else
    if(g_show&show_drawgui)
    {
        char  sz[32];
        float px=0,py=0;

        if(w->hiscore<w->score)
            w->hiscore=w->score;

        sprintf(sz,"HIGH SCORE %05d",w->hiscore);
        px=os_video_w-sx/2-strlen(sz)*sx/2;
        py=0;
        drawTEXT(px,py,sx,sy,sz);

        sprintf(sz,"SCORE %05d",w->score);
        px=sx/2;
        py=0;
        drawTEXT(px,py,sx,sy,sz);
        gfx_drawBOX(px,py+sy,sx*6*w->speed,sy/4.0f,0,0,0.5f,0.5f);


        sprintf(sz,"TRIS %05d",os_tris_3d);
        px=sx/2;
        py=os_video_h-sy;
        drawTEXT(px,py,sx,sy,sz);

    }
}

void drawHOME(WORLD*w)
{
    float sx=os_video_w/screen_sx;
    float sy=os_video_h/screen_sy;
    float brd=sx/4;
    float x,y,ww,h;
    float px,py;
    char  sz[64];

    x=sx;
    y=sy;
    ww=os_video_w-sx*2;
    h=os_video_h-sy*2;

    gfx_drawBOX(x-brd,y-brd,ww+brd*2,brd*2,0,0,0.5f,1);
    gfx_drawBOX(x-brd,y+h-brd,ww+brd*2,brd*2,0,0,0.5f,1);
    gfx_drawBOX(x-brd,y-brd,brd*2,h+brd*2,0,0,0.5f,1);
    gfx_drawBOX(x+ww-brd,y-brd,brd*2,h+brd*2,0,0,0.5f,1);

    gfx_drawBOX(x,y,ww,h,0,0,0,0.5f);

    py=1*sy;
    strcpy(sz,"Z@XXON CLONE");
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
//   INGAME DRAW
//
// --------------------------------------------------------------------

int addtile(float*ver,float*tex,
            int v,int t,
            float x1,float z1,float y1,
            float x2,float z2,float y2,
            float x3,float z3,float y4,
            float x4,float z4,float y3,
            float tx1,float ty1,float tx2,float ty2);

void drawSKY(WORLD*world)
{
    float     p=0;
    int       v=0,t=0,h=0;
    float     u1=1,v1=1,u2=0,v2=0;
    quickMESH sky;

    sky.vert=gfx_ver;
    sky.tex=gfx_tex;

    p=world->ship.obj.pos.x+40;

    h=addtile(gfx_ver,gfx_tex,v,t,p,-40,   15,
              p, 40,   15,
              p, 40,   -2.5f,
              p,-40,   -2.5f,
              u1,v1,u2,v2);
    v+=3*3*h;
    t+=3*2*h;

    sky.tris=v/9;
    quickmesh_draw(&sky,&g_textID[1],NULL);
}

void drawROAD(WORLD*world)
{
    float     p=0;
    int       v=0,t=0,h=0;
    float     u1=0,v1=0,u2=0.5f,v2=0.5f;
    quickMESH ground;

    if(g_show&show_drawroadfull)
     ;
    else
     p=4;

    ground.vert=gfx_ver;
    ground.tex=gfx_tex;

    while(p<world->ship.obj.pos.x+48)
    {

        if(p<world->ship.obj.pos.x-6)
        {
            p+=4;
            continue;
        }

        u1=0,v1=0.5f,u2=0.5f,v2=1.0f;

        // ground
        h=addtile(gfx_ver,gfx_tex,v,t, p,  -4,-1,
                  p+4,-4,-1,
                  p+4, 0,-1,
                  p,   0,-1,u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;
        h=addtile(gfx_ver,gfx_tex,v,t, p,   0,-1,
                  p+4, 0,-1,
                  p+4, 4,-1,
                  p,   4,-1,u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;

        // left
        u1=0,v1=0,u2=0.5f,v2=0.5f;
        h=addtile(gfx_ver,gfx_tex,v,t,p,  -4, -1,
                  p+4,-4, -1,
                  p+4,-4, 1,
                  p,  -4, 1,
                  u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;

        u1=0.5f,v1=0.0f,u2=1.0f,v2=0.5f;
        h=addtile(gfx_ver,gfx_tex,v,t,p,  -4, 1,
                  p+4,-4, 1,
                  p+4,-8, 1.5f,
                  p,  -8, 1.5f,
                  u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;

        // right
        u1=0,v1=0,u2=0.5f,v2=0.5f;
        h=addtile(gfx_ver,gfx_tex,v,t,p+4, 4, -1,
                  p,   4, -1,
                  p,   4, 1,
                  p+4, 4, 1,
                  u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;

        u1=0.5f,v1=0.0f,u2=1.0f,v2=0.5f;
        h=addtile(gfx_ver,gfx_tex,v,t,p+4, 4, 1,
                  p,   4, 1,
                  p,   8, 1.5f,
                  p+4, 8, 1.5f,
                  u1,v1,u2,v2);
        v+=3*3*h;
        t+=3*2*h;

        p+=4;

        if(g_show&show_drawroadfull)
         ;
        else
         break;
    }

    ground.tris=v/9;
    quickmesh_draw(&ground,&g_textID[0],NULL);

    if(g_show&show_collision)
     {
      if(world->ship.obj.pos.z+world->ship.aabb.zm<=-4)
          world->ship.status|=status_collision;
      else if(world->ship.obj.pos.z+world->ship.aabb.zM>=4)
          world->ship.status|=status_collision;
     }
}

void drawTOWERS(WORLD*world)
{
    int       v=0,t=0,h=0,i;
    float     u1=0,v1=0,u2=0.5f,v2=0.5f;
    quickMESH tower;

    tower.vert=gfx_ver;
    tower.tex=gfx_tex;

    for(i=0; i<world->itowers; i++)
    {
        int    bInside=0;
        float  p=world->towers[i].y;
        float  x=world->towers[i].x;
        float  w=world->towers[i].w;
        float  hh=1.25f+world->towers[i].h-1;
        AABB   aabb;

        v=t=0;

        if((x==-4)||(x+w==4))
        {
            u1=0,v1=0,u2=0.5f,v2=0.5f;
            bInside=0;
            hh=1;
        }
        else
        {
            u1=0.5f,v1=0.5f,u2=1.0f,v2=1.0f;
            bInside=1;
        }

        if(bInside&&(w==1)&&bUseTOWERMODEL)
         {
          coord3D obj;
          memset(&obj,0,sizeof(obj));
          obj.pos.x+=p;
          obj.pos.y+=-1;
          obj.pos.z+=x+0.5f;
          quickmesh_getAABB(&g_meshTOWER,&aabb);
          aabb.xM+=p;
          aabb.xm+=p;
          aabb.yM+=-1;
          aabb.ym+=-1;
          aabb.zM+=x+0.5f;
          aabb.zm+=x+0.5f;
          if(frustum3D_checkAABB(&world->frustum,&aabb))
          {
              quickmesh_draw(&g_meshTOWER,&g_towerID,&obj);
              if(g_show&show_collision)
               if(AABB_interset(&aabb,NULL,&world->ship.aabb,&world->ship.obj))
                  world->ship.status|=status_collision;
          }
         }
        else
         {
          float b=-1;
          if(w==8) {b=0.50f;v2=0.1f;}
          h=addtile(gfx_ver,gfx_tex,v,t, p,  x,b,
                    p+1,x,b,
                    p+1,x, hh,
                    p,  x, hh,
                    u1,v1,u2,v2);
          v+=3*3*h;
          t+=3*2*h;

          h=addtile(gfx_ver,gfx_tex,v,t, p,  x+w,b,
                    p+1,x+w,b,
                    p+1,x+w, hh,
                    p,  x+w, hh,
                    u1,v1,u2,v2);
          v+=3*3*h;
          t+=3*2*h;

          h=addtile(gfx_ver,gfx_tex,v,t, p+1,x,  b,
                    p+1,x+w,b,
                    p+1,x+w, hh,
                    p+1,x,   hh,
                    u1,v1,u2,v2);
          v+=3*3*h;
          t+=3*2*h;

          h=addtile(gfx_ver,gfx_tex,v,t, p,x,  b,
                    p,x+w,b,
                    p,x+w, hh,
                    p,x,   hh,
                    u1,v1,u2,v2);
          v+=3*3*h;
          t+=3*2*h;

          u1=0,v1=0.5f,u2=0.5f,v2=1.0f;
          h=addtile(gfx_ver,gfx_tex,v,t, p,x,    hh,
                    p+1,x,  hh,
                    p+1,x+w,hh,
                    p,x+w,  hh,
                    u1,v1,u2,v2);
          v+=3*3*h;
          t+=3*2*h;

          tower.tris=v/9;

          quickmesh_getAABB(&tower,&aabb);

          if(frustum3D_checkAABB(&world->frustum,&aabb))
          {
              quickmesh_draw(&tower,&g_textID[0],NULL);
              if(g_show&show_collision)
               if(AABB_interset(&aabb,NULL,&world->ship.aabb,&world->ship.obj))
                  world->ship.status|=status_collision;
          }
         }


        if(p<world->ship.obj.pos.x-6)
        {
            float faraway=0;
            int   j,kind=rand()%2;
            for(j=0; j<world->itowers; j++)
                if(world->towers[j].y>faraway)
                    faraway=world->towers[j].y;
            if(kind==0)
            {
                world->towers[i].w=(float)(1+rand()%5);
                world->towers[i].x=(float)(-3+rand()%(7-(int)world->towers[i].w));
                world->towers[i].y=faraway+6+rand()%12;
                world->towers[i].h=(rand()%4)*0.25f;
            }
            else
            {
                kind=rand()%2;
                if((rand()%4)==1)
                 world->towers[i].w=8;
                else
                 world->towers[i].w=(float)(1+rand()%4);
                if(kind==0)
                    world->towers[i].x=-4;
                else
                    world->towers[i].x=4-world->towers[i].w;
                world->towers[i].y=faraway+6+rand()%12;
                world->towers[i].h=(rand()%4)*0.25f;
            }
        }
    }

}

void drawSHIP(WORLD*w)
{
    if(w->ship.status&status_collision_handled)
        ;
    else
        quickmesh_draw(w->ship.mesh,&w->ship.textID,&w->ship.obj);
}


// --------------------------------------------------------------------
//
//   INGAME CALLBACKS
//
// --------------------------------------------------------------------

int INGAME_handleUI(WORLD*w)
{
    w->keys=0;
    if(w->ship.status&status_collision_handled)
        ;
    else
    {
        int i;
#if defined(OS_WIN32)
        if(bSHOWTIME)
         {
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

        if(GetKeyState(VK_UP) & 0x80 )
        {
            if(w->hview<3.0f)
                w->hview+=0.2f;
        }
        else if(GetKeyState(VK_DOWN) & 0x80 )
        {
            if(w->hview>0.6)
                w->hview-=0.2f;
        }
        else if(GetKeyState(VK_LEFT) & 0x80 )
            w->keys=dirLEFT;
        else if(GetKeyState(VK_RIGHT) & 0x80 )
            w->keys=dirRIGHT;
#elif defined(OS_MAC)||defined(OS_LINUX)        
        if(os_keys[GLFW_KEY_UP])
        {
            if(w->hview<3.0f)
                w->hview+=0.2f;
        }
        else if(os_keys[GLFW_KEY_DOWN])
        {
            if(w->hview>0.6)
                w->hview-=0.2f;
        }
        else if(os_keys[GLFW_KEY_LEFT])
            w->keys=dirLEFT;
        else if(os_keys[GLFW_KEY_RIGHT])
            w->keys=dirRIGHT;        
#endif
        for(i=0; i<os_np; i++)
        {
            if((os_status[i]!=touchNONE)&&(os_x[i]<os_video_w/4))
            {
                w->keys=dirLEFT;
            }
            else if((os_status[i]!=touchNONE)&&(os_x[i]>os_video_w*3/4))
            {
                w->keys=dirRIGHT;
            }
            if(os_status[i]==touchUP)
                os_status[i]=touchNONE;
        }

    }
    return 1;
}

int INGAME_action(WORLD*w)
{
    if(w->ship.status&status_collision_handled)
    {
        if(w->speed>0.1f)
            w->speed=w->speed-0.01f;
        else
            w->speed=0.1f;
    }
    else
    {
        if(w->keys)
        {
            if(w->keys==dirRIGHT)
            {
                if(g_show&show_moveroad)
                if(w->ship.obj.rot.x<30.0f)
                    w->ship.obj.rot.x+=1.50f;
                w->ship.obj.pos.z+=0.05f*3.0f;
            }
            else if(w->keys==dirLEFT)
            {
                w->ship.obj.pos.z-=0.05f*3.0f;
                if(g_show&show_moveroad)
                if(w->ship.obj.rot.x>-30.0f)
                    w->ship.obj.rot.x-=1.50f;
            }
        }
        else
        if(g_show&show_moveroad)
         if(w->ship.obj.rot.x!=0.0f)
            w->ship.obj.rot.x+=(0.0f-w->ship.obj.rot.x)/16.0f;

        if(w->game_start)
            w->score=((os_getMilliseconds()-w->game_start)/500)*5;
        if(g_show&show_drawgui)
        if(w->speed<1.0f)
            w->speed+=0.0005f;
    }

    if(g_show&show_moveroad)
     w->ship.obj.pos.x+=w->speed;


    return 1;
}

int INGAME_postaction(WORLD*w)
{
    if(w->ship.status&status_collision)
    {
        if(w->ship.status&status_collision_handled)
        {
        }
        else
        {
            w->ship.status|=status_collision_handled;
            if(w->ship.obj.pos.z+w->ship.aabb.zm<=-4)
                w->ship.obj.pos.z=-4-w->ship.aabb.zm*2;
            else if(w->ship.obj.pos.z+w->ship.aabb.zM>=4)
                w->ship.obj.pos.z=4-w->ship.aabb.zM*2;
            EVENT_set(w,event_EXPLOSION,500);
        }
    }
    return 1;
}

void INGAME_setcamera(WORLD*w)
{
    if(g_show==0)
     {
      static float angle=0;
      static float pi = (float)3.1415926535;
      float ofy=(float)(1.0f*sin(angle*pi/180));
      float ofz=(float)(1.0f*cos(angle*pi/180));
      camera3D_setPOSITION(&w->camera,w->ship.obj.pos.x+ofy,w->ship.obj.pos.y+1.0f,w->ship.obj.pos.z+ofz,
                           w->ship.obj.pos.x,w->ship.obj.pos.y,w->ship.obj.pos.z,
                           0,1,0);
      angle+=0.4f;
     }
    else
     camera3D_setPOSITION(&w->camera,w->ship.obj.pos.x-2.5f,w->ship.obj.pos.y+w->hview,w->ship.obj.pos.z,
                          w->ship.obj.pos.x+2.0f,w->ship.obj.pos.y,w->ship.obj.pos.z,
                          0,1,0);
    camera3D_lookAT(&w->camera);

    frustum3D_setVIEW(&w->frustum,os_video_w,os_video_h);
}

int INGAME_draw(WORLD*w)
{
    gfx_cleanSCREEN();

    INGAME_setcamera(w);

    w->ship.status=w->ship.status&0x2;

    if(g_show&show_drawsky)
     drawSKY(w);

    gfx_enablefog(1,0.0f,0.0f,0.0f,4,34);
    if(g_show&show_drawroad)
     drawROAD(w);

    if(g_show&show_drawtower)
     drawTOWERS(w);

    drawSHIP(w);

    gfx_enablefog(0,0,0,0,4,34);

    if(g_show&show_collision)
     drawGUI(w);

    return 1;
}

// --------------------------------------------------------------------
//
//   HOME CALLBACKS
//
// --------------------------------------------------------------------

float vx1=-2.4f,vy1=3.7f,vz1=-4.7f;
float vx2=14.9f,vy2=-12.8f,vz2=7.2f;

int HOME_handleUI(WORLD*w)
{
    int   i;
    float sx=os_video_w/screen_sx;
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
    #ifdef WIN32
    /*
          if(GetKeyState('1') & 0x80 )
           vx1+=0.1f;
          if(GetKeyState('2') & 0x80 )
           vx1-=0.1f;
          if(GetKeyState('3') & 0x80 )
           vy1+=0.1f;
          if(GetKeyState('4') & 0x80 )
           vy1-=0.1f;
          if(GetKeyState('5') & 0x80 )
           vz1+=0.1f;
          if(GetKeyState('6') & 0x80 )
           vz1-=0.1f;

         if(GetKeyState('Q') & 0x80 )
           vx2+=0.1f;
         if(GetKeyState('W') & 0x80 )
          vx2-=0.1f;
         if(GetKeyState('E') & 0x80 )
          vy2+=0.1f;
         if(GetKeyState('R') & 0x80 )
          vy2-=0.1f;
         if(GetKeyState('T') & 0x80 )
          vz2+=0.1f;
         if(GetKeyState('Y') & 0x80 )
          vz2-=0.1f;
    */
    if(GetKeyState('9') & 0x80 )
          g_show|=show_drawhome3D;
    if(GetKeyState('0') & 0x80 )
     if(g_show&show_drawhome3D)
          g_show-=show_drawhome3D;

    #endif
    return 1;
}

int HOME_action(WORLD*w)
{
    if(w->keys)
    {
        INGAME_set(w);
    }
    w->ship.obj.pos.x+=0.4f;
    return 1;
}

void HOME_setcamera(WORLD*w)
{
    camera3D_setPOSITION(&w->camera,w->ship.obj.pos.x+vx1,w->ship.obj.pos.y+vy1,w->ship.obj.pos.z+vz1,
                          w->ship.obj.pos.x+vx2,w->ship.obj.pos.y+vy2,w->ship.obj.pos.z+vz2,
                          0,1,0);
    camera3D_lookAT(&w->camera);

    frustum3D_setVIEW(&w->frustum,os_video_w,os_video_h);
}

int HOME_draw(WORLD*w)
{
    gfx_cleanSCREEN();

    if(g_show&show_drawhome3D)
     {
      HOME_setcamera(w);
      drawSKY(w);
      gfx_enablefog(1,0.0f,0.0f,0.0f,4,34);
      drawROAD(w);
      drawTOWERS(w);
      //drawSHIP(w);
      gfx_enablefog(0,0,0,0,4,34);
     }

    drawHOME(w);
    return 1;
}

// --------------------------------------------------------------------
//
//   GAME SCENE SELECTORS
//
// --------------------------------------------------------------------

void HOME_set(WORLD*w)
{
    g_G.handleUI=HOME_handleUI;
    g_G.action=HOME_action;
    g_G.postaction=NULL;
    g_G.draw=HOME_draw;
    w->keys=0;

    MATCH_init(w);
}

void INGAME_set(WORLD*w)
{
    g_G.handleUI=INGAME_handleUI;
    g_G.action=INGAME_action;
    g_G.postaction=INGAME_postaction;
    g_G.draw=INGAME_draw;

    w->keys=0;

    MATCH_init(w);

    EVENT_set(w,event_GETREADY,1000);
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

    GFXRES_init();

    if(bSHOWTIME==0)
     g_show=-1;

    if(g_show&show_drawhome)
     HOME_set(w);
    else
     INGAME_set(w);

    return 1;
}

int GAME_reset(WORLD*w)
{
    GFXRES_reset();

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
