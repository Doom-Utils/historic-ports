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
 *  The core of MOD player. */


#include <allegro.h>
#include "jgmod.h"

extern volatile int mod_finetune[];
volatile MUSIC_INFO mi;
volatile CHANNEL_INFO ci[32];
int volatile mod_volume = 255;

JGMOD *of=NULL;
volatile int voice_table[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1 };


static volatile int vib_table[] = {
    0,   24,  49,  74,  97,  120, 141, 161,
    180, 197, 212, 224, 235, 244, 250, 253,
    255, 253, 250, 244, 235, 224, 212, 197,
    180, 161, 141, 120, 97,  74,  49,  24};



// -- Prototypes -------------------------------------------------------------
int calc_volume (int volume);
int calc_hz (int pitch, int c2spd);
NOTE_INFO *get_note (JGMOD *j, int pat, int pos, int chn);
void lock_jgmod_stuff (void);

// change the volume from scale of 64 to 255
int calc_volume (int volume)
{
    int temp;

    if (volume <= 0)
        return 0;

    temp = (volume << 2) - 1;
    temp = temp * mi.global_volume * mod_volume;

    temp = (temp >> 6) / 255;
    return temp;
}END_OF_FUNCTION (calc_volume)



// calculate the hz with c2spd is taken into consideration
int calc_hz (int pitch, int c2spd)
{
    if (pitch)
        return ( pitch * c2spd/ 8363  );
    else
        return 0;
}END_OF_FUNCTION (calc_hz)

// return the position of the note
NOTE_INFO *get_note (JGMOD *j, int pat, int pos, int chn)
{
    PATTERN_INFO *pi;
    NOTE_INFO *ni;

    pi = j->pi + pat;
    ni = pi->ni + (pos*j->no_chn + chn);

    return ni;
}END_OF_FUNCTION (get_note)

