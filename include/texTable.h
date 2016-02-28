#ifndef _TEXTABLE_H
#define _TEXTABLE_H

#include "cs2.h"

typedef struct _texTable {
	ulong    width;
	ulong    height;
	ulong    Bpp;
	ulong    texIndex;
	uchar    *RGBABuff;
	char     *texName;
	struct _texTable *prev;
	struct _texTable *next;
} texTable;

texTable *addTex(texTable *next, char *name, ulong texIndex, ulong width, ulong height, ulong Bpp, uchar *RGBABuff);
texTable *catTex(texTable *prev, texTable *next);
texTable *texSeek(texTable *tbl, long offset, int whence);
texTable *texSearch(texTable *tbl, char *name, ulong texIndex);

#define rwdTex(t) while(t->prev) t = t->prev;
#define fwdTex(t) while(t->next) t = t->next;

#endif

