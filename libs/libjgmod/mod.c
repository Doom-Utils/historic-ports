/*   
 *
 *                _______  _______  __________  _______  _____ 
 *               /____  / / _____/ /         / / ___  / / ___ \
 *               __  / / / / ____ / //   // / / /  / / / /  / /
 *             /  /_/ / / /__/ / / / /_/ / / / /__/ / / /__/ /
 *            /______/ /______/ /_/     /_/ /______/ /_____/
 *
 *
 *
 *  Guan Foo Wah
 *  18, SS 17 / 1H
 *  47500 Subang Jaya 
 *  Selangor
 *  Malaysia
 *
 *  Email : jgfw@usa.net
 *
 *  Most of the user functions are located here. */


#include <allegro.h>
#include "jgmod.h"


int detect_m31 (char *filename);
int detect_m15 (char *filename);
int detect_s3m (char *filename);
int detect_xm (char *filename);
JGMOD *load_m (char *filename, int no_inst);
JGMOD *load_s3m (char *filename);
JGMOD *load_xm (char *filename);
int calc_volume (int volume);
void lock_jgmod_player (void);
void lock_jgmod_mod (void);
void lock_mod (JGMOD *j);
void _unlock_dpmi_data(void *addr, int size);

static SAMPLE *fake_sample = NULL;
static int mod_init=0;

int enable_m15 = FALSE;

int install_mod(int max_chn)
{
    static int unlocked = 1;

    if (mod_init != 0)      // don't need to initialize many times
        return 1;

    if ( (max_chn > 32) || (max_chn < 0) )
        return -1;

    if (fake_sample == NULL)
        fake_sample = (SAMPLE *)malloc (sizeof (SAMPLE));     // use to trick allegro
                                                    // into giving me voice
    if (fake_sample == NULL)                        // channels
        return -1;

    fake_sample->freq = 1000;
    fake_sample->loop_start = 0;
    fake_sample->loop_end = 1;
    fake_sample->priority = 255;
    fake_sample->stereo = 0;
    fake_sample->bits = 8;
    fake_sample->len  = 1;
    fake_sample->param = -1;
    fake_sample->data = malloc (1);

    if (fake_sample->data == NULL)
        {
        free (fake_sample);
        return -1;
        }

    *((uchar *)fake_sample->data) = 128;
/*
    for (index=0; index<max_chn; index++)    //allocate all the voices
        {
        voice_table[index] = allocate_voice (fake_sample);
        if (voice_table[index] == -1)
            temp = -1;
        else
            voice_start (voice_table[index]);
        }
    
    if (temp == -1)
        {
        for (index=0; index<max_chn; index++)
            if (voice_table[index] != -1)
                deallocate_voice (voice_table[index]);

        return -1;
        }
*/
    mi.max_chn = max_chn;
    mi.is_playing = FALSE;
    mod_init = 1;

    if (unlocked) {
      lock_jgmod_player();    //lock functions and variables located in player.c
      lock_jgmod_mod();       // and mod.c
      unlocked = 0;
    }

    return 1;
}

static inline int get_voices(int number)
{
    int index, temp = 0;
    for (index=number; index-- > 0;)    //allocate all the voices
        {
        voice_table[index] = allocate_voice (fake_sample);
        if (voice_table[index] == -1)
            temp = -1;
        else
            voice_start (voice_table[index]);
        }
    for (index = mi.max_chn; index-- > number; ) {
       if (voice_table[index] != -1) {
	 deallocate_voice(voice_table[index]);
	 voice_table[index] = -1;
       }
    }
    return temp;
}



int remove_mod(void)
{

    if (mod_init != 1)      // don't need to initialize many times
        return 1;

    get_voices(0);

    if (fake_sample->data) free(fake_sample->data);
    if (fake_sample) free(fake_sample);
    fake_sample = NULL;
    mod_init = 0;
    return 1;
}


// load all types of mod files. Detect the type first.
// Then call the appropriate loader.
JGMOD *load_mod (char *filename)
{
    if (detect_xm (filename) == 1)
        return load_xm (filename);

    if (detect_s3m (filename) == 1)
        return load_s3m (filename);

    if (detect_m31 (filename) == 1)
        return load_m (filename, 31);

    if (enable_m15 == TRUE)            //detect this last
        {
        if (detect_m15 (filename) == 1)        
            return load_m (filename, 15);
        }

    return NULL;
}


