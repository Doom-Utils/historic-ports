#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "doomstat.h"
#include "doomdef.h"
#include "p_local.h"
#include "m_cheat.h"
#include "dstrings.h"
#include "sounds.h"
#include "i_system.h"

//dehacked misc stuff
int deh_inithealth=100;
int deh_initbullets=50;
int deh_maxhealth=200;
int deh_maxarmor=200;
int deh_greenac=1;
int deh_blueac=2;
int deh_maxsoulhealth=200;
int deh_soulhealth=100;
int deh_megahealth=200;
int deh_godhealth=100;
int deh_idfaarmor=200;
int deh_idfaac=2;
int deh_idkfaarmor=200;
int deh_idkfaac=2;
int deh_bfgcells=40;
int deh_infight;  //not yet implemented!

#define nummodes 10

char modestring[nummodes][10]=
  {"Thing ",
   "Sound ",
   "Frame ",
   "Sprite ",
   "Ammo ",
   "Weapon ",
   "Pointer ",
   "Cheat ",
   "Misc ",
   "Text "};

int checknum(char *line,char *string,unsigned int *thenum);
int checkstring(char *line,char *string,char *thestring);

void dehloadThing(char *line, int modevar);   //done
void dehloadSound(char *line, int modevar);   //done
void dehloadFrame(char *line, int modevar);   //done
void dehloadSprite(char *line, int modevar);
void dehloadAmmo(char *line, int modevar);    //done
void dehloadWeapon(char *line, int modevar);  //done
void dehloadPointer(char *line, int modevar); //done
void dehloadCheat(char *line, int modevar);   //done
void dehloadMisc(char *line, int modevar);    //done - EXCEPT for INFIGHTING!
void dehloadText(FILE *thefile, int origlen,int replacelen); //mostly done

actionf_t origstate[NUMSTATES];
char* origsfxname[108];
char* origsprname[NUMSPRITES];

void loaddeh(char *dehfile)
  {
  FILE* blah;
  char currline[1000];
  int currmode=-1;
  int modevar=-1,modevar2=-1;
  int i;
  int changemodeflag=false;

  for (i=0;i<NUMSTATES;i++)
    origstate[i]=states[i].action;
  for (i=0;i<108;i++)
    origsfxname[i]=S_sfx[i].name;
  for (i=0;i<NUMSPRITES;i++)
    origsprname[i]=sprnames[i];

  blah=fopen(dehfile,"r");
  if (blah==NULL)
    I_Error("Unable to open DeHackEd file!");
  while (!feof(blah))
    {
    changemodeflag=false;
    do{
      fgets(currline,1000,blah);
      for (i=0;i<nummodes;i++)
        {
        if (strncmp(currline,modestring[i],strlen(modestring[i]))==0)
          {
          currmode=i;
          changemodeflag=true;
          if (currmode==9) //hack for text strings
            {
            sscanf(currline+strlen(modestring[i]),"%d %d",&modevar,&modevar2);
            dehloadText(blah,modevar,modevar2);
            changemodeflag=false;
            }
          else
            sscanf(currline+strlen(modestring[i]),"%d",&modevar);
          }
        }
      }while ((changemodeflag==false)&&(!feof(blah)));
    if (feof(blah)) break;

//    printf ("Processing %d on %d\n",currmode,modevar);

    do{
      fgets(currline,1000,blah);
      if (currline[0]!='#')
        {
        switch (currmode)
          {
          case 0: dehloadThing(currline,modevar); break;
          case 1: dehloadSound(currline,modevar); break;
          case 2: dehloadFrame(currline,modevar); break;
          case 3: dehloadSprite(currline,modevar); break;
          case 4: dehloadAmmo(currline,modevar); break;
          case 5: dehloadWeapon(currline,modevar); break;
          case 6: dehloadPointer(currline,modevar); break;
          case 7: dehloadCheat(currline,modevar); break;
          case 8: dehloadMisc(currline,modevar); break;
          case 9: break;  //text strings are handled previously
          }
        }
      } while ((strlen(currline)>2)&&(!feof(blah)));
    }
  fclose(blah);
  }

