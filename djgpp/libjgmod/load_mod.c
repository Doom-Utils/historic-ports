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
 *  Protracker 15 and 31 instruments loader. */

#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "jgmod.h"


typedef struct MODTYPE{    /* struct to identify type of module */
        char    id[5];
        char   no_channel;
} MODTYPE;

static MODTYPE modtypes[] = {
    {"M.K.", 4},     // protracker 4 channel
    {"M!K!", 4},     // protracker 4 channel
    {"FLT4", 4},     // startracker 4 channel
    {"1CHN", 1},     // fasttracker 1 channel
    {"2CHN", 2},     // fasttracker 2 channel
    {"3CHN", 3},     // fasttracker 3 channel 
    {"4CHN", 4},     // fasttracker 4 channel
    {"5CHN", 5},     // fasttracker 5 channel
    {"6CHN", 6},     // fasttracker 6 channel
    {"4CHN", 7},     // fasttracker 7 channel 
    {"8CHN", 8},     // fasttracker 8 channel
    {"9CHN", 9},     // fasttracker 9 channel 
    {"10CH", 10},    // fasttracker 10 channel
    {"11CH", 11},    // fasttracker 11 channel 
    {"12CH", 12},    // fasttracker 12 channel
    {"13CH", 13},    // fasttracker 13 channel
    {"14CH", 14},    // fasttracker 14 channel
    {"15CH", 15},    // fasttracker 15 channel
    {"16CH", 16},    // fasttracker 16 channel
    {"17CH", 17},    // fasttracker 17 channel
    {"18CH", 18},    // fasttracker 18 channel
    {"19CH", 19},    // fasttracker 19 channel
    {"20CH", 20},    // fasttracker 20 channel
    {"21CH", 21},    // fasttracker 21 channel
    {"22CH", 22},    // fasttracker 22 channel
    {"23CH", 23},    // fasttracker 23 channel 
    {"24CH", 24},    // fasttracker 24 channel
    {"25CH", 25},    // fasttracker 25 channel
    {"26CH", 26},    // fasttracker 26 channel
    {"27CH", 27},    // fasttracker 27 channel
    {"28CH", 28},    // fasttracker 28 channel
    {"29CH", 29},    // fasttracker 29 channel
    {"30CH", 30},    // fasttracker 30 channel
    {"31CH", 31},    // fasttracker 31 channel
    {"32CH", 32},    // fasttracker 32 channel
    {"CD81", 8},     // atari oktalyzer 8 channel
    {"OKTA", 8},     // atari oktalyzer 8 channel
    {"16CN", 16},     // taketracker 16 channel
    {"32CN", 32}     // taketracker 32 channel
};

volatile int mod_finetune[]=
    {
    8363,   8413,   8463,   8529,   8581,   8651,   8723,   8757,
    7895,   7941,   7985,   8046,   8107,   8169,   8232,   8280
};

// -- Prototypes -------------------------------------------------------------
void pack_skip (PACKFILE *f, int skip);
int detect_m31 (char *filename);
int detect_m15 (char *filename);
int detect_xm (char *filename);
JGMOD *load_m (char *filename, int no_inst);
JGMOD *load_xm (char *filename);
int get_mod_no_pat (int *table, int max_trk);
void lock_mod (JGMOD *j);

//-- Codes -------------------------------------------------------------------

//To detect protracker with 31 instruments
int detect_m31 (char *filename)
{
    PACKFILE *f;
    char id[4];
    int index;
    
    f = pack_fopen (filename, "r");
    if (f== NULL)
        return -1;

    pack_fseek (f, 1080);
    pack_fread (id, 4, f);
    pack_fclose (f);

    for (index=0; index<40; index++)
        if (memcmp (id, modtypes[index].id, 4) == 0)
            return 1;

    return -1;
}

