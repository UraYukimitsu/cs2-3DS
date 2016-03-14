#include "HGx.h"
#include "bitBuffer.h"

#define smalloc(ptr, size) 	ptr = malloc(size);\
				if(!ptr) exit(1);

texTable *openHGx(const char *filename)
{
	int      fd;
	texTable *ret = NULL;
	fd  = open(filename, O_RDONLY | O_BINARY);
	ret = readHGx(fd, filename);
	close(fd);
	return ret;
}


//This way, we can read HGx files right from int archives.
texTable *readHGx(int fd, const char *filename)
{
	HGxHeader    fileHdr;
	HG3Tag       tag;
	HG3StdInfo   stdInfo;
	HG3ImgHeader imgHdr;
	ulong        imgIndex = 0;
	texTable     *ret = NULL;
	char         *texName = strdup(filename);
	
	read(fd, &fileHdr, sizeof(fileHdr));
	if(!memcmp(fileHdr.magic, "HG-3", 4)) //Seems like a correct HG3 file for now
	{
		while(1)
		{
            		memset(&tag, 0, sizeof(tag));
			read(fd, &tag, sizeof(tag));
			if(memcmp(tag.name, "stdinfo", 7))
				break;
			if(tag.length != sizeof(stdInfo))
			{
				printf("stdinfo size mismatch.\nExpected %lu, got %lu.\n", (ulong)sizeof(stdInfo), tag.length);
				return NULL;
			}
			read(fd, &stdInfo, sizeof(stdInfo));
			printf("OffNext: %08lx.\n", tag.offsetNextTag);
			while(tag.offsetNextTag)
			{
				read(fd, &tag, sizeof(tag));
				printf("OffNext: %08lx.\n", tag.offsetNextTag);
				if(!memcmp(tag.name, "img0000", 7))
				{
					read(fd, &imgHdr, sizeof(imgHdr));
					ret = readHG3Image(fd, stdInfo, imgHdr, texName, imgIndex, ret);
					return ret;
					imgIndex++;
				} else {
					//Save unknown data to file
				}
				lseek(fd, 12, SEEK_CUR);
				printf("Next image: %lu.\n", imgIndex);
			}
		}
	} else {
		printf("File is not HG3. (%s)\n", fileHdr.magic);
		return NULL;
	}
	free(texName);
	
	return ret;
}

texTable *readHG3Image(int fd, HG3StdInfo stdInfo, HG3ImgHeader imgHdr, char *texName, ulong texIndex, texTable *next)
{	
	uchar    *tempBuff = NULL, *buff = NULL, *cmdBuff = NULL, *RGBABuff = NULL, *outBuff = NULL;
	ulong    originalLen = imgHdr.originalLen, originalCmdLen = imgHdr.originalCmdLen, outLen;
	int      err;
	texTable *ret = NULL;
	
	smalloc(tempBuff, imgHdr.compressedLen);
	smalloc(buff, imgHdr.originalLen);
	read(fd, tempBuff, imgHdr.compressedLen);
	err = uncompress(buff, &originalLen, tempBuff, imgHdr.compressedLen);
	if(err != Z_OK)
	{
		printf("uncompress error on buff\n");
		printf("%d %d %d %d\n", err, Z_BUF_ERROR, Z_MEM_ERROR, Z_DATA_ERROR);
		return NULL;
	}
	free(tempBuff); tempBuff = NULL;

	smalloc(tempBuff, imgHdr.cmdLen);
	smalloc(cmdBuff, imgHdr.originalCmdLen);
	read(fd, tempBuff, imgHdr.cmdLen);
	err = uncompress(cmdBuff, &originalCmdLen, tempBuff, imgHdr.cmdLen);
	if(err != Z_OK)
	{
		printf("uncompress error on cmdBuff\n");
		printf("%d %d %d %d\n", err, Z_BUF_ERROR, Z_MEM_ERROR, Z_DATA_ERROR);
		return NULL;
	}
	free(tempBuff); tempBuff = NULL;

	outBuff = unRLE(buff, originalLen, cmdBuff, originalCmdLen, &outLen);
	smalloc(RGBABuff, outLen);
	unDelta(outBuff, outLen, stdInfo.width, stdInfo.height, stdInfo.bpp / 8, RGBABuff);

	free(buff);
	free(cmdBuff);
	free(outBuff);

	ret = addTex(next, texName, texIndex, stdInfo.width, stdInfo.height, stdInfo.bpp / 8, RGBABuff);
	free(RGBABuff);

	return ret;
}

uchar *unRLE(uchar *buff, ulong buffLen, uchar *cmdBuff, ulong cmdLen, ulong *outLen)
{
	bitBuffer *bitBuff = NULL;
	uchar copyFlag = 0;
	ulong n = 0, i = 0;
	uchar *outBuff = NULL;
	
	bitBuff  = bitBuffer_new(cmdBuff, cmdLen);
	copyFlag = getBit(bitBuff);

	*outLen = getEliasGamma(bitBuff);
	smalloc(outBuff, *outLen);
	for(i = 0; i < *outLen; i += n)
	{
		n = getEliasGamma(bitBuff);
		if(copyFlag)
		{
			memcpy(outBuff + i, buff, n);
			buff += n;
		} else
			memset(outBuff + i, 0, n);
		copyFlag = !copyFlag;
	}
	free(bitBuff);

	return outBuff;
}

inline uchar unpackValue(uchar c)
{
    uchar z = c & 1 ? 0xFF : 0x00;
    return (c >> 1) ^ z;
}

void unDelta(uchar *buff, ulong buffLen, ulong width, ulong height, ulong Bpp, uchar *RGBABuff)
{
#include "tables.h"
	ulong sectLen = buffLen / 4;
	ulong lineLen = width * Bpp;
	ulong x, y, val;
	uchar *sect1  = buff;
	uchar *sect2  = sect1 + sectLen;
	uchar *sect3  = sect2 + sectLen;
	uchar *sect4  = sect3 + sectLen;
	uchar *outPtr = RGBABuff;
	uchar *outEnd = RGBABuff + buffLen;
	uchar *line   = NULL;
	uchar *prev   = NULL;

	while(outPtr < outEnd)
	{
		val = table1[*sect1++] | table2[*sect2++] | table3[*sect3++] | table4[*sect4++];
		*outPtr++ = unpackValue((uchar)(val >>  0));
		*outPtr++ = unpackValue((uchar)(val >>  8));
		*outPtr++ = unpackValue((uchar)(val >> 16));
		*outPtr++ = unpackValue((uchar)(val >> 24));
	}
	for(x = Bpp; x < lineLen; x++)
		RGBABuff[x] += RGBABuff[x - Bpp];
	for(y = 1; y < height; y++)
	{
		line = RGBABuff + y * lineLen;
		prev = RGBABuff + (y - 1) * lineLen;
		for(x = 0; x < lineLen; x++)
			line[x] += prev[x];
	}
}

#undef smalloc

