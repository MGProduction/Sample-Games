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
float os_scale;
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
#if defined(OS_IPHONE) || defined(OS_MAC) || defined(OS_ANDROID)
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

//  -------------------------------------------------------------

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

int tex_readTGA(const char*s,int mode)
{
    int  textID=-1;
    int  pp;
    void*mem=os_readFILE(s,&pp);
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
