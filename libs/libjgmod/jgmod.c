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
 *  The player. Just to demonstrate how JGMOD sounds. */


#include <stdio.h>
#include <conio.h>
#include <allegro.h>
#include "jgmod.h"

#define font_color 7

JGMOD *the_mod;


// -- Functions --------------------------------------------------------------
int main(int argc, char **argv)
{
    int index;
    int input_key;

    //setvbuf(stdout, NULL, _IONBF, 0);

    if (argc != 2)
        {
        printf ("JGMOD mod %s player by Guan Foo Wah\n", JGMOD_VERSION_STR);
        printf ("Date : %s\n\n", JGMOD_DATE_STR);
        printf ("Syntax : jgmod filename\n");
        return (1);
        }

    if (exists (argv[1]) == 0)
        {
        printf ("Error : %s not found\n", argv[1]);
        return (1);
        }

    allegro_init();
    i_love_bill = TRUE;
    install_timer();
    install_keyboard();
    text_mode (0);
    
    reserve_voices (32, -1);
    install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL);
    set_volume (255, -1);
    fast_loading = FALSE;
    enable_m15 = TRUE;

    if (install_mod (32) < 0)
        {
        printf ("Error : Unable to allocate 32 voices\n");
        return (1);
        }

    the_mod = load_mod (argv[1]);
    if (the_mod == NULL)
        {
        printf ("Error : Unsupported Mod type\n");
        return (1);
        }

    if (set_gfx_mode (GFX_AUTODETECT, 640, 480, 0,0) < 0)
        {
        printf ("Unable to switch to 640 x 480 256 colors mode");
        return (1);
        }

    textprintf (screen, font, 0,0, font_color,  "Song name   : %s", the_mod->name);
    textprintf (screen, font, 0,12, font_color, "No Channels : %2d", the_mod->no_chn);
    textprintf (screen, font, 0,24, font_color, "No Tracks   : %2d  No Patterns : %2d", the_mod->no_trk, the_mod->no_pat);
    
    play_mod (the_mod, TRUE);
    while (is_mod_playing() == TRUE)
        {
        textprintf (screen, font, 0,36, font_color, "Tempo : %3d  Bpm : %3d ", mi.tempo, mi.bpm);
        textprintf (screen, font, 0,48, font_color, "Global volume : %2d  User volume : %2d ", mi.global_volume, get_mod_volume());

        textprintf (screen, font, 0,70, font_color, "%03d-%02d: ", mi.trk, mi.pos);

        for (index=0; index<(mi.no_chn); index++)
            {
            if (index >= mi.max_chn)
                continue;

            if (voice_get_position(voice_table[index]) >= 0 && ci[index].volume >= 1)
                textprintf (screen, font, 0,82+index*12, font_color, "%2d: %2d %2d %6dHz %3d  ", index+1, ci[index].sample+1, ci[index].volume,  voice_get_frequency(voice_table[index]), ci[index].pan);
            else
                textprintf (screen, font, 0,82+index*12, font_color, "%2d: %2s %2s %6sHz %3s  ", index+1, "--", "--",  " -----", "---"); 
            }

        //get the keyboard inputs
        if (keypressed())
            {
            input_key = readkey();

            if      ( (input_key >> 8) == KEY_LEFT )
                prev_mod_track();
            else if ( (input_key >> 8) == KEY_RIGHT )
                next_mod_track();
            else if ( (input_key >> 8) == KEY_PLUS_PAD || (input_key >> 8) == KEY_EQUALS )
                set_mod_volume ( get_mod_volume() + 5);
            else if ( (input_key >> 8) == KEY_MINUS_PAD || (input_key >> 8) == KEY_MINUS )
                set_mod_volume ( get_mod_volume() - 5);
            else if ( (input_key >> 8) == KEY_P)
                {
                if (is_mod_paused() == TRUE)
                    resume_mod();
                else
                    pause_mod();
                }
            else if ( (input_key >> 8) == KEY_ESC || (input_key >> 8) == KEY_SPACE )
                {
                stop_mod();
                destroy_mod (the_mod);
                break;
                }

            }
        } 

    return 0;
}
