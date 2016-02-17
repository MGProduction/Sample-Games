#include "alutInternal.h"
#include <ctype.h>

/****************************************************************************/

#define RIFF		0x46464952		/* "RIFF" */
#define WAVE		0x45564157		/* "WAVE" */
#define FACT		0x74636166		/* "fact" */
#define LIST		0x5453494c		/* "LIST" */
#define FMT		0x20746D66		/* "fmt " */
#define DATA		0x61746164		/* "data" */
#define PCM_CODE	0x0001
#define MS_ADPCM_CODE	0x0002
#define IMA_ADPCM_CODE	0x0011
#define MP3_CODE	0x0055
#define WAVE_MONO	1
#define WAVE_STEREO	2

#define AUDIO_S16 32784

typedef signed char Sint8;
typedef unsigned char Uint8;
typedef signed short Sint16;
typedef unsigned short Uint16;
typedef signed int Sint32;
typedef unsigned int Uint32;
//typedef unsigned int size_t;
typedef unsigned long uintptr_t;

#define SDL_SwapLE32(a) a
#define SDL_SwapLE16(a) a
#define SDL_arraysize(array) (sizeof(array)/sizeof(array[0])) 

/* Normally, these three chunks come consecutively in a WAVE file */
typedef struct WaveFMT {
/* Not saved in the chunk we read:
	Uint32	FMTchunk;
	Uint32	fmtlen;
*/
	Uint16	encoding;	
	Uint16	channels;		/* 1 = mono, 2 = stereo */
	Uint32	frequency;		/* One of 11025, 22050, or 44100 Hz */
	Uint32	byterate;		/* Average bytes per second */
	Uint16	blockalign;		/* Bytes per sample block */
	Uint16	bitspersample;		/* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

/* The general chunk found in the WAVE file */
typedef struct Chunk {
	Uint32 magic;
	Uint32 length;
	Uint8 *data;
} Chunk;

struct MS_ADPCM_decodestate {
	Uint8 hPredictor;
	Uint16 iDelta;
	Sint16 iSamp1;
	Sint16 iSamp2;
};
static struct MS_ADPCM_decoder {
	WaveFMT wavefmt;
	Uint16 wSamplesPerBlock;
	Uint16 wNumCoef;
	Sint16 aCoeff[7][2];
	/* * * */
	struct MS_ADPCM_decodestate state[2];
} MS_ADPCM_state;

static int InitMS_ADPCM(WaveFMT *format)
{
	Uint8 *rogue_feel;
	Uint16 extra_info;
	int i;

	/* Set the rogue pointer to the MS_ADPCM specific data */
	MS_ADPCM_state.wavefmt.encoding = SDL_SwapLE16(format->encoding);
	MS_ADPCM_state.wavefmt.channels = SDL_SwapLE16(format->channels);
	MS_ADPCM_state.wavefmt.frequency = SDL_SwapLE32(format->frequency);
	MS_ADPCM_state.wavefmt.byterate = SDL_SwapLE32(format->byterate);
	MS_ADPCM_state.wavefmt.blockalign = SDL_SwapLE16(format->blockalign);
	MS_ADPCM_state.wavefmt.bitspersample =
					 SDL_SwapLE16(format->bitspersample);
	rogue_feel = (Uint8 *)format+sizeof(*format);
	if ( sizeof(*format) == 16 ) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(Uint16);
	}
	MS_ADPCM_state.wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(Uint16);
	MS_ADPCM_state.wNumCoef = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(Uint16);
	if ( MS_ADPCM_state.wNumCoef != 7 ) {
		//SDL_SetError("Unknown set of MS_ADPCM coefficients");
		return(-1);
	}
	for ( i=0; i<MS_ADPCM_state.wNumCoef; ++i ) {
		MS_ADPCM_state.aCoeff[i][0] = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(Uint16);
		MS_ADPCM_state.aCoeff[i][1] = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(Uint16);
	}
	return(0);
}

static Sint32 MS_ADPCM_nibble(struct MS_ADPCM_decodestate *state,
					Uint8 nybble, Sint16 *coeff)
{
	const Sint32 max_audioval = ((1<<(16-1))-1);
	const Sint32 min_audioval = -(1<<(16-1));
	const Sint32 adaptive[] = {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};
	Sint32 new_sample, delta;

	new_sample = ((state->iSamp1 * coeff[0]) +
		      (state->iSamp2 * coeff[1]))/256;
	if ( nybble & 0x08 ) {
		new_sample += state->iDelta * (nybble-0x10);
	} else {
		new_sample += state->iDelta * nybble;
	}
	if ( new_sample < min_audioval ) {
		new_sample = min_audioval;
	} else
	if ( new_sample > max_audioval ) {
		new_sample = max_audioval;
	}
	delta = ((Sint32)state->iDelta * adaptive[nybble])/256;
	if ( delta < 16 ) {
		delta = 16;
	}
	state->iDelta = (Uint16)delta;
	state->iSamp2 = state->iSamp1;
	state->iSamp1 = (Sint16)new_sample;
	return(new_sample);
}

