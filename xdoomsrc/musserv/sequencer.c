/*************************************************************************
 *  sequencer.c
 *
 *  Copyright (C) 1995-1997 Michael Heasley (mheasley@hmc.edu)
 *
 *  Portions Copyright (c) 1997 Takashi Iwai (iwai@dragon.mm.t.u-tokyo.ac.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include "musserver.h"


struct synth_info sinfo[5];
struct midi_info minfo[5];
int seqfd;
int mixfd;
int use_dev = -1;
int seq_dev = -1;
int synth_patches[16];
int chanvol[16];
int volscale = 100;
unsigned int voxdate;
struct synth_voice *voices;
struct opl_instr fm_instruments[175];
struct sbi_instrument fm_sbi[175];
extern int verbose;

extern void cleanup(int status);
extern void reset_midi();
extern void read_genmidi(FILE *wadfile);

struct opl_instr fm_instruments[175];
struct sbi_instrument fm_sbi[175];


SEQ_USE_EXTBUF();
SEQ_DEFINEBUF(2048);

void seqbuf_dump(void)
{
  if (_seqbufptr)
  if (write(seqfd, _seqbuf, _seqbufptr) == -1)
    {
     perror("write /dev/sequencer");
     cleanup(-1);
     }
  _seqbufptr = 0;
}


int find_midi(int m_type, int dev_num)
{
int num_midi;
int use_midi = -1;
int m = 0;

  if (ioctl(seqfd, SNDCTL_SEQ_NRMIDIS, &num_midi) == -1)
    return 0;
  for (m = 0; m < num_midi; m++)
    {
    minfo[m].device = m;
    ioctl(seqfd, SNDCTL_MIDI_INFO, &minfo[m]);
    use_midi = m;
    if (minfo[m].dev_type == dev_num)
      break;
    }
  if (m_type == NO_SYNTH)
    return num_midi;
  else
    return use_midi;
}

int find_synth(int s_type, int dev_num)
{
int num = 0;
int fm = -1;
#ifdef AWE32_SYNTH_SUPPORT
int awe = -1;
#endif
int s = 0;

  if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &num) == -1)
    return 0;
  else
    for (s = 0; s < num; s++)
      {
      sinfo[s].device = s;
      ioctl(seqfd, SNDCTL_SYNTH_INFO, &sinfo[s]);
      if (sinfo[s].synth_type == SYNTH_TYPE_SAMPLE)
        {
#ifdef AWE32_SYNTH_SUPPORT
        if (sinfo[s].synth_subtype == SAMPLE_TYPE_AWE32)
          {
          awe = s;
	  if ((dev_num == SYNTH_TYPE_SAMPLE) && (s_type == AWE32_SYNTH))
	    break;
	  }
#endif
	}
      else
	{
        fm = s;
        if ((sinfo[s].synth_type == dev_num) && (s_type == FM_SYNTH))
          break;
        }
      }
  switch(s_type)
    {
    case FM_SYNTH:
      return fm;
      break;
#ifdef AWE32_SYNTH_SUPPORT
    case AWE32_SYNTH:
      return awe;
      break;
#endif
    case NO_SYNTH:
      return num;
      break;
    default:
      return -1;
      break;
    }
}

#ifdef AWE32_SYNTH_SUPPORT
void setup_awe(int num_awe)
{
  use_dev = AWE32_SYNTH;
  seq_dev = num_awe;
  if (verbose)
    printf("Using synth device number %d (%s)\n", seq_dev+1, sinfo[seq_dev].name);
}
#endif

void setup_midi(int num_midi)
{
  use_dev = EXT_MIDI;
  seq_dev = num_midi;
  if (verbose)
    printf("Using midi device number %d (%s)\n", seq_dev+1, minfo[seq_dev].name);
}

void setup_fm(int num_fm)
{
int x;
int y;
FILE *sndstat;
char sndver[100];
char snddate[100];


  use_dev = FM_SYNTH;
  seq_dev = num_fm;

  sndstat = fopen("/dev/sndstat", "r");
  if (sndstat == NULL)
    {
    printf("musserver: could not open /dev/sndstat, exiting.\n");
    exit(1);
    }
  fgets(sndver, 100, sndstat);
  for (x = 0; x < strlen(sndver); x++)
    if (sndver[x] == '-')
      {
      x++;
      for (y = x; y < strlen(sndver); y++)
        if (sndver[y] != ' ')
          snddate[y-x] = sndver[y];
        else
          {
          snddate[y-x] = 0;
          break;
          }
      break;
      }
  voxdate = atoi(snddate);
  fclose(sndstat);

  voices = malloc(sinfo[seq_dev].nr_voices * sizeof(struct synth_voice));
  for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
    {
    voices[x].note = -1;
    voices[x].channel = -1;
    }
  for (x = 0; x < 16; x++)
    synth_patches[x] = -1;

  mixfd = open("/dev/mixer", O_WRONLY, 0);

  if (verbose)
    printf("Using synth device number %d (%s)\n", seq_dev+1, sinfo[seq_dev].name);
}

void list_devs()
{
int dummy;
int x;

  if ((seqfd = open("/dev/sequencer", O_WRONLY, 0)) < 0)
    {
    perror("open /dev/sequencer");
    exit(1);
    }

  dummy = find_midi(NO_SYNTH, NO_SYNTH);
  for (x = 0; x < dummy; x++)
    printf("Found a midi device of type %d (%s)\n", minfo[x].dev_type,
            minfo[x].name);
  dummy = find_synth(NO_SYNTH, NO_SYNTH);
  for (x = 0; x < dummy; x++)
    printf("Found a synth device of type %d (%s)\n", sinfo[x].synth_type,
            sinfo[x].name);
  exit(0);
}

void seq_setup(int pref_dev, int dev_type)
{
int num_midi;
int num_awe;
int num_fm;

  if ((seqfd = open("/dev/sequencer", O_WRONLY, 0)) < 0)
    {
    perror("open /dev/sequencer");
    exit(1);
    }

  num_midi = find_midi(EXT_MIDI, dev_type);
  num_fm = find_synth(FM_SYNTH, dev_type);
  num_awe = find_synth(AWE32_SYNTH, dev_type);

  if ((num_midi == -1) && (num_fm == -1) && (num_awe == -1))
    {
    printf("musserver: no music devices found, exiting.\n");
    exit(1);
    }

  switch(pref_dev) {
    case EXT_MIDI:
      if (num_midi != -1)
	setup_midi(num_midi);
#ifdef AWE32_SYNTH_SUPPORT
      else if (num_awe != -1)
	setup_awe(num_awe);
#endif
      else if (num_fm != -1)
	setup_fm(num_fm);
      break;
    case FM_SYNTH:
      if (num_fm != -1)
	setup_fm(num_fm);
      else if (num_midi != -1)
	setup_midi(num_midi);
#ifdef AWE32_SYNTH_SUPPORT
      else if (num_awe != -1)
	setup_awe(num_awe);
#endif
      break;
#ifdef AWE32_SYNTH_SUPPORT
    case AWE32_SYNTH:
      if (num_awe != -1)
	setup_awe(num_awe);
      else if (num_midi != -1)
	setup_midi(num_midi);
      else if (num_fm != -1)
	setup_fm(num_fm);
      break;
#endif
    default:
      break;
  }
  reset_midi();
}

void cleanup_midi(void)
{
  reset_midi();
  close(seqfd);
  if (use_dev == FM_SYNTH)
    close(mixfd);
}

void reset_midi(void)
{
unsigned int channel;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
    {
    AWE_SET_CHANNEL_MODE(seq_dev, 1);
    AWE_NOTEOFF_ALL(seq_dev);
    for (channel = 0; channel < 16; channel++)
      {
      SEQ_BENDER_RANGE(seq_dev, channel, 200);
      SEQ_BENDER(seq_dev, channel, 0);
      }
    }
  else
#endif
  if (use_dev == EXT_MIDI)
    for (channel = 0; channel < 16; channel++)
      {
      /* all notes off */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + channel);
      SEQ_MIDIOUT(seq_dev, 0x7b);
      SEQ_MIDIOUT(seq_dev, 0);
      /* reset pitch bender */
      SEQ_MIDIOUT(seq_dev, MIDI_PITCH_BEND + channel);
      SEQ_MIDIOUT(seq_dev, 64 >> 7);
      SEQ_MIDIOUT(seq_dev, 64 & 127);
      /* reset volume to 100 */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + channel);
      SEQ_MIDIOUT(seq_dev, CTL_MAIN_VOLUME);
      SEQ_MIDIOUT(seq_dev, volscale);
      chanvol[channel] = 100;
      /* reset pan */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + channel);
      SEQ_MIDIOUT(seq_dev, CTL_PAN);
      SEQ_MIDIOUT(seq_dev, 64);

      }
  else
    for (channel = 0; channel < sinfo[seq_dev].nr_voices; channel++)
      {
        SEQ_STOP_NOTE(seq_dev, channel, voices[channel].note, 64);
        SEQ_BENDER_RANGE(seq_dev, channel, 200);
        voices[channel].note = -1;
        voices[channel].channel = -1;
      }
  SEQ_DUMPBUF();
}

