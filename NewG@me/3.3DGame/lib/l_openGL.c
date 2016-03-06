/* ---------------------------------------------------------------
   l_openGL.c
   openGL graphic (pretty basic) engine
   ---------------------------------------------------------------

   Copyright (c) 2013 by Marco Giorgini <marco.giorgini@gmail.com>
   This file is part of the New G@ame: Z@xxon (Loosy) Clone sample code.

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

#ifdef WIN32
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum target);
typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
PFNGLMULTITEXCOORD2FARBPROC			  glMultiTexCoord2fARB;
PFNGLACTIVETEXTUREARBPROC			    glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
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
#if defined(WIN32) || defined(MAC) || defined(OS_LINUX)
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

Texture tex_core_create(int width,int height,void*data,int mode)
{
    GLuint  textID;
    Texture t;
    if(mode==4)
        textID=SetTexture4444(width,height,data,0);
    else
        textID=SetTexture5551(width,height,data,0);
    t.textID=textID;
    t.sx=width;
    t.sy=height;
    return t;
}

void   tex_delete(Texture*textID)
{
    glDeleteTextures(1,&textID->textID);
    textID->textID=-1;
    textID->sx=textID->sy=0;
}

// ---------------------------------------------

void gfx_enable2D(int enable)
{
    if(enable)
        glEnable2D();
    else
        glDisable2D();
}

// ---------------------------------------------

#define M_PI 3.14
void PerspectiveMatrix(float fovy, float aspect, float zNear, float zFar)
{
    float f = 1.0f / (float)tan(fovy * (M_PI / 360.f));
    float z1 = (zFar + zNear) / (zNear - zFar);
    float z2 = (2 * zFar * zNear) / (zNear - zFar);
    float m[] =
    {
        f / aspect,  0,      0,      0,
        0,           f,      0,      0,
        0,           0,     z1,     -1,
        0,           0,     z2,      0
    };
    glMultMatrixf(m);
}

void SetPerspective()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(os_portrait)
    {
        glViewport(0,0,(int)os_video_w,(int)os_video_h);
        PerspectiveMatrix(45.0f, os_video_w / os_video_h, 0.5f, 64);
    }
    else
    {
        glViewport(0,0,(int)os_video_w,(int)os_video_h);
        PerspectiveMatrix(60.0f, os_video_h / os_video_w, 0.5f, 64);
    }
    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix
}

void gfx_enable3D(int enable)
{
    if(enable)
    {
        float mcolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor);

#ifdef WIN32
        glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)		wglGetProcAddress("glActiveTextureARB");
        glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)		wglGetProcAddress("glMultiTexCoord2fARB");
        glClientActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)   wglGetProcAddress("glClientActiveTextureARB");
        if(!glActiveTextureARB || !glMultiTexCoord2fARB)
            printf("glActiveTextureARB or glMultiTexCoord2fARB not supported");
#else
#endif

        SetPerspective();

        glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
        //glClearDepth(1.0f);									// Depth Buffer Setup
        glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
        glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    }
    else
    {
        glDisable(GL_DEPTH_TEST);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

// ---------------------------------------------

void gfx_cleanSCREEN()
{
    os_tris_2d=os_tris_3d=0;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
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
        os_tris_2d+=2;


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
void gfx_drawSPRITE(float x,float y,float w,float h,Texture*textID,float tx,float ty,float tw,float th,float alpha,float scale,int dir)
{
    if(alpha<=0)
        ;
    else
    {
        GLfloat vertex[3*4];
        GLfloat coord[2*4];
        int     v=0,p=0,itsx=textID->sx,itsy=textID->sy;
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

        glEnableTexture2D((GLuint)textID->textID);

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


        if(alpha!=1.0f)
         glColor4f(1,1,1,alpha);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if(alpha!=1.0f)
         glColor4f(1,1,1,1);

        os_tris_2d++;

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

void gfx_getPROJECTIONMATRIX(float*proj)
{
    glGetFloatv(GL_PROJECTION_MATRIX,proj);
}

void gfx_getMODELVIEWMATRIX(float*modl)
{
    glGetFloatv(GL_MODELVIEW_MATRIX,modl);
}

// ---------------------------------------------

// code from  Mesa or The Mesa 3-D graphics library
// License: http://www.mesa3d.org/license.html
// Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.

void gfx_LookAt(float eyex, float eyey, float eyez,
                float centerx, float centery, float centerz,
                float upx, float upy, GLfloat upz)
{
    float zoomF=1.0f;
    GLfloat m[16];
    GLfloat x[3], y[3], z[3];
    GLfloat mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = (eyex - centerx);
    z[1] = (eyey - centery);
    z[2] = (eyez - centerz);
    mag = (GLfloat)sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag)
    {
        /* mpichler, 19950515 */
        z[0] /= (mag*zoomF);
        z[1] /= (mag*zoomF);
        z[2] /= (mag*zoomF);
    }

    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    mag =(GLfloat)sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag)
    {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = (GLfloat)sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag)
    {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

#define M(row,col) m[col*4+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    glMultMatrixf(m);

    glTranslatef(-eyex, -eyey, -eyez);
}

