#include "string.h"

///////////////////////////////////////////////////////////////////////////
//
//  US_GetBin() - Returns a binary byte as a char *, P.Harvey-Smith, 19/07/96.
//
///////////////////////////////////////////////////////////////////////////
char *US_GetBin(unsigned int n)
{
    static char    bin[34];
    unsigned int   p;

    memset(bin,0,34);
    for(p=0x8000;p>0;p=p>>1)
	{
		if(p&n)
			strcat(bin,"1");
		else
			strcat(bin,"0");
	}
    return bin;
}