// the core of the mod player.
void mod_interrupt (void)
{
    int chn;
    int  sample_no;
    NOTE_INFO *ni=NULL;
    SAMPLE_INFO *si=NULL;
    PATTERN_INFO *pi;


    if (of == NULL)             //return if not playing music
        return;

    if (mi.forbid == TRUE)
        return;

    if (mi.pause == TRUE)       //return if music is paused
        return;

    // prev_pattern() or next_pattern()
    if (mi.skip_pos)
        {
        mi.pos = mi.skip_pos-1;
        mi.trk = mi.skip_trk-1;
        mi.pat = *(of->pat_table + mi.trk);
        mi.tick = 0;

        mi.skip_pos = 0;
        mi.skip_trk = 0;
        mi.new_pos = 0;
        mi.new_trk = 0;

        for (chn=0; chn<mi.max_chn; chn++)
            {
            ci[chn].loop_start = 0;
            ci[chn].loop_times = 0;
            }

        // cut all the samples;
        for (chn=0; chn<mi.max_chn; chn++)
            {
            ci[chn].volume = 0;
            voice_set_volume (voice_table[chn], 0);
            voice_stop (voice_table[chn]);
            }
        }

    if (mi.trk >= of->no_trk)       //check for end of song
        {
        for (chn=0; chn<mi.max_chn; chn++)
            voice_stop (voice_table[chn]);

        if (mi.loop == FALSE)
            {
            of = NULL;
            mi.is_playing = FALSE;
            return;
            }
       else
           {
           play_mod (of, TRUE);
           goto_mod_track (of->restart_pos);
           return;
           }
        }

    if (mi.tick == 0)
        {
        for (chn=0; chn<(of->no_chn); chn++)
            {
            ni = get_note (of, mi.pat, mi.pos, chn);

            if (chn<32)
                ci[chn].loop_on = FALSE;

            // these are global commands. Should not be skipped
            if (ni->command == 11)     //position jump
                {
                mi.new_trk = ni->extcommand + 1;

                if (!mi.new_pos)
                    mi.new_pos = 1;
                }
            else if (ni->command == 13)     //pattern break
                {
                if (!mi.new_trk)
                    mi.new_trk = mi.trk+2;

                if (mi.new_trk > of->no_trk)
                    mi.new_trk -= of->no_trk;

                mi.new_pos = (ni->extcommand >> 4) * 10 + (ni->extcommand & 0xF) + 1;

                pi = of->pi + *(of->pat_table + mi.new_trk-1);
                if ( (mi.new_pos-1) >= pi->no_pos)
                    mi.new_pos -= 1;
                }
            else if (ni->command == 15)     //set tempo or bpm
                {
                if (ni->extcommand == 0)
                    mi.tempo = 1;
                else if (ni->extcommand <= 32)
                    mi.tempo = ni->extcommand;
                else
                    {
                    mi.bpm = ni->extcommand;
                    remove_int (mod_interrupt);
                    install_int_ex (mod_interrupt, BPM_TO_TIMER (mi.bpm * 24));
                    }
                }
            else if (ni->command == 16)     // S3M set tempo
                {
                if (ni->extcommand == 0)
                    mi.tempo = 1;
                else 
                    mi.tempo = ni->extcommand;
                }

            else if (ni->command == 25)     // S3M set bpm
                {
                if (ni->extcommand >= 32)
                    mi.bpm = ni->extcommand;
                else if (ni->extcommand <= 15)
                    mi.bpm += ni->extcommand;
                else if ( (ni->extcommand > 15) && (ni->extcommand < 32) )
                    mi.bpm -= ni->extcommand - 16;

                remove_int (mod_interrupt);
                install_int_ex (mod_interrupt, BPM_TO_TIMER (mi.bpm * 24));
                }
            else if (ni->command == 27)     // set global volume
                {
                int temp;

                mi.global_volume = ni->extcommand;
                for (temp=0; temp<of->no_chn; temp++)
                    {
                    if (temp >= mi.max_chn)
                        continue;

                    voice_set_volume (voice_table[temp], calc_volume(ci[temp].volume));
                    }
                }

            // pattern loop
            else if ( (ni->command == 14) && (ni->extcommand >> 4 == 6 ) )
                {
                if ( (ni->extcommand & 0xF) == 0)
                    ci[chn].loop_start = mi.pos;
                else
                    {
                    if (ci[chn].loop_times > 0)
                        ci[chn].loop_times--;
                    else
                        ci[chn].loop_times = ni->extcommand & 0xF;

                    if (ci[chn].loop_times > 0)
                        ci[chn].loop_on = TRUE;
                    else
                        ci[chn].loop_start = mi.pos+1;

                    }
                }

            // pattern delay
            else if ( (ni->command == 14) &&  (ni->extcommand >> 4 == 14 ) )
                mi.pattern_delay = mi.tempo * (ni->extcommand & 0xF);
            }


        // the following are not global commands. Can be skipped
        for (chn=0; chn<(of->no_chn); chn++)
            {
            if (chn >= mi.max_chn)
                continue;

            ni = get_note (of, mi.pat, mi.pos, chn);

            ci[chn].s3m_volume_slide_on = FALSE;
            ci[chn].s3m_pitch_slide_on = FALSE;
            ci[chn].pro_volume_slide = 0;
            ci[chn].pro_pitch_slide = 0;
            ci[chn].retrig = 0;
            ci[chn].cut_sample = 0;
            ci[chn].delay_sample = 0;
            ci[chn].arpeggio = 0;
            sample_no = ni->sample -1;

            if ( (ni->pitch > 0 || sample_no >= 0) && ni->command != 3 && ni->command != 5)
                {
                ci[chn].tremor_count = 0;

                //check for delay sample
                if ( (ni->command == 14) && (ni->extcommand >> 4) == 13 )
                    ci[chn].delay_sample = (ni->extcommand & 0xF);

                // check for vibrato waveform
                if (ci[chn].vibrato_waveform <= 2)
                    ci[chn].vibrato_pointer = 0;

                if (ni->pitch > 0 && sample_no >= 0)    //pitch and sample specified
                    {
                    si = of->si + sample_no;
                    ci[chn].sample = sample_no;
                    ci[chn].volume = si->volume;
                    ci[chn].c2spd  = si->c2spd;
                    ci[chn].pitch  = calc_hz (ni->pitch, ci[chn].c2spd);

                    }
                else if (ni->pitch > 0 && sample_no < 0)    //only pitch specified
                    {
                    si = of->si + ci[chn].sample;
                    ci[chn].pitch  = calc_hz (ni->pitch, ci[chn].c2spd);
                    }
                else if (ni->pitch == 0 && sample_no>= 0)   //only sample specified
                    {
                    si = of->si + sample_no;
                    ci[chn].sample = sample_no;
                    ci[chn].volume = si->volume;
                    ci[chn].c2spd  = si->c2spd;
                    voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                    }

                if (ci[chn].delay_sample == 0 && (ni->pitch > 0))
                    {
                    reallocate_voice (voice_table[chn], of->s + ci[chn].sample);
                    voice_set_frequency (voice_table[chn], ci[chn].pitch);
                    voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                    voice_set_pan (voice_table[chn], ci[chn].pan);

                    voice_start (voice_table[chn]);

                    if (si->loop == LOOP_ON) 
                        voice_set_playmode (voice_table[chn], PLAYMODE_LOOP);
                    }
                }


            if (ni->command == 0)       //arpeggio. Not supported yet.
                ci[chn].arpeggio = ni->extcommand;

            else if (ni->command == 1)  //period slide up
                {
                if (!ci[chn].pitch)
                    break;

                ci[chn].period = NTSC  / ci[chn].pitch; //get the period
                ci[chn].pro_pitch_slide = -(ni->extcommand);
                }
            else if (ni->command == 2)  //period slide down
                {
                if (!ci[chn].pitch)
                    break;

                ci[chn].period = NTSC  / ci[chn].pitch; //get the period
                ci[chn].pro_pitch_slide = ni->extcommand;
                }
            
            if (ni->command == 3)  //slide to note 
                {
                if (!ci[chn].pitch)
                    {
                    ci[chn].slide2pitch = 0; 
                    break;
                    }

                ci[chn].period = NTSC / ci[chn].pitch;

                if (ni->extcommand > 0)
                    ci[chn].slide2pitch_spd = ni->extcommand;

                if (ni->pitch > 0)
                    {
                    ci[chn].slide2pitch = calc_hz (ni->pitch, ci[chn].c2spd);
                    if (ci[chn].slide2pitch > ci[chn].pitch )
                        ci[chn].slide2pitch_spd = -ABS(ci[chn].slide2pitch_spd);
                    }
                }
            else if (ni->command == 5 || ni->command == 22)     // slide to note + vol slide
                {
                if (!ci[chn].pitch)
                    {
                    ci[chn].slide2pitch = 0; 
                    break;
                    }

                ci[chn].period = NTSC / ci[chn].pitch;

                if (ni->pitch > 0)
                    {
                    ci[chn].slide2pitch = calc_hz (ni->pitch, ci[chn].c2spd);
                    if (ci[chn].slide2pitch > ci[chn].pitch )
                        ci[chn].slide2pitch_spd = -ABS(ci[chn].slide2pitch_spd);
                    }
                }
            else
                ci[chn].slide2pitch = 0; 

            if (ni->command == 4 || ni->command == 26)           // vibrato and fine vibrato
                {
                if (!ci[chn].pitch)
                    break;

                ci[chn].period = (NTSC << 2) / ci[chn].pitch;

                if ( (ni->extcommand >> 4) != 0)
                    ci[chn].vibrato_spd = (ni->extcommand & 0xF0) >> 2;

                if ( (ni->extcommand & 0xF) != 0)
                    ci[chn].vibrato_depth = ni->extcommand & 0xF;

                ci[chn].vibrato_on = TRUE;

                if (ni->extcommand > 11)
                    {
                    if (ni->command == 4)
                        ci[chn].vibrato_shift = 5;
                    else
                        ci[chn].vibrato_shift = 7;
                    }
                }
            else if (ni->command == 6 || ni->command == 21)
                {
                if (!ci[chn].period)
                    break;

                if (!ci[chn].vibrato_shift)
                    break;

                ci[chn].period = (NTSC << 2) / ci[chn].pitch;
                ci[chn].vibrato_on = TRUE;
                }
            else
                {
                voice_set_frequency (voice_table[chn], ci[chn].pitch);
                ci[chn].vibrato_on = FALSE;
                }


            if (ni->command == 8)               // set pan
                {
                if (ni->extcommand == 0)
                    {
                    ci[chn].pan = 0;
                    voice_set_pan (voice_table[chn], ci[chn].pan);
                    }
                else if (ni->extcommand <= 0x80)
                    {
                    ci[chn].pan = (ni->extcommand << 1) - 1;
                    voice_set_pan (voice_table[chn], ci[chn].pan);
                    }
                }

            else if (ni->command == 9)          // set sample offset
                {
                int temp;

                temp = ni->extcommand << 8;
                if (temp == 0)
                    temp = ci[chn].sample_offset;
                else
                    ci[chn].sample_offset = temp;

                voice_set_position (voice_table[chn], temp);
                }

            //volume slide
            else if (ni->command == 10 || ni->command == 5 || ni->command == 6)
                {
                if (ni->extcommand >> 4)
                    ci[chn].pro_volume_slide = ni->extcommand >> 4;
                else if (ni->extcommand & 0xF)
                    ci[chn].pro_volume_slide = -(ni->extcommand & 0xF);
                }
            else if (ni->command == 12)     //set volume
                {
                ci[chn].volume = ni->extcommand;
                if (ci[chn].volume > 64)
                    ci[chn].volume = 64;

                voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                }

            else if (ni->command == 14)
                {
                switch (ni->extcommand >> 4)
                    {
                    case 4 :        // vibrato waveform
                        {
                        ci[chn].vibrato_waveform = (ni->extcommand & 0xF);
                        break;
                        }
                    case 5 :        //set finetune value
                        {
                        ci[chn].pitch = ci[chn].pitch * 8363 / ci[chn].c2spd;
                        ci[chn].c2spd = mod_finetune [ni->extcommand & 0xF];

                        ci[chn].pitch = calc_hz (ci[chn].pitch, ci[chn].c2spd);


                        voice_set_frequency (voice_table[chn], ci[chn].pitch);
                        break;
                        }
                    case 8 :        // 16 position panning
                        {
                        ci[chn].pan = (ni->extcommand & 0xF) * 17;
                        voice_set_pan (voice_table[chn], ci[chn].pan);
                        break;
                        }

                    case 9 :        //retrigger sample
                        {
                        ci[chn].retrig = (ni->extcommand & 0xF);
                        break;
                        }

                    case 10 :       //fine tune volume up
                        {
                        ci[chn].volume += (ni->extcommand & 0xF);
                        if (ci[chn].volume > 64)
                            ci[chn].volume = 64;

                        voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                        break;
                        }
                    case 11 :       //fine tune volume down
                        {
                        ci[chn].volume -= (ni->extcommand & 0xF);
                        if (ci[chn].volume < 0)
                            ci[chn].volume = 0;

                        voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                        break;
                        }

                    case 12 :       //cut sample
                        {
                        ci[chn].cut_sample = (ni->extcommand & 0xF);
                        if (ci[chn].cut_sample == 0)    // cut immediately
                            {
                            ci[chn].volume = 0;
                            voice_set_volume (voice_table[chn], 0);
                            }
                        break;
                        }

                    case 16 :       // stereo control
                        {
                        ci[chn].pan = (ni->extcommand & 0xF);

                        if (ci[chn].pan < 8)
                            ci[chn].pan += 8;
                        else
                            ci[chn].pan -= 8;

                        ci[chn].pan *= 17;
                        voice_set_pan (voice_table[chn], ci[chn].pan);
                        break;
                        }

                    }
                }

            // S3M volume slides
            else if (ni->command == 17 || ni->command == 21 || ni->command == 22 )  
                {
                // fine volume slide down
                if ( (ni->extcommand & 0xF0) == 0xF0  && (ni->extcommand & 0xF) != 0) 
                    {
                    ci[chn].s3m_fine_volume_slide = -(ni->extcommand & 0xF);
                    ci[chn].s3m_volume_slide = 0;
                    ci[chn].s3m_volume_slide_on = TRUE;
                    }
                // fine volume slide up
                else if ( (ni->extcommand & 0xF) == 0xF && (ni->extcommand & 0xF0) != 0)
                    {
                    ci[chn].s3m_fine_volume_slide = ((ni->extcommand & 0xF0) >> 4);
                    ci[chn].s3m_volume_slide = 0;
                    ci[chn].s3m_volume_slide_on = TRUE;
                    }
                // volume slide up
                else if ( (ni->extcommand & 0xF) == 0  && (ni->extcommand & 0xF0) != 0)
                    {
                    ci[chn].s3m_volume_slide = ni->extcommand >> 4;
                    ci[chn].s3m_fine_volume_slide = 0;
                    ci[chn].s3m_volume_slide_on = TRUE;
                    }
                // volume slide down
                else if ( (ni->extcommand & 0xF0) == 0  && (ni->extcommand & 0xF) != 0) 
                    {
                    ci[chn].s3m_volume_slide = -(ni->extcommand & 0xF);
                    ci[chn].s3m_fine_volume_slide = 0;
                    ci[chn].s3m_volume_slide_on = TRUE;
                    }
                else if (ni->extcommand == 0)          // continue volume slide 
                    ci[chn].s3m_volume_slide_on = TRUE;
                }

            else if (ni->command == 18)         // portamento down
                {
                if (ci[chn].pitch == 0)
                    break;

                ci[chn].period = (NTSC << 2) / ci[chn].pitch; //get the period

                if ( (ni->extcommand > 0) && (ni->extcommand <= 0xDF) )
                    {
                    ci[chn].s3m_fine_pitch_slide = 0;
                    ci[chn].s3m_pitch_slide = ni->extcommand * 4;
                    }
                else if ( (ni->extcommand >= 0xE0) && (ni->extcommand <= 0xEF))
                    {
                    ci[chn].s3m_pitch_slide = 0;
                    ci[chn].s3m_fine_pitch_slide = ni->extcommand & 0xF;
                    }
                else if ( (ni->extcommand >= 0xF0) && (ni->extcommand <= 0xFF))
                    {
                    ci[chn].s3m_pitch_slide = 0;
                    ci[chn].s3m_fine_pitch_slide = (ni->extcommand & 0xF) * 4;
                    }
                else if (ni->extcommand == 0)
                    {
                    ci[chn].s3m_pitch_slide = ABS(ci[chn].s3m_pitch_slide);
                    ci[chn].s3m_fine_pitch_slide = ABS(ci[chn].s3m_fine_pitch_slide);
                    }

                ci[chn].s3m_pitch_slide_on = TRUE;
                }
            else if (ni->command == 19)             //portamento up
                {
                if (ci[chn].pitch == 0)
                    break;

                ci[chn].period = (NTSC << 2) / ci[chn].pitch; //get the period

                if ( (ni->extcommand > 0) && (ni->extcommand <= 0xDF) )
                    {
                    ci[chn].s3m_fine_pitch_slide = 0;
                    ci[chn].s3m_pitch_slide = -(ni->extcommand) * 4;
                    }
                else if ( (ni->extcommand >= 0xE0) && (ni->extcommand <= 0xEF) )
                    {
                    ci[chn].s3m_fine_pitch_slide = -(ni->extcommand & 0xF);
                    ci[chn].s3m_pitch_slide = 0;
                    }
                else if ( (ni->extcommand >= 0xF0) && (ni->extcommand <= 0xFF) )
                    {
                    ci[chn].s3m_fine_pitch_slide = -(ni->extcommand & 0xF) * 4;
                    ci[chn].s3m_pitch_slide = 0;
                    }
                else if ( ni->extcommand == 0)
                    {
                    ci[chn].s3m_pitch_slide = -ABS(ci[chn].s3m_pitch_slide);
                    ci[chn].s3m_fine_pitch_slide = -ABS(ci[chn].s3m_fine_pitch_slide);
                    }

                ci[chn].s3m_pitch_slide_on = TRUE;
                }

            if (ni->command == 20)             // s3m tremor
                {
                int on, off;

                on  = (ni->extcommand >> 4) + 1;
                off = (ni->extcommand & 0xF) + 1;

                ci[chn].tremor_set = (on << 8) + off;
                ci[chn].tremor_on = TRUE;
                }
            else
                {
                if (ci[chn].tremor_on == TRUE)
                    {
                    voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                    ci[chn].tremor_on = FALSE;
                    }
                }


            if (ni->volume >= 0x10000L)  //the volume column
                {
                ci[chn].volume = ni->volume - 0x10000L;
                voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                }

            if (ni->pitch == -1)        // note cut;
                {
                ci[chn].volume = 0;
                voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                }

            }                    
        }

    

//-- for effects updated not including tick 0 --------------------------------
    if (mi.tick != 0)
        {
        for (chn=0; chn<(of->no_chn); chn++)
            {
            if (chn >= mi.max_chn)
                continue;

            if (ci[chn].pro_volume_slide)       // volume slide
                {
                ci[chn].volume += ci[chn].pro_volume_slide;
                if (ci[chn].volume > 64)
                    ci[chn].volume = 64;
                else if (ci[chn].volume < 0)
                    ci[chn].volume = 0;
                
                voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                }

            if (ci[chn].pro_pitch_slide)        // pormento slide
                {
                ci[chn].period += ci[chn].pro_pitch_slide;

                if (ci[chn].period <= 0)
                    ci[chn].period = 1;

                ci[chn].pitch = NTSC / ci[chn].period;
                voice_set_frequency (voice_table[chn], ci[chn].pitch);
                }
            
            if (ci[chn].slide2pitch > 0)        // pormento to note
                {
                ci[chn].period += ci[chn].slide2pitch_spd;
                if (ci[chn].period <= 0)
                    ci[chn].period = 1;

                ci[chn].pitch = NTSC / ci[chn].period;

                if (ci[chn].slide2pitch_spd > 0)  //sliding down
                    {
                    if (ci[chn].pitch < ci[chn].slide2pitch)
                        ci[chn].pitch = ci[chn].slide2pitch;
                    }
                else                                //sliding up
                    {
                    if (ci[chn].pitch > ci[chn].slide2pitch)
                        ci[chn].pitch = ci[chn].slide2pitch;
                    }

                if (ci[chn].vibrato_on == FALSE)     // if vibrato, dont slide first
                    voice_set_frequency (voice_table[chn], ci[chn].pitch);
                }

            if (ci[chn].retrig > 0)     // retrigger sample
                {
                if ((mi.tick % ci[chn].retrig) == 0)
                    {
                    si = of->si + ci[chn].sample;
                    reallocate_voice (voice_table[chn], of->s + ci[chn].sample);
                    voice_set_frequency (voice_table[chn], ci[chn].pitch);
                    voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                    voice_set_pan (voice_table[chn], ci[chn].pan);
                    voice_start (voice_table[chn]);
                    if (si->loop == LOOP_ON) 
                        voice_set_playmode (voice_table[chn], PLAYMODE_LOOP);
                    }  
                }

            if (mi.tick == ci[chn].cut_sample)
                {
                ci[chn].volume = 0;
                voice_set_volume (voice_table[chn], 0);
                }

            if (ci[chn].delay_sample == mi.tick)
                {
                si = of->si + ci[chn].sample;
                reallocate_voice (voice_table[chn], of->s + ci[chn].sample);
                voice_set_frequency (voice_table[chn], ci[chn].pitch);
                voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                voice_set_pan (voice_table[chn], ci[chn].pan);
                voice_start (voice_table[chn]);
                if (si->loop == LOOP_ON) 
                    voice_set_playmode (voice_table[chn], PLAYMODE_LOOP);
                }

            if (ci[chn].vibrato_on == TRUE)
                {
                int temp=0;
                int q;

                q = (ci[chn].vibrato_pointer >> 2) & 0x1F;

                // handles vibrato waveform 
                if ( (ci[chn].vibrato_waveform & 3) == 0)
                    temp = vib_table[q];
                else if ( (ci[chn].vibrato_waveform & 3) == 1)
                    {
                    q <<= 3;
                    if (ci[chn].vibrato_pointer < 0)
                        q = 255 - q;
                    temp = q;
                    }
                else if ( (ci[chn].vibrato_waveform & 3) == 2)
                    temp = 255;
                else
                    temp = random () % 255;

                temp *= ci[chn].vibrato_depth;
                temp >>= ci[chn].vibrato_shift;

                if (ci[chn].vibrato_pointer >= 0)
                    temp += ci[chn].period;
                else
                    temp = ci[chn].period - temp;

                if (temp < 40)
                    temp = 40;

                temp = (NTSC << 2) / temp;
                
                voice_set_frequency (voice_table[chn], temp);
                ci[chn].vibrato_pointer += ci[chn].vibrato_spd;
                }

            if (ci[chn].tremor_on == TRUE)
                {
                int on;
                int off;

                on  = (ci[chn].tremor_set >> 8) & 31;
                off = ci[chn].tremor_set & 31;

                ci[chn].tremor_count %= (on+off);

                if (ci[chn].tremor_count < on)
                    voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
                else
                    voice_set_volume (voice_table[chn], 0);

                ci[chn].tremor_count++;
                }
            }
        } 


//-- for effects that is updated including tick 0 ----------------------------
    for (chn=0; chn<(of->no_chn); chn++)
        {
        if (chn >= mi.max_chn)
            continue;

        if (ci[chn].s3m_volume_slide_on == TRUE)
            {
            if (mi.tick == 0)   //finetunes
                ci[chn].volume += ci[chn].s3m_fine_volume_slide;
            else                // volume slide
                ci[chn].volume += ci[chn].s3m_volume_slide;

            if (ci[chn].volume < 0)
                ci[chn].volume = 0;
            else if (ci[chn].volume > 64)
                ci[chn].volume = 64;

            voice_set_volume (voice_table[chn], calc_volume(ci[chn].volume));
            }

        if (ci[chn].s3m_pitch_slide_on == TRUE)
            {
            if (mi.tick == 0)
                ci[chn].period += ci[chn].s3m_fine_pitch_slide;
            else
                ci[chn].period += ci[chn].s3m_pitch_slide;

            if (ci[chn].period <= 40)
                ci[chn].period = 40;

            ci[chn].pitch = (NTSC << 2) / ci[chn].period;
            voice_set_frequency (voice_table[chn], ci[chn].pitch);
            }
        }


    mi.tick++;
    if (mi.tick >= mi.tempo)
        if (mi.pattern_delay > 0)
            {
            mi.pattern_delay--;
            return;
            }

    if (mi.tick >= mi.tempo)
        {
        mi.tick = 0;


        // those darn pattern loop
        for (chn=0; chn<(of->no_chn); chn++)
            {
            if (chn >= mi.max_chn)
                continue;

            if (ci[chn].loop_on == FALSE)
                continue;

            if (ci[chn].loop_times > 0)
                {
                mi.new_pos = ci[chn].loop_start+1;
                mi.new_trk = mi.trk+1;
                }
            }

        if (mi.new_pos)     //if pattern break or position jump
            {
            mi.pos = mi.new_pos-1;
            mi.trk = mi.new_trk-1;
            mi.pat = *(of->pat_table + mi.trk);

            mi.new_pos = 0;
            mi.new_trk = 0;
            }
        else
            {
            mi.pos++;

            pi = of->pi + mi.pat;
            if (mi.pos >= pi->no_pos)
                {
                mi.pos = 0;
                mi.trk++;
                mi.pat = *(of->pat_table + mi.trk);

                for (chn=0; chn<mi.max_chn; chn++)
                    {
                    ci[chn].loop_start = 0;
                    ci[chn].loop_times = 0;
                    }
                }

            }
        }
}

END_OF_FUNCTION (mod_interrupt)

// to lock stuff in player.c
void lock_jgmod_player (void)
{
    LOCK_FUNCTION(mod_interrupt);
    LOCK_FUNCTION(get_note);
    LOCK_FUNCTION(calc_volume);
    LOCK_FUNCTION(calc_hz);

    _go32_dpmi_lock_data((void *)mod_finetune, sizeof(int) * 16);
    _go32_dpmi_lock_data((void *)ci, sizeof(ci) * 32);
    _go32_dpmi_lock_data((void *)voice_table, sizeof(voice_table) * 32);
    _go32_dpmi_lock_data((void *)vib_table, sizeof(vib_table) * 24);

    LOCK_VARIABLE(mi);
    LOCK_VARIABLE(mod_volume);
    LOCK_VARIABLE (of);
}
