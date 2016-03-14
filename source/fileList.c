#include "fileList.h"

fileList *addFile(fileList *next, char *fileName, char *intArchive, ulong intOffset, ulong length)
{
	fileList *ret = malloc(sizeof(fileList));
	if(!ret)
	{
		printf("Couldn't malloc fileList.\n");
		return NULL;
	}
	if(next)
		next->prev = ret;

	ret->prev = NULL;
	ret->next = next;
	ret->fileName = strdup(fileName);
	ret->intArchive = strdup(intArchive);
	ret->intOffset = intOffset;
	ret->length = length;
	return ret;
}

fileList *findFile(fileList *list, char *fileName)
{
	rwdFileList(list);
	do {
		if(!strcmp(fileName, list->fileName))
			return list;
	} while((list = list->next));
	return NULL; //Weren't able to find anything
}

int getFD(fileList *list, char *fileName)
{
	int ret;
	if((list = findFile(list, fileName)))
	{
		ret = open(list->intArchive, O_RDONLY | O_BINARY);
		lseek(ret, list->intOffset, SEEK_SET);
		return ret;
	}
	return 0;
}

fileList *catFileList(fileList *prev, fileList *next)
{
	fwdFileList(prev);
	rwdFileList(next);
	prev->next = next;
	next->prev = prev;
	rwdFileList(prev);
	return prev;
}

fileList *fileListSeek(fileList *list, long offset, int whence)
{
	long i;
	switch(whence)
	{
		case SEEK_CUR:
			break;
		case SEEK_SET:
			rwdFileList(list);
			break;
		case SEEK_END:
			fwdFileList(list);
			break;
		default:
			break;
	}
	if(offset > 0)
		for(i = 0; i < offset; i++)
		{
			if(!list->next)
				return list;
			list = list->next;
		}
	if(offset < 0)
		for(i = 0; i > offset; i--)
		{
			if(!list->prev)
				return list;
			list = list->prev;
		}
	return list;
}

void freeFileList(fileList *list)
{
	fileList *next;
	while(list)
	{
		next = list->next;
		free(list->fileName);
		free(list->intArchive);
		free(list);
		list = next;
	}
}

