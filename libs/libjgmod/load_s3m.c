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
 *  S3M loader. */


#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "jgmod.h"

//#define debug

int fast_loading=TRUE;


int detect_s3m (char *filename);
JGMOD *load_s3m (char *filename);
void pack_skip (PACKFILE *f, int skip);
void jgmod_fseek (PACKFILE *f, char *filename, int offset);
void S3M_get_num_chn(PACKFILE *f);
void S3M_load_pat(PACKFILE *f, JGMOD *j, NOTE_INFO *n, int no_chn);
void convert_s3m_command (int *command, int *extcommand);
void convert_s3m_pitch (int *pitch);
int get_mod_no_pat (int *table, int max_trk);
void lock_mod (JGMOD *j);

static uchar chn_set[32];
static char remap[32];

static int s3m_noteperiod[] = {
6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3628
};

// to detect s3m files
int detect_s3m (char *filename)
{
    PACKFILE *f;
    char id[4];

    f = pack_fopen (filename, "r");
    if (f == NULL)
        return NULL;
    
    pack_fseek (f, 0x2c);
    pack_fread (id, 4, f);
    pack_fclose (f);
    
    if (memcmp (id, "SCRM", 4) == 0)    //detect successful
        return 1;

    return -1;      //not a s3m
}

// fseek. Used to seek to file position relative to the beginning of file
void jgmod_fseek (PACKFILE *f, char *filename, int offset)
{
    pack_fclose (f);
    f = pack_fopen (filename, "r");
    pack_fseek (f, offset);
}

// to get the number of channels actually used.
// must seek to the pattern first
void S3M_get_num_chn(PACKFILE *f)
{
    int row=0, flag, ch;

    while (row<64)
        {
        flag = pack_getc (f);

        if (flag)
            {
            ch = flag & 31;
            if (chn_set[ch] < 16)
                remap[ch] = 0;

            if (flag & 32)
                pack_skip (f, 2);

            if (flag & 64)
                pack_getc(f);

            if (flag & 128)
                pack_skip (f, 2);
            }
        else
            row++;
        }
}


// similar to s3m_get_num_chn but load the notes into the jgmod structure
void S3M_load_pat(PACKFILE *f, JGMOD *j, NOTE_INFO *n, int no_chn)
{
    NOTE_INFO dummy;
    NOTE_INFO *ni;
    int temp;

    int row=0, flag, ch;

    while (row<64)
        {
        flag = pack_getc(f);

        if (flag)
            {
            ch = remap[flag & 31];

            if (ch != -1)
                ni = n + (row*no_chn)+ch;
            else
                ni = &dummy;

            if (flag & 32)
                {
                ni->pitch = pack_getc(f);
                ni->sample = pack_getc(f);
                convert_s3m_pitch (&ni->pitch);

                if (ni->sample > j->no_sample)
                    ni->sample = 0;
                }
            
            if (flag & 64)
                {
                temp = pack_getc(f);
                ni->volume = temp + 0x10000L;
                }

            if (flag & 128)
                {
                ni->command = pack_getc(f);
                ni->extcommand = pack_getc(f);
                convert_s3m_command (&ni->command, &ni->extcommand);
                }
            }
        else
            row++;
        }
}


//convert the s3m note table into hz
void convert_s3m_pitch (int *pitch)
{
    int octave;

    if (*pitch == 254)  // note cut
        {
        *pitch = -1;
        return;
        }

    if ( (*pitch % 16) > 11)
        {
        *pitch = 0;
        return;
        }

    octave = *pitch >> 4;    //pitch / 16
    *pitch = s3m_noteperiod[*pitch % 16] >> octave;
    *pitch = NTSC / *pitch;
}


