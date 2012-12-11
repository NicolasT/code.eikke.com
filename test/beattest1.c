/*
 * Very simple wave player. 
 *
 * Paul van der Mark
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef AUDIODEV
#define AUDIODEV "/dev/dsp"
#endif

#define FILENAME "test.wav"

typedef struct {
    char            RiffID [4] ;
    u_long            RiffSize ;
    char            WaveID [4] ;
    char            FmtID  [4] ;
    u_long          FmtSize ;
    u_short         wFormatTag ;
    u_short         nChannels ;
    u_long          nSamplesPerSec ;
    u_long          nAvgBytesPerSec ;
    u_short         nBlockAlign ;
    u_short         wBitsPerSample ;
} wave_header;

typedef struct {
    int filefd, audiofd;
    ssize_t buflen,reallen;
    /*@NULL@*/ char *buffer;
    /*@NULL@*/ wave_header *wh;
} audio_info;

static void cleanup(audio_info *);
static int openwav(/*@out@*/audio_info *, char *);
static int setupDSP(/*@out@*/audio_info *);
static int readbuf(audio_info *);
static void playDSP(audio_info *);

int main(int argc, char **argv) {
	audio_info *ai;

	if(0==(ai=malloc(sizeof(audio_info)))) {
		perror("malloc");
		return 1;
	}
	(void) memset(ai, 0, sizeof(audio_info));

	/* open wave file for reading + check if it is
	   a wave file */
	if(-1==(openwav(ai, FILENAME)))  { 
		cleanup(ai);
		free(ai);
		return 1;
	}

	/* setup audio device for playing */
	if(-1==setupDSP(ai)) {
		cleanup(ai);
		free(ai);
		return 1;
	}

	if(-1==ioctl(ai->audiofd, SNDCTL_DSP_SYNC))
		printf("Synchronisation of audio-device failed\n");

	/* read a buffer and play it */
	while(readbuf(ai)!=0) {
	    playDSP(ai);
	}
	
	cleanup(ai);
	free(ai);

	return 0;
}
	
static void cleanup(audio_info *ai) {
	if(ai->buffer!=0) 
		free(ai->buffer); /* release buffer */
	if(ai->audiofd>0) 
		(void) close(ai->audiofd); /* close audio device */
	if(ai->filefd>0) 
		(void) close(ai->filefd); /* close input file */
	if(ai->wh!=0)
	    free(ai->wh);
	ai->buffer=0;
	ai->wh=0;
}

/* open file for reading and parse RIFF header:
   A riff header is constructed of a header word+opt. size of chunk+chunk(data)
   where a chunk can contain multiple headers

   So a wave file looks like
   'R','I','F','F' size 
	'W','A','V','E'
	    'f','m','t',' ' size
	       chunk with format parameters
	    optional chunks like fact
	    'd','a','t','a', size 
		chunk with samples
   RIFFssssWAVEfmt ssssbbbbbbbbbdatabbbbbbb
   If you think this is complicated:look at compressed audio/video(AVI) riffs :)
*/

typedef u_char uc;
#define TOFOUR(a) (((uc) a[0]<<24)|((uc) a[1]<<16)|((uc) a[2]<<8)|((uc) a[3]))
#define RIFF TOFOUR("RIFF")
#define WAVE TOFOUR("WAVE")
#define FMT  TOFOUR("fmt ")
#define DATA TOFOUR("data")

static int openwav(audio_info *ai, char *filename) {
    char data[4]={'\0','\0','\0','\0'};
    float playlength;
    size_t size=0;

    if(0==(ai->wh=malloc(sizeof(wave_header)))) {
	perror("malloc");
	return -1;
    }
   
    (void) memset(ai->wh, 0, sizeof(wave_header)); 
    if(-1==(ai->filefd=open(filename, O_RDONLY))) {
	perror("openwav");
	return -1;
    }
    if(read(ai->filefd, ai->wh, sizeof(wave_header))!=(ssize_t) sizeof(wave_header)) {
	fprintf(stderr, "Failed to read wave header from file %s\n",
		filename);
	return -1;
    }
    
    if( (TOFOUR(ai->wh->RiffID) != RIFF) ||
	(TOFOUR(ai->wh->WaveID) != WAVE) ||
	(TOFOUR(ai->wh->FmtID) != FMT)) {
	fprintf(stderr, "Not a wave file %s\n", filename);
	return -1;
    }
    (void) lseek(ai->filefd, ai->wh->FmtSize+16+4,SEEK_SET); /* skip offset after fmt */
    do {
	(void) read(ai->filefd, data, 4);
	(void) read(ai->filefd, &size, 4);
	if(TOFOUR(data)!=DATA) {
	    char *dummy;
	    printf("Found chunk %c%c%c%c with size %d\n", 
		   data[0], data[1],data[2],data[3],(int) size);
	    if(0==(dummy=malloc(size))) {
		perror("malloc");
		return -1;
	    }
	    (void) memset(dummy, 0, size);
	    if((ssize_t) size!=read(ai->filefd, dummy, size)) {
		fprintf(stderr, "Unexpected end of file\n");
		free(dummy);
		return -1;
	    }
	    free(dummy);
	}
    } while(TOFOUR(data)!=DATA);
    
    playlength=(float) size/(ai->wh->nChannels*ai->wh->nSamplesPerSec);

    switch(ai->wh->wBitsPerSample) {
    case 8: break;
    case 16: 
	playlength/=2;
	break;
    default: fprintf(stderr, "Can not play samples with bitsize %ud\n",
		     ai->wh->wBitsPerSample);
	return -1;
    }
    printf("File        : %s\n"
	   "Mode        : %s\n"
	   "SampleRate  : %lu\n"
	   "Time        : %.1f sec\n", filename, 
	   ai->wh->nChannels==2?"Stereo":"Mono",
	   ai->wh->nSamplesPerSec, playlength);
    return 1;
}

