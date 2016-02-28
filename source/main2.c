#include <3ds.h>
#include <sf2d.h>
#include <stdio.h>

#include "texTable.h"
#include "HGx.h"

int main(int argc, char **argv)
{
	sf2d_texture *tex1    = NULL;
	sf2d_texture *tex2    = NULL;
	texTable     *texTbl  = NULL;
	//texTable     *tempTex = NULL;
	u32          keys;
	float        scale;
	
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);
	printf("\x1b[0;0H");
	
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	texTbl = openHGx("a.hg3");
	catTex(texTbl, openHGx("b.hg3"));
	tex1 = sf2d_create_texture_mem_RGBA8(texTbl->RGBABuff, texTbl->width, texTbl->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	texTbl = texTbl->next;
	tex2 = sf2d_create_texture_mem_RGBA8(texTbl->RGBABuff, texTbl->width, texTbl->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);


	scale = (400.0 / tex1->width < 240.0 / tex1->height) ? 400.0 / tex1->width : 240.0 / tex1->height;

	while(aptMainLoop())
	{
		hidScanInput();
		keys = hidKeysDown();
		if(keys & KEY_START)
			break;
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sf2d_draw_texture_scale(tex1, 200 - scale*tex1->width/2, 120 - scale*tex1->height/2, scale, scale);
		sf2d_draw_texture_scale(tex2, 200 - scale*tex2->width/2, 120 - scale*tex2->height/2, scale, scale);
		sf2d_end_frame();
		sf2d_swapbuffers();
	}

	sf2d_free_texture(tex1);
	//freeTexTbl(texTbl);
	free(texTbl->RGBABuff);
	free(texTbl);

	sf2d_fini();
	
	return 0;
}

