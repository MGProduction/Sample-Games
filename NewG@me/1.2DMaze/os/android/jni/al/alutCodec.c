#include "alutInternal.h"

ALboolean alimIMA_ADPCM_decodeMono16(ALshort *dst, ALbyte* src, int src_size, int src_sizeblock );
ALboolean alimIMA_ADPCM_decodeStereo16(ALshort *dst, ALbyte* src, int src_size, int src_sizeblock );


static const uint16_t IMA_ADPCMStepTable[89] =
	{
		7,	  8,	9,	 10,   11,	 12,   13,	 14,
	   16,	 17,   19,	 21,   23,	 25,   28,	 31,
	   34,	 37,   41,	 45,   50,	 55,   60,	 66,
	   73,	 80,   88,	 97,  107,	118,  130,	143,
	  157,	173,  190,	209,  230,	253,  279,	307,
	  337,	371,  408,	449,  494,	544,  598,	658,
	  724,	796,  876,	963, 1060, 1166, 1282, 1411,
	 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
	};


static const int IMA_ADPCMIndexTable[8] =
	{
	-1, -1, -1, -1, 2, 4, 6, 8,
	};

	int16_t PredictedValue;

	uint8_t StepIndex;

int IMA_ADPCM_Decode(unsigned adpcm)
	{
	int stepIndex = StepIndex;
	int step = IMA_ADPCMStepTable[stepIndex];
 int diff;
 int predicedValue;

	stepIndex += IMA_ADPCMIndexTable[adpcm&7];
	if(stepIndex<0)
		stepIndex = 0;
	else if(stepIndex>88)
		stepIndex = 88;
	StepIndex = stepIndex;

	diff = step>>3;
	if(adpcm&4)
		diff += step;
	if(adpcm&2)
		diff += step>>1;
	if(adpcm&1)
		diff += step>>2;

	predicedValue = PredictedValue;
	if(adpcm&8)
		predicedValue -= diff;
	else
		predicedValue += diff;
	if(predicedValue<-0x8000)
		predicedValue = -0x8000;
	else if(predicedValue>0x7fff)
		predicedValue = 0x7fff;
	PredictedValue = predicedValue;

	return predicedValue;
	}


unsigned IMA_ADPCM_DecodeBuff(int16_t* dst, const uint8_t* src, int srcOffset, unsigned srcSize)
	{
  int16_t* out;
  int16_t* end;
	// use given bit offset
	src += srcOffset>>3;

	// calculate pointers to iterate output buffer
	out = dst;
	end = out+(srcSize>>2);

	while(out<end)
		{
		// get byte from src
		unsigned adpcm = *src;

		// pick which nibble holds a adpcm value...
		if(srcOffset&4)
			{
			adpcm >>= 4;  // use high nibble of byte
			++src;		  // move on a byte for next sample
			}

		*out++ = IMA_ADPCM_Decode(adpcm);  // decode value and store it

		// toggle which nibble in byte to write to next
		srcOffset ^= 4;
		}

	// return number of bytes written to dst
	return (unsigned)out-(unsigned)dst;
}

/* Intel ADPCM step variation table */
static int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

struct adpcm_state {
    short	valprev;	/* Previous output value */
    char	index;		/* Index into stepsize table */
};

void
adpcm_decoder(indata, outdata, len, state)
    char indata[];
    short outdata[];
    int len;
    struct adpcm_state *state;
{
    signed char *inp;		/* Input buffer pointer */
    short *outp;		/* output buffer pointer */
    int sign;			/* Current adpcm sign bit */
    int delta;			/* Current adpcm output value */
    int step;			/* Stepsize */
    int valpred;		/* Predicted value */
    int vpdiff;			/* Current change to valpred */
    int index;			/* Current step change index */
    int inputbuffer;		/* place to keep next 4-bit value */
    int bufferstep;		/* toggle between inputbuffer/input */

    outp = outdata;
    inp = (signed char *)indata;

    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];

    bufferstep = 0;
    
    for ( ; len > 0 ; len-- ) {
	
	/* Step 1 - get the delta value */
	if ( bufferstep ) {
	    delta = inputbuffer & 0xf;
	} else {
	    inputbuffer = *inp++;
	    delta = (inputbuffer >> 4) & 0xf;
	}
	bufferstep = !bufferstep;

	/* Step 2 - Find new index value (for later) */
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 ) index = 88;

	/* Step 3 - Separate sign and magnitude */
	sign = delta & 8;
	delta = delta & 7;

	/* Step 4 - Compute difference and new predicted value */
	/*
	** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
	** in adpcm_coder.
	*/
	vpdiff = step >> 3;
	if ( delta & 4 ) vpdiff += step;
	if ( delta & 2 ) vpdiff += step>>1;
	if ( delta & 1 ) vpdiff += step>>2;

	if ( sign )
	  valpred -= vpdiff;
	else
	  valpred += vpdiff;

	/* Step 5 - clamp output value */
	if ( valpred > 32767 )
	  valpred = 32767;
	else if ( valpred < -32768 )
	  valpred = -32768;

	/* Step 6 - Update step value */
	step = stepsizeTable[index];

	/* Step 7 - Output value */
	*outp++ = valpred;
    }

    state->valprev = valpred;
    state->index = index;
}



