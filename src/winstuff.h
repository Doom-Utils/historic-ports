#ifndef _WINSTUFF_H_
#define _WINSTUFF_H_

#if __cplusplus
extern "C" {
#endif

void Done_Winstuff(void);
int Init_Winstuff(void);
void V_SetPal (unsigned char *pal);
void V_EndFrame (void);
void V_GetMessages (void);
void I_ConPrintString(char *msg);

#if __cplusplus
}
#endif

#endif


