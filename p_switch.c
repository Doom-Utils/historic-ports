//
// DOSDoom Switch Handling Code  
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include "i_system.h"
#include "dm_defs.h"
#include "p_local.h"

#include "g_game.h"

#include "s_sound.h"

// Data.
#include "lu_sound.h"

// State.
#include "dm_state.h"
#include "r_state.h"

#include "z_zone.h"

// -KM- 98/07/31 DDF
#include "ddf_main.h"

//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
// -KM- 98/07/31 Move to DDF
extern switchlist_t *alphSwitchList;

int		maxswitches = MAXSWITCHES * 2;
int*		switchlist;
int		numswitches;

int		maxbuttons = MAXBUTTONS;
button_t*       buttonlist;

//
// -KM- 1998/09/01 lines.ddf.  Should have this in a header file,
//                             but then practically every file has to
//                             include ddf_main.h
//
extern linedeftype_t* P_GetFromLineHashTable(int trignum);
boolean P_ActivateSpecialLine(line_t *line, int side, mobj_t *thing, trigger_e trig);

//
// P_InitSwitchList
// Only called at game initialization.
//
void P_InitSwitchList(void)
{
  int i;
  int index;

  switchlist = Z_Malloc(sizeof(int) * maxswitches, PU_STATIC, NULL);
  buttonlist = Z_Malloc(sizeof(button_t) * maxbuttons, PU_STATIC, NULL);

  for (index = 0,i = 0;;i++)
  {
    if (index == maxswitches)
    {
      maxswitches += 2;
      switchlist = Z_ReMalloc(switchlist, sizeof(int) * maxswitches);
    }

    // -KM- 98/07/31 Change version check, episode is not always 0
    if (!alphSwitchList[i].name1[0])
    {
      numswitches = index/2;
      switchlist[index] = -1;
      break;
    }

    // -KM- 1998/12/16 Ignore missing animations.  eg might be doom 2 anims
    //  and we are trying to play doom 1.
    switchlist[index++] = R_CheckTextureNumForName(alphSwitchList[i].name1);
    switchlist[index++] = R_CheckTextureNumForName(alphSwitchList[i].name2);
  }
}


//
// Start a button counting down till it turns off.
//
void P_StartButton (line_t* line, bwhere_e w, int texture, int time)
{
    int		i;
    
    // See if button is already pressed
    for (i = 0;i < maxbuttons;i++)
    {
	if (buttonlist[i].btimer
	    && buttonlist[i].line == line)
	{
	    
	    return;
	}
    }
    

    
    for (i = 0;i < maxbuttons;i++)
    {
	if (!buttonlist[i].btimer)
	{
	    buttonlist[i].line = line;
	    buttonlist[i].where = w;
	    buttonlist[i].btexture = texture;
	    buttonlist[i].btimer = time;
	    buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
	    return;
	}
    }

    buttonlist = Z_ReMalloc(buttonlist, sizeof(button_t) * ++maxbuttons);
    buttonlist[i].line = line;
    buttonlist[i].where = w;
    buttonlist[i].btexture = texture;
    buttonlist[i].btimer = time;
    buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
}



//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
// -KM- 1998/09/01 All switches referencing a certain tag are switched
//
void P_ChangeSwitchTexture (line_t* line, int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i, j;
    int     tag = line->tag;
    int     type = line->special;
    sfx_t*     sound;
	
    for(j = 0; j < numlines; j++)
    {
       line = &lines[j];
       if ((line->tag != tag) || (type != line->special))
         continue;

       texTop = sides[line->sidenum[0]].toptexture;
       texMid = sides[line->sidenum[0]].midtexture;
       texBot = sides[line->sidenum[0]].bottomtexture;
   	
       // -KM- 98/07/31 Implement switch sounds
       sound = 0;
   
       // EXIT SWITCH?
       if (line->special == 11)                
   	sound = sfx_swtchx;
   	
       for (i = 0;i < numswitches*2;i++)
       {
   	if (switchlist[i] == texTop)
   	{
   	    // -KM- 98/07/31 Implement Switch Sounds
   	    S_StartSound(buttonlist->soundorg,sound?sound:alphSwitchList[i/2].sfx);
   	    sides[line->sidenum[0]].toptexture = switchlist[i^1];
   
   	    if (useAgain)
   		P_StartButton(line,top,switchlist[i],BUTTONTIME);
   
   	    continue;
   	}
   	else
   	{
   	    if (switchlist[i] == texMid)
   	    {
                // -KM- 98/07/31 Implement sounds
                S_StartSound(buttonlist->soundorg,sound?sound:alphSwitchList[i/2].sfx);
   		sides[line->sidenum[0]].midtexture = switchlist[i^1];
   
   		if (useAgain)
   		    P_StartButton(line, middle,switchlist[i],BUTTONTIME);
   
   		continue;
   	    }
   	    else
   	    {
   		if (switchlist[i] == texBot)
   		{
   		    // -KM- 98/07/31 Implement sounds
   		    S_StartSound(buttonlist->soundorg,sound?sound:alphSwitchList[i/2].sfx);
   		    sides[line->sidenum[0]].bottomtexture = switchlist[i^1];
   
   		    if (useAgain)
   			P_StartButton(line, bottom,switchlist[i],BUTTONTIME);
   
   		    continue;
   		}
   	    }
   	}
       }
   }
}

