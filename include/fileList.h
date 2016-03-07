#ifndef _FILELIST_H
#define _FILELIST_H

#include "cs2.h"

typedef struct _fileList {
	char  *fileName;
	char  *intArchive;
	ulong intOffset;
	ulong length;
	struct _fileList *prev;
	struct _fileList *next;
} fileList;

fileList *addFile(fileList *next, char *fileName, char *intArchive, ulong intOffset, ulong length);
fileList *findFile(fileList *list, char *fileName);
int getFD(fileList *list, char *fileName);
fileList *catFileList(fileList *prev, fileList *next);
fileList *fileListSeek(fileList *list, long offset, int whence);
void freeFileList(fileList *list);

#define rwdFileList(l) while(l->prev) l = l->prev;
#define fwdFileList(l) while(l->next) l = l->next;

#endif //_FILELIST_H

