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
//void pack_skip (PACKFILE *f, int skip);
int detect_m31 (char *data);
int detect_m15 (char *data);
int detect_xm (char *data);
JGMOD *load_m (char *data, int no_inst);
JGMOD *load_xm (char *data);
int get_mod_no_pat (int *table, int max_trk);
void lock_mod (JGMOD *j);

//-- Codes -------------------------------------------------------------------

//To detect protracker with 31 instruments
int detect_m31 (char *data)
{
    int index;
    
    for (index=0; index<40; index++)
        if (memcmp (&data[1080], modtypes[index].id, 4) == 0)
            return 1;

    return -1;
}

// These macros convert big endian to little endian and back
// They will only work on a 486+
// Be careful; don't use SWAPx(foo++)!!!!!!!!!
//#define SWAPS(x) x <<= 8; SWAPL((long) x); x >>= 8
static inline int SWAPL(int x)
{
 __asm__("bswapl %0"
	 : "=r" (x) /* Outputs */
	 : "0" (x)  /* Inputs */
	 );
 return x;
}

static inline short SWAPS(short x)
{
 int y = x & 0xffff;
 y <<= 8;
 __asm__("bswapl %0"
	 : "=r" (y) /* Outputs */
	 : "0" (y)  /* Inputs */
	 );
 y >>= 8;
 return (short) y;
}
// Load protracker 15 or 31 instruments. no_inst is used for
// determining no of instruments.
JGMOD *load_m (char *fdata, int no_inst)
{
    JGMOD *j;
    PATTERN_INFO *pi;
    SAMPLE_INFO *si;
    NOTE_INFO *ni;
    SAMPLE *s;
    char *data;
    int index;
    int counter;
    int temp;
    char *id;

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
            *(j->panning+index) = 0;
        else
            *(j->panning+index) = 255;
        }


    memcpy(j->name, fdata, 20);        //get the song name
    fdata += 20;
    for (index=0; index<no_inst; index++)    //get the sample info
        {
        si = j->si + index;

        fdata += 22;
	temp = *(short *)fdata;
        si->lenght = SWAPS(temp);
        fdata += 2;
        si->c2spd = *fdata;   //get finetune and change to c2spd
        fdata++;
        si->volume = *fdata;
        fdata++;
        si->repoff = SWAPS(*(short *)fdata) * 2;
        fdata += 2;
        si->replen = SWAPS(*(short *)fdata);
        fdata += 2;

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

    j->no_trk = *fdata;       // get no of track
    fdata++;
    j->restart_pos = *fdata;  // restart position
    fdata++;

    for (index=0; index < 128; index++)
        *(j->pat_table + index) = *fdata++;
    
    j->no_pat = get_mod_no_pat (j->pat_table, 128);

    if (no_inst == 31)
        {
        id = fdata;              // get the id
        fdata += 4;
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
            temp = SWAPL(*(long *)fdata);
            fdata += 4;
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
        //s->data         = malloc (s->len);

/*        if (s->len)
            if (s->data == NULL)
                {
                destroy_mod (j);
                return NULL;
                }
*/
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

        s->data = fdata;
        fdata += s->len;
        for (temp=0; temp< (signed)(s->len); temp++)
            {
            data = (char *)s->data;
            data[temp] = data[temp] ^ 0x80;
            }
        }


    // process the restart position stuff
    if (j->restart_pos > j->no_trk)
        j->restart_pos = 0;

    lock_mod (j);
    return j;
}

// to detect protracker with 15 instruments.
// not very reliable
int detect_m15 (char *data)
{
    int index;
    int temp;

    data += 20;  //skip the name of the music;

    for (index=0; index<15; index++)
        {
        data += 24;      //skip sample name and sample length
        temp = *data++;   //get sample finetune
        if (temp != 0)          //finetune should be 0
            {
            return NULL;
            }

        temp = *data++;    //get sample volume
        if (temp > 64)          //should be <= 64
            {
            return NULL;
            }
        data += 4;
        }

    return 1;
}

//to skip no of bytes.
/*
void pack_skip (PACKFILE *f, int skip)
{
    int index;

    for (index=0; index<skip; index++)
        pack_getc(f);
}
*/
// to detect xm files.
int detect_xm (char *data)
{
    if (memcmp (data, "Extended Module: ", 17) == 0)    //detect successful
        return 1;

    return -1;
}


// Load the xm file. Very incomplete
JGMOD *load_xm (char *data)
{
    JGMOD *j;
    int unsigned_flag;

    j = malloc (sizeof(j));
    if (j == NULL)
        {
        return (NULL);
        }

    data += 17;
    memcpy (j->name, data, 20);
    data += 47;                         // read the music name
    j->name[20] = 0;             // make sure it is padded with 0

    j->no_trk = *(short *)data;          // get no of track
    data += 4;                      // skip restart position
    j->no_chn = *(short *)data;          // get no of channels
    data += 2;
    j->no_pat = *(short *)data;          // get no of patterns
    data += 2;
    j->no_sample = *(short *)data;       // get no of instruments
    data += 2;
    unsigned_flag = *(short *)data & 1;  // get the bit 0 only
    data += 2;
    j->tempo = *(short *)data;           // get tempo
    data += 2;
    j->bpm = *(short *)data;             // get bpm
    data += 2;
    memcpy(j->pat_table, data, 256);
    data += 256;  // get the pattern table




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