int checknum(char *line,char *string,unsigned int *thenum)
  {
  int numfound=false;
  int numstart=0;
  int i;

  if (strncmp(line,string,strlen(string))==0)
    {
    numfound=false;
    for (i=strlen(string);i<strlen(line);i++)
      {
      if (isdigit(line[i]))
        {
        numfound=true;
        numstart=i;
        break;
        }
      }
    if (numfound==true)
      {
      sscanf(line+numstart,"%ud",thenum);
//      printf("number:%d\n",*thenum);
      return true;
      }
    else
      {
      printf ("Error in DeHackEd file - skipping over!!!\n");
      return false;
      }
    }
  return false;
  }

int checkstring(char *line,char *string,char *thestring)
  {
  int strfound=false;
  int strstart=0;
  int i;

  if (strncmp(line,string,strlen(string))==0)
    {
    strfound=false;
    strstart=strlen(string);
    for (i=strstart;i<strlen(line);i++)
      {
      if (line[i]=='=')
        {
        strstart=i+1;
        break;
        }
      }
    for (i=strstart;i<strlen(line);i++)
      {
      if ((line[i]!=' ')&&(line[i]!='\t'))
        {
        strfound=true;
        strstart=i;
        break;
        }
      }
    if (strfound==true)
      {
      int j=0;
      for (i=strstart;i<(strlen(line)+1);i++)
        {
        if ((line[i]=='\n')||(line[i]==' ')||(line[i]==10)||(line[i]==0))
          break;
        thestring[j]=line[i];j++;
        }
      thestring[j]=0;
//      printf("string found:%s\n",thestring);
      return true;
      }
    else
      {
      printf ("Error in DeHackEd file - skipping over!!!\n");
      return false;
      }
    }
  return false;
  }


