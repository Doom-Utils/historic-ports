#include <stdlib.h>
#include "SDL.h"
#include "l_sdl.h"

void I_ShutdownSDL(void)
{
	//SDL_Quit();
	return;
}

void I_InitSDL(void)
{
        atexit(I_ShutdownSDL);
        return;
}
