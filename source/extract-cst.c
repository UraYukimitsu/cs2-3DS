#include "extract-cst.h"

unsigned int* makeOffsetList(unsigned char *src, unsigned int indexOff, unsigned int baseOff)
{
	unsigned int count, i, ptr, *ret;

	ptr = indexOff;
	count = (baseOff - indexOff) / 4;
	ret = malloc((count + 1) * sizeof(unsigned int));
 	for(i = 0; i < count; i++)
	{
		memcpy(ret + i, (src + ptr), 4);
		ret[i] += baseOff;
		ptr += 4;
	}

	ret[count] = 0;
	return ret;
}

char *dumpString(unsigned char *src, unsigned int offset)
{
	int i;
	unsigned int len = 0;
	char *ret = NULL;

	for(len = 0; src[len + offset]; len++) continue;
	ret = malloc(len + 1);

	for(i = 0; src[i + offset]; i++)
		ret[i] = src[i + offset];
	ret[i + 1] = 0;
	ret[i] = '\n';

	return ret;
}

char **makeStringList(unsigned char *src, unsigned int *offsets)
{
	unsigned int i;
	char **ret;
	for(i = 0; offsets[i]; i++) continue;
	ret = malloc((i + 1) * sizeof(char*));
	for(i = 0; offsets[i]; i++)
		ret[i] = dumpString(src, offsets[i] + 2);
	ret[i] = NULL;
	return ret;
}

void openCst(char *filename, char *output) //CST files are strings with an offset index compressed using zlib.
{
	int fd, indexOffset, baseOffset;
	unsigned int compressedLen, decompressedLen, *offsetList = NULL, i, j, tempStorage;
	char magic[9], **stringList = NULL;
	unsigned char *compressedBuff = NULL, *decompressedBuff = NULL;

	fd = open(filename, O_RDONLY | O_BINARY);
	magic[8] = 0;
	read(fd, &magic, 8);
	if(strcmp(magic, "CatScene"))
	{
		printf("Invalid cst file, magic does not match.\nExpecting CatScene, was %s", magic);
		return;
	}

	read(fd, &compressedLen, 4);
	read(fd, &decompressedLen, 4);

	compressedBuff = malloc(compressedLen);
	decompressedBuff = malloc(decompressedLen);
	read(fd, compressedBuff, compressedLen);
	j = decompressedLen;
	i = uncompress(decompressedBuff, &decompressedLen, compressedBuff, compressedLen);
	if(i != Z_OK)
	{
		printf("Error: decompression error (%d)", i);
		return;
	}
	if(decompressedLen != j)
	{
		printf("Error: decompression error.\nExpected decompressed size: %d\nActual decompressed size: %d\n", j, decompressedLen);
		return;
	}

	memcpy(&tempStorage, (decompressedBuff + 8), 4);
	indexOffset = 0x10 + tempStorage;
	memcpy(&tempStorage, (decompressedBuff + 12), 4);
	baseOffset = 0x10 + tempStorage;

	offsetList = makeOffsetList(decompressedBuff, indexOffset, baseOffset);

	stringList = makeStringList(decompressedBuff, offsetList);

	close(fd);
	fd = open(output, O_WRONLY | O_TRUNC | O_CREAT);
	for(i = 0; stringList; i++)
		write(fd, stringList[i], strlen(stringList[i]));
	close(fd);

	free(compressedBuff);
	free(decompressedBuff);
	free(offsetList);
	free(stringList);
}

void openFes(char *filename, char *output) //FES files are just plain compressed text with a small header.
{
	int fd;
	unsigned int compressedLen, decompressedLen, i, j, tempStorage;
	char magic[4];
	unsigned char *compressedBuff, *decompressedBuff;

	fd = open(filename, O_RDONLY | O_BINARY);
	read(fd, &magic, 4);
	if(strcmp(magic, "FES"))
	{
		printf("Invalid fes file, magic does not match.\nExpecting FES, was %s", magic);
		return;
	}

	read(fd, &compressedLen, 4);
	read(fd, &decompressedLen, 4);
	read(fd, &tempStorage, 4); //4 unknown bytes
	compressedBuff = malloc(compressedLen);
	decompressedBuff = malloc(decompressedLen);
	read(fd, compressedBuff, compressedLen);
	j = decompressedLen;
	i = uncompress(decompressedBuff, &decompressedLen, compressedBuff, compressedLen);
	if(i != Z_OK)
	{
		printf("Error: decompression error (%d)", i);
		return;
	}
	if(decompressedLen != j)
	{
		printf("Error: decompression error.\nExpected decompressed size: %d\nActual decompressed size: %d\n", j, decompressedLen);
		return;
	}	
	close(fd);

	i = 0;
   	fd = open(output, O_WRONLY | O_TRUNC | O_CREAT);
	write(fd, decompressedBuff, decompressedLen);
	close(fd);

	free(compressedBuff);
	free(decompressedBuff);
}