// load a s3m file
JGMOD *load_s3m (char *filename)
{
    PACKFILE *f;
    JGMOD *j;
    int sf;                 //sample format (signed or unsigned)
    int dp;                 //default pan positons
    int *parapointer;       //to store instruments and patterns parapointers.
    int actual_pat;         //store the number of actual patterns.
    int index;
    int temp;
    int pan[32];
    SAMPLE_INFO *si;
    PATTERN_INFO *pi;
    SAMPLE *s=NULL;


    f = pack_fopen (filename, "r");
    if (f == NULL)
        return NULL;

    j = malloc ( sizeof(JGMOD) );
    if (j == NULL)
        {
        pack_fclose (f);
        return NULL;
        }
    
    memset (j, 0, sizeof (JGMOD));
    pack_fread (j->name, 28, f);
    pack_skip(f, 4);
    j->no_trk = pack_igetw (f);
    j->no_sample = pack_igetw (f);
    j->no_pat = pack_igetw (f);

    j->si = malloc (sizeof (SAMPLE_INFO) * j->no_sample);
    j->s  = malloc (sizeof (SAMPLE) * j->no_sample);
    if ( (j->si == NULL) || (j->s == NULL))
        {
        destroy_mod (j);
        pack_fclose(f);
        return NULL;
        }

    memset (j->si, 0, sizeof (SAMPLE_INFO) * j->no_sample);
    memset (j->s, 0, sizeof (SAMPLE) * j->no_sample);

    //skip the flag and tracker info first
    pack_igetw(f);
    pack_igetw(f);
    sf = pack_igetw(f); 
    pack_skip (f, 4);  //skip SCRM
    j->global_volume = pack_getc(f);
    j->tempo = pack_getc(f);
    j->bpm = pack_getc(f);
    pack_getc(f);
    pack_getc(f);
    dp = pack_getc(f);

    pack_skip (f, 10);
    pack_fread (chn_set, 32, f);

    // read the order number
    for (index=0; index< j->no_trk; index++)
        j->pat_table[index] = pack_getc(f);

    parapointer = malloc ( (j->no_sample + j->no_pat) * sizeof (int));
    if (parapointer == NULL)
        {
        destroy_mod (j);
        pack_fclose (f);
        return NULL;
        }

    for (index=0; index< (j->no_sample + j->no_pat); index++)
        parapointer[index] = pack_igetw(f) * 16;

    // load panning table
    if (dp == 252)
        {
        for (index=0; index<32; index++)
            pan[index] = pack_getc(f);
        }

    // load those instruments
    //------------------------------------------------------------------
    for (index=0; index< j->no_sample; index++)
        {
        char id[4];
        int memseg;
        int type;
        uint counter;

        si = j->si + index;
        s =  j->s + index;

        jgmod_fseek (f, filename, parapointer[index]);
        if (pack_getc(f) != 1)               // is not a sample structure
            {
            s->data = malloc(0);
            continue;
            }

        pack_skip (f, 12);          // skip the filename
        memseg = (int)(pack_getc(f)<<16) + (int)pack_getc(f) + (int)(pack_getc(f)<<8);
        memseg = memseg * 16;

        si->lenght = pack_igetl (f);
        si->repoff = pack_igetl (f);
        si->replen = pack_igetl (f);
        si->volume = pack_getc(f);
        pack_getc(f);
        pack_getc(f);               // skip packing type
        type = pack_getc(f);
        si->c2spd  = pack_igetl (f) & 0xFFFF;
        pack_skip (f, 12);
        pack_skip (f, 28);          // skip sample name

        pack_fread (id, 4, f);

        // now load the samples
        jgmod_fseek (f, filename, memseg);

        s->freq = si->c2spd;
        //s->stereo = 0;
        s->len = si->lenght;
        s->priority = 255;
        s->loop_start = si->repoff;
        s->loop_end = si->replen;
        s->param = -1;

        if ( memcmp (id, "SCRS", 4) != 0)   // dont load the samples
            {
            s->data = malloc(0);
            continue;
            }

        if (type&4)
            s->data = malloc (s->len*2);
        else if (!(type&4))
            s->data = malloc (s->len);

        //should check for s->data == NULL
        // but havent done.



        if (type&4)     //sample is 16bit
            {
            short *data;

            s->bits = 16;
            data = (short *)s->data;

            for (counter=0; counter<s->len; counter++)
                data[counter] = pack_igetw (f);

            if (sf == 1)
                for (counter=0; counter<s->len; counter++)
                    data[counter] = data[counter] ^ 0x8000;
            }
        else if (!(type & 4))       //otherwise 8bit
            {
            char *data;

            s->bits = 8;
            data = (char *)s->data;

            pack_fread (s->data, s->len, f);
            if (sf == 1)
                for (counter=0; counter<s->len; counter++)
                    data[counter] = data[counter] ^ 0x80;
            }

        if (type & 1)
            si->loop = LOOP_ON;
        else
            si->loop = LOOP_OFF;
        }


    // detect the no of channels used
    //-------------------------------------------------------------------
    j->no_chn = 0;
    memset (remap, -1, 32*sizeof(char));


    if (fast_loading == TRUE)   // fast detection but less accurate
        {
       for (index=0; index<32; index++)
            {
            if (chn_set[index] < 16)
                remap[index] = 0;
            }
        }
    else                        // slow detection but accurate
        {
        for (index=0; index<j->no_pat; index++)
            {
            jgmod_fseek (f, filename, parapointer[j->no_sample + index] + 2);
            S3M_get_num_chn(f);
            }
        }
 

    for (index=0; index<32; index++)
        if (remap[index] == 0)
            {
            remap[index] = j->no_chn;
            j->no_chn++;
            }


    // get the pannings ------------------------------------------------------
    for (index=0; index<32; index++)
        {
        if ( (chn_set[index] < 16) && (remap[index] != -1) )
            {
            if (chn_set[index] < 8)
                j->panning[(int)remap[index]] = 64;
            else
                j->panning[(int)remap[index]] = 192;
            }
        }
    
    if (dp == 252)
        {
        for (index=0; index<32; index++)
            {
            if ( (pan[index] & 0x20) && (chn_set[index] < 16) && (remap[index] != -1) )
                j->panning[(int)remap[index]] = (pan[index] & 0xf) * 17;
            }
        }

    //rearrange the pattern order
    temp = 0;
    for (index=0; index<j->no_trk; index++)
        {
        j->pat_table[temp] = j->pat_table[index];
        if (j->pat_table[index] < 254)
            temp++;
        }
    j->no_trk = temp;

    actual_pat = get_mod_no_pat (j->pat_table, j->no_trk);


    // -- this section initialize and load all the patterns -----------------
    // allocate patterns
    j->pi = malloc (sizeof(PATTERN_INFO) * actual_pat);
    if (j->pi == NULL)
        {
        free (parapointer);
        destroy_mod (j);
        pack_fclose (f);
        return NULL;
        }

    for (index=0; index<actual_pat; index++)
        {
        pi = j->pi + index;
        pi->no_pos = 64;

        pi->ni = malloc ( sizeof(NOTE_INFO) * 64 * j->no_chn);
        if (pi->ni == NULL)
            {
            free (parapointer);
            destroy_mod (j);
            pack_fclose (f);
            return NULL;
            }

        memset (pi->ni, 0, sizeof(NOTE_INFO) * 64 * j->no_chn );
        }


    // now load all those patterns
    for (index=0; index<actual_pat; index++)
        {
        if (index >= j->no_pat)
            continue;

        pi = j->pi + index;
        jgmod_fseek (f, filename, parapointer[j->no_sample + index] + 2);
        S3M_load_pat(f, j, pi->ni, j->no_chn);
        }
    j->no_pat = actual_pat;


    #ifdef debug
    for (index=0; index<j->no_trk; index++)
        printf ("%2d\n", j->pat_table[index]);

    printf ("\n\nActual pattern : %d", actual_pat);

    for (index=0; index<j->no_pat; index++)
        {
        NOTE_INFO *ni;

        pi = j->pi + index;
        ni = pi->ni;
        
        printf ("\n\nPattern %d\n", index);
        for (temp=0; temp<(64 * j->no_chn); temp++)
            {
            if ( (temp % j->no_chn) == 0 )
                printf ("\n");
                
             printf ("%06d %02d %05d %02d %03d    ", ni->pitch, ni->sample, ni->volume, ni->command, ni->extcommand);
            
            ni++;
            }
        } 
    #endif


    free (parapointer);
    pack_fclose (f);
    lock_mod (j);
    return j;
}


