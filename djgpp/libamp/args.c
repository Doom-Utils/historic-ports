/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
/* args.c  created by Andrew Richards  <A.Richards@phys.canterbury.ac.nz>
*/

#include "amp.h"
#include "audio.h"

#ifdef __BEOS__
#define __STDC__               1
#include <getopt.h>
#else
#include "getopt.h"
#endif

#include <stdio.h>
#include <stdlib.h>


void
displayDisclaimer()
{
	/* print a warm, friendly message
	 */
	msg("\namp %d.%d.%d, (C) Tomislav Uzelac 1996,1997\n",MAJOR,MINOR,PATCH);
	msg("THIS PROGRAM COMES WITH ABSOLUTELY NO WARRANTY\n");
	msg("PLEASE READ THE DOCUMENTATION FOR DETAILS\n\n");
}

void
displayUsage()
{
	int idx;

	/* Losts of printfs for stupid compilers that can't handle literal newlines */
	/* in strings */

	printf("\
usage: amp [options] [ MPEG audio streams... ]\n\
       amp -convert [ MPEG-audio stream ] [ output file ]\n\n\
  -h, -help           display this usage information and exit\n\
  -v, -version        display the version information and exit\n\
  -c, -convert        convert the MPEG audio stream to output file format\n\
  -p, -play           play the specified MPEG audio streams (default action)\n\
  -q, -quiet          supress printing of messages to STDERR\n\
  -b, -buffer <size>  set the audio buffer size to <size> k\n\
  -d, -dump           dump binary data to STDERR\n\
  -s, -frame          show a frame counter\n\
  -t, -time           show time\n\
  -g, -gui            output messages to stdout instead of stderr\n\
                      (for use with xmpeg3 or similar GUIs)\n\
  -w                  wav output\n\
      -downmix        downmix stereo streams to one channel\n\
      -nobuffer       do not use an audio buffer\n\
      -volume <vol>   set the volume to <vol> (0-100)\n\
      -debug <opts..> When compiled in debug <opt, opt2,...> code sections\n\
                      Options: ");

	for(idx=0;debugLookup[idx].name!=0;idx++)
		printf("%s,",debugLookup[idx].name);
	printf("\010 \n");

	exit(0);
}

void
displayVersion()
{
	printf("amp - %d.%d.%d\n",MAJOR,MINOR,PATCH);
	exit(0);
}



int
argVal(char *name, char *num, int min, int max)
{
	int val;
	
	val=atoi(num);
	if ((val<min) || (val>max))
		die("%s parameter %d - out of range (%d-%d)\n",name,val,min,max);
	return(val);
}


int
args(int argc,char **argv)
{
	int c;

	static int showusage=0, showversion=0;

	AUDIO_BUFFER_SIZE=300*1024;
	A_DUMP_BINARY=FALSE;
	A_QUIET=FALSE;
	A_FORMAT_WAVE=FALSE;
	A_SHOW_CNT=FALSE;
	A_SET_VOLUME=-1;
	A_SHOW_TIME=0;
	A_AUDIO_PLAY=TRUE;
	A_WRITE_TO_FILE=FALSE;
	A_MSG_STDOUT=FALSE;
	A_DOWNMIX=FALSE;

	while (1) {
		static struct option long_options[] =	{
			{"help", no_argument, 0, 'h'},
			{"debug", required_argument, 0, 1},
			{"version", no_argument, 0, 'v'},
			{"quiet", no_argument, 0, 'q'},
			{"play", no_argument, 0, 'p'},
			{"convert", no_argument, 0, 'c'},
			{"volume", required_argument, 0, 2},
			{"time", no_argument, 0, 't'},
			{"frame", no_argument, 0, 's'},
			{"gui", no_argument, 0, 'g'},
			{"buffer", required_argument, 0, 'b'},
			{"nobuffer", no_argument, &AUDIO_BUFFER_SIZE, 0},
			{"dump", required_argument, 0, 'd'},
			{"downmix",no_argument,0,'x'},
			{0, 0, 0, 0}
		};

		c = getopt_long_only(argc, argv, "xqgstwdpcb:vh", long_options,0);

		if (c == -1) break;
		switch (c) {
		case	1 : debugSetup(optarg); break;
		case	2 : A_SET_VOLUME=argVal("Volume",optarg,0,100); break;
		case 'q': A_QUIET=TRUE; break;
		case 's': A_SHOW_CNT=TRUE; break;
		case 't': A_SHOW_TIME=TRUE; break;
		case 'w': A_FORMAT_WAVE=TRUE; break;
		case 'd': A_DUMP_BINARY=TRUE; break;
		case 'g': A_MSG_STDOUT=TRUE; break;
		case 'p': A_AUDIO_PLAY=TRUE;A_WRITE_TO_FILE=FALSE;break;
		case 'c': A_AUDIO_PLAY=FALSE;A_WRITE_TO_FILE=TRUE;break;
		case 'b': 
#if !defined(LINUX_REALTIME)
			  AUDIO_BUFFER_SIZE=1024*argVal("Buffer size",optarg,64,10000);
#endif
			  break;
		case 'v': showversion=1; break;
		case 'h': showusage=1; break;
		case 'x': A_DOWNMIX=TRUE;break;
		case '?': exit(1);
		case ':': printf("Missing parameter for option -%c\n",c); break;
		}
	}
	if (showversion) displayVersion();
	if (showusage) displayUsage();
	displayDisclaimer();

	return(optind);
}

