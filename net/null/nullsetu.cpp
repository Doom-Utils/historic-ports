// ipxsetup.c

#define DOOM2

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <stdarg.h>
#include <bios.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nullnet.h"
#include "nullstr.h"
//#include "ipx_frch.h"	// FRENCH VERSION

int gameid;
int numnetnodes;
int socketid = 0x869c;        // 0x869c is the official DOOM socket
int	myargc;
char **myargv;
char *argused;


/*
=================
=
= Error
=
= For abnormal program terminations
=
=================
*/

void Error (char *error, ...)
{
     va_list argptr;

	 if (vectorishooked)
   {
		  _go32_dpmi_set_real_mode_interrupt_vector (node.doomcom.intnum, &olddoomvect);
      _go32_dpmi_free_real_mode_callback(&newdoomvect);
      vectorishooked = 0;
   }

   va_start (argptr,error);
   vprintf (error,argptr);
   va_end (argptr);
	 printf ("\n");
   ShutdownNetwork ();
   exit (1);
}


/*
=============
=
= NetISR
=
=============
*/

void NetISR (void)
{
   node.service();
}



/*
===================
=
= LookForNodes
=
= Finds all the nodes for the game and works out player numbers among 
them
=
= Exits with nodesetup[0..numnodes] and nodeadr[0..numnodes] filled in
===================
*/

void LookForNodes (void)
{
   int            total, console;
   int            i;
   nodeadr_t     me;
   char          filename[256];

//
// Wait for wanted net players
//
	 printf(STR_ATTEMPT, numnetnodes);

	 printf (STR_LOOKING);

   node.doomcom.numnodes = 0;
//
// register us
//
   // Find other nodes.
   do
   {
     int file;
  //
  // check for aborting
  //
       while ( bioskey(1) )
         if ( (bioskey (0) & 0xff) == 27)
           Error ("\n\n"STR_NETABORT);
  
  //
  // listen to the network
  //
     sprintf(filename, "node%d.net", node.doomcom.numnodes);
     if (!access(filename, R_OK))
     {
       printf ("\n"STR_FOUND"\n");
       file = open(filename, O_RDWR | O_BINARY | O_NONBLOCK);
       while (file < 0)
       {
         __dpmi_yield();
         file = open(filename, O_RDWR | O_BINARY | O_NONBLOCK);
       }
       read(file, &i, sizeof(int));
       lseek(file, -sizeof(int), SEEK_CUR);
       if (i < numnetnodes)
       {
         write(file, &numnetnodes, sizeof(int));
         lseek(file, -sizeof(int), SEEK_CUR);
       } else
         numnetnodes = i;
       node.doomcom.numnodes++;
       close(file);
 
       if (node.doomcom.numnodes < numnetnodes)
 	        printf (STR_LOOKING);
     } else if (mynode == -1) {
       file = open(filename, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, S_IRUSR|S_IWUSR);
       while (file < 0)
       {
         __dpmi_yield();
         file = open(filename, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, S_IRUSR|S_IWUSR);
       }
       write(file, &numnetnodes, sizeof(int));
       close(file);
       mynode = node.doomcom.numnodes++;
     }
     if (mynode != -1)
     {
       sprintf(filename, "node%d.net", mynode);
       file = open(filename, O_BINARY|O_RDONLY);
       while (file < 0)
       {
         __dpmi_yield();
         file = open(filename, O_BINARY|O_RDONLY);
       }
       read(file, &numnetnodes, sizeof(int));
       close(file);
     }
  //
  // we are done if all nodes have found all other nodes
  //
     __dpmi_yield();
	 } while (node.doomcom.numnodes < numnetnodes);

//
// count players
//
	 node.doomcom.consoleplayer = mynode;
	 node.doomcom.numplayers = numnetnodes;

	 printf ("\n"STR_CONSOLEIS"\n", mynode+1, node.doomcom.numnodes);
}