//
// P_UseSpecialLine
//
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
// -KM- 1998/09/01 Uses new lines.ddf code in p_spec.c
//
// -ACB- 1998/09/07 Uses the return value to discern if a move if possible.
//
boolean P_UseSpecialLine (mobj_t* thing, line_t* line, int side)
{               
  return P_ActivateSpecialLine(line, side, thing, line_pushable);
}
    // Err...
    // Use the back sides of VERY SPECIAL lines...
/*    if (side) Old code
    {
	switch(line->special)
	{
	  case 124:
	    // Sliding door open&close
	    // UNUSED?
	    break;

	  default:
	    return false;
	    break;
	}
    }

    
    // Switches that other things can activate.
    if (!thing->player)
    {
	// never open secret doors
	if (line->flags & ML_SECRET)
	    return false;
	
	switch(line->special)
	{
	  case 1: 	// MANUAL DOOR RAISE
	  case 32:	// MANUAL BLUE
	  case 33:	// MANUAL RED
	  case 34:	// MANUAL YELLOW
	    break;
	    
	  default:
	    return false;
	    break;
	}
    }

    
    // do something  
    switch (line->special)
    {
	// MANUALS
      case 1:		// Vertical Door
      case 26:		// Blue Door/Locked
      case 27:		// Yellow Door /Locked
      case 28:		// Red Door /Locked

      case 31:		// Manual door open
      case 32:		// Blue locked door open
      case 33:		// Red locked door open
      case 34:		// Yellow locked door open

      case 117:		// Blazing door raise
      case 118:		// Blazing door open
	EV_VerticalDoor (line, thing);
	break;
	
	//UNUSED - Door Slide Open&Close
	// case 124:
	// EV_SlidingDoor (line, thing);
	// break;

	// SWITCHES
      case 7:
	// Build Stairs
	if (EV_BuildStairs(line,build8))
	    P_ChangeSwitchTexture(line,0);
	break;

      case 9:
	// Change Donut
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 11:
	// Exit level
	P_ChangeSwitchTexture(line,0);
	G_ExitLevel ();
	break;
	
      case 14:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 15:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 18:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 20:
	// Raise Plat next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 21:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 23:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 29:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 41:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,0);
	break;

      case 49:
	// Ceiling Crush And Raise
	if (EV_DoCeiling(line,crushAndRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 50:
	// Close Door
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 51:
	// Secret EXIT
	P_ChangeSwitchTexture(line,0);
	G_SecretExitLevel ();
	break;
	
      case 55:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,0);
	break;

      case 71:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,0);
	break;
	

      case 101:
	// Raise Floor
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 102:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 103:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 111:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 112:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 113:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 122:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 127:
	// Build Stairs Turbo 16
	if (EV_BuildStairs(line,turbo16))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 131:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 133:
	// BlzOpenDoor BLUE
      case 135:
	// BlzOpenDoor RED
      case 137:
	// BlzOpenDoor YELLOW
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,0);
	break;
	
      case 140:
	// Raise Floor 512
	if (EV_DoFloor(line,raiseFloor512))
	    P_ChangeSwitchTexture(line,0);
	break;
	
	// BUTTONS
      case 42:
	// Close Door
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 43:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 45:
	// Lower Floor to Surrounding floor height
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 60:
	// Lower Floor to Lowest
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 61:
	// Open Door
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 62:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line,downWaitUpStay,1))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 63:
	// Raise Door
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 64:
	// Raise Floor to ceiling
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 66:
	// Raise Floor 24 and change texture
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 67:
	// Raise Floor 32 and change texture
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 65:
	// Raise Floor Crush
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 68:
	// Raise Plat to next highest floor and change texture
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 69:
	// Raise Floor to next highest floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 70:
	// Turbo Lower Floor
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,1);
	break;

      case 99: // BlzOpenDoor BLUE
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 114:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 115:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 116:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 123:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 132:
	// Raise Floor Turbo
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 134: // BlzOpenDoor RED
      case 136: // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
	break;
	
      case 138:
	// Light Turn On
	EV_LightTurnOn(line,255);
	P_ChangeSwitchTexture(line,1);
	break;
	
      case 139:
	// Light Turn Off
	EV_LightTurnOn(line,35);
	P_ChangeSwitchTexture(line,1);
	break;
			
    }
*/

