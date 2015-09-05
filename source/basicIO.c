#include <3ds.h>

void pausec() 
{
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_A) break;
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		gspWaitForVBlank();
	}
}
	
