/* ---------------------------------------------------------------
   l_module.c
   Generic library code (deferring implementation to engines)
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

//  -------------------------------------------------------------

int   os_portrait=1,os_flip=0,os_touch_flip=0;
float os_roll=50.0,os_pitch=50.0,os_Z=95.0;
int   os_x[10],os_y[10],os_status[10],os_np,os_keys[1024];
float os_scale,os_gfxratio=1;
float os_video_w,os_video_h;
char  os_szResPath[256];
int   os_BACK_pressed;
int   os_PAUSE;

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
#if defined(OS_IPHONE) || defined(OS_MAC) || defined(OS_ANDROID)||defined(OS_LINUX)
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
   t->texID=tex_read(nm,4);
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
