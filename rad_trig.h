// (JC) Radius Trigger header file
//
// ---

#ifndef __RAD_TRIG__
#define __RAD_TRIG__

//Action Definitions
//Tip Function <"TEXT"> <DISPLAYTIME> <SOUND ON/OFF>
#define MAXTIPLEN     50 // -ACB- 1998/06/09 reduced to 50 from 56

typedef struct
{
  char tip_text[MAXTIPLEN];
  int  display_time;
  boolean playsound;
  char tip_graphic[9];
}
s_tip_t;

// Tip Prottypes
void RAD_ResetTipStructure(void);
void TIP_SendTip(s_tip_t *tip);
void TIP_DisplayTips(int y);

// RadiusTrigger & Scripting Prototypes
void RAD_ClearRadiTriggersTimers(void);
void RAD_RadiShutdown(void);
void RAD_ResetRadiTriggers(void);
void RAD_DoRadiTrigger(player_t *p);
void RAD_LoadScript(char* name, int lump);

#endif