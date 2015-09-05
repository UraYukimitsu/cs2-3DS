//#include "blowfish.h"
#include <3ds.h>
#include <time.h>
#include "extract-int.h"
#include "extract-cst.h"

int main(int argc, char **argv)
{
	time_t start, end;
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	printf("\x1b[0;0H");
	printf("Hello, world!\n");
	//file_list("update01.int", "FW-6JD55162");
	
	start = time(NULL);
	printf("Conversion started on %d.\n", start);
	open_cst("ama_002.cst", "ama_002.txt");
	end = time(NULL);
	printf("Conversion finished on %d in %d seconds.\n", end, end - start);
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_START) break;
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		gspWaitForVBlank();
	}
	
	gfxExit();
	return 0;
}









































