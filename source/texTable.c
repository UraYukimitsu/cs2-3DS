#include "texTable.h"

texTable *addTex(texTable *next, char *name, ulong texIndex, ulong width, ulong height, ulong Bpp, uchar *RGBABuff)
{
	texTable *ret = NULL;
	ulong i;
	uchar temp;

	printf("Entered function addTex.\n");

	ret = malloc(sizeof(texTable));
	if(!ret)
	{
		printf("Couldn't malloc texTable.\n");
		return NULL;
	}
	if(next)
	{
		rwdTex(next);
		next->prev = ret;
	}
	ret->prev     = NULL;
	ret->next     = next;
	ret->width    = width;
	ret->height   = height;
	ret->Bpp      = Bpp;
	ret->texIndex = texIndex;
	ret->texName  = strdup(name);
	ret->RGBABuff = malloc(width * height * Bpp);
	if(!ret->RGBABuff)
	{
		printf("Couldn't malloc ret->RGBABuff.\n");
		return NULL;
	}
	for(i = 0; i < width * height; i++)
	{
		memcpy(&ret->RGBABuff[4*i], &RGBABuff[4 * ((height - 1 - i/width) * width + i % width)], 4);
		temp = ret->RGBABuff[4*i];
		ret->RGBABuff[4*i] = ret->RGBABuff[4*i + 2];
		ret->RGBABuff[4*i + 2] = temp;
	}


	printf("Exited function addTex returning %08x.\n", (ulong)ret);

	return ret;
}

texTable *catTex(texTable *prev, texTable *next)
{
	fwdTex(prev); //Forwarding the table to last elt
	rwdTex(next); //Rewinding the table to first elt
	prev->next = next;
	next->prev = prev;
	rwdTex(prev);
	return prev;
}

texTable *texSeek(texTable *tbl, long offset, int whence)
{
	long i;
	switch(whence)
	{
		case SEEK_CUR:
			break;
		case SEEK_SET:
			rwdTex(tbl);
			break;
		case SEEK_END:
			fwdTex(tbl);
			break;
		default:
			break;
	}
	if(offset > 0)
		for(i = 0; i < offset; i++)
		{
			if(!tbl->next)
				return tbl;
			tbl = tbl->next;
		}
	if(offset < 0)
		for(i = 0; i > offset; i--)
		{
			if(!tbl->prev)
				return tbl;
			tbl = tbl->prev;
		}
	return tbl;
}

texTable *texSearch(texTable *tbl, char *name, ulong texIndex)
{
	rwdTex(tbl);
	while(tbl->next)
	{
		if(!strcmp(tbl->texName, name) && tbl->texIndex == texIndex)
			return tbl;
		tbl = tbl->next;
	}
	return NULL; //If we didn't find anything, return NULL
}