void note_off(int note, int channel, int volume)
{
int x = 0;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    SEQ_STOP_NOTE(seq_dev, channel, note, volume);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    SEQ_MIDIOUT(seq_dev, MIDI_NOTEOFF + channel);
    SEQ_MIDIOUT(seq_dev, note);
    SEQ_MIDIOUT(seq_dev, volume);
    }
  else
    {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
      if ((voices[x].note == note) && (voices[x].channel == channel))
        break;
    if (x != sinfo[seq_dev].nr_voices)
      {
      voices[x].note = -1;
      voices[x].channel = -1;
      SEQ_STOP_NOTE(seq_dev, x, note, volume);
      }
    }
  SEQ_DUMPBUF();
}

void all_notes_off(void)
{
  unsigned int channel;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
    {
    AWE_NOTEOFF_ALL(seq_dev);
    }
  else
#endif
  if (use_dev == EXT_MIDI)
    for (channel = 0; channel < 16; channel++)
      {
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + channel);
      SEQ_MIDIOUT(seq_dev, 0x7b);
      SEQ_MIDIOUT(seq_dev, 0);
      }
  else
    for (channel = 0; channel < sinfo[seq_dev].nr_voices; channel++)
      {
        SEQ_STOP_NOTE(seq_dev, channel, voices[channel].note, 64);
      }
  SEQ_DUMPBUF();
}