static int MS_ADPCM_decode(Uint8 **audio_buf, Uint32 *audio_len)
{
	struct MS_ADPCM_decodestate *state[2];
	Uint8 *freeable, *encoded, *decoded;
	Sint32 encoded_len, samplesleft;
	Sint8 nybble, stereo;
	Sint16 *coeff[2];
	Sint32 new_sample;

	/* Allocate the proper sized output buffer */
	encoded_len = *audio_len;
	encoded = *audio_buf;
	freeable = *audio_buf;
	*audio_len = (encoded_len/MS_ADPCM_state.wavefmt.blockalign) * 
				MS_ADPCM_state.wSamplesPerBlock*
				MS_ADPCM_state.wavefmt.channels*sizeof(Sint16);
	*audio_buf = (Uint8 *)malloc(*audio_len);
	if ( *audio_buf == NULL ) {
		//SDL_Error(SDL_ENOMEM);
		return(-1);
	}
	decoded = *audio_buf;

	/* Get ready... Go! */
	stereo = (MS_ADPCM_state.wavefmt.channels == 2);
	state[0] = &MS_ADPCM_state.state[0];
	state[1] = &MS_ADPCM_state.state[stereo];
	while ( encoded_len >= MS_ADPCM_state.wavefmt.blockalign ) {
		/* Grab the initial information for this block */
		state[0]->hPredictor = *encoded++;
		if ( stereo ) {
			state[1]->hPredictor = *encoded++;
		}
		state[0]->iDelta = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(Sint16);
		if ( stereo ) {
			state[1]->iDelta = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(Sint16);
		}
		state[0]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(Sint16);
		if ( stereo ) {
			state[1]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(Sint16);
		}
		state[0]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
		encoded += sizeof(Sint16);
		if ( stereo ) {
			state[1]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
			encoded += sizeof(Sint16);
		}
		coeff[0] = MS_ADPCM_state.aCoeff[state[0]->hPredictor];
		coeff[1] = MS_ADPCM_state.aCoeff[state[1]->hPredictor];

		/* Store the two initial samples we start with */
		decoded[0] = state[0]->iSamp2&0xFF;
		decoded[1] = state[0]->iSamp2>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp2&0xFF;
			decoded[1] = state[1]->iSamp2>>8;
			decoded += 2;
		}
		decoded[0] = state[0]->iSamp1&0xFF;
		decoded[1] = state[0]->iSamp1>>8;
		decoded += 2;
		if ( stereo ) {
			decoded[0] = state[1]->iSamp1&0xFF;
			decoded[1] = state[1]->iSamp1>>8;
			decoded += 2;
		}

		/* Decode and store the other samples in this block */
		samplesleft = (MS_ADPCM_state.wSamplesPerBlock-2)*
					MS_ADPCM_state.wavefmt.channels;
		while ( samplesleft > 0 ) {
			nybble = (*encoded)>>4;
			new_sample = MS_ADPCM_nibble(state[0],nybble,coeff[0]);
			decoded[0] = new_sample&0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample&0xFF;
			decoded += 2;

			nybble = (*encoded)&0x0F;
			new_sample = MS_ADPCM_nibble(state[1],nybble,coeff[1]);
			decoded[0] = new_sample&0xFF;
			new_sample >>= 8;
			decoded[1] = new_sample&0xFF;
			decoded += 2;

			++encoded;
			samplesleft -= 2;
		}
		encoded_len -= MS_ADPCM_state.wavefmt.blockalign;
	}
	free(freeable);
	return(0);
}

struct IMA_ADPCM_decodestate {
	Sint32 sample;
	Sint8 index;
};
static struct IMA_ADPCM_decoder {
	WaveFMT wavefmt;
	Uint16 wSamplesPerBlock;
	/* * * */
	struct IMA_ADPCM_decodestate state[2];
} IMA_ADPCM_state;