//convert s3m commands to protracker like commands
void convert_s3m_command (int *command, int *extcommand)
{
    int no;

    no = (*extcommand & 0xF0 ) >> 4;

    if (*command == 1)                      // s3m set tempo
        *command = 16;
    else if (*command == 2)                 // pattern jump
        *command = 11;
    else if (*command == 3)                 // pattern break
        *command = 13;
    else if (*command == 4)                 // S3M volume slide
        *command = 17;
    else if (*command == 5)                 // S3M portamento down
        *command = 18;
    else if (*command == 6)                 // S3M portamento up
        *command = 19;
    else if (*command == 7)                 // slide to note
        *command = 3;
    else if (*command == 8)                 // vibrato
        *command = 4;
    else if (*command == 9)                 // s3m tremor
        *command = 20;
    else if (*command == 11)                // vibrato+volume slide 
        *command = 21;
    else if (*command == 12)                // porta to note
        *command = 22;
    else if (*command == 15)                // set sample offset
        *command = 9;
    else if (*command == 20)                // set bpm
        *command = 25;
    else if (*command == 21)                // fine vibrato
        *command = 26;
    else if (*command == 22)                // set global volume
        *command = 27;
    else if (*command == 24)                // set panning
        *command = 8;
    else if(*command == 19 && no == 2)      // set finetune
        {
        *command = 15;
        *extcommand = (*extcommand & 0xF) | 0x50;
        }
    else if (*command == 19 && no == 3)     // set vibrato waveform
        {
        *command = 14;
        *extcommand = (*extcommand & 0xF) | 0x40;
        }
    else if (*command == 19 && no == 8)     // set 16 pan position
        *command = 14;
    else if (*command == 19 && no == 0xA)   // stereo control
        {
        *command = 14;
        *extcommand = (*extcommand & 0xF) | 0x100;
        }
    else if (*command == 19 && no == 0xB)   // pattern loop    
        {
        *command = 14;
        *extcommand = (*extcommand & 0xF) | 0x60;
        }

    else if (*command == 19 && no == 0xC)   // note cut
        *command = 14;
    else if (*command == 19 && no == 0xD)   // note delay
        *command = 14;
    else if (*command == 19 && no == 0xE)   // pattern delay
        *command = 14;
    else
        {
        *command = 0;
        *extcommand = 0;
        }
}