void note_on(int note, int channel, int volume)
{
int x = 0;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    SEQ_START_NOTE(seq_dev, channel, note, volume);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    SEQ_MIDIOUT(seq_dev, MIDI_NOTEON + channel);
    SEQ_MIDIOUT(seq_dev, note);
    SEQ_MIDIOUT(seq_dev, volume);
    }
  else
    {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
      if ((voices[x].note == -1) && (voices[x].channel == -1))
        break;
    if (x != sinfo[seq_dev].nr_voices)
      {
      voices[x].note = note;
      voices[x].channel = channel;
      if (channel == 9)         /* drum note */
        {
        if (use_dev == FM_SYNTH)
          {
          SEQ_SET_PATCH(seq_dev, x, note + 93);
          note = fm_instruments[note + 93].note;
          }
        else
          SEQ_SET_PATCH(seq_dev, x, note + 128);
        }
      else
        {
        SEQ_SET_PATCH(seq_dev, x, synth_patches[channel]);
        if ((voxdate != 950728) && (use_dev == FM_SYNTH))
          note = note + 12;
        }
      SEQ_START_NOTE(seq_dev, x, note, volume);
      }
    }
  SEQ_DUMPBUF();
}

void pitch_bend(int channel, signed int value)
{
int x;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    SEQ_BENDER(seq_dev, channel, 128 * value);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    SEQ_MIDIOUT(seq_dev, MIDI_PITCH_BEND + channel);
    SEQ_MIDIOUT(seq_dev, value >> 7);
    SEQ_MIDIOUT(seq_dev, value & 127);
    }
  else
    {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
      if (voices[x].channel == channel)
        {
        SEQ_BENDER_RANGE(seq_dev, x, 200);
        SEQ_BENDER(seq_dev, x, 128*value);
        }
    }
  SEQ_DUMPBUF();
}

void control_change(int controller, int channel, int value)
{
int x;

  if (controller == CTL_MAIN_VOLUME)
    {
    chanvol[channel] = value;
    value = value * volscale / 100;
    }

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    SEQ_CONTROL(seq_dev, channel, controller, value);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + channel);
    SEQ_MIDIOUT(seq_dev, controller);
    SEQ_MIDIOUT(seq_dev, value);
    }
  else
    {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
      if ((voices[x].channel == channel) && (controller == CTL_MAIN_VOLUME))
          SEQ_MAIN_VOLUME(seq_dev, x, value);
    }
  SEQ_DUMPBUF();
}