static int InitIMA_ADPCM(WaveFMT *format)
{
	Uint8 *rogue_feel;
	Uint16 extra_info;

	/* Set the rogue pointer to the IMA_ADPCM specific data */
	IMA_ADPCM_state.wavefmt.encoding = SDL_SwapLE16(format->encoding);
	IMA_ADPCM_state.wavefmt.channels = SDL_SwapLE16(format->channels);
	IMA_ADPCM_state.wavefmt.frequency = SDL_SwapLE32(format->frequency);
	IMA_ADPCM_state.wavefmt.byterate = SDL_SwapLE32(format->byterate);
	IMA_ADPCM_state.wavefmt.blockalign = SDL_SwapLE16(format->blockalign);
	IMA_ADPCM_state.wavefmt.bitspersample =
					 SDL_SwapLE16(format->bitspersample);
	rogue_feel = (Uint8 *)format+sizeof(*format);
	if ( sizeof(*format) == 16 ) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(Uint16);
	}
	IMA_ADPCM_state.wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
	return(0);
}

static Sint32 IMA_ADPCM_nibble(struct IMA_ADPCM_decodestate *state,Uint8 nybble)
{
	const Sint32 max_audioval = ((1<<(16-1))-1);
	const Sint32 min_audioval = -(1<<(16-1));
	const int index_table[16] = {
		-1, -1, -1, -1,
		 2,  4,  6,  8,
		-1, -1, -1, -1,
		 2,  4,  6,  8
	};
	const Sint32 step_table[89] = {
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
		34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
		143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
		449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
		1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
		3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
		9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
		22385, 24623, 27086, 29794, 32767
	};
	Sint32 delta, step;

	/* Compute difference and new sample value */
	step = step_table[state->index];
	delta = step >> 3;
	if ( nybble & 0x04 ) delta += step;
	if ( nybble & 0x02 ) delta += (step >> 1);
	if ( nybble & 0x01 ) delta += (step >> 2);
	if ( nybble & 0x08 ) delta = -delta;
	state->sample += delta;

	/* Update index value */
	state->index += index_table[nybble];
	if ( state->index > 88 ) {
		state->index = 88;
	} else
	if ( state->index < 0 ) {
		state->index = 0;
	}

	/* Clamp output sample */
	if ( state->sample > max_audioval ) {
		state->sample = max_audioval;
	} else
	if ( state->sample < min_audioval ) {
		state->sample = min_audioval;
	}
	return(state->sample);
}

/* Fill the decode buffer with a channel block of data (8 samples) */
static void Fill_IMA_ADPCM_block(Uint8 *decoded, Uint8 *encoded,
	int channel, int numchannels, struct IMA_ADPCM_decodestate *state)
{
	int i;
	Sint8 nybble;
	Sint32 new_sample;

	decoded += (channel * 2);
	for ( i=0; i<4; ++i ) {
		nybble = (*encoded)&0x0F;
		new_sample = IMA_ADPCM_nibble(state, nybble);
		decoded[0] = new_sample&0xFF;
		new_sample >>= 8;
		decoded[1] = new_sample&0xFF;
		decoded += 2 * numchannels;

		nybble = (*encoded)>>4;
		new_sample = IMA_ADPCM_nibble(state, nybble);
		decoded[0] = new_sample&0xFF;
		new_sample >>= 8;
		decoded[1] = new_sample&0xFF;
		decoded += 2 * numchannels;

		++encoded;
	}
}

static int IMA_ADPCM_decode(Uint8 **audio_buf, Uint32 *audio_len)
{
	struct IMA_ADPCM_decodestate *state;
	Uint8 *freeable, *encoded, *decoded;
	Sint32 encoded_len, samplesleft;
	unsigned int c, channels;

	/* Check to make sure we have enough variables in the state array */
	channels = IMA_ADPCM_state.wavefmt.channels;
	if ( channels > SDL_arraysize(IMA_ADPCM_state.state) ) {
		//SDL_SetError("IMA ADPCM decoder can only handle %d channels",SDL_arraysize(IMA_ADPCM_state.state));
		return(-1);
	}
	state = IMA_ADPCM_state.state;

	/* Allocate the proper sized output buffer */
	encoded_len = *audio_len;
	encoded = *audio_buf;
	freeable = *audio_buf;
	*audio_len = (encoded_len/IMA_ADPCM_state.wavefmt.blockalign) * 
				IMA_ADPCM_state.wSamplesPerBlock*
				IMA_ADPCM_state.wavefmt.channels*sizeof(Sint16);
	*audio_buf = (Uint8 *)malloc(*audio_len);
	if ( *audio_buf == NULL ) {
		//SDL_Error(SDL_ENOMEM);
		return(-1);
	}
	decoded = *audio_buf;

	/* Get ready... Go! */
	while ( encoded_len >= IMA_ADPCM_state.wavefmt.blockalign ) {
		/* Grab the initial information for this block */
		for ( c=0; c<channels; ++c ) {
			/* Fill the state information for this block */
			state[c].sample = ((encoded[1]<<8)|encoded[0]);
			encoded += 2;
			if ( state[c].sample & 0x8000 ) {
				state[c].sample -= 0x10000;
			}
			state[c].index = *encoded++;
			/* Reserved byte in buffer header, should be 0 */
			if ( *encoded++ != 0 ) {
				/* Uh oh, corrupt data?  Buggy code? */;
			}

			/* Store the initial sample we start with */
			decoded[0] = (Uint8)(state[c].sample&0xFF);
			decoded[1] = (Uint8)(state[c].sample>>8);
			decoded += 2;
		}

		/* Decode and store the other samples in this block */
		samplesleft = (IMA_ADPCM_state.wSamplesPerBlock-1)*channels;
		while ( samplesleft > 0 ) {
			for ( c=0; c<channels; ++c ) {
				Fill_IMA_ADPCM_block(decoded, encoded,
						c, channels, &state[c]);
				encoded += 4;
				samplesleft -= 8;
			}
			decoded += (channels * 8 * 2);
		}
		encoded_len -= IMA_ADPCM_state.wavefmt.blockalign;
	}
	free(freeable);
	return(0);
}