ALvoid *
_alutCodecADPCM (ALvoid *data, size_t length, ALint numChannels,
                  ALint bitsPerSample, ALfloat sampleFrequency)
{
 size_t nsize=length*4;
 char*  ndata=(char*)malloc( nsize + 31 );
 memset(ndata+nsize,0,31);
 memcpy(ndata+length*3,data,length);
 //*size = ChunkHdr.Size * 4;
 //*data = malloc( *size + 31 );
 if(ndata)
  { 
    int bytesPerBlock = 1024;//((int)sampleFrequency + 14)/8 * 4 * numChannels;
    if  ( numChannels == 1 )
      alimIMA_ADPCM_decodeMono16( (ALshort*) ndata, data, length, bytesPerBlock );
    else
     {
      //struct adpcm_state state;
      //memset(&state,0,sizeof(state));
      //adpcm_decoder(data,ndata,length,&state);
      //IMA_ADPCM_DecodeBuff((short*)ndata,data,0,length);
      alimIMA_ADPCM_decodeStereo16( (ALshort*) ndata, data, nsize, bytesPerBlock );
     }
  }
 return _alutBufferDataConstruct (ndata, nsize, numChannels, bitsPerSample, sampleFrequency);
}

ALvoid *
_alutCodecLinear (ALvoid *data, size_t length, ALint numChannels,
                  ALint bitsPerSample, ALfloat sampleFrequency)
{
  return _alutBufferDataConstruct (data, length, numChannels, bitsPerSample,
                                   sampleFrequency);
}

ALvoid *
_alutCodecPCM8s (ALvoid *data, size_t length, ALint numChannels,
                 ALint bitsPerSample, ALfloat sampleFrequency)
{
  int8_t *d = (int8_t *) data;
  size_t i;
  for (i = 0; i < length; i++)
    {
      d[i] += (int8_t) 128;
    }
  return _alutBufferDataConstruct (data, length, numChannels, bitsPerSample,
                                   sampleFrequency);
}

ALvoid *
_alutCodecPCM16 (ALvoid *data, size_t length, ALint numChannels,
                 ALint bitsPerSample, ALfloat sampleFrequency)
{
  int16_t *d = (int16_t *) data;
  size_t i, l = length / 2;
  for (i = 0; i < l; i++)
    {
      int16_t x = d[i];
      d[i] = ((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF);
    }
  return _alutBufferDataConstruct (data, length, numChannels, bitsPerSample,
                                   sampleFrequency);
}

/*
 * From: http://www.multimedia.cx/simpleaudio.html#tth_sEc6.1
 */
static int16_t
mulaw2linear (uint8_t mulawbyte)
{
  static const int16_t exp_lut[8] = {
    0, 132, 396, 924, 1980, 4092, 8316, 16764
  };
  int16_t sign, exponent, mantissa, sample;
  mulawbyte = ~mulawbyte;
  sign = (mulawbyte & 0x80);
  exponent = (mulawbyte >> 4) & 0x07;
  mantissa = mulawbyte & 0x0F;
  sample = exp_lut[exponent] + (mantissa << (exponent + 3));
  if (sign != 0)
    {
      sample = -sample;
    }
  return sample;
}

ALvoid *
_alutCodecULaw (ALvoid *data, size_t length, ALint numChannels,
                ALint bitsPerSample, ALfloat sampleFrequency)
{
  uint8_t *d = (uint8_t *) data;
  int16_t *buf = (int16_t *) _alutMalloc (length * 2);
  size_t i;
  if (buf == NULL)
    {
      return NULL;
    }
  for (i = 0; i < length; i++)
    {
      buf[i] = mulaw2linear (d[i]);
    }
  free (data);
  return _alutBufferDataConstruct (buf, length * 2, numChannels,
                                   bitsPerSample, sampleFrequency);
}

/*
 * From: http://www.multimedia.cx/simpleaudio.html#tth_sEc6.1
 */
#define SIGN_BIT (0x80)         /* Sign bit for a A-law byte. */
#define QUANT_MASK (0xf)        /* Quantization field mask. */
#define SEG_SHIFT (4)           /* Left shift for segment number. */
#define SEG_MASK (0x70)         /* Segment field mask. */
static int16_t
alaw2linear (uint8_t a_val)
{
  int16_t t, seg;
  a_val ^= 0x55;
  t = (a_val & QUANT_MASK) << 4;
  seg = ((int16_t) a_val & SEG_MASK) >> SEG_SHIFT;
  switch (seg)
    {
    case 0:
      t += 8;
      break;
    case 1:
      t += 0x108;
      break;
    default:
      t += 0x108;
      t <<= seg - 1;
    }
  return (a_val & SIGN_BIT) ? t : -t;
}

ALvoid *
_alutCodecALaw (ALvoid *data, size_t length, ALint numChannels,
                ALint bitsPerSample, ALfloat sampleFrequency)
{
  uint8_t *d = (uint8_t *) data;
  int16_t *buf = (int16_t *) _alutMalloc (length * 2);
  size_t i;
  if (buf == NULL)
    {
      return NULL;
    }
  for (i = 0; i < length; i++)
    {
      buf[i] = alaw2linear (d[i]);
    }
  free (data);
  return _alutBufferDataConstruct (buf, length * 2, numChannels,
                                   bitsPerSample, sampleFrequency);
}
