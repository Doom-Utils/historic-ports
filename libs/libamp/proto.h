#if !defined (DJGPP) && !defined(__CYGWIN__)
/* From: util.c */
void die(char *, ...);
void warn(char *, ...);
void msg(char *, ...);
void debugSetup(char *);
void debugOptions();
/* From: audioIO_<OSTYPE>.c */
void audioOpen(int frequency, int stereo, int volume);
void audioSetVolume(int);
void audioFlush();
void audioClose();
int  audioWrite(char *, int);
int  getAudioFd();
void audioBufferOn(int);

/* From: buffer.c */
void printout(void);
int  audioBufferOpen(int, int, int);
void audioBufferClose();
void audioBufferWrite(char *, int);
void audioBufferFlush();
#else
#define die(args...) printf( ## args);\
                          exit(1);
#define msg(args...) printf( ## args)
#define warn(args...) printf(## args)
#endif
/* From: audio.c */
void displayUsage();


extern int A_DOWNMIX;