/****************************************************************************/

static const ALint stepAdjust[16] = {
   -1, -1, -1, -1, 2,  4,  6,  8,   
   -1, -1, -1, -1, 2,  4,  6,  8,   
};

static const ALint stepSizeTable[89] = {
       7,     8,     9,    10,    11,    12,    13,    14,    
      16,    17,    19,    21,    23,    25,    28,    31, 
      34,    37,    41,    45,    50,    55,    60,    66,    
      73,    80,    88,    97,   107,   118,   130,   143,
     157,   173,   190,   209,   230,   253,   279,   307,   
     337,   371,   408,   449,   494,   544,   598,   658,   
     724,   796,   876,   963,  1060,  1166,  1282,  1411, 
    1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,  
    3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,  
    7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899, 
   15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 
   32767
};

#define HEADER_SIZE 4
#define SAMPLE_SIZE 2

ALboolean alimIMA_ADPCM_decodeMono16(ALshort *dst, ALbyte* src, int src_size, int src_sizeblock )
{
   ALint prediction = 0;
   ALint stepIndex  = 0;
   ALint samples;
   ALint samplesPerPacket;
   ALint process;

   int step;
   int difference;
   int deltaCode;

   samples = src_size / SAMPLE_SIZE;  
   samplesPerPacket = ((src_sizeblock - HEADER_SIZE) * SAMPLE_SIZE) + 1;

   while (samples)
   {
      if (samples >= samplesPerPacket)
      {
         process = (samplesPerPacket-1)/2;
         samples -= samplesPerPacket;
      }   
      else
      {
         process = (samples-1)/2;
         samples = 0;
      }

      // Packet Format:
      //   Header
      //     byte 0: Sample0 Lo
      //     byte 1: Sample0 Hi
      //     byte 2: Step Index
      //     byte 3: Reserved
      //   Data
      //     bytes 0-n:
      //       bits 0-4 Sample m
      //       bits 5-7 Sample m+1

      // get initial header step index
      stepIndex = *(src+2);

      // reality check
      //if (stepIndex < 0 || stepIndex > 88)
      //{
      //   alimDebugger("step index out of range");
      //   return AL_FALSE;
      //}   

      // get initial header prediction
      prediction = *src + (*(src+1) << 8);
      src += 4;   // advance past Header

      *dst++ = prediction;

      // decode rest of packet
      for (; process > 0; process--, src++)
      {
         deltaCode = *src & 0x0f;

         // Get the current step size                                   
         step = stepSizeTable[stepIndex];                           
         difference = step>>3;
              
         // Update the step for the next sample                         
         stepIndex += stepAdjust[deltaCode];                            
         if (stepIndex < 0)         stepIndex = 0;                                              
         else if (stepIndex > 88)   stepIndex = 88;                                          
                                                                
         // Construct the difference by scaling the current step size   
         // This is approximately: difference = (deltaCode+.5)*step/4   
         if ( deltaCode & 4 )  difference += step;
         if ( deltaCode & 2 )  difference += step>>1;
         if ( deltaCode & 1 )  difference += step>>2;
         if ( deltaCode & 8 )  prediction -= difference;
         else prediction += difference;
                                                                  
         if (prediction > 32767)        prediction = 32767;
         else if (prediction < -32768)  prediction = -32768;
         
         *dst++ = prediction; 
            
         //--------------------------------------

         deltaCode = (*src >> 4) & 0x0f;

         // Get the current step size                                   
         step = stepSizeTable[stepIndex];                           
         difference = step>>3;
              
         // Update the step for the next sample                         
         stepIndex += stepAdjust[deltaCode];                            
         if (stepIndex < 0)         stepIndex = 0;                                              
         else if (stepIndex > 88)   stepIndex = 88;                                          
                                                                
         // Construct the difference by scaling the current step size   
         // This is approximately: difference = (deltaCode+.5)*step/4   
         if ( deltaCode & 4 )  difference += step;
         if ( deltaCode & 2 )  difference += step>>1;
         if ( deltaCode & 1 )  difference += step>>2;
         if ( deltaCode & 8 )  prediction -= difference;
         else prediction += difference;
                                                                  
         if (prediction > 32767)        prediction = 32767;
         else if (prediction < -32768)  prediction = -32768;
         
         *dst++ = prediction; 
      }
   }

//   time = GetTickCount()-time;
//   alimDebugger("%d ms, %d samples %f samples/second", time, (info->size / SAMPLE_SIZE), (float)(info->size / SAMPLE_SIZE)*1000.0f/(float)time );
   return AL_TRUE;
}    




