#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"
#include "../../kernel/arch/x86-32/interrupts/pit.h"

pixel_t* DG_ScreenBuffer = NULL;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

	DG_Init();
	init_pit(1000);

	D_DoomMain ();
}