// Load protracker 15 or 31 instruments. no_inst is used for
// determining no of instruments.
JGMOD *load_m (char *filename, int no_inst)
{
    PACKFILE *f;
    JGMOD *j;
    PATTERN_INFO *pi;
    SAMPLE_INFO *si;
    NOTE_INFO *ni;
    SAMPLE *s;
    char *data;
    int index;
    int counter;
    int temp;
    char id[4];

    if (no_inst != 15 && no_inst != 31)
        return NULL;

    j = malloc ( sizeof (JGMOD ));
    if (j == NULL)
        return NULL;

    memset (j, 0, sizeof (JGMOD));
    j->si = malloc (sizeof (SAMPLE_INFO) * no_inst);    
    if (j->si == NULL)
        {
        destroy_mod (j);
        return NULL;
        }

    memset (j->si, 0, sizeof (SAMPLE_INFO) * no_inst);
    j->s = malloc (sizeof (SAMPLE) * no_inst);
    if (j->s == NULL)
        {
        destroy_mod (j);
        return NULL;
        }

    memset (j->s, 0, sizeof (SAMPLE) * no_inst);
    j->no_sample = no_inst;
    j->global_volume = 64;
    j->tempo = 6;
    j->bpm = 125;
    for (index=0; index<32; index++)            //set the panning position
        {
        if ( (index%4) == 0 || (index%4) == 3)
            *(j->panning + index) = 0;
        else
            *(j->panning + index) = 255;
        }


    f = pack_fopen (filename, "r");
    if (f == NULL)
        {
        destroy_mod (j);
        return NULL;
        }

    pack_fread (j->name, 20, f);        //get the song name
    for (index=0; index<no_inst; index++)    //get the sample info
        {
        si = j->si + index;

        pack_skip (f, 22);
        si->lenght = pack_mgetw (f);
        si->c2spd = pack_getc(f);   //get finetune and change to c2spd
        si->volume = pack_getc(f);
        si->repoff = pack_mgetw (f) * 2;
        si->replen = pack_mgetw (f);

        si->c2spd = mod_finetune[si->c2spd];

        if (si->lenght == 1)
            si->lenght = 0;
        else
            si->lenght *= 2;

        if (si->replen == 1)
            si->replen = 0;
        else
            si->replen *= 2;
        }

    j->no_trk = pack_getc(f);       // get no of track
    j->restart_pos = pack_getc(f);  // restart position

    for (index=0; index < 128; index++)
        *(j->pat_table + index) = pack_getc(f);
    
    j->no_pat = get_mod_no_pat (j->pat_table, 128);

    if (no_inst == 31)
        {
        pack_fread (id, 4, f);              // get the id
        for (index=0; index<40; index++)    // get no of channels
            {
            if (memcmp (id, modtypes[index].id, 4) == 0)
                break;
            }
        j->no_chn = modtypes[index].no_channel;
        }
    else
        j->no_chn = 4;

    j->pi = malloc (j->no_pat * sizeof(PATTERN_INFO));
    if (j->pi == NULL)
        {
        pack_fclose (f);
        destroy_mod(j);
        return NULL;
        }

    // allocate patterns;
    for (index=0; index<j->no_pat; index++)
        {
        pi = j->pi+index;
        pi->ni = malloc (sizeof(NOTE_INFO) * 64 * j->no_chn);
        if (pi->ni == NULL)
            {
            pack_fclose (f);
            destroy_mod (j);
            return NULL;
            }
        }


    for (index=0; index<j->no_pat; index++)
        {
        pi = j->pi + index;
        pi->no_pos = 64;
        }

    // load notes
    for (counter=0; counter<j->no_pat; counter++)
        {
        pi = j->pi+counter;
        ni = pi->ni;
        
        for (index=0; index<(64 * j->no_chn); index++)
            {
            temp = pack_mgetl (f);
            ni->sample = ((temp >> 24) & 0xF0) + ((temp >> 12) & 0xF);
            ni->pitch = (temp >> 16) & 0xFFF;
            ni->command = (temp >> 8) & 0xF;
            ni->extcommand = temp & 0xFF;
            ni->volume = 0;

            if (ni->pitch)
                ni->pitch = NTSC / ni->pitch;   //change to hz

            ni++;
            }
        }

    // load the instrument 
    for (index=0; index<no_inst; index++)
        {
        s  = j->s + index;
        si = j->si +index;

        s->bits         = 8;
        //s->stereo       = 0;
        s->freq         = 1000;
        s->priority     = 255;
        s->len          = si->lenght;
        s->param        = -1;
        s->data         = malloc (s->len);

        if (s->len)
            if (s->data == NULL)
                {
                pack_fclose (f);
                destroy_mod (j);
                return NULL;
                }

        if (si->replen > 0)         //sample does loop
            {
            si->loop = LOOP_ON;
            s->loop_start   = si->repoff;
            s->loop_end     = si->repoff + si->replen;
            }
        else
            {
            si->loop = LOOP_OFF;
            s->loop_start   = 0;
            s->loop_end     = si->lenght;
            }

        pack_fread (s->data, s->len, f);
        for (temp=0; temp< (signed)(s->len); temp++)
            {
            data = (char *)s->data;
            data[temp] = data[temp] ^ 0x80;
            }
        }


    // process the restart position stuff
    if (j->restart_pos > j->no_trk)
        j->restart_pos = 0;

    pack_fclose (f);
    lock_mod (j);
    return j;
}