static ALvoid adpcmDecode( ALbyte deltaCode, ALint *prediction, ALint *stepIndex )
{
   // Get the current step size                                   
   int step = stepSizeTable[*stepIndex];                           
   int difference = step>>3;
        
   // Update the step for the next sample                         
   *stepIndex += stepAdjust[deltaCode];                            
   if (*stepIndex < 0)         *stepIndex = 0;                                              
   else if (*stepIndex > 88)   *stepIndex = 88;                                          
                                                          
   // Construct the difference by scaling the current step size   
   // This is approximately: difference = (deltaCode+.5)*step/4   
   if ( deltaCode & 4 )  difference += step;
   if ( deltaCode & 2 )  difference += step>>1;
   if ( deltaCode & 1 )  difference += step>>2;
   if ( deltaCode & 8 )  *prediction -= difference;
   else *prediction += difference;
                                                                  
   if (*prediction > 32767)        *prediction = 32767;
   else if (*prediction < -32768)  *prediction = -32768;
}                                                                 


ALboolean alimIMA_ADPCM_decodeStereo16(ALshort *dst, ALbyte* src, int src_size, int src_sizeblock )
{
   ALint l_prediction, r_prediction;
   ALint l_stepIndex,  r_stepIndex;
   ALint samples;
   ALint samplesPerPacket;
   ALint process;
   ALshort *r_dst = dst+1;

   l_prediction = r_prediction = 0;
   l_stepIndex  = r_stepIndex = 0;

   samples = src_size / SAMPLE_SIZE;  

   samplesPerPacket = ((src_sizeblock - (HEADER_SIZE*2)) * SAMPLE_SIZE) + 2;

   while (samples)
   {
      if (samples >= samplesPerPacket)
      {
         process = (samplesPerPacket-2)/16;
         samples -= samplesPerPacket;
      }   
      else
      {
         process = (samples-2)/16;
         samples = 0;
      }

      // Packet Format:
      //   Header (left)
      //     byte 0: Sample0 Lo
      //     byte 1: Sample0 Hi
      //     byte 2: Step Index
      //     byte 3: Reserved
      //   Header (right)
      //     byte 0: Sample0 Lo
      //     byte 1: Sample0 Hi
      //     byte 2: Step Index
      //     byte 3: Reserved
      //   Data
      //     bytes 0-3:    (left channel)
      //       bits 0-4 Sample m
      //       bits 5-7 Sample m+1
      //     bytes 4-7:    (right channel)
      //       bits 0-4 Sample m
      //       bits 5-7 Sample m+1

      // LEFT CHANNEL
      // get initial header step index
      l_stepIndex = *(src+2);

      // get initial header prediction
      l_prediction = *src + (*(src+1) << 8);
      src += 4;   // advance past Header

      *dst = l_prediction; dst+=2;

      // RIGHT CHANNEL
      // get initial header step index
      r_stepIndex = *(src+2);

      // get initial header prediction
      r_prediction = *src + (*(src+1) << 8);
      src += 4;   // advance past Header

      *r_dst = r_prediction; r_dst+=2;
      
      // decode rest of packet
      for (; process>0; process--)
      {
         // left channel
         adpcmDecode(*src & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2; src++;
         adpcmDecode(*src & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2; src++;
         adpcmDecode(*src & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2; src++;
         adpcmDecode(*src & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &l_prediction, &l_stepIndex); *dst = l_prediction; dst += 2; src++;

         // right channel
         adpcmDecode(*src & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2; src++;
         adpcmDecode(*src & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2; src++;
         adpcmDecode(*src & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2; src++;
         adpcmDecode(*src & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2;
         adpcmDecode((*src >> 4) & 0x0f, &r_prediction, &r_stepIndex); *r_dst = r_prediction; r_dst += 2; src++;
      }
   }
   return AL_TRUE;
}    

/****************************************************************************/

typedef enum
{
  LittleEndian,
  BigEndian,
  UnknwonEndian                 /* has anybody still a PDP11? :-) */
} Endianess;

/* test from Harbison & Steele, "C - A Reference Manual", section 6.1.2 */
static Endianess
endianess (void)
{
  union
  {
    long l;
    char c[sizeof (long)];
  } u;

  u.l = 1;
  return (u.c[0] == 1) ? LittleEndian :
    ((u.c[sizeof (long) - 1] == 1) ? BigEndian : UnknwonEndian);
}

/****************************************************************************/

static int
safeToLower (int c)
{
  return isupper (c) ? tolower (c) : c;
}

static int
hasSuffixIgnoringCase (const char *string, const char *suffix)
{
  const char *stringPointer = string;
  const char *suffixPointer = suffix;

  if (suffix[0] == '\0')
    {
      return 1;
    }

  while (*stringPointer != '\0')
    {
      stringPointer++;
    }

  while (*suffixPointer != '\0')
    {
      suffixPointer++;
    }

  if (stringPointer - string < suffixPointer - suffix)
    {
      return 0;
    }

  while (safeToLower (*--suffixPointer) == safeToLower (*--stringPointer))
    {
      if (suffixPointer == suffix)
        {
          return 1;
        }
    }

  return 0;
}

static BufferData *
loadWavFile (InputStream *stream)
{
  ALboolean found_header = AL_FALSE;
  WaveFMT*   wavefmt;
  UInt32LittleEndian chunkLength;
  Int32BigEndian magic;
  UInt16LittleEndian audioFormat;
  UInt16LittleEndian numChannels;
  UInt32LittleEndian sampleFrequency;
  UInt32LittleEndian byteRate;
  UInt16LittleEndian blockAlign;
  UInt16LittleEndian bitsPerSample;
  Codec *codec = _alutCodecLinear;

  if (!_alutInputStreamReadUInt32LE (stream, &chunkLength) ||
      !_alutInputStreamReadInt32BE (stream, &magic))
    {
      return NULL;
    }

  if (magic != 0x57415645)      /* "WAVE" */
    {
      _alutSetError (ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
      return NULL;
    }

  while (1)
    {
      if (!_alutInputStreamReadInt32BE (stream, &magic) ||
          !_alutInputStreamReadUInt32LE (stream, &chunkLength))
        {
          return NULL;
        }

      if (magic == 0x666d7420)  /* "fmt " */
        {
          found_header = AL_TRUE;

          if (chunkLength < 16)
            {
              _alutSetError (ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
              return NULL;
            }
          wavefmt=(WaveFMT*)_alutInputStreamHere(stream);
          if (!_alutInputStreamReadUInt16LE (stream, &audioFormat) ||
              !_alutInputStreamReadUInt16LE (stream, &numChannels) ||
              !_alutInputStreamReadUInt32LE (stream, &sampleFrequency) ||
              !_alutInputStreamReadUInt32LE (stream, &byteRate) ||
              !_alutInputStreamReadUInt16LE (stream, &blockAlign) ||
              !_alutInputStreamReadUInt16LE (stream, &bitsPerSample))
            {
              return NULL;
            }

          if (!_alutInputStreamSkip (stream, chunkLength - 16))
            {
              return NULL;
            }

          switch (audioFormat)
            {
            case 1:            /* PCM */
              codec = (bitsPerSample == 8
                       || endianess () ==
                       LittleEndian) ? _alutCodecLinear : _alutCodecPCM16;
              break;
            case 7:            /* uLaw */
              bitsPerSample *= 2;       /* ToDo: ??? */
              codec = _alutCodecULaw;
              break;
            case 17: /* adpcm */
             {                            
              InitIMA_ADPCM(wavefmt);
              codec = _alutCodecADPCM;
              bitsPerSample *= 4;
             }
            break;
            default:
              _alutSetError (ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
              return NULL;
            }
        }
      else if (magic == 0x64617461)     /* "data" */
        {
          ALvoid *data;
          if (!found_header)
            {
              /* ToDo: A bit wrong to check here, fmt chunk could come later... */
              _alutSetError (ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
              return NULL;
            }
          data = _alutInputStreamRead (stream, chunkLength);
          if (data == NULL)
            {
              return NULL;
            }
          if(codec==_alutCodecADPCM)
           {
            Uint32 audio_len=chunkLength;
            int ret=IMA_ADPCM_decode((Uint8**)&data, &audio_len);
            int samplesize = ((AUDIO_S16 & 0xFF)/8)*wavefmt->channels;
	           audio_len &= ~(samplesize-1);
            return _alutBufferDataConstruct (data, audio_len, numChannels, bitsPerSample, sampleFrequency);
           }
          else
           return codec (data, chunkLength, numChannels, bitsPerSample,
                         (ALfloat) sampleFrequency);
        }
      else
        {
          if (!_alutInputStreamSkip (stream, chunkLength))
            {
              return NULL;
            }
        }

      if ((chunkLength & 1) && !_alutInputStreamEOF (stream)
          && !_alutInputStreamSkip (stream, 1))
        {
          return NULL;
        }
    }
}

static BufferData *
loadAUFile (InputStream *stream)
{
  Int32BigEndian dataOffset;    /* byte offset to data part, minimum 24 */
  Int32BigEndian len;           /* number of bytes in the data part, -1 = not known */
  Int32BigEndian encoding;      /* encoding of the data part, see AUEncoding */
  Int32BigEndian sampleFrequency;       /* number of samples per second */
  Int32BigEndian numChannels;   /* number of interleaved channels */
  size_t length;
  Codec *codec;
  char *data;
  ALint bitsPerSample;

  if (!_alutInputStreamReadInt32BE (stream, &dataOffset) ||
      !_alutInputStreamReadInt32BE (stream, &len) ||
      !_alutInputStreamReadInt32BE (stream, &encoding) ||
      !_alutInputStreamReadInt32BE (stream, &sampleFrequency) ||
      !_alutInputStreamReadInt32BE (stream, &numChannels))
    {
      return AL_FALSE;
    }

  length = (len == -1) ?
    (_alutInputStreamGetRemainingLength (stream) - AU_HEADER_SIZE -
     dataOffset) : (size_t) len;

  if (!
      (dataOffset >= AU_HEADER_SIZE && length > 0 && sampleFrequency >= 1
       && numChannels >= 1))
    {
      _alutSetError (ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA);
      return AL_FALSE;
    }

  if (!_alutInputStreamSkip (stream, dataOffset - AU_HEADER_SIZE))
    {
      return AL_FALSE;
    }

  switch (encoding)
    {
    case AU_ULAW_8:
      bitsPerSample = 16;
      codec = _alutCodecULaw;
      break;
    case AU_PCM_8:
      bitsPerSample = 8;
      codec = _alutCodecPCM8s;
      break;
    case AU_PCM_16:
      bitsPerSample = 16;
      codec =
        (endianess () == BigEndian) ? _alutCodecLinear : _alutCodecPCM16;
      break;
    case AU_ALAW_8:
      bitsPerSample = 16;
      codec = _alutCodecALaw;
      break;
    default:
      _alutSetError (ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE);
      return AL_FALSE;
    }

  data = _alutInputStreamRead (stream, length);
  if (data == NULL)
    {
      return NULL;
    }
  return codec (data, length, numChannels, bitsPerSample,
                (ALfloat) sampleFrequency);
}

static BufferData *
loadRawFile (InputStream *stream)
{
  size_t length = _alutInputStreamGetRemainingLength (stream);
  ALvoid *data = _alutInputStreamRead (stream, length);
  if (data == NULL)
    {
      return NULL;
    }
  /* Guesses */
  return _alutCodecLinear (data, length, 1, 8, 8000);
}

static BufferData *
loadFile (InputStream *stream)
{
  const char *fileName;
  Int32BigEndian magic;

  /* Raw files have no magic number - so use the fileName extension */

  fileName = _alutInputStreamGetFileName (stream);
  if (fileName != NULL && hasSuffixIgnoringCase (fileName, ".raw"))
    {
      return loadRawFile (stream);
    }

  /* For other file formats, read the quasi-standard four byte magic number */
  if (!_alutInputStreamReadInt32BE (stream, &magic))
    {
      return AL_FALSE;
    }

  /* Magic number 'RIFF' == Microsoft '.wav' format */
  if (magic == 0x52494646)
    {
      return loadWavFile (stream);
    }

  /* Magic number '.snd' == Sun & Next's '.au' format */
  if (magic == 0x2E736E64)
    {
      return loadAUFile (stream);
    }

  _alutSetError (ALUT_ERROR_UNSUPPORTED_FILE_TYPE);
  return AL_FALSE;
}

ALuint
_alutCreateBufferFromInputStream (InputStream *stream)
{
  BufferData *bufferData;
  ALuint buffer;

  if (stream == NULL)
    {
      return AL_NONE;
    }

  bufferData = loadFile (stream);
  _alutInputStreamDestroy (stream);
  if (bufferData == NULL)
    {
      return AL_NONE;
    }

  buffer = _alutPassBufferData (bufferData);
  _alutBufferDataDestroy (bufferData);

  return buffer;
}

ALuint
alutCreateBufferFromFile (const char *fileName)
{
  InputStream *stream;
  if (!_alutSanityCheck ())
    {
      return AL_NONE;
    }
  stream = _alutInputStreamConstructFromFile (fileName);
  return _alutCreateBufferFromInputStream (stream);
}

ALuint
alutCreateBufferFromFileImage (const ALvoid *data, ALsizei length)
{
  InputStream *stream;
  if (!_alutSanityCheck ())
    {
      return AL_NONE;
    }
  stream = _alutInputStreamConstructFromMemory (data, length);
  return _alutCreateBufferFromInputStream (stream);
}

void *
_alutLoadMemoryFromInputStream (InputStream *stream, ALenum *format,
                                ALsizei *size, ALfloat *frequency)
{
  BufferData *bufferData;
  ALenum fmt;
  void *data;

  if (stream == NULL)
    {
      return NULL;
    }

  bufferData = loadFile (stream);
  if (bufferData == NULL)
    {
      _alutInputStreamDestroy (stream);
      return NULL;
    }
  _alutInputStreamDestroy (stream);

  if (!_alutGetFormat (bufferData, &fmt))
    {
      _alutBufferDataDestroy (bufferData);
      return NULL;
    }

  if (size != NULL)
    {
      *size = (ALsizei) _alutBufferDataGetLength (bufferData);
    }

  if (format != NULL)
    {
      *format = fmt;
    }

  if (frequency != NULL)
    {
      *frequency = _alutBufferDataGetSampleFrequency (bufferData);
    }

  data = _alutBufferDataGetData (bufferData);
  _alutBufferDataDetachData (bufferData);
  _alutBufferDataDestroy (bufferData);
  return data;
}

ALvoid *
alutLoadMemoryFromFile (const char *fileName, ALenum *format,
                        ALsizei *size, ALfloat *frequency)
{
  InputStream *stream;
  if (!_alutSanityCheck ())
    {
      return NULL;
    }
  stream = _alutInputStreamConstructFromFile (fileName);
  return _alutLoadMemoryFromInputStream (stream, format, size, frequency);
}

ALvoid *
alutLoadMemoryFromFileImage (const ALvoid *data, ALsizei length,
                             ALenum *format, ALsizei *size,
                             ALfloat *frequency)
{
  InputStream *stream;
  if (!_alutSanityCheck ())
    {
      return NULL;
    }
  stream = _alutInputStreamConstructFromMemory (data, length);
  return _alutLoadMemoryFromInputStream (stream, format, size, frequency);
}

/*
  Yukky backwards compatibility crap.
*/

void
alutLoadWAVFile (ALbyte *fileName, ALenum *format, void **data, ALsizei *size,
                 ALsizei *frequency
#if !defined(__APPLE__)
                 , ALboolean *loop
#endif
  )
{
  InputStream *stream;
  ALfloat freq;

  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  stream = _alutInputStreamConstructFromFile (fileName);
  *data = _alutLoadMemoryFromInputStream (stream, format, size, &freq);
  if (*data == NULL)
    {
      return;
    }

  if (frequency)
    {
      *frequency = (ALsizei) freq;
    }

#if !defined(__APPLE__)
  if (loop)
    {
      *loop = AL_FALSE;
    }
#endif
}

void
alutLoadWAVMemory (ALbyte *buffer, ALenum *format, void **data, ALsizei *size,
                   ALsizei *frequency
#if !defined(__APPLE__)
                   , ALboolean *loop
#endif
  )
{
  InputStream *stream;
  ALfloat freq;

  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  /* ToDo: Can we do something less insane than passing 0x7FFFFFFF? */
  stream = _alutInputStreamConstructFromMemory (buffer, 0x7FFFFFFF);
  _alutLoadMemoryFromInputStream (stream, format, size, &freq);
  if (*data == NULL)
    {
      return;
    }

  if (frequency)
    {
      *frequency = (ALsizei) freq;
    }

#if !defined(__APPLE__)
  if (loop)
    {
      *loop = AL_FALSE;
    }
#endif
}

void
alutUnloadWAV (ALenum UNUSED (format), ALvoid *data, ALsizei UNUSED (size),
               ALsizei UNUSED (frequency))
{
  /* Don't do an _alutSanityCheck () because it's not required in ALUT 0.x.x */

  free (data);
}

const char *
alutGetMIMETypes (ALenum loader)
{
  if (!_alutSanityCheck ())
    {
      return NULL;
    }

  /* We do not distinguish the loaders yet... */
  switch (loader)
    {
    case ALUT_LOADER_BUFFER:
      return "audio/basic,audio/x-raw,audio/x-wav";

    case ALUT_LOADER_MEMORY:
      return "audio/basic,audio/x-raw,audio/x-wav";

    default:
      _alutSetError (ALUT_ERROR_INVALID_ENUM);
      return NULL;
    }
}
