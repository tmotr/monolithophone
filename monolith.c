
/******************************************************/
/***    ~Monolithophone~                            ***/
/*** Generates modulating sine and cosine waves     ***/
/***     plays 16 bit uncompressed pcm to alsa.     ***/
/***       -----written in C by sage, the tmotr     ***/
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
 * SAMPLERATE (sample rate 44100),
 * PI (pi to ten places),
 * VOLUME (amplify double to short by this value),
 * MAXFREQUENCY (the highest frequency audible by humans),
 * MINFREQUENCY (the lowest frequency audible by humans),
 * MIDFREQUENCY (the average of max and min frequency),
 * SECONDSOFSAMPLES(x) (number of samples in x seconds of waveform at given sample rate).
 *
 ***/

#define SAMPLERATE 44100
#define PI 3.1415926535
#define VOLUME 32323
#define AVERAGE(a,b,n) (a+b)/(n)
#define MAXFREQUENCY 2000
#define MINFREQUENCY   20
#define MIDFREQUENCY AVERAGE(MAXFREQUENCY,MINFREQUENCY,2)
#define SECONDSOFSAMPLES(x) (SAMPLERATE*x)

/***
 *
 *   ~Functions Block~
 * void dumpwav (short, int); -- dump product to pcm
 * double mkwav (int, int, int); -- generate primary sine
 * int modfreq (int, int); -- modulate a sine frequency
 *
 ***/

// The writes wavform to alsa PCM.
void dump_wav (snd_pcm_t * h, short w[]) {
// snd_pcm_t * alsa device pointer
// short wavform
  snd_pcm_sframes_t frames;
  int data_length = sizeof(short)*SECONDSOFSAMPLES(4);

  frames = snd_pcm_writei(h, w, data_length);
  if (frames < 0)
    frames = snd_pcm_recover(h, frames, 0);

}

// mkwav defines generation of wavform sample
double mkwav (int i, int f, int m){
/***
  * x = finite arithmetic sequence, 
  * ... { x | 0 < x < limit }
  * Let f(x) = the input of another function 
  * Let F = Wavelength of a trigonometric function
  * ... ... x*\f(x)/44100
  * ... m = an input -  
  * ... T = F * an Angle in Radians
  * ... ... F*\pi divided-by 4*m+16  , if m = {0,1} (mod 4)
  * ... ... F*\pi*m+4\pi divided-by 4 , if m = {2,3} (mod 4) 
  * ... R(x) = the trigonometric function of T
  * ... .... \sin(T). . . , if m = {0,3} (mod 4)
  * ... .... \cos(T) . . ., if m = {1,4} (mod 4)
 ***/

  switch (m % 4) {
  case 0:
    return sin(f*((double)i/SAMPLERATE)*(PI/((m+4)/4)));
  case 1:
    return cos(f*((double)i/SAMPLERATE)*(PI/((m+4)/4)));
  case 2:
    return sin(f*((double)i/SAMPLERATE)*(PI*((m+4)/4)));
  case 3:
    return cos(f*((double)i/SAMPLERATE)*(PI*((m+4)/4)));
  default:
    return 1;
  }

  return -1; //something's obviously wrong if you're here.
}

// *the variable f (above) defined below--modfreq (m,i)* //

int modfreq (int m, int i) {

/***
  * m = an input (same as mkwav)
  * x = a sequence (same as mkwav)
  * Z = \pi divided-by 4*m+16 	, if m = {0,1} (mod 4)
  * ... \pi*m/4 plus \pi	, if m = {2,3} (mod 4)
  * Y = Z * 1010*x/44100  
  * F = piecewise trigonometric function 
  * ... 2020*\sin(Y) 		, if m = {0,2} (mod 4) 
  * ... 2020*\cos(Y) 		, if m = {1,3} (mod 4)
 ***/

  switch (m % 4) {
  case 0:
    return MAXFREQUENCY*(sin(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI/((m+4)/4))));
  case 1:
    return MAXFREQUENCY*(cos(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI/((m+4)/4))));
  case 2:
    return MAXFREQUENCY*(sin(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI*((m+4)/4))));
  case 3:
    return MAXFREQUENCY*(cos(MIDFREQUENCY*((double)i/SAMPLERATE)*(PI*((m+4)/4))));
  default:
    return 1;
  }
  return -1;
}

/***
 *
 *   ~Main Block~
 *
 ***/

int main (int argv, char ** ArgV) {

  //
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

  //fill shorts with trig wave defined by the function (above).
  //  define two iterators, global and local (for modfreq function)
  //  only iterate while the global iterator is lessor=to length-of-array
  //  only increment global iterator

  for(int i_global, i_freq = 1; \
            i_global <= SECONDSOFSAMPLES(seconds); \
            i_global++) { 

  // if the global iterator is a divisible by a one hundreth of the array-length
  //   increment local iterator
  // IOW-- increment local iterator every thousanth [minus one...] iteration 

    if (!(i_global % (SECONDSOFSAMPLES(seconds)/1000)))
        i_freq++;

  // m = input
  // Sample = mkwav(\global, modfreq(m,\local) ,m) 
  // modfreq defines the f in mkwav(i,f,m)
  // the i in mkwav is the current (in loop) value of the global iterator
  // at Current Index (in loop), 
  // 	(integer, short)<- Sample[floating-point] multiplied-by anAmplitude[integer] 

    wavform[i_global] = \
    VOLUME*\
        mkwav(i_global, modfreq( argv, i_freq ), argv);

  }//out-loop, end

  //play wavform
  dump_wav(handle,wavform);

  //done.
  return 0;
}