void dehloadThing(char *line, int modevar)
  {
  unsigned int currnum;

  modevar--;
  if (checknum(line,"ID #",&currnum))
    mobjinfo[modevar].doomednum=currnum;
  else if (checknum(line,"Initial frame",&currnum))
    mobjinfo[modevar].spawnstate=currnum;
  else if (checknum(line,"Hit points",&currnum))
    mobjinfo[modevar].spawnhealth=currnum;
  else if (checknum(line,"First moving frame",&currnum))
    mobjinfo[modevar].seestate=currnum;
  else if (checknum(line,"Alert sound",&currnum))
    mobjinfo[modevar].seesound=currnum;
  else if (checknum(line,"Reaction time",&currnum))
    mobjinfo[modevar].reactiontime=currnum;
  else if (checknum(line,"Attack sound",&currnum))
    mobjinfo[modevar].attacksound=currnum;
  else if (checknum(line,"Injury frame",&currnum))
    mobjinfo[modevar].painstate=currnum;
  else if (checknum(line,"Pain chance",&currnum))
    mobjinfo[modevar].painchance=currnum;
  else if (checknum(line,"Pain sound",&currnum))
    mobjinfo[modevar].painsound=currnum;
  else if (checknum(line,"Close attack frame",&currnum))
    mobjinfo[modevar].meleestate=currnum;
  else if (checknum(line,"Far attack frame",&currnum))
    mobjinfo[modevar].missilestate=currnum;
  else if (checknum(line,"Death frame",&currnum))
    mobjinfo[modevar].deathstate=currnum;
  else if (checknum(line,"Exploding frame",&currnum))
    mobjinfo[modevar].xdeathstate=currnum;
  else if (checknum(line,"Death sound",&currnum))
    mobjinfo[modevar].deathsound=currnum;
  else if (checknum(line,"Speed",&currnum))
    mobjinfo[modevar].speed=currnum;

  else if (checknum(line,"Width",&currnum))
    mobjinfo[modevar].radius=currnum;
  else if (checknum(line,"Height",&currnum))
    mobjinfo[modevar].height=currnum;

  else if (checknum(line,"Mass",&currnum))
    mobjinfo[modevar].mass=currnum;
  else if (checknum(line,"Missile damage",&currnum))
    mobjinfo[modevar].damage=currnum;
  else if (checknum(line,"Action sound",&currnum))
    mobjinfo[modevar].activesound=currnum;
  else if (checknum(line,"Bits",&currnum))
    mobjinfo[modevar].flags=currnum;
  else if (checknum(line,"Respawn frame",&currnum))
    mobjinfo[modevar].raisestate=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadSound(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Offset",&currnum))
    S_sfx[modevar].name=origsfxname[0]+(currnum-129380-21960);  //This is BAD
  else if (checknum(line,"Zero/One",&currnum))
    S_sfx[modevar].singularity=currnum;
  else if (checknum(line,"Value",&currnum))
    S_sfx[modevar].priority=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadFrame(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Sprite number",&currnum))
    states[modevar].sprite=currnum;
  else if (checknum(line,"Sprite subnumber",&currnum))
    states[modevar].frame=currnum;
  else if (checknum(line,"Duration",&currnum))
    states[modevar].tics=currnum;
  else if (checknum(line,"Next frame",&currnum))
    states[modevar].nextstate=currnum;
  else if (checknum(line,"Unknown 1",&currnum))
    states[modevar].misc1=currnum;
  else if (checknum(line,"Unknown 2",&currnum))
    states[modevar].misc2=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadSprite(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Offset",&currnum))
    sprnames[modevar]=origsprname[modevar]+(currnum-129380-22928); //This is BAD

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadAmmo(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Max ammo",&currnum))
    maxammo[modevar]=currnum;
  else if (checknum(line,"Per ammo",&currnum))
    clipammo[modevar]=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadWeapon(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Ammo type",&currnum))
    weaponinfo[modevar].ammo=currnum;
  else if (checknum(line,"Deselect frame",&currnum))
    weaponinfo[modevar].downstate=currnum;
  else if (checknum(line,"Select frame",&currnum))
    weaponinfo[modevar].upstate=currnum;
  else if (checknum(line,"Bobbing frame",&currnum))
    weaponinfo[modevar].readystate=currnum;
  else if (checknum(line,"Shooting frame",&currnum))
    weaponinfo[modevar].atkstate=currnum;
  else if (checknum(line,"Firing frame",&currnum))
    weaponinfo[modevar].flashstate=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadPointer(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Codep Frame",&currnum))
    states[modevar].action=origstate[currnum];
  else if (strlen(line)>2)
    printf("Invalid line: %s",line);

  }

void dehloadCheat(char *line, int modevar)
  {
  char cheattext[200];

  if (checkstring(line,"Change music",cheattext))
    {
    cheat_mus.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_mus.sequence,cheattext);
    cheat_mus.sequence[strlen(cheattext)]=1;
    cheat_mus.sequence[strlen(cheattext)+1]=0;
    cheat_mus.sequence[strlen(cheattext)+2]=0;
    cheat_mus.sequence[strlen(cheattext)+3]=0xff;
    }
  else if (checkstring(line,"Chainsaw",cheattext))
    {
    cheat_choppers.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_choppers.sequence,cheattext);
    cheat_choppers.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"God mode",cheattext))
    {
    cheat_god.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_god.sequence,cheattext);
    cheat_god.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Ammo & Keys",cheattext))
    {
    cheat_ammo.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_ammo.sequence,cheattext);
    cheat_ammo.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Ammo",cheattext))
    {
    cheat_ammonokey.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_ammonokey.sequence,cheattext);
    cheat_ammonokey.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"No Clipping 1",cheattext))
    {
    cheat_noclip.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_noclip.sequence,cheattext);
    cheat_noclip.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"No Clipping 2",cheattext))
    {
    cheat_commercial_noclip.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_commercial_noclip.sequence,cheattext);
    cheat_commercial_noclip.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Invincibility",cheattext))
    {
    cheat_powerup[0].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[0].sequence,cheattext);
    cheat_powerup[0].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Berserk",cheattext))
    {
    cheat_powerup[1].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[1].sequence,cheattext);
    cheat_powerup[1].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Invisibility",cheattext))
    {
    cheat_powerup[2].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[2].sequence,cheattext);
    cheat_powerup[2].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Radiation Suit",cheattext))
    {
    cheat_powerup[3].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[3].sequence,cheattext);
    cheat_powerup[3].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Auto-map",cheattext))
    {
    cheat_powerup[4].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[4].sequence,cheattext);
    cheat_powerup[4].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Lite-Amp Goggles",cheattext))
    {
    cheat_powerup[5].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[5].sequence,cheattext);
    cheat_powerup[5].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"BEHOLD menu",cheattext))
    {
    cheat_powerup[6].sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_powerup[6].sequence,cheattext);
    cheat_powerup[6].sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Level Warp",cheattext))
    {
    cheat_clev.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_clev.sequence,cheattext);
    cheat_clev.sequence[strlen(cheattext)]=1;
    cheat_clev.sequence[strlen(cheattext)+1]=0;
    cheat_clev.sequence[strlen(cheattext)+2]=0;
    cheat_clev.sequence[strlen(cheattext)+3]=0xff;
    }
  else if (checkstring(line,"Player Position",cheattext))
    {
    cheat_mypos.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_mypos.sequence,cheattext);
    cheat_mypos.sequence[strlen(cheattext)]=0xff;
    }
  else if (checkstring(line,"Map cheat",cheattext))
    {
    cheat_amap.sequence=(char *)malloc(strlen(cheattext)+5);
    strcpy(cheat_amap.sequence,cheattext);
    cheat_amap.sequence[strlen(cheattext)]=0xff;
    }

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadMisc(char *line, int modevar)
  {
  unsigned int currnum;

  if (checknum(line,"Initial Health",&currnum))
    deh_inithealth=currnum;
  else if (checknum(line,"Initial Bullets",&currnum))
    deh_initbullets=currnum;
  else if (checknum(line,"Max Health",&currnum))
    deh_maxhealth=currnum;
  else if (checknum(line,"Max Armor",&currnum))
    deh_maxarmor=currnum;
  else if (checknum(line,"Green Armor Class",&currnum))
    deh_greenac=currnum;
  else if (checknum(line,"Blue Armor Class",&currnum))
    deh_blueac=currnum;
  else if (checknum(line,"Max Soulsphere",&currnum))
    deh_maxsoulhealth=currnum;
  else if (checknum(line,"Soulsphere Health",&currnum))
    deh_soulhealth=currnum;
  else if (checknum(line,"Megasphere Health",&currnum))
    deh_megahealth=currnum;
  else if (checknum(line,"God Mode Health",&currnum))
    deh_godhealth=currnum;
  else if (checknum(line,"IDFA Armor",&currnum))
    deh_idfaarmor=currnum;
  else if (checknum(line,"IDFA Armor Class",&currnum))
    deh_idfaac=currnum;
  else if (checknum(line,"IDKFA Armor",&currnum))
    deh_idkfaarmor=currnum;
  else if (checknum(line,"IDKFA Armor Class",&currnum))
    deh_idkfaac=currnum;
  else if (checknum(line,"BFG Cells/Shot",&currnum))
    deh_bfgcells=currnum;
  else if (checknum(line,"Monsters Infight",&currnum))
    deh_infight=currnum;

  else if (strlen(line)>2)
    printf("Invalid line: %s",line);
  }