// ---------------------------------------------

void gfx_enablefog(int enable,float r,float g,float b,float start,float end)
{
    if(enable)
    {
        float fogColor[4]= {0.5f,0.5f,0.5f,0.5f};
        float g_FogDensity=1.0f;
        fogColor[0]=r;
        fogColor[1]=g;
        fogColor[2]=b;
        glFogf(GL_FOG_MODE,GL_LINEAR);
        glFogfv(GL_FOG_COLOR,fogColor);
        glFogf(GL_FOG_DENSITY,g_FogDensity);
        glHint(GL_FOG_HINT,GL_DONT_CARE);
        glFogf(GL_FOG_START,start);
        glFogf(GL_FOG_END,end);
        glEnable(GL_FOG);
    }
    else
        glDisable(GL_FOG);
}

// ---------------------------------------------

int quickmesh_draw(quickMESH*mesh,Texture*textID,coord3D*obj)
{
    if(obj)
    {
        glPushMatrix();
        glTranslatef(obj->pos.x, obj->pos.y, obj->pos.z);
        if((obj->scale!=1.0f)&&(obj->scale!=0.0f)) glScalef(obj->scale, obj->scale, obj->scale);
        glRotatef(obj->rot.x, 1.0f, 0.0f, 0.0f);
        glRotatef(obj->rot.y, 0.0f, 1.0f, 0.0f);
        glRotatef(obj->rot.z, 0.0f, 0.0f, 1.0f);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,textID->textID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexCoordPointer(2,GL_FLOAT, 0, mesh->tex);
    glVertexPointer(3, GL_FLOAT, 0, mesh->vert);
    glDrawArrays(GL_TRIANGLES, 0, mesh->tris*3);

    os_tris_3d+=mesh->tris*3;

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);

    if(obj)
        glPopMatrix();
    return 1;
}

// ---------------------------------------------

int addtile(float*ver,float*tex,
            int v,int t,
            float x1,float z1,float y1,
            float x2,float z2,float y2,
            float x3,float z3,float y4,
            float x4,float z4,float y3,
            float tx1,float ty1,float tx2,float ty2)
{
    int h=0;

    ver[v++]=x1;
    ver[v++]=y1;
    ver[v++]=z1;
    if(tex)
    {
        tex[t++]=tx1;
        tex[t++]=ty1;
    }
    ver[v++]=x2;
    ver[v++]=y2;
    ver[v++]=z2;
    if(tex)
    {
        tex[t++]=tx2;
        tex[t++]=ty1;
    }
    ver[v++]=x3;
    ver[v++]=y3;
    ver[v++]=z3;
    if(tex)
    {
        tex[t++]=tx2;
        tex[t++]=ty2;
    }
    h++;

    ver[v++]=x3;
    ver[v++]=y3;
    ver[v++]=z3;
    if(tex)
    {
        tex[t++]=tx2;
        tex[t++]=ty2;
    }
    ver[v++]=x1;
    ver[v++]=y1;
    ver[v++]=z1;
    if(tex)
    {
        tex[t++]=tx1;
        tex[t++]=ty1;
    }
    ver[v++]=x4;
    ver[v++]=y4;
    ver[v++]=z4;
    if(tex)
    {
        tex[t++]=tx1;
        tex[t++]=ty2;
    }
    h++;

    return h;
}

void gfx_test3D(quickMESH*mesh,int textID)
{
    static float rtri;

    glTranslatef(0.0f,0.0f,-1.0f);
    glRotatef(rtri,0.0f,1.0f,0.0f);
    glRotatef(rtri,1.0f,0.0f,0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,textID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(mesh==NULL)
    {
        int v=0,t=0,h=0;
        glTexCoordPointer(2,GL_FLOAT, 0, gfx_tex);
        glVertexPointer(3, GL_FLOAT, 0, gfx_ver);

        h=addtile(gfx_ver,gfx_tex,v,t,-1,-1,0,1,-1,0,1,1,0,-1,1,0,0,0,1,1);
        v+=3*3*h;
        t+=3*2*h;

        h=addtile(gfx_ver,gfx_tex,v,t,-1,-1, 0,
                  1,-1, 0,
                  1,-1, 2,
                  -1,-1, 2,
                  0,0,1,1);
        v+=3*3*h;
        t+=3*2*h;

        h=addtile(gfx_ver,gfx_tex,v,t,-1, 1, 0,
                  1, 1, 0,
                  1, 1, 2,
                  -1, 1, 2,
                  0,0,1,1);
        v+=3*3*h;
        t+=3*2*h;
        glDrawArrays(GL_TRIANGLES, 0, v/3);
    }
    else
    {
        glTexCoordPointer(2,GL_FLOAT, 0, mesh->tex);
        glVertexPointer(3, GL_FLOAT, 0, mesh->vert);
        glDrawArrays(GL_TRIANGLES, 0, mesh->tris*3);

    }


    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);

    rtri+=0.2f;
}

// ---------------------------------------------
