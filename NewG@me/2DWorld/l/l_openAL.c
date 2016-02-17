/* ---------------------------------------------------------------
   l_openAL.c
   openAL sound (pretty basic) engine
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
#if defined(OS_IPHONE)
#include "../al/al.h"
#else
#include "al/al.h"
#endif

ALvoid * alutLoadMemoryFromFileImage (const ALvoid *data, ALsizei length,
                                      ALenum *format, ALsizei *size,
                                      ALfloat *frequency);
ALboolean alutInit (int *argcp, char **argv);
ALboolean alutExit (void);

#define NUM_BUFFERS 64
#define NUM_SOURCES 16

ALuint  Buffers[NUM_BUFFERS];
ALbyte  bBuffers[NUM_BUFFERS];
ALuint  Sources[NUM_SOURCES];
ALfloat SourcesPos[NUM_SOURCES][3];
ALfloat SourcesVel[NUM_SOURCES][3];
ALbyte  bSources[NUM_SOURCES];

int  wav_memory;

int  wav_read(const char*sound,int mode)
{
   int    i,sizep,err;
   void*  soundp;
   ALenum format;
   ALsizei size;
   ALvoid* data;
   ALfloat freq;

   for(i=0;i<NUM_BUFFERS;i++)
    if(bBuffers[i]==0)
     break;

   if(i==NUM_BUFFERS)
    return -1;

  // Load wav data into a buffer.
   alGenBuffers(1, &Buffers[i]);
   err=alGetError();
   if (err != AL_NO_ERROR)
    return -1;


   soundp=os_readFILE(sound,&sizep);
    if(soundp)
    {
        if((data=alutLoadMemoryFromFileImage(soundp,sizep, &format, &size, &freq))!=NULL)
        {
            free(soundp);
            alBufferData(Buffers[i], format, data, size, (ALsizei)freq);
            free(data);
            bBuffers[i]=1;
            return i;
        }
        else
            return -1;
    }
    else
         return -1;
}

int  wav_delete(int*waveID)
{
    if((*waveID>=0)&&(*waveID<NUM_BUFFERS))
     if(bBuffers[*waveID])
      {
       alDeleteBuffers(1, &Buffers[*waveID]);
       if (alGetError() != AL_NO_ERROR)
        return 0;
       else
        {
         bBuffers[*waveID]=0;
         *waveID=-1;
        }
      }
    return 1;
}

void  wav_deleteall()
{
 int i;
 for(i=0;i<NUM_BUFFERS;i++)
  if(bBuffers[i])
   {
    alDeleteBuffers(1, &Buffers[i]);
    if (alGetError() != AL_NO_ERROR)
     {
      int k;
      k=0;
     }
    else
     bBuffers[i]=0;
   }
}

int  snd_init()
{
    int  i;
    int  argc=1;
    char*argv[2]={NULL,NULL};
    if(alutInit (&argc, argv))
     {
      for(i=0;i<NUM_SOURCES;i++)
       {
        alGenSources(1, &Sources[i]);
        if (alGetError() != AL_NO_ERROR)
         break;
        bSources[i]=1;
       }
      return 1;
     }
    else
     return 0;
}

int  snd_reset()
{
    int i;
    wav_deleteall();
    for(i=0;i<NUM_SOURCES;i++)
     if(bSources[i])
      {
       alDeleteSources(1, &Sources[i]);
       bSources[i]=0;
      }
    alutExit();
    return 0;
}

int  snd_play(int waveID,int loop,int mode)
{
    int sound;
    // ------------------
    for(sound=0;sound<NUM_SOURCES;sound++)
     {
      ALint val;
      alGetSourcei(Sources[sound],AL_LOOPING,&val);
      if(!val)
       {
        alGetSourcei(Sources[sound],AL_SOURCE_STATE,&val);
        if(val != AL_PLAYING)
         break;
       }
     }
    if(sound==NUM_SOURCES)
     {
      for(sound=0;sound<NUM_SOURCES;sound++)
       {
        ALint val;
        alGetSourcei(Sources[sound],AL_LOOPING,&val);
        if(!val)
         {
          alSourceStop(Sources[sound]);
          break;
         }
       }
     }
    if(sound==NUM_SOURCES)
     {
      sound=rand()%NUM_SOURCES;
      alSourceStop(Sources[sound]);
     }
    // ------------------
    alSourcei (Sources[sound], AL_BUFFER,   Buffers[waveID]   );
    alSourcef (Sources[sound], AL_PITCH,    1.0f     );
    alSourcef (Sources[sound], AL_GAIN,     1.0f     );
    if(loop)
     alSourcei (Sources[sound], AL_LOOPING,  AL_TRUE );
    else
     alSourcei (Sources[sound], AL_LOOPING,  AL_FALSE );

    switch(mode)
     {
      case 0:
       alSourcePlay(Sources[sound]);
       return sound;
      break;
      case 1:
       {
        ALint val;
        alGetSourcei(Sources[sound],AL_SOURCE_STATE,&val);
        if(val != AL_PLAYING)
         {
          alSourcePlay(Sources[sound]);
          return sound;
         }
       }
     }

    return -1;
}

int  snd_stop(int playwaveID)
{
    alSourceStop(Sources[playwaveID]);
    alSourcei (Sources[playwaveID], AL_BUFFER,   0   );
    alSourcei (Sources[playwaveID], AL_LOOPING,  AL_FALSE );
    return 0;
}

int snd_stopall()
{
 return 0;
}

int  snd_pause(int playwaveID)
{
    alSourcePause(Sources[playwaveID]);
    return 0;
}
