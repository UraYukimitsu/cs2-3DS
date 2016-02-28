#ifndef _HGX_H
#define _HGX_H

#include "cs2.h"
#include "bitBuffer.h"
#include "texTable.h"

typedef struct _HGxHeader {
		uchar magic[4];
		ulong unknown[3];
		ulong entries;
} HGxHeader;

typedef struct _HG3Tag {
	uchar name[8];
	ulong offsetNextTag;
	ulong length;
} HG3Tag;

typedef struct _HG3StdInfo {
	ulong width;
	ulong height;
	ulong bpp;
	ulong offsetX;
	ulong offsetY;
	ulong totalWidth;
	ulong totalHeight;
	ulong unknown[3];
} HG3StdInfo;

typedef struct _HG3ImgHeader {
	ulong unknown;
	ulong height;
	ulong compressedLen;
	ulong originalLen;
	ulong cmdLen;
	ulong originalCmdLen;
} HG3ImgHeader;

texTable *openHGx(const char *filename);
texTable *readHG3Image(int fd, HG3StdInfo stdInfo, HG3ImgHeader imgHdr, char *outName, ulong texIndex, texTable *next);
uchar *unRLE(uchar *buff, ulong buffLen, uchar *cmdBuff, ulong cmdLen, ulong *outLen);
void unDelta(uchar *buff, ulong buffLen, ulong width, ulong height, ulong Bpp, uchar *RGBABuff);

#endif //_HGX_H
