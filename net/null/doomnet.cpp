//#define DOOM2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <conio.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/segments.h>
#include <unistd.h>

#include "nullstr.h"
#include "nullnet.h"
//#include "ipx_frch.h"		// FRENCH VERSION

int            vectorishooked = 0;
_go32_dpmi_seginfo olddoomvect = {0, 0, 0, 0, 0};
_go32_dpmi_seginfo newdoomvect = {0, 0, 0, 0, 0};
_go32_dpmi_registers regs;



/*
=============
=
= LaunchDOOM
=
These fields in doomcom should be filled in before calling:

     short     numnodes;      // console is allways node 0
     short     ticdup;             // 1 = no duplication, 2-5 = dup for 
slow nets
     short     extratics;          // 1 = send a backup tic in every 
packet

	 short     consoleplayer; // 0-3 = player number
	 short     numplayers;         // 1-4
	 short     angleoffset;   // 1 = left, 0 = center, -1 = right
	 short     drone;              // 1 = drone
=============
*/

void LaunchDOOM (void)
{
	 char **newargv;
   int newargc;
   int i, j;
	 char adrstring[10];
	 long      flatadr;
   __dpmi_paddr a;

// hook the interrupt vector
   _go32_dpmi_get_real_mode_interrupt_vector(node.doomcom.intnum, &olddoomvect);

   newdoomvect.pm_selector = _my_cs();
   newdoomvect.pm_offset = (int) NetISR;
   _go32_dpmi_allocate_real_mode_callback_iret(&newdoomvect, &regs);
   _go32_dpmi_set_real_mode_interrupt_vector(node.doomcom.intnum, &newdoomvect);
   vectorishooked = 1;

// build the argument list for DOOM, adding a -net &node.doomcom
   for (i = 0; i < myargc; i++)
   {
     if (!argused[i])
       newargc++;
   }
   newargv = (char **) malloc(sizeof(char*) * (newargc + 3));
   for (i = 0, j = 0; i < myargc; i++)
   {
     if (!argused[i])
       newargv[j++] = myargv[i];
   }
	 newargv[j++] = "-net";
	 flatadr = node.GetDoomcom();
	 sprintf (adrstring,"%lu",flatadr);
	 newargv[j++] = adrstring;
   newargv[j++] = NULL;

   if (!access("dosdoom.exe", 0))
    spawnv  (P_WAIT, "dosdoom", newargv);
	 else if (!access("doom2.exe",0))
		spawnv  (P_WAIT, "doom2", newargv);
	 else
		spawnv  (P_WAIT, "doom", newargv);

	 #ifdef DOOM2
	 printf (STR_RETURNED"\n");
	 #else
	 printf ("Returned from DOOM\n");
	 #endif

   free(newargv);
}


/*
=================
=
= CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc-1) or 0 if not present
=
=================
*/

int CheckParm(char *parm)
{
   int i;

	 for(i = 1; i < myargc; i++)
   {
		  if(stricmp(parm, myargv[i]) == 0)
      {
               argused[i] = 1;
               return i;
      }
   }
   return 0;
}


