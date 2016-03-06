/* ---------------------------------------------------------------
   l_openGL.c
   openGL graphic (pretty basic) engine
   ---------------------------------------------------------------

   Copyright (c) 2013 by Marco Giorgini <marco.giorgini@gmail.com>
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
#include "l_module.h"

int   g_iTris;
int   bDontDisableTextures;
int   bAuto2D=1;
int   i_glEnableBLEND,i_glEnableTexture2D,i_textID=-1;

#ifdef WIN32
#define GL_UNSIGNED_SHORT_5_6_5     0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_5_5_5_1   0x8034
#define GL_UNSIGNED_SHORT_4_4_4_4   0x8033
#define GL_UNSIGNED_INT_8_8_8_8     0x8035
#define GL_CLAMP_TO_EDGE	0x812F						// This is for our skybox textures
#endif

GLuint SetTexture5551(int width,int height,void*data,int flags)
{
    GLuint g_Texture;
    GLint  g_TextureS;
    glGenTextures(1, &g_Texture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &g_TextureS);
    glBindTexture(GL_TEXTURE_2D, g_Texture);
    if(flags&1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef WIN32
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, data);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, data);
#endif
    return g_Texture;
}

GLuint SetTexture4444(int width,int height,void*data,int flags)
{
    GLuint g_Texture;
    GLint  g_TextureS;
    glGenTextures(1, &g_Texture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &g_TextureS);
    glBindTexture(GL_TEXTURE_2D, g_Texture);
    if(flags&1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef WIN32
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, data);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, data);
#endif
    return g_Texture;
}

void glEnable2D()
{
    if(bAuto2D)
    {
        int vPort[4];
        glGetIntegerv(GL_VIEWPORT, vPort);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
#if defined(WIN32) || defined(OS_MAC)|| defined(OS_LINUX)
        glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
#else
        glOrthof(0, vPort[2], 0, vPort[3], -1, 1);
#endif
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
    }
}

void glDisable2D()
{
    if(bAuto2D)
    {
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void glEnableBLEND()
{
    i_glEnableBLEND++;
    if(i_glEnableBLEND==1)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
}

void glEnableTexture2D(int textID)
{
    i_glEnableTexture2D++;
    if(i_glEnableTexture2D==1)
    {
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    if(i_textID!=textID)
    {
        if(textID!=-1)
            glBindTexture(GL_TEXTURE_2D,textID);
        i_textID=textID;
    }
}

void glDisableTexture2D()
{
    i_glEnableTexture2D--;
    if(i_glEnableTexture2D==0)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
        i_textID=-1;
    }
}

void glDisableBLEND()
{
    i_glEnableBLEND--;
    if(i_glEnableBLEND==0)
        glDisable(GL_BLEND);
}

// ---------------------------------------------

int tex_core_create(int width,int height,void*data,int mode)
{
    GLuint textID;
    if(mode==4)
        textID=SetTexture4444(width,height,data,0);
    else
        textID=SetTexture5551(width,height,data,0);
    return (int)textID;
}

void   tex_delete(int*textID)
{
    glDeleteTextures(1,(GLuint*)textID);
    *textID=-1;
}

// ---------------------------------------------

void gfx_enable2D(int enable)
{
    if(enable)
        glEnable2D();
    else
        glDisable2D();
}

void gfx_cleanSCREEN()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_drawBOX(float x,float y,float w,float h,float r,float g,float b,float alpha)
{
    if(alpha<=0)
        ;
    else
    {
        GLfloat vertex[3*4];
        int     v=0;

        glEnable2D();

        if(!os_portrait)
            ;
        else
            y=os_video_h-(y+h);

        glEnableBLEND();

        glLoadIdentity();

        if(bDontDisableTextures)
            ;
        else
        {
            glEnableTexture2D(-1);

            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }


        glColor4f(r,g,b,alpha);


        glVertexPointer(3, GL_FLOAT, 0, vertex);


        if(!os_portrait)
        {
            float t=x;
            x=y;
            y=t;
            t=w;
            w=h;
            h=t;
        }

        vertex[v++]=x+0;
        vertex[v++]=y+0;
        vertex[v++]=0;

        vertex[v++]=x+0;
        vertex[v++]=y+h;
        vertex[v++]=0;

        vertex[v++]=x+w;
        vertex[v++]=y+0;
        vertex[v++]=0;

        vertex[v++]=x+w;
        vertex[v++]=y+h;
        vertex[v++]=0;

        if(os_flip)
        {
            int i;
            if(!os_portrait)
                for(i=0; i<v; i+=3)
                {
                    vertex[i]=os_video_h-vertex[i];
                    vertex[i+1]=os_video_w-vertex[i+1];
                }
            else
                for(i=0; i<v; i+=3)
                {
                    vertex[i]=os_video_w-vertex[i];
                    vertex[i+1]=os_video_h-vertex[i+1];
                }
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        g_iTris++;


        glColor4f(1,1,1,1);


        if(bDontDisableTextures)
            ;
        else
        {
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glDisableTexture2D();
        }

        glDisableBLEND();

        glDisable2D();
    }
}

#define ROUND(a) a
void gfx_drawSPRITE(float x,float y,float w,float h,int textID,float tx,float ty,float tw,float th,float alpha,float scale,int dir)
{
    if(alpha<=0)
        ;
    else
    {
        GLfloat vertex[3*4];
        GLfloat coord[2*4];
        int     v=0,p=0,itsx=256,itsy=256;
        float   tsx,tsy;

        tsx=(float)itsx;
        tsy=(float)itsy;

        glEnable2D();

        if(!os_portrait)
            ;
        else
            y=((float)os_video_h-(y+h*scale));

        ty=tsy-th-ty;

        glEnableBLEND();

        glLoadIdentity();

        glEnableTexture2D((GLuint)textID);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2,GL_FLOAT, 0, coord);

        if(!os_portrait)
        {
            if(dir==0)
            {
                coord[p++]=tx/tsx;//(tx+w)/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=(tx+tw)/tsx;//tx/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=tx/tsx;//(tx+w)/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=(tx+tw)/tsx;//tx/tsx;
                coord[p++]=ty/tsy;
            }
            else
            {
                coord[p++]=(tx+tw)/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=tx/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=(tx+tw)/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=tx/tsx;
                coord[p++]=ty/tsy;
            }
        }
        else
        {
            if(dir==0)
            {
                coord[p++]=tx/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=tx/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=(tx+tw)/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=(tx+tw)/tsx;
                coord[p++]=(ty+th)/tsy;
            }
            else
            {
                coord[p++]=(tx+tw)/tsx;//tx/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=(tx+tw)/tsx;//tx/tsx;
                coord[p++]=(ty+th)/tsy;
                coord[p++]=tx/tsx;//(tx+w)/tsx;
                coord[p++]=ty/tsy;
                coord[p++]=tx/tsx;//(tx+w)/tsx;
                coord[p++]=(ty+th)/tsy;
            }
        }

        if(!os_portrait)
        {
            float t=x;
            x=y;
            y=t;
            t=w;
            w=h;
            h=t;
        }

        vertex[v++]=(float)ROUND(x+0);
        vertex[v++]=(float)ROUND(y+0);
        vertex[v++]=0;

        vertex[v++]=(float)ROUND(x+0);
        vertex[v++]=(float)(ROUND(y)+ROUND(h*scale));
        vertex[v++]=0;

        vertex[v++]=(float)ROUND(x)+ROUND(w*scale);
        vertex[v++]=(float)ROUND(y+0);
        vertex[v++]=0;

        vertex[v++]=(float)ROUND(x)+ROUND(w*scale);
        vertex[v++]=(float)(ROUND(y)+ROUND(h*scale));
        vertex[v++]=0;

        if(os_flip)
        {
            int i;
            if(!os_portrait)
                for(i=0; i<v; i+=3)
                {
                    vertex[i]=os_video_h-vertex[i];
                    vertex[i+1]=os_video_w-vertex[i+1];
                }
            else
                for(i=0; i<v; i+=3)
                {
                    vertex[i]=os_video_w-vertex[i];
                    vertex[i+1]=os_video_h-vertex[i+1];
                }
        }


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        g_iTris++;

        glDisableTexture2D();

        glDisableBLEND();

        glColor4f(1,1,1,1);

        glDisable2D();
    }
}

int  gfx_init()
{
    return 1;
}

int  gfx_reset()
{
    return 1;
}

// ---------------------------------------------