void play_mod (JGMOD *j, int loop)
{
    int index;

    if (j == NULL)
        return;

    mi.forbid = TRUE;
    mi.is_playing = FALSE;
    mi.trk = 0;

    for (index = mi.max_chn; index--;) {
       if (voice_table[index] != -1) {
	 voice_stop(voice_table[index]);
	 voice_set_volume(voice_table[index], 0);
       }
    }

    of = NULL;
    mi.forbid = FALSE;

    if (get_voices(j->no_chn)) {
      get_voices(0);
      return;
    }

    for (index=0 ;index<32; index++)
        {
        ci[index].sample = 0;
        ci[index].volume = 64;
        ci[index].pitch  = 0;
        ci[index].period = 0;

        ci[index].s3m_volume_slide_on = FALSE;
        ci[index].s3m_volume_slide = 0;
        ci[index].s3m_fine_volume_slide = 0;
        ci[index].pro_volume_slide = 0;

        ci[index].s3m_pitch_slide_on = FALSE;
        ci[index].s3m_pitch_slide = 0;
        ci[index].s3m_fine_pitch_slide = 0;
        ci[index].pro_pitch_slide = 0;


        ci[index].c2spd = 8363;

        ci[index].slide2pitch = 0;
        ci[index].slide2pitch_spd = 0;
        ci[index].retrig = 0;
        ci[index].cut_sample = 0;
        ci[index].delay_sample = 0;
        ci[index].arpeggio = 0;
        ci[index].sample_offset = 0;

        ci[index].vibrato_on = FALSE;
        ci[index].vibrato_shift = 0;
        ci[index].vibrato_spd = 0;
        ci[index].vibrato_depth = 0;
        ci[index].vibrato_pointer =  0;
        ci[index].vibrato_waveform = 0;

        ci[index].loop_on = FALSE;
        ci[index].loop_start = 0;
        ci[index].loop_times = 0;

        ci[index].tremor_on = FALSE;
        ci[index].tremor_count = 0;
        ci[index].tremor_set = 0;

        if (index < mi.max_chn)
            ci[index].pan = *(j->panning + index);
        }

    mi.no_chn = j->no_chn;
    mi.tick = 0;
    mi.pos = 0;
    mi.pat = *(j->pat_table);
    mi.trk = 0;

    mi.bpm = j->bpm;
    mi.tempo = j->tempo;
    mi.global_volume = j->global_volume;

    mi.new_pos = 0;
    mi.new_trk = 0;
    mi.pattern_delay = 0;
    mi.pause   = FALSE;
    mi.forbid  = FALSE;
    mi.is_playing = TRUE;

    if (loop == FALSE)
        mi.loop = FALSE;
    else
        mi.loop = TRUE;

    of = j;

    remove_int (mod_interrupt);
    install_int_ex (mod_interrupt, BPM_TO_TIMER (24 * j->bpm));
}END_OF_FUNCTION (play_mod)

void stop_mod (void)
{

    if (of == NULL)
        return;

    mi.forbid = TRUE;
    mi.is_playing = FALSE;
    mi.trk = 0;

    get_voices(0);
    /*    for (index = mi.max_chn; index--;) {
       if (voice_table[index] != -1) {
	 voice_stop(voice_table[index]);
	 voice_set_volume(voice_table[index], 0);
       }
    }
*/
    of = NULL;
    mi.forbid = FALSE;
}END_OF_FUNCTION (stop_mod)

void next_mod_track (void)
{
    mi.forbid = TRUE;

    mi.skip_pos = 1;
    mi.skip_trk = mi.trk + 2;

    mi.forbid = FALSE;
}

void prev_mod_track (void)
{
    mi.forbid = TRUE;

    mi.skip_pos = 1;
    mi.skip_trk = mi.trk;

    if (mi.skip_trk < 1)
        mi.skip_trk = 1;

    mi.forbid = FALSE;
}

void goto_mod_track (int new_track)
{
    mi.forbid = TRUE;

    mi.skip_pos = 1;
    mi.skip_trk = new_track+1;
    if (mi.skip_trk < 1)
        mi.skip_trk = 1;

    mi.forbid = FALSE;
}END_OF_FUNCTION (goto_mod_track)


int is_mod_playing (void)
{
    return (mi.is_playing);
}