// to detect protracker with 15 instruments.
// not very reliable
int detect_m15 (char *filename)
{
    PACKFILE *f;
    int index;
    int temp;

    f = pack_fopen (filename, "r");
    if (f == NULL)
        return NULL;

    pack_skip (f, 20);  //skip the name of the music;

    for (index=0; index<15; index++)
        {
        pack_skip (f, 24);      //skip sample name and sample length
        temp = pack_getc (f);   //get sample finetune
        if (temp != 0)          //finetune should be 0
            {
            pack_fclose (f);
            return NULL;
            }

        temp = pack_getc(f);    //get sample volume
        if (temp > 64)          //should be <= 64
            {
            pack_fclose (f);
            return NULL;
            }

        pack_skip (f, 4);       //skip sample repeat offset and length
        }

    pack_fclose (f);
    return 1;
}

//to skip no of bytes.
void pack_skip (PACKFILE *f, int skip)
{
    int index;

    for (index=0; index<skip; index++)
        pack_getc(f);
}

// to detect xm files.
int detect_xm (char *filename)
{
    PACKFILE *f;
    char id[17];

    f =  pack_fopen (filename, "r");
    if (f == NULL)
        return NULL;

    pack_fread (id, 17, f);
    pack_fclose (f);

    if (memcmp (id, "Extended Module: ", 4) == 0)    //detect successful
        return 1;

    return -1;
}


// Load the xm file. Very incomplete
JGMOD *load_xm (char *filename)
{
    JGMOD *j;
    PACKFILE *f;
    int unsigned_flag;

    f = pack_fopen (filename, "r");
    if (f == NULL)
        return (NULL);

    j = malloc (sizeof(j));
    if (j == NULL)
        {
        pack_fclose (f);
        return (NULL);
        }

    pack_skip (f, 17);
    pack_fread (j->name, 20, f);        // read the music name
    *(j->name + 20) = '\0';             // make sure it is padded with 0
    pack_getc(f);
    pack_skip (f, 20);                  // skip the tracker name
    pack_skip (f,2);                    // skip the version number
    pack_igetl(f);                      // skip the header size info
                                        // not sure if it is really important

    j->no_trk = pack_igetw(f);          // get no of track
    pack_igetw(f);                      // skip restart position
    j->no_chn = pack_igetw(f);          // get no of channels
    j->no_pat = pack_igetw(f);          // get no of patterns
    j->no_sample = pack_igetw(f);       // get no of instruments
    unsigned_flag = pack_igetw(f) & 1;  // get the bit 0 only
    j->tempo = pack_igetw(f);           // get tempo
    j->bpm = pack_igetw(f);             // get bpm
    pack_fread (j->pat_table, 256, f);  // get the pattern table




    printf ("Name : %s\n", j->name);
    printf ("No channels : %d\n", j->no_chn);
    printf ("No Tracks : %d  No Patterns : %d\n", j->no_trk, j->no_pat);
    printf ("Tempo : %d  Bpm : %d\n", j->tempo, j->bpm);
    
    printf ("XM is still unsupported. Only MOD and S3M are currently supported\n");
    exit (1);
}

// to detect the no of patterns in protracker files.
int get_mod_no_pat (int *table, int max_trk)
{
    int index;
    int max=0;

    for (index=0; index<max_trk; index++)
        if (table[index] > max)
            max = table[index];

    max++;
    return max;
}

