// (JC) Radius Trigger header file
//
// ---

#ifndef __RAD_TRIG__
#define __RAD_TRIG__

// Tip Prottypes
void ResetTipStructure(void);
void TIP_SendTip(char *tiptext, int showtime, boolean dosound);
void TIP_DisplayTips(int y);

// RadiusTrigger & Scripting Prototypes
void ClearRadiTriggersTimers(void);
void RadiShutdown(void);
void ResetRadiTriggers(void);
void DoRadiTrigger(player_t *p);
void LoadScript(void);

#endif