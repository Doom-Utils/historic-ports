// 
// I_Input.h
//

typedef unsigned char uchar;

void I_InitKeyTrans(void);
unsigned short int I_ScanCode2DoomCode(unsigned short int);
unsigned short int I_DoomCode2ScanCode(unsigned short int);
void StartTic(void);
void I_InitInputs(void);
void I_DestroyInputs(void);