void pause_mod (void)
{
    int index;

    mi.forbid = TRUE;
    mi.pause = TRUE;
    for (index=0; index<(mi.max_chn); index++)
        voice_stop (voice_table[index]);

    mi.forbid = FALSE;
}

void resume_mod (void)
{
    int index;

    mi.forbid = TRUE;
    mi.pause = FALSE;
    for (index=0; index<(mi.max_chn); index++)
        {
        if (voice_get_position (voice_table[index]) >=0)
            voice_start (voice_table[index]);
        }

    mi.forbid = FALSE;
}

int is_mod_paused (void)
{
    if (is_mod_playing() == FALSE)
        return FALSE;

    return (mi.pause);
}


void destroy_mod (JGMOD *j)
{
    int index;
    PATTERN_INFO *pi;

    if (j == NULL)
        return;

    if (of == j)
        stop_mod();

    if (j->si != NULL)
        {
        _unlock_dpmi_data (j->si, sizeof (SAMPLE_INFO) * j->no_sample);
        free (j->si);
        }

    if (j->pi != NULL)
        {
        for (index=0; index<j->no_pat; index++)
            {
            pi = j->pi+index;
            if (pi->ni != NULL)
                {
                _unlock_dpmi_data (pi->ni, sizeof (NOTE_INFO) * j->no_chn * pi->no_pos);
                free (pi->ni);
                }
            }
        _unlock_dpmi_data (j->pi, sizeof (PATTERN_INFO) * j->no_pat);
        free (j->pi);
        }

    for (index=0; index < j->no_sample; index++)
        {
        if (j->s + index)
            {
            if (j->s[index].data)
                {
                _unlock_dpmi_data (j->s[index].data, j->s[index].len * (j->s[index].bits / 8) );
                free (j->s[index].data);
                }
            }
        }

    _unlock_dpmi_data (j->s, sizeof (SAMPLE) * j->no_sample);
    free (j->s);

    _unlock_dpmi_data (j, sizeof (JGMOD));
    free (j);
    j = NULL;
}


void set_mod_volume (int volume)
{
    int chn;

    if (volume < 0)
        volume = 0;
    else if (volume > 255)
        volume = 255;
        
    mod_volume = volume;

    for (chn=0; chn<mi.max_chn ; chn++)
        voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
}

int get_mod_volume (void)
{
    return mod_volume;
}


//to lock stuff in mod.c
void lock_jgmod_mod(void)
{
    LOCK_FUNCTION (goto_mod_track);
    LOCK_FUNCTION (play_mod);
    LOCK_FUNCTION (stop_mod);
}

void lock_mod (JGMOD *j)
{
    int index;
    PATTERN_INFO *pi;

    //  _go32_dpmi_lock_data(j->ni, sizeof (NOTE_INFO) * 64 * j->no_pat * j->no_chn);


    if (j == NULL)
        return;

    _go32_dpmi_lock_data(j, sizeof(JGMOD));
    _go32_dpmi_lock_data(j->si, sizeof(SAMPLE_INFO) * j->no_sample);
    _go32_dpmi_lock_data(j->pi, sizeof(PATTERN_INFO) * j->no_pat);

    for (index=0; index<j->no_pat; index++)
        {
        pi = j->pi + index;
        _go32_dpmi_lock_data(pi->ni, sizeof(NOTE_INFO) * pi->no_pos * j->no_chn);
        }
    
    for (index=0; index < j->no_sample; index++)
        lock_sample (j->s + index);
}

extern void _unlock_dpmi_data(void *address, int size); // Found in Allegro
void unlock_mod (JGMOD *j)
{
    int index;
    PATTERN_INFO *pi;

    //  _go32_dpmi_lock_data(j->ni, sizeof (NOTE_INFO) * 64 * j->no_pat * j->no_chn);


    if (j == NULL)
        return;

    _unlock_dpmi_data(j, sizeof(JGMOD));
    _unlock_dpmi_data(j->si, sizeof(SAMPLE_INFO) * j->no_sample);
    _unlock_dpmi_data(j->pi, sizeof(PATTERN_INFO) * j->no_pat);

    for (index=0; index<j->no_pat; index++)
        {
        pi = j->pi + index;
        _unlock_dpmi_data(pi->ni, sizeof(NOTE_INFO) * pi->no_pos * j->no_chn);
        }
    
}