//========================================================
//
//	Find a Response File
//
//========================================================
void FindResponseFile (void)
{
	int		i;
	#define	MAXARGVS	100

	for (i = 1;i < myargc;i++)
		if (myargv[i][0] == '@')
		{
			FILE *		handle;
			int		size;
			int		k;
			int		index;
			int		indexinfile;
			char	*infile;
			char	*file;
			char	*moreargs[20];
			char	*firstargv;

			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen (&myargv[i][1],"rb");
			if (!handle)
				Error (STR_NORESP);
			printf(STR_FOUNDRESP" \"%s\"!\n",strupr(&myargv[i][1]));
			fseek (handle,0,SEEK_END);
			size = ftell(handle);
			fseek (handle,0,SEEK_SET);
			file = (char *) malloc (size);
			fread (file,size,1,handle);
			fclose (handle);

			// KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
			for (index = 0,k = i+1; k < myargc; k++)
				moreargs[index++] = myargv[k];

			firstargv = myargv[0];
			myargv = (char **) malloc(sizeof(char *)*MAXARGVS);
			memset(myargv,0,sizeof(char *)*MAXARGVS);
			myargv[0] = firstargv;

			infile = file;
			indexinfile = k = 0;
			indexinfile++;	// SKIP PAST ARGV[0] (KEEP IT)
			do
			{
				myargv[indexinfile++] = infile+k;
				while(k < size &&
					((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
					k++;
				*(infile+k) = 0;
				while(k < size &&
					((*(infile+k)<= ' ') || (*(infile+k)>'z')))
					k++;
			} while(k < size);

			for (k = 0;k < index;k++)
				myargv[indexinfile++] = moreargs[k];
			myargc = indexinfile;

			// DISPLAY ARGS
//			printf("%d command-line args:\n",myargc);
//			for (k=1;k<myargc;k++)
//				printf("%s\n",myargv[k]);

			break;
		}
}


/*
=============
=
= main
=
=============
*/

int main (int _argc, char** _argv)
{
	 int  i;
	 int vector;

//
// determine game parameters
//
	 gameid = 0;
	 numnetnodes = 2;
	 node.doomcom.ticdup = 1;
	 node.doomcom.extratics = 0;
	 node.doomcom.episode = 1;
	 node.doomcom.map = 1;
	 node.doomcom.skill = 2;
	 node.doomcom.deathmatch = 0;
// prepare for DOOM
	 node.doomcom.id = DOOMCOM_ID;

	 printf("\n"
			 "-----------------------------\n"
	 #ifdef DOOM2
			 STR_DOOMNETDRV" "
	 #else
			 "DOOM NETWORK DEVICE DRIVER "
	 #endif
			 "v1.22\n"
			 "-----------------------------\n");

	 myargc = _argc;
	 myargv = _argv;
   argused = (char *) malloc(myargc);
   memset (argused, 0, myargc);
	 FindResponseFile();

	 if((i = CheckParm("-nodes")) != 0)
		  numnetnodes = strtol(myargv[i+1], NULL, 0);

	 if((i = CheckParm("-vector")) != 0)
	 {
      _go32_dpmi_seginfo a;
		  node.doomcom.intnum = strtol (myargv[i+1], NULL, 0);
		  _go32_dpmi_get_real_mode_interrupt_vector(node.doomcom.intnum, &a);
/*
		  if(a.offset32)
		  {
        if (_farpeekb(a.selector, a.offset32) != 0xcf)
        {
  		    printf(STR_VECTSPEC"\n", node.doomcom.intnum);
  			   exit(-1);
        }
		  }*/
	 }
	 else
   {
     _go32_dpmi_seginfo a;
     for(node.doomcom.intnum = 0x60 ; node.doomcom.intnum <= 0x66 ; node.doomcom.intnum++)
     {
  		 _go32_dpmi_get_real_mode_interrupt_vector(node.doomcom.intnum, &a);

       if (!a.rm_offset && !a.rm_segment)
         break;
       if (_farpeekb(_dos_ds, a.rm_segment * 16 + a.rm_offset) == 0xcf)
         break;
  	 }
  	 if(node.doomcom.intnum == 0x67)
 		 {
 		   printf(STR_NONULL"\n");
 		   exit(-1);
 		 }
	 }
	 printf(STR_COMMVECT"\n",node.doomcom.intnum);

	 InitNetwork ();

	 LookForNodes ();

   dosmemput(&node.doomcom, sizeof(doomcom_t), node.GetDoomcom());

	 LaunchDOOM ();

	 ShutdownNetwork ();

	 if (vectorishooked)
   {
		  _go32_dpmi_set_real_mode_interrupt_vector (node.doomcom.intnum, &olddoomvect);
      _go32_dpmi_free_real_mode_callback(&newdoomvect);
      vectorishooked = 0;
   }

   free (argused);

	 return 0;
}


