//
//	BeOS code for amp-0.7.4, (C) 1997 Andy Lo A Foe	 	
//


#include <MediaKit.h>
#include <KernelKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amp.h"
#include "transform.h"
#include "audioIO.h"
#include "audio.h"

// Define the streambuf size here. streambuf is used
// as a simple fit buffer. 

#define STREAMBUF_SIZE	(32*1152)

long                bytes_in_streambuf;

// streambuf definition and base indicator

BSubscriber			*the_sub;
BDACStream			*the_stream;

sem_id             ok_to_read;
sem_id			    ok_to_write;


char                streambuf[STREAMBUF_SIZE];
char                *readbase;

static	int			au_vol = 100;

// Define our own printout function since we are
// first writing in the streambuf (not needed, I think,
// but I don't know if the DACStream accepts a buffer
// size of 1152. Try it...

void printout(void)
{
if (A_WRITE_TO_FILE) {
#ifndef NO_BYTE_SWAPPING
int i,j;
short *ptr;

        if (nch==2) {
                ptr=(short*)stereo_samples;
                i=j=32 * 18 * 2;
        } else {
                ptr=(short*)mono_samples;
                i=j=32 * 18;
        }

        for (;i>=0;--i)
                ptr[i] = ptr[i] << 8 | ptr[i] >> 8;
#endif

        if (nch==2)
                fwrite(stereo_samples,1,sizeof stereo_samples,out_file);
        else
                fwrite(mono_samples,1,sizeof mono_samples,out_file);
}


        if (A_AUDIO_PLAY) {
			static char au_buf[STREAMBUF_SIZE];
			static int au_ptr =0, avail = 0;
			static int num_smp;
			static char *readbase = &au_buf[0];
			
			// Write amount of samples to copy somewhere
			num_smp  =  (nch == 2 ? sizeof stereo_samples : sizeof mono_samples);
			
			// Copy samples in the fit buffer
			memcpy(au_buf+au_ptr,(nch == 2 ? (char *)stereo_samples : (char *)mono_samples),
                num_smp);
                
            // Increase fit buffer pointer and available sample count    
	        au_ptr+=num_smp;
	        avail+=num_smp; 
	        
    	    if (avail >= 4096) { // Are there enough smps to feed the stream?
                audioWrite((char*)readbase,4096);	// Feed it!
                readbase+=4096;	// Increase readbase
                avail-=4096;		// Decrease avail smps count
                if (au_ptr == STREAMBUF_SIZE) {	// At end of fit buffer?
                	au_ptr=0;				// Reset all pointers
                	readbase=&au_buf[0];
                }
	        }
	    }

}


// Fake Buffer functions, just to keep the sources clean,
// buffer.c should not be included in the link process...

int AUDIO_BUFFER_SIZE;

int
audioBufferOpen(int frequency, int stereo, int volume)
{
	audioOpen(frequency, stereo, volume);
}


inline void
audioBufferWrite(char *buf,int bytes)
{
	audioWrite(buf, bytes);
}


void
audioBufferClose()
{
	audioClose();
}


int audioRead(char *buffer, int count)
{
    //printf("acquiring ok_to_read (%d bytes)\n", count);
    if (acquire_sem(ok_to_read)==B_NO_ERROR) {
		for (register int i=0; i < count;i++) {
           *(buffer++)+=*(readbase++);
    	}  
        bytes_in_streambuf-=count;
        
        if (bytes_in_streambuf <= 0) {
            release_sem(ok_to_write);
       		bytes_in_streambuf = 0;
    	} else {
        	release_sem(ok_to_read);
    	}  
    }    
    return (0);
}


bool stream_func(void *arg, char *buf, size_t count, void *header)
{
	audioRead(buf, count);
	return TRUE;
}


void audioOpen(int frequency, int stereo, int volume)
{
	int channels;
	
	readbase = &streambuf[0];
	
	bytes_in_streambuf = 0;
	  
	the_sub = new BSubscriber("amp DAC writer");
	the_stream = new BDACStream();
	
	the_sub->Subscribe(the_stream);
	
	the_stream->SetSamplingRate(frequency);
	
	// Create semaphores
    ok_to_read = create_sem(0, "read sem");
    ok_to_write = create_sem(1, "write sem");
   
	// Initialize the streambuf
    bytes_in_streambuf = 0;
    memset(&streambuf, 0, STREAMBUF_SIZE);
	
	// Set up volume
	if (volume != -1)
		audioSetVolume(volume);
		
	// Enter the stream
	the_sub->EnterStream(NULL, TRUE, NULL, stream_func, NULL, TRUE);
}


void audioSetVolume(int volume)
{
	if (volume > 128)	// This allows for a modest volume boost
		volume = 128;
	au_vol = volume;
}


void audioClose()
{
	the_sub->ExitStream(TRUE);
	the_sub->Unsubscribe();
	
    delete_sem(ok_to_read);
    delete_sem(ok_to_write);
        
    delete the_sub;
    delete the_stream;
}


// audioWrite is called from the player thread

int audioWrite(char *buffer, int count)
{
        //printf("acquiring ok_to_write (%d bytes)\n", count);
        if(acquire_sem(ok_to_write)==B_NO_ERROR)
        {
            memcpy(&streambuf, buffer, count);
            
            if (au_vol != 100) {	// Handle volume scaling here
            	short *b=(short *)&streambuf;
            	for (int i=0; i < count/2; i++) {
            		int v=((int)b[i]*au_vol)/100;
            		b[i]=(v>32767) ? 32767 : ((v<-32768) ? -32768 : v);
            	}
            }
            	
            bytes_in_streambuf = count;
            readbase = &streambuf[0];

            release_sem(ok_to_read);
        }   
    return 0;
}