/* open /dev/dsp for writing : setup the device with the
   appropriate ioctls (SAMPLESIZE,STEREO,SPEED) */
static int setupDSP(audio_info *ai) {
	ssize_t blocksize=0;
	int bits, stereo;

	if ( (ai->audiofd = open(AUDIODEV, O_WRONLY)) == -1) {
		perror("open audiodevice");
		return -1;
	}
	if ( ioctl(ai->audiofd, SNDCTL_DSP_GETBLKSIZE, &blocksize) == -1) {
		perror("ioctl blocksize");
		return -1;
	}
	if (blocksize < 4096) { /* why? */
		fprintf(stderr, "ioctl blocksize too small\n");
		return -1;
	}
	ai->buflen=blocksize;
	if((ai->buffer=malloc((size_t) blocksize)) == 0) {
		perror("malloc buffer");
		return -1;
	}

	/* ioctl needs a (32bit) integer; wBitsPerSample is 16 bit */
	bits=(int) ai->wh->wBitsPerSample;
	stereo=ai->wh->nChannels==2?1:0;
	if(-1==ioctl(ai->audiofd, SNDCTL_DSP_SAMPLESIZE, &bits) ||
	   -1==ioctl(ai->audiofd, SNDCTL_DSP_STEREO, &stereo) ||
	   -1==ioctl(ai->audiofd, SNDCTL_DSP_SPEED, &(ai->wh->nSamplesPerSec))) {
	    perror("ioctl");
	    return -1;
	}

	return 1;
}

/* We only use 1 buffer because file-io is much faster than we can play audio
   For a network this doens't has to be true */
static int readbuf(audio_info *ai) {
	if(ai->buffer==0) 
		return 0;
	ai->reallen=read(ai->filefd, ai->buffer, (size_t) ai->buflen);
	if(ai->reallen<=0) 
		return 0;
	return 1;
}

static unsigned long calculateEnergy(const char *bufftmp) {
        unsigned long e;
        int c;

        for(c = 0; c < 1024 * 2; c += 2) {
                /* e = sum(left[k]^2 + right[k]^2, k=0...1024) */
                e += ( (int)bufftmp[c] * (int)bufftmp[c] ) + ( (int)bufftmp[c+1] * (int)bufftmp[c+1] );
        }

        return e;
}

static void playDSP(audio_info *ai) {
	int t;
        static unsigned long E[43];
        unsigned long e = 0;
        int count = (ai->reallen / 2) / 1024;
        unsigned long average;
        unsigned long long total = 0;
        int i;

        for( t = 0; t < 43; t++ ) {
                E[t] = 0;
        }

        if(ai->buffer == 0)
                return;

        for( t = 0; t < count; t++) {
                /* Always calculate the energy of in 1024-sample blocks (actually, 1024 * 2 because of left-right) */
                e = calculateEnergy(ai->buffer + (2 * 1024 * count * sizeof(char)));
                printf("t = %d, Energy = %u\n", t, e);

                /* Check whether we got a beat */
                /* average = 1/43 * sum(E[i]^2, i=0...43) */
                for(i = 0; i < 43; i++) {
                        total += E[i] * E[i];
                }
                average = 1/43 * total;

/*                printf("total = %u, average = %u\n", total, average);*/

                /* shift E */
                for(i = 0; i < 42; i++) {
/*                        printf("%u ", E[i]);*/
                        E[i+1] = E[i];
                }
/*                printf("\n");*/
                E[0] = e;
                total = 0;
        }

        if(write(ai->audiofd, ai->buffer, (size_t) ai->reallen)!=ai->reallen) 
	        perror("writing of audio");
}
