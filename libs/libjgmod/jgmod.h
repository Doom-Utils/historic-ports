#ifndef JGMOD_H
#define JGMOD_H

#ifndef uchar
#define uchar   unsigned char
#endif

#ifndef ushort
#define ushort  unsigned short
#endif

#ifndef unit
#define uint    unsigned int
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ALLEGRO_H
#error JGMOD can only be used with Allegro
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define NTSC 3579546L

#define LOOP_OFF    0
#define LOOP_ON     1
#define LOOP_BIDI   2

#define JGMOD_VERSION       0
#define JGMOD_SUB_VERSION   23
#define JGMOD_VERSION_STR   "0.23"
#define JGMOD_DATE_STR      "5 April 1998"

#ifndef DJGPP
#define _go32_dpmi_lock_data(d, p)
#endif

//-- Header ------------------------------------------------------------------
typedef struct CHANNEL_INFO
{
    int sample;
    int volume;
    int pitch;
    int period;
    int c2spd;
    int pan;

    int s3m_volume_slide_on;
    int s3m_volume_slide;
    int s3m_fine_volume_slide;
    int pro_volume_slide;


    int pro_pitch_slide;
    int s3m_pitch_slide_on;
    int s3m_pitch_slide;
    int s3m_fine_pitch_slide;


    int slide2pitch;
    int slide2pitch_spd;
    int retrig;
    int cut_sample;
    int delay_sample;
    int arpeggio;
    int sample_offset;

    int vibrato_on;
    int vibrato_shift;
    int vibrato_spd;
    int vibrato_depth;
    signed char vibrato_pointer;
    int vibrato_waveform;

    int loop_on;
    int loop_start;
    int loop_times;

    int tremor_on;
    int tremor_count;
    int tremor_set;         //1st byte - off.  2nd byte - on

}CHANNEL_INFO; 

typedef struct MUSIC_INFO
{
    int max_chn;
    int no_chn;

    int tick;
    int pos;
    int pat;
    int trk;

    int bpm;
    int tempo;
    int global_volume;

    int new_pos;        // for pattern break
    int new_trk;        // or position jump
    int pattern_delay;  // pattern delay

    int skip_pos;       // for next_pattern
    int skip_trk;       // or prev_pattern
    int loop;           // replay the music if ended
    int pause;          // for pause function
    int forbid;
    int is_playing;

}MUSIC_INFO; 

typedef struct NOTE_INFO
{
    int sample;
    int pitch;
    int volume;
    int command;
    int extcommand;
}NOTE_INFO;


typedef struct SAMPLE_INFO
{
    int lenght;
    int c2spd;
    int volume;
    int repoff;
    int replen;
    int loop;
}SAMPLE_INFO;

typedef struct PATTERN_INFO
{
    NOTE_INFO *ni;
    int no_pos;
}PATTERN_INFO;


typedef struct JGMOD
{
    char name[29];
    SAMPLE_INFO *si;
    PATTERN_INFO *pi;
    SAMPLE *s;

    int no_trk;
    int no_pat;
    int pat_table[256];
    int panning[32];

    int tempo;
    int bpm;

    int restart_pos;       
    int no_chn;
    int no_sample;
    int global_volume;

}JGMOD;


//-- externs -----------------------------------------------------------------

extern JGMOD *of;
extern volatile MUSIC_INFO mi;
extern volatile int voice_table[];
extern volatile CHANNEL_INFO ci[32];
extern volatile int mod_volume;
extern int fast_loading;
extern int enable_m15;


//-- Prototypes --------------------------------------------------------------
int install_mod(int no_voices);
int remove_mod(void);
JGMOD *load_mod (char *filename);
void mod_interrupt (void);
void play_mod (JGMOD *j, int loop);
void next_mod_track (void);
void prev_mod_track (void);
void goto_mod_track (int new_track);
void stop_mod (void);
int is_mod_playing (void);
void pause_mod (void);
void resume_mod (void);
int is_mod_paused (void);
void destroy_mod (JGMOD *j);
void set_mod_volume (int volume);
int get_mod_volume (void);

#ifdef __cplusplus
}
#endif

#endif  // for JGMOD_H
