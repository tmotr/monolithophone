
/******************************************************/
/***    ~Monolithophone~                            ***/
/*** Generates modulating sine and cosine waves     ***/
/*** dumps as mono 16 bit uncompressed for alsa     ***/
/*** pcm.  -----written in C by sage, the tmotr     ***/
/******************************************************/

/***
 *
 *   ~Include Block~
 * math (sin and cos functions utilized liberally),
 * stdio (scanf and printf utilized once each),
 * string (strncpy utilized to write RIFF/WAV headers),
 * inttypes (sizeof in file header),
 *
 ***/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <alsa/asoundlib.h>

/***
 *
 *   ~Define Block~
 * SAMPLERATE (sample rate of cdda standard),
 * PI (pi to ten places),
 * VOLUME (amplify double to short by this value),
 * MAXFREQUENCY (the highest frequency audible by humans),
 * MIDFREQUENCY (the mean of the frequencies audible by humans),
 * MINFREQUENCY (the lowest frequency audible by humans),
 * SECONDSOFSAMPLES(x) (number of samples in x seconds of waveform at given sample rate).
 * 
 ***/

#define SAMPLERATE 44100
#define PI 3.1415926535
#define VOLUME 32323
#define MAXFREQUENCY 2000
#define MIDFREQUENCY 1010
#define MINFREQUENCY   20 
#define SECONDSOFSAMPLES(x) (SAMPLERATE*x)

/***
 *
 *   ~Functions Block~
 * void dumpwav (short, int); -- dump product to pcm
 * double mkwav (int, int, int); -- generate primary sine
 * int modfreq (int, int); -- modulate a sine frequency
 *
 ***/

// The following dumps the WAV data into alsa PCM.
void dump_wav (snd_pcm_t * h, short w[]) {

  snd_pcm_sframes_t frames;
  snd_output_t *output = NULL;
  int data_length = sizeof(short)*SECONDSOFSAMPLES(4);

  frames = snd_pcm_writei(h, w, data_length);
  if (frames < 0)
    frames = snd_pcm_recover(h, frames, 0);

}

// The following defines generation of the desired waveform.
double mkwav (int i, int f, int m){
  switch (m % 4) {
  case 0:
    return sin(f*((double)i/SAMPLERATE)*(PI/((m+4)/4)));
  case 1:
    return cos(f*((double)i/SAMPLERATE)*(PI/((m+4)/4)));
  case 2:
    return sin(f*((double)i/SAMPLERATE)*(PI*((m+4)/4)));
  case 3:
    return cos(f*((double)i/SAMPLERATE)*(PI*((m+4)/4)));
  }
}

// The following defines the desired frequency modulation.
int modfreq (int m, int i) {
  switch (m % 4) {
  case 0:
    return MAXFREQUENCY*(sin(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI/((m+4)/4))));
  case 1:
    return MAXFREQUENCY*(cos(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI/((m+4)/4))));
  case 2:
    return MAXFREQUENCY*(sin(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI*((m+4)/4))));
  case 3:
    return MAXFREQUENCY*(cos(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI*((m+4)/4))));
  }
}

/***
 *
 *   ~Main Block~
 *
 ***/

int main (void) {

  //
  int modifier;
  short seconds = 4;
  short wavform [SECONDSOFSAMPLES(seconds)];

  //
  snd_pcm_t *handle;
  static char *device = "default";
  snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0);
  snd_pcm_set_params(handle,
		     SND_PCM_FORMAT_U8,
		     SND_PCM_ACCESS_RW_INTERLEAVED,
		     1,
		     44100,
		     1,
		     500000);
      

  
  //
  scanf(" %d", &modifier);

  //
  for(int i_global, i_freq = 1; i_global <= SECONDSOFSAMPLES(seconds) ; i_global++) {
    if (!(i_global % (SECONDSOFSAMPLES(seconds)/1000))) i_freq++;
    wavform[i_global] = VOLUME*mkwav(i_global,modfreq(modifier,i_freq),modifier);
  }

  //
  dump_wav(handle,wavform);
    
  //
  return 0;
}