void patch_change(int patch, int channel)
{
int x;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    SEQ_SET_PATCH(seq_dev, channel, patch);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    SEQ_MIDIOUT(seq_dev, MIDI_PGM_CHANGE + channel);
    SEQ_MIDIOUT(seq_dev, patch);
    }
  else
    {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
      if (((voices[x].channel == -1) && (voices[x].note == -1)) || (voices[x].channel == channel))
	{
        synth_patches[channel] = patch;
	break;
	}
    }
  SEQ_DUMPBUF();
}

void midi_wait(float time)
{
  ioctl(seqfd, SNDCTL_SEQ_SYNC);
  SEQ_WAIT_TIME((int) time);
  SEQ_DUMPBUF();
}

void midi_timer(int action)
{
  switch (action)
    {
    case 0:
      SEQ_START_TIMER();
      break;
    case 1:
      SEQ_STOP_TIMER();
      break;
    case 2:
      SEQ_CONTINUE_TIMER();
      break;
    }
}

void vol_change(int volume)
{
int x;
int logscale[16] = {0, 25, 40, 50, 58, 65, 70, 75, 79, 83, 86, 90, 93, 95,
                    98, 100};

  volume = (volume < 0 ? 0 : (volume > 15 ? 15 : volume));
  volscale = logscale[volume];
#ifdef AWE32_SYNTH_SUPPORT
  if (use_dev == AWE32_SYNTH)
  {
    for (x = 0; x < 16; x++)
      SEQ_CONTROL(seq_dev, x, CTL_MAIN_VOLUME, chanvol[x] * volscale / 100);
  }
  else
#endif
  if (use_dev == EXT_MIDI)
    {
    for (x = 0; x < 16; x++)
      {
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + x);
      SEQ_MIDIOUT(seq_dev, CTL_MAIN_VOLUME);
      SEQ_MIDIOUT(seq_dev, chanvol[x] * volscale / 100);
      }
    }
  else
    {
    volume = volscale;
    volume |= (volume << 8);
    //if (-1 == ioctl(mixfd, SOUND_MIXER_WRITE_SYNTH, &volume))
    //  perror("volume change");
    ioctl(mixfd, SOUND_MIXER_WRITE_SYNTH, &volume);
    ioctl(mixfd, SOUND_MIXER_WRITE_LINE2, &volume);
    }
  SEQ_DUMPBUF();
}



void fmload(void)
{
int x;

  for (x = 0; x < 175; x++)
    {
    fm_sbi[x].key = FM_PATCH;
    fm_sbi[x].device = seq_dev;
    fm_sbi[x].channel = x;
    fm_sbi[x].operators[0] = fm_instruments[x].patchdata[0];
    fm_sbi[x].operators[1] = fm_instruments[x].patchdata[7];
    fm_sbi[x].operators[2] = fm_instruments[x].patchdata[4] + fm_instruments[x].patchdata[5];
    fm_sbi[x].operators[3] = fm_instruments[x].patchdata[11] + fm_instruments[x].patchdata[12];
    fm_sbi[x].operators[4] = fm_instruments[x].patchdata[1];
    fm_sbi[x].operators[5] = fm_instruments[x].patchdata[8];
    fm_sbi[x].operators[6] = fm_instruments[x].patchdata[2];
    fm_sbi[x].operators[7] = fm_instruments[x].patchdata[9];
    fm_sbi[x].operators[8] = fm_instruments[x].patchdata[3];
    fm_sbi[x].operators[9] = fm_instruments[x].patchdata[10];
    fm_sbi[x].operators[10] = fm_instruments[x].patchdata[6];
    fm_sbi[x].operators[11] = fm_instruments[x].patchdata[16];
    fm_sbi[x].operators[12] = fm_instruments[x].patchdata[23];
    fm_sbi[x].operators[13] = fm_instruments[x].patchdata[20] + fm_instruments[x].patchdata[21];
    fm_sbi[x].operators[14] = fm_instruments[x].patchdata[27] + fm_instruments[x].patchdata[28];
    fm_sbi[x].operators[15] = fm_instruments[x].patchdata[17];
    fm_sbi[x].operators[16] = fm_instruments[x].patchdata[24];
    fm_sbi[x].operators[17] = fm_instruments[x].patchdata[18];
    fm_sbi[x].operators[18] = fm_instruments[x].patchdata[25];
    fm_sbi[x].operators[19] = fm_instruments[x].patchdata[19];
    fm_sbi[x].operators[20] = fm_instruments[x].patchdata[26];
    fm_sbi[x].operators[21] = fm_instruments[x].patchdata[22];
    SEQ_WRPATCH(&fm_sbi[x], sizeof(fm_sbi[x]));
    }
}
