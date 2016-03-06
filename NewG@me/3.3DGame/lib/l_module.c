/* ---------------------------------------------------------------
   l_module.c
   Generic library code (deferring implementation to engines)
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

//  -------------------------------------------------------------

int             os_portrait=1,os_flip=0,os_touch_flip=0;
float           os_roll=50.0,os_pitch=50.0,os_Z=95.0;
int             os_x[10],os_y[10],os_status[10],os_np,os_keys[1024];
float           os_scale,os_gfxratio=1;
float           os_video_w,os_video_h;
char            os_szResPath[256];
int             os_BACK_pressed;
int             os_PAUSE;

//  -------------------------------------------------------------

int             os_tris_3d,os_tris_2d;

//  -------------------------------------------------------------

float           gfx_tex[2*3*GFX3D_CACHE];
float           gfx_texM[2*3*GFX3D_CACHE];
float           gfx_ver[3*3*GFX3D_CACHE];
unsigned char   gfx_light[4*3*GFX3D_CACHE];

//  -------------------------------------------------------------

#ifdef WIN32
int os_getMilliseconds(void)
{
    static int freq=0;
    static LARGE_INTEGER t_freq;
    LARGE_INTEGER t_1;
    if(freq==0)
    {
        QueryPerformanceFrequency(&t_freq);
        freq=1;
    }
    QueryPerformanceCounter(&t_1);
    return (int)(((double)t_1.QuadPart/(double)t_freq.QuadPart)*1000.0);
}
#else
#if defined(OS_IPHONE) || defined(OS_ANDROID) || defined(OS_MAC) || defined(OS_LINUX)
#include <sys/time.h>
int curtime;
int Sys_Milliseconds( void )
{
    struct timeval tp;
    struct timezone tzp;
    static int		secbase;

    gettimeofday( &tp, &tzp );

    if( ! secbase )
    {
        secbase = tp.tv_sec;
        return tp.tv_usec / 1000;
    }

    curtime = (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;

    return curtime;
}
int Sys_Nanoseconds( void )
{
    struct timeval tp;
    struct timezone tzp;
    static int		secbase;

    gettimeofday( &tp, &tzp );

    if( ! secbase )
    {
        secbase = tp.tv_sec;
        return tp.tv_usec ;
    }

    curtime = (tp.tv_sec - secbase) * 1000000 + tp.tv_usec ;

    return curtime;
}
#endif
int os_getMilliseconds(void)
{
    return Sys_Milliseconds();
}
#endif

//  -------------------------------------------------------------

#if defined(OS_ANDROID)
#include "zip/unzip.h"
unzFile zipPACK = NULL;
void os_init(const char*szname,float sx,float sy)
{
     os_video_w=sx;
     os_video_h=sy;
	 strcpy(os_szResPath, szname);
	 zipPACK=unzOpen(os_szResPath);
}

void os_reset(void)
{
	if (zipPACK)
	{
		unzClose(zipPACK);
		zipPACK = NULL;
	}
}

void* os_readFILE(const char* szname,int* size)
{
	char path[256];
	strcpy(path, "assets/");
	strcat(path, szname);
	if(zipPACK)
	{
		int found=unzLocateFile(zipPACK,path,0);
		if(found==UNZ_OK)
		{
			void*p;
			unz_file_info info;
			unzGetCurrentFileInfo(zipPACK,&info,NULL,0,NULL,0,NULL,0);
			if(unzOpenCurrentFile(zipPACK)==UNZ_OK)
			{
				p=calloc(1,info.uncompressed_size+1);
				if((uLong)unzReadCurrentFile(zipPACK,p,info.uncompressed_size)==info.uncompressed_size)
				{
					if(size) *size=info.uncompressed_size;
					unzCloseCurrentFile(zipPACK);
					return p;
				}
			}
		}
	}
	return NULL;
}
#else
void os_init(const char*szname,float width,float height)
{
	strcpy(os_szResPath, szname);
	os_video_w=width;
    os_video_h=height;
}

void os_reset(void)
{
}
void*os_readFILE(const char*szname,int*size)
{
    char path[256];
    FILE*f;
    strcpy(path,os_szResPath);
    strcat(path,szname);
    f=fopen(path,"rb");
    if(f)
    {
        int  pp;
        void*p;
        fseek(f,0,2);
        pp=ftell(f);
        fseek(f,0,0);
        p=malloc(pp+8);
        fread(p,1,pp,f);
        fclose(f);
        ((unsigned char*)p)[pp]=0;
        if(size) *size=pp;
        return p;
    }
    else
        return NULL;
}
#endif

//  -------------------------------------------------------------

int AREA2D_isinbound(float x,float y,AREA2D*area,float bound)
{
    if((x<area->x-bound)||(x>area->x+area->w+bound)||(y<area->y-bound)||(y>area->y+area->h+bound))
        return 0;
    else
        return 1;
}

int AREA2D_intersect(AREA2D*a,AREA2D*b)
{
 return (a->x <= b->x+b->w) &&
        (b->x <= a->x+a->w) &&
        (a->y <= b->y+b->h) &&
        (b->y <= a->y+a->h);
}

//  -------------------------------------------------------------
//
//  GFX 2D: TGA loader
//
//  -------------------------------------------------------------
#if defined(HAVE_STBIMAGE)
void img_flip(unsigned char* src, int w, int h, int way)
{
 int x,y;
 unsigned char*tmp=(unsigned char*)malloc(w*4);
 for(y=0;y<h/2;y++)
  {
   unsigned char*s=src+y*w*4;
   unsigned char*d=src+(h-y-1)*w*4;
   memcpy(tmp,d,w*4);
   memcpy(d,s,w*4);
   memcpy(s,tmp,w*4);
  }
 free(tmp);
}
Texture tex_read(const char*s,int mode)
{
    Texture textID;
    int     pp;
    void   *mem=os_readFILE(s,&pp);
    memset(&textID,0,sizeof(textID));
    if(mem)
    {
        int comp,ret=0,width,height;
        unsigned char*pixel=stbi_load_from_memory((stbi_uc*)mem,pp,&width,&height,&comp,4);
        if(pixel)
        {
            unsigned short*data=(unsigned short*)((unsigned char*)pixel);
            {
                unsigned char*p=(unsigned char*)data;
                int           i=width*height,j=0;
                data=(unsigned short*)malloc(width*height*2);
                img_flip(pixel,width,height,0);
                while(i--)
                {
                    BYTE r=*p++;
                    BYTE g=*p++;
                    BYTE b=*p++;
                    BYTE a=*p++;
                    if(a==0)
                        data[j++]=0;
                    else
                    {
                        unsigned short val;
                        if(mode==4)
                        {
                            r >>= 4;
                            g >>= 4;
                            b >>= 4;
                            a >>= 4;
                            val=(unsigned short)a | (b << 4)  | (g << 8) | (r << 12);
                        }
                        else
                            val=(unsigned short)(((((((r)/8)<<10)+(((g)/8)<<5)+((b)/8)))<<1)|(a!=0));
                        data[j++]=val;
                    }
                }
                textID=tex_core_create(width,height,data,mode);
                free(data);
            }
        }
        free(pixel);
        free(mem);
    }
    return textID;
}
#endif
#pragma pack(1)
typedef struct TGAHEADER_TYPE
{
    char	identsize;              // Size of ID field that follows header (0)
    char	colorMapType;           // 0 = None, 1 = paletted
    char	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
    unsigned short	colorMapStart;  // First colour map entry
    unsigned short	colorMapLength; // Number of colors
    unsigned char 	colorMapBits;   // bits per palette entry
    unsigned short	xstart;         // image x origin
    unsigned short	ystart;         // image y origin
    unsigned short	width;          // width in pixels
    unsigned short	height;         // height in pixels
    char	bits;                   // bits per pixel (8, 16, 24, 32)
    char	descriptor;             // image descriptor
} TGAHEADER, *TGAHEADER_PTR;

Texture tex_readTGA(const char*s,int mode)
{
    Texture textID;
    int     pp;
    void   *mem=os_readFILE(s,&pp);
    memset(&textID,0,sizeof(textID));
    if(mem)
    {
        TGAHEADER head;
        int       pos=0;
        memcpy(&head,(char*)mem+pos,sizeof(head));
        pos+=sizeof(head);
        if(head.imageType==10)
        {
        }
        else
        {
            unsigned short*data=(unsigned short*)((unsigned char*)mem+pos);
            if(head.bits==32)
            {
                unsigned char*p=(unsigned char*)data;
                int           i=head.width*head.height,j=0;
                data=(unsigned short*)malloc(head.width*head.height*2);
                while(i--)
                {
                    BYTE b=*p++;
                    BYTE g=*p++;
                    BYTE r=*p++;
                    BYTE a=*p++;
                    if(a==0)
                        data[j++]=0;
                    else
                    {
                        unsigned short val;
                        if(mode==4)
                        {
                            r >>= 4;
                            g >>= 4;
                            b >>= 4;
                            a >>= 4;
                            val=(unsigned short)a | (b << 4)  | (g << 8) | (r << 12);
                        }
                        else
                            val=(unsigned short)(((((((r)/8)<<10)+(((g)/8)<<5)+((b)/8)))<<1)|(a!=0));
                        data[j++]=val;
                    }
                }
                textID=tex_core_create(head.width,head.height,data,mode);
                free(data);
            }
            free(mem);
        }
    }
    return textID;
}

//  -------------------------------------------------------------
//
//  GFX 3D: MD2 loader
//
//  -------------------------------------------------------------
//  MD2 is a model format used by id Software's id Tech 2 engine and is thus used by Quake II as well as many other games
//  (http://en.wikipedia.org/wiki/MD2_(file_format))
//  Reading code derived from http://svn.assembla.com/svn/raytracer-cc52b/trunk/El-Zahir/Zahir/Models/Cargadores/MD2/md2_utils.hpp
//                            by David Henry, http://tfc.duke.free.fr/

typedef struct
{
    int magic;
    int version;
    int skinWidth;
    int skinHeight;
    int frameSize;
    int numSkins;
    int numVertices;
    int numTexCoords;
    int numTriangles;
    int numGlCommands;
    int numFrames;
    int offsetSkins;
    int offsetTexCoords;
    int offsetTriangles;
    int offsetFrames;
    int offsetGlCommands;
    int offsetEnd;
} md2_header_t;

typedef struct
{
    short vertexIndices[3];
    short textureIndices[3];
} md2_triangle_t;

typedef struct
{
    short s, t;
} md2_textureCoordinate_t;

typedef struct
{
    unsigned char vertex[3];
    unsigned char lightNormalIndex;
} md2_alias_triangleVertex_t;

typedef struct
{
    float scale[3];
    float translate[3];
    char name[16];
    md2_alias_triangleVertex_t alias_vertices[1];
} md2_alias_frame_t;


typedef char md2_skin_t[64];

int quickmesh_readMD2(quickMESH*mesh,const char*s,Texture*textID)
{
    int           pp;
    unsigned char*mem=(void*)os_readFILE(s,&pp);
    if(mem)
    {
        char                    ver[5];
        float                   scale=1.0f;
        float*                  v;
        md2_header_t            head;
        md2_triangle_t *        triangles;
        md2_textureCoordinate_t*texCoords;
        md2_skin_t*             skins;
        md2_alias_frame_t*      frame;
        int                     pos,i,j,k,seq,seqf;
        memset(mesh,0,sizeof(quickMESH));
        memcpy(&head,mem,sizeof(md2_header_t));
        sprintf(ver, "%c%c%c%c", head.magic, head.magic>>8, head.magic>>16, head.magic>>24);
        if(strcmp(ver,"IDP2")||head.version != 8)
        {
            free(mem);
            return 0;
        }
        skins=(md2_skin_t *)&mem[head.offsetSkins];
        texCoords=(md2_textureCoordinate_t*)&mem[head.offsetTexCoords];
        triangles=(md2_triangle_t*)&mem[head.offsetTriangles];
        if(textID)
        {
            char*text=skins[0];
            int  l=strlen(text);
            while(l--)
                if(text[l]=='/')
                    break;
            #if defined(HAVE_STBIMAGE)
            *textID=tex_read(text+l+1,0);
            #else
            *textID=tex_readTGA(text+l+1,0);
            #endif
        }

        mesh->tris=head.numTriangles;
        mesh->tex=(float*)malloc(sizeof(float)*mesh->tris*2*3);
        mesh->vert=(float*)malloc(sizeof(float)*mesh->tris*3*3);

        pos=head.offsetFrames;
        frame=(md2_alias_frame_t*)&mem[pos];
        v=(float*)malloc(sizeof(float)*3*head.numVertices);
        for(j=0; j<head.numVertices; j++)
        {
            v[j*3+0]= (float) ((int) frame->alias_vertices[j].vertex[0]) * frame->scale[0]*scale + frame->translate[0]*scale;
            v[j*3+1]= (float) ((int) frame->alias_vertices[j].vertex[2]) * frame->scale[2]*scale + frame->translate[2]*scale;
            v[j*3+2]= -((float) ((int) frame->alias_vertices[j].vertex[1]) * frame->scale[1]*scale + frame->translate[1]*scale);
        }

        for(i=0; i<head.numTriangles; i++)
        {
            int trisoff=i*3*3;
            int ttrisoff=i*2*3;
            for(j=0; j<3; j++)
            {
                int id=triangles[i].vertexIndices[j];
                mesh->vert[trisoff+j*3+0]=v[id*3+0];
                mesh->vert[trisoff+j*3+1]=v[id*3+1];
                mesh->vert[trisoff+j*3+2]=v[id*3+2];
                mesh->tex[ttrisoff+j*2+0]=(float)texCoords[triangles[i].textureIndices[j]].s/(float)head.skinWidth;
                mesh->tex[ttrisoff+j*2+1]=(float)1-(float)texCoords[triangles[i].textureIndices[j]].t/(float)head.skinHeight;
            }
        }

        pos=head.offsetFrames;
        for(seq=-1,seqf=0,k=0; k<head.numFrames; k++,pos+=head.frameSize)
        {
            frame=(md2_alias_frame_t*)&mem[pos];
        }

        free(mem);
        return 1;
    }
    else
        return 0;
}

int quickmesh_delete(quickMESH*mesh)
{
    free(mesh->vert);
    mesh->vert=0;
    free(mesh->tex);
    mesh->tex=0;
    mesh->tris=0;
    return 1;
}

void quickmesh_getAABB(quickMESH*mesh,AABB*aabb)
{
    int   i,j,nv=mesh->tris;
    float*v=mesh->vert;
    aabb->xM=aabb->xm=v[0];
    aabb->yM=aabb->ym=v[1];
    aabb->zM=aabb->zm=v[2];
    for(i=1; i<nv; i++)
        for(j=0; j<3; j++)
        {
            float x=v[i*3*3+j*3+0],y=v[i*3*3+j*3+1],z=v[i*3*3+j*3+2];
            if(x>aabb->xM) aabb->xM=x;
            if(y>aabb->yM) aabb->yM=y;
            if(z>aabb->zM) aabb->zM=z;
            if(x<aabb->xm) aabb->xm=x;
            if(y<aabb->ym) aabb->ym=y;
            if(z<aabb->zm) aabb->zm=z;
        }
}

//  -------------------------------------------------------------

int AABB_interset(AABB*a1,coord3D*c1,AABB*a2,coord3D*c2)
{
    float x1=0,y1=0,z1=0;
    float x2=0,y2=0,z2=0;
    if(c1)
    {
        x1=c1->pos.x;
        y1=c1->pos.y;
        z1=c1->pos.z;
    }
    if(c2)
    {
        x2=c2->pos.x;
        y2=c2->pos.y;
        z2=c2->pos.z;
    }
    if(a1->xM+x1<a2->xm+x2)
        return 0;
    if(a1->yM+y1<a2->ym+y2)
        return 0;
    if(a1->zM+z1<a2->zm+z2)
        return 0;
    if(a1->xm+x1>a2->xM+x2)
        return 0;
    if(a1->ym+y1>a2->yM+y2)
        return 0;
    if(a1->zm+z1>a2->zM+z2)
        return 0;
    return 1;
}

//  -------------------------------------------------------------
//
//  MATH: 3D VECTORS
//
//  -------------------------------------------------------------

void vector3_new(Vector3*th,float X, float Y, float Z)
{
    th->x=X;
    th->y=Y;
    th->z=Z;
}

void vector3_add(Vector3*th,Vector3*v)
{
    th->x+=v->x;
    th->y+=v->y;
    th->z+=v->z;
}

void vector3_sub(Vector3*th,Vector3*v)
{
    th->x-=v->x;
    th->y-=v->y;
    th->z-=v->z;
}

void vector3_mul(Vector3*th,float num)
{
    th->x*=num;
    th->y*=num;
    th->z*=num;
}

void vector3_div(Vector3*th,float num)
{
    th->x/=num;
    th->y/=num;
    th->z/=num;
}

void vector3_Cross(Vector3*vNormal,Vector3*vVector1,Vector3*vVector2)
{
    vNormal->x = ((vVector1->y * vVector2->z) - (vVector1->z * vVector2->y));
    vNormal->y = ((vVector1->z * vVector2->x) - (vVector1->x * vVector2->z));
    vNormal->z = ((vVector1->x * vVector2->y) - (vVector1->y * vVector2->x));
}

float vector3_Magnitude(Vector3*vNormal)
{
    return (float)sqrt( (vNormal->x * vNormal->x) +
                        (vNormal->y * vNormal->y) +
                        (vNormal->z * vNormal->z) );
}

void vector3_Normalize(Vector3*vVector)
{
    float magnitude = vector3_Magnitude(vVector);
    vector3_div(vVector,magnitude);
}

//  -------------------------------------------------------------
//
//  MATH: 3D CAMERA
//
//  -------------------------------------------------------------

void camera3D_new(camera3D*th)
{
    vector3_new(&th->m_vPosition,0.0, 0.0, 0.0);
    vector3_new(&th->m_vView,0.0, 1.0, 0.5);
    vector3_new(&th->m_vUpVector,0.0, 0.0, 1.0);
}

void camera3D_setPOSITION(camera3D*th,float positionX, float positionY, float positionZ,
                          float viewX,     float viewY,     float viewZ,
                          float upVectorX, float upVectorY, float upVectorZ)
{
    vector3_new(&th->m_vPosition,positionX, positionY, positionZ);
    vector3_new(&th->m_vView,viewX, viewY, viewZ);
    vector3_new(&th->m_vUpVector,upVectorX, upVectorY, upVectorZ);
}

void camera3D_lookAT(camera3D*th)
{
    gfx_LookAt(th->m_vPosition.x, th->m_vPosition.y, th->m_vPosition.z,
               th->m_vView.x,	 th->m_vView.y,     th->m_vView.z,
               th->m_vUpVector.x, th->m_vUpVector.y, th->m_vUpVector.z);
}

//  -------------------------------------------------------------


//  -------------------------------------------------------------
//
//  GFX: FRUSTUM
//
//  -------------------------------------------------------------

// Code from: http://praetoriansmapeditor.googlecode.com/svn/trunk/source/Viewer/Frustum.cpp

enum frustum3D_Side
{
    RIGHT=0,
    LEFT=1,
    BOTTOM=2,
    TOP=3,
    BACK=4,
    FRONT=5
};

enum frustum3D_PlaneData
{
    A = 0,
    B = 1,
    C = 2,
    D = 3
};

#define NORM(X,Y,Z,R) {                                                      \
    R=(float)sqrt(X*X+Y*Y+Z*Z);                                                     \
    if (R>1e-24) { R=1.0f/R; X*=R; Y*=R; Z*=R; }                              \
    else { X=1.0f; Y=0.0f; Z=0.0f; }                                            \
}


#define CROSS(X0,Y0,Z0,X1,Y1,Z1,X,Y,Z) \
    { X=Y0*Z1-Z0*Y1; Y=Z0*X1-X0*Z1; Z=X0*Y1-Y0*X1; }

void frustum3D_vm_apply(float *vm,float *p,float *pr)
{
    int i,j;
    float s,r[4],a[4];
    for (i=0; i<3; i++) a[i]=p[i];
    a[3]=1.0;
    for (i=0; i<4; i++)
    {
        s=0.0;
        for (j=0; j<4; j++) s+=vm[j*4+i]*a[j];
        r[i]=s;
    }
    for (i=0; i<3; i++) pr[i]=r[i]/r[3];
}


void frustum3D_pnts2plane(frustum3D*fr,int i,float *pc,float *pa,float *pb,float *g)
{
    int j;
    float a[3],b[3],c[3],nx,ny,nz,nr;
    float *vh;

    for (j=0; j<3; j++)
    {
        a[j]=pa[j]-pc[j];
        b[j]=pb[j]-pc[j];
        c[j]=pc[j];
    }
    CROSS(a[0],a[1],a[2],b[0],b[1],b[2],nx,ny,nz)
    NORM(nx,ny,nz,nr)
    if (nx*(g[0]-c[0])+ny*(g[1]-c[1])+nz*(g[2]-c[2])>0.0)
    {
        nx= -nx;
        ny= -ny;
        nz= -nz;
    }
    vh=fr->frust_planes[i];
    vh[0]=nx;
    vh[1]=ny;
    vh[2]=nz;
    vh[3]= -(nx*c[0]+ny*c[1]+nz*c[2]);
}


void frustum3D_vm_inv(float *vm,float *vmi)
{
    int i,j,n,imax,j0,pivot[4],pivot1[4];
    float a[4][4],b[4][4],q,qmax;
    n=4;
    for (i=0; i<n; i++)
    {
        for (j=0; j<n; j++)
        {
            a[i][j]=vm[j*n+i];
            b[i][j]=(i==j?1.0f:0.0f);
        }
    }
    for (i=0; i<n; i++)
    {
        pivot[i]= -1;
        pivot1[i]= -1;
    }
#define ABS(X) (X<0.0f?-X:X)
    for (j0=0; j0<n; j0++)
    {
        for (i=0,imax= -1,qmax=0.0f; i<n; i++)
        {
            q=a[i][j0];
            if (pivot[i]<0 && (imax<0 || ABS(q)>ABS(qmax)))
            {
                imax=i;
                qmax=q;
            }
        }
        if (imax<0 || ABS(qmax)<1e-20)
        {
            fprintf(stderr,"vm_inv: degenerate matrix\n");
            exit(1);
        }
        pivot[imax]=j0;
        pivot1[j0]=imax;
        q=1.0f/qmax;
        for (j=0; j<n; j++)
        {
            a[imax][j]*=q;
            b[imax][j]*=q;
        }
        for (i=0; i<n; i++)
        {
            if (i!=imax)
            {
                q=a[i][j0];
                for (j=0; j<n; j++)
                {
                    a[i][j]-=a[imax][j]*q;
                    b[i][j]-=b[imax][j]*q;
                }
            }
        }
    }
    for (i=0; i<n; i++)
    {
        for (j=0; j<n; j++)
        {
            vmi[j*n+i]=b[pivot1[i]][j];
        }
    }
}

void frustum3D_normalizePLANE(float frustum[6][4], int side)
{
    float magnitude = (float)sqrt( frustum[side][A] * frustum[side][A] +
                                   frustum[side][B] * frustum[side][B] +
                                   frustum[side][C] * frustum[side][C] );
    frustum[side][A] /= magnitude;
    frustum[side][B] /= magnitude;
    frustum[side][C] /= magnitude;
    frustum[side][D] /= magnitude;
}


void frustum3D_setVIEW(frustum3D*fr,float pxsize,float pysize)
{
    int j,k;
    float g[3],*vmi;
    float   proj[16];								// This will hold our projection matrix
    float   modl[16];								// This will hold our modelview matrix
    float   clip[16];								// This will hold the clipping planes
    float*  vm;

    gfx_getPROJECTIONMATRIX(proj);
    gfx_getMODELVIEWMATRIX(modl);

    clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
    clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
    clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
    clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

    clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
    clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
    clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
    clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

    clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
    clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
    clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

    vm=clip;

    fr->setcnt++;

    for (j=0; j<16; j++) fr->vm[j]=vm[j];
    vm=fr->vm;
    vmi=fr->vmi;

#define p fr->frust_points

    frustum3D_vm_inv(vm,vmi);

    for (k=0; k<8; k++)
    {
        for (j=0; j<3; j++) p[k][j]=((k&(1<<j))?1.0f:-1.0f);
        frustum3D_vm_apply(vmi,p[k],p[k]);
    }

    for (j=0; j<3; j++) g[j]=0.5f*(p[0][j]+p[7][j]);

    frustum3D_pnts2plane(fr,0,p[0],p[1],p[2],g);
    frustum3D_pnts2plane(fr,1,p[0],p[2],p[4],g);
    frustum3D_pnts2plane(fr,2,p[0],p[4],p[1],g);
    frustum3D_pnts2plane(fr,3,p[7],p[6],p[5],g);
    frustum3D_pnts2plane(fr,4,p[7],p[5],p[3],g);
    frustum3D_pnts2plane(fr,5,p[7],p[3],p[6],g);

    fr->frust_planes[RIGHT][A] = clip[ 3] - clip[ 0];
    fr->frust_planes[RIGHT][B] = clip[ 7] - clip[ 4];
    fr->frust_planes[RIGHT][C] = clip[11] - clip[ 8];
    fr->frust_planes[RIGHT][D] = clip[15] - clip[12];

    frustum3D_normalizePLANE(fr->frust_planes, RIGHT);

    fr->frust_planes[LEFT][A] = clip[ 3] + clip[ 0];
    fr->frust_planes[LEFT][B] = clip[ 7] + clip[ 4];
    fr->frust_planes[LEFT][C] = clip[11] + clip[ 8];
    fr->frust_planes[LEFT][D] = clip[15] + clip[12];

    frustum3D_normalizePLANE(fr->frust_planes, LEFT);

    fr->frust_planes[BOTTOM][A] = clip[ 3] + clip[ 1];
    fr->frust_planes[BOTTOM][B] = clip[ 7] + clip[ 5];
    fr->frust_planes[BOTTOM][C] = clip[11] + clip[ 9];
    fr->frust_planes[BOTTOM][D] = clip[15] + clip[13];

    frustum3D_normalizePLANE(fr->frust_planes, BOTTOM);

    fr->frust_planes[TOP][A] = clip[ 3] - clip[ 1];
    fr->frust_planes[TOP][B] = clip[ 7] - clip[ 5];
    fr->frust_planes[TOP][C] = clip[11] - clip[ 9];
    fr->frust_planes[TOP][D] = clip[15] - clip[13];

    frustum3D_normalizePLANE(fr->frust_planes, TOP);

    fr->frust_planes[BACK][A] = clip[ 3] - clip[ 2];
    fr->frust_planes[BACK][B] = clip[ 7] - clip[ 6];
    fr->frust_planes[BACK][C] = clip[11] - clip[10];
    fr->frust_planes[BACK][D] = clip[15] - clip[14];

    frustum3D_normalizePLANE(fr->frust_planes, BACK);

    fr->frust_planes[FRONT][A] = clip[ 3] + clip[ 2];
    fr->frust_planes[FRONT][B] = clip[ 7] + clip[ 6];
    fr->frust_planes[FRONT][C] = clip[11] + clip[10];
    fr->frust_planes[FRONT][D] = clip[15] + clip[14];

    frustum3D_normalizePLANE(fr->frust_planes, FRONT);

#undef p
}

int frustum3D_checkAABB(frustum3D*fr,AABB*aabb)
{
    int   i;
    float q,*vh;
    float tx=aabb->xM,ty=aabb->yM,tz=aabb->zM,bx=aabb->xm,by=aabb->ym,bz=aabb->zm;
    for(i=0; i<6; i++)
    {
        vh=fr->frust_planes[i];

        q=tx*vh[0]+ty*vh[1]+tz*vh[2]+vh[3];
        if(q>0) continue;

        q=bx*vh[0]+ty*vh[1]+tz*vh[2]+vh[3];
        if(q>0) continue;

        q=tx*vh[0]+by*vh[1]+tz*vh[2]+vh[3];
        if(q>0) continue;

        q=bx*vh[0]+by*vh[1]+tz*vh[2]+vh[3];
        if(q>0) continue;

        q=tx*vh[0]+ty*vh[1]+bz*vh[2]+vh[3];
        if(q>0) continue;

        q=bx*vh[0]+ty*vh[1]+bz*vh[2]+vh[3];
        if(q>0) continue;

        q=tx*vh[0]+by*vh[1]+bz*vh[2]+vh[3];
        if(q>0) continue;

        q=bx*vh[0]+by*vh[1]+bz*vh[2]+vh[3];
        if(q>0) continue;

        return 0;
    }
    return 1;
}


//  -------------------------------------------------------------

const char*gettag(const char*x,char*tagname,char*what)
{
 *what=*tagname=0;
 while(*x&&(!*x=='<')) x++;
 if(*x)
  {
   char*pwhat=what;
   while(*x&&(*x!='>'))
    *what++=*x++;
   if(*x=='>')
    *what++=*x++;
   *what=0;
   what=pwhat;
   if(*what)
    {
     *what++;
     while(*what&&(*what!=' '))
      *tagname++=*what++;
     *tagname++=0;
    }
  }
 return x;
}

int getattr(const char*x,const char*attr,char*what)
{
 char*s=strstr(x,attr);
 int  len=strlen(attr);
 while(s)
  {
   if(s&&((s==x)||(s[-1]==' '))&&(s[len]=='='))
    {
     s+=len+1;
     if(*s=='"')
      {
       s++;
       while(*s&&(*s!='"'))
        *what++=*s++;
       *what=0;
       return 1;
      }
     else
      return 0;
    }
   else
    s=strstr(s+1,attr);
  }
 return 0;
}

int TEXATL_read(const char*name,TEXATL*t)
{
 int  size;
 char*xml;
 char nm[64];
 strcpy(nm,name);strcat(nm,".xml");
 xml=os_readFILE(nm,&size);
 if(xml)
  {
   int  i;
   char nm[64],tag[256],r[64];
   const char*x=gettag(xml,nm,tag);
   getattr(tag,"numImages",r);
   t->cnt=atoi(r);
   t->tai=(TEXATLI*)calloc(t->cnt,sizeof(TEXATLI));
   x=gettag(x,nm,tag);
   for(i=0;i<t->cnt;i++)
    {
     x=gettag(x,nm,tag);
     getattr(tag,"name",t->tai[i].nm);
     getattr(tag,"x",r);
     t->tai[i].x=(float)atof(r);
     getattr(tag,"y",r);
     t->tai[i].y=(float)atof(r);
     getattr(tag,"width",r);
     t->tai[i].rw=(float)atof(r);
     getattr(tag,"height",r);
     t->tai[i].rh=(float)atof(r);
     if(getattr(tag,"xOffset",r))
      t->tai[i].bx=(float)atof(r);
     else
      t->tai[i].bx=0;
     if(getattr(tag,"yOffset",r))
      t->tai[i].by=(float)atof(r);
     else
      t->tai[i].by=0;
     if(getattr(tag,"transWidth",r))
      t->tai[i].w=(float)atof(r);
     else
      t->tai[i].w=t->tai[i].rw;
     if(getattr(tag,"transHeight",r))
      t->tai[i].h=(float)atof(r);
     else
      t->tai[i].h=t->tai[i].rh;
    }
   free(xml);
   #if defined(HAVE_STBIMAGE)
   strcpy(nm,name);strcat(nm,".png");
   t->texID=tex_readTGA(nm,4);
   #else
   strcpy(nm,name);strcat(nm,".tga");
   t->texID=tex_readTGA(nm,4);
   #endif
   return 1;
  }
 else
  return 0;
}

int TEXATL_delete(TEXATL*t)
{
 if(t->tai)
  {
   free(t->tai);
   t->tai=NULL;
   t->cnt=0;
   tex_delete(&t->texID);
   return 1;
  }
 else
  return 0;
}

int TEXATL_find(TEXATL*t,const char*name)
{
 int i;
 for(i=0;i<t->cnt;i++)
  if(strcmp(t->tai[i].nm,name)==0)
   return i;
 return -1;
}

int TEXATL_findanim(TEXATL*t,const char*name,int*nframes)
{
 int  i=0,ret=-1;
 while(1)
  {
   char nm[256];
   int  id;
   sprintf(nm,"%s%d",name,i+1);
   if((id=TEXATL_find(t,nm))!=-1)
    {
     if(ret==-1) ret=id;
     i++;
    }
   else
    break;
  }
 if(i==0)
  {
   ret=TEXATL_find(t,name);
   if((ret!=-1)&&nframes);
    *nframes=1;
  }
 else
 if((ret!=-1)&&nframes);
  *nframes=i;
 return ret;
}

int gfx_drawSPRITETEXATL(float x,float y,TEXATL*t,int id,float ratio,float alpha,int dir)
{
 if((id>=0)&&(id<t->cnt))
  {
   TEXATLI*i=&t->tai[id];
   float   xx=(float)floor(x+i->bx*ratio);
   float   yy=(float)floor(y+i->by*ratio);
   float   ww=(float)floor(i->rw*ratio),hh=(float)floor(i->rh*ratio);
   float   ix=i->x,iy=i->y,iw=i->rw,ih=i->rh;
   gfx_drawSPRITE(xx,yy,ww,hh,&t->texID,ix,iy,iw,ih,alpha,1,dir);
   return 1;
  }
 else
  return 0;
}

//  -------------------------------------------------------------