void dehloadText(FILE *thefile, int origlen,int replacelen)
  {
  char origtext[10000]; //allow for REALLY long text strings
  char newtext[10000];
  int i;

  for (i=0;i<origlen;i++)
    {
    origtext[i]=fgetc(thefile);
    }
  origtext[i]=0;
  for (i=0;i<replacelen;i++)
    {
    newtext[i]=fgetc(thefile);
    }
  newtext[i]=0;

  //first check sounds
  for (i=1;i<108;i++)
    {
    if (strcmp(S_sfx[i].name,origtext)==0)
      {
      S_sfx[i].name=(char *)malloc(strlen(newtext)+1);
      strcpy(S_sfx[i].name,newtext);
      return;
      }
    }
  //now, check music
  for (i=1;i<68;i++)
    {
    if (strcmp(S_music[i].name,origtext)==0)
      {
      S_music[i].name=(char *)malloc(strlen(newtext)+1);
      strcpy(S_music[i].name,newtext);
      return;
      }
    }
  //now, check sprite names
  for (i=0;i<NUMSPRITES;i++)
    {
    if (strcmp(sprnames[i],origtext)==0)
      {
      sprnames[i]=(char *)malloc(strlen(newtext)+1);
      strcpy(sprnames[i],newtext);
      return;
      }
    }
  //some misc stuff
  //now, do standard text strings
  if (!searchstring(origtext,newtext))
    {
    printf("Unable to replace %s with %s!\n",origtext,newtext);
    }
  }



