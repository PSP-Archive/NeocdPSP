/***************************************************************************

  streams.c

  Handle general purpose audio streams

***************************************************************************/
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>

#include "../neocd.h"

#define MIXER_MAX_CHANNELS 15
#define MIXER_OUTPUT_SHIFT 0
#define BUFFER_LEN 16384

#define CPU_FPS 60

extern int frame;

s32 left_buffer[BUFFER_LEN];
s32 right_buffer[BUFFER_LEN];


extern volatile int neogeo_soundmute;

int SamplePan[] = {//{ 0, 255, 128, 0, 255 };
	0,255,128,0,255,
	0,255,128,0,255,
	0,255,128,0,255
};


static int stream_joined_channels[MIXER_MAX_CHANNELS];

static int stream_vol[MIXER_MAX_CHANNELS];

struct {
    s16 *buffer;
    int param;
    void (*callback) (int param, s16 ** buffer, int length);
} stream[MIXER_MAX_CHANNELS];

void mixer_set_volume(int channel,int volume) {
    stream_vol[channel]=volume;
}

int streams_sh_start(void)
{
    int i;

    for (i = 0; i < MIXER_MAX_CHANNELS; i++) {
	stream_joined_channels[i] = 1;
	stream[i].buffer = 0;
    }
    return 0;
}


void streams_sh_stop(void)
{
    int i;

    for (i = 0; i < MIXER_MAX_CHANNELS; i++) {
	if (stream[i].buffer)
		free(stream[i].buffer);
	stream[i].buffer = 0;
    }
}

#define RANGE_CHECK(a) (a>32767?32767:(a<-32768?-32768:a))
		

void streamupdate(s16 *mix_buffer,int len) {
	/*static int current_pos;*/
  int channel, i,j;
  int there_is_data;
  u16 *bl, *br;
  u16 *pl;
  
	there_is_data=0;
  for (channel = 0; channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel]) {
		if (stream[channel].buffer) {
	    int buflen;
			there_is_data=1;
	    buflen = len >> 1;	// BUFFER_LEN; // newpos - stream_buffer_pos[channel];
	    if (stream_joined_channels[channel] > 1) {
				s16 *buf[MIXER_MAX_CHANNELS];
				// on update le YM2610
				if (buflen > 0) {
			    for (i = 0; i < stream_joined_channels[channel]; i++)
					buf[i] = stream[channel + i].buffer;	// + stream[channel+i].buffer_pos;
		    	(*stream[channel].callback) (stream[channel].param,buf, buflen);
				}
	    }
		}
  }
  
  if (there_is_data&&!neogeo_soundmute) {    
    /* init output buffers */ 
    memset(left_buffer, 0, len*2);
    memset(right_buffer, 0, len*2);    

		/* update all the output buffers */
    bl = left_buffer;
    br = right_buffer;

    for (channel = 0; channel < MIXER_MAX_CHANNELS;channel += stream_joined_channels[channel]) {
			if (stream[channel].buffer) {			
	    	for (i = 0; i < stream_joined_channels[channel]; i++) {				
					if (SamplePan[channel + i] <= 128){
						int x;s32 *dst;s16 *src;			
						src=(s16 *) stream[channel + i].buffer;
						dst=bl;
						for (x=len>>1;x;x--) *dst++ =*dst+(s32)(*src++);
					}		    
					if (SamplePan[channel + i] >= 128){		    
						int x;s32 *dst;s16 *src;			
						src=(s16 *) stream[channel + i].buffer;
						dst=br;
						for (x=len>>1;x;x--) *dst++ =*dst+(s32)(*src++);
					}
	    	}
			}
    }
    //SDL_LockAudio();    
    s32 *mix_b32=(s32*)mix_buffer;
    s32 *sl,*sr;
    sl=left_buffer;sr=right_buffer;
    s32 l;
    for (i = len >> 1; i ; i--,sl++,sr++ ) {    	    	
    	l=RANGE_CHECK((*sl)>>MIXER_OUTPUT_SHIFT);
			*mix_b32++ = (l<<16)|l;
			l=RANGE_CHECK((*sr)>>MIXER_OUTPUT_SHIFT);
			*mix_b32++ = (l<<16)|l;
    }
  }else{
  	memset(mix_buffer,0,len<<2);
	}
 

}

int stream_init_multi(int channels, int param,
		      void (*callback) (int param, s16 ** buffer,
					int length))
{
    static int channel, i;

    stream_joined_channels[channel] = channels;

    for (i = 0; i < channels; i++) {

	if ((stream[channel + i].buffer =
	     malloc(sizeof(s16) * BUFFER_LEN)) == 0)
	    return -1;
    }

    stream[channel].param = param;
    stream[channel].callback = callback;
    channel += channels;
    return channel - channels;
}
