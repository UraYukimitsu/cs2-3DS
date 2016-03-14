#ifndef EXTRACT_INT_H_
#define EXTRACT_INT_H_

//#include "mt.h"
//#include "blowfish.h"
#include "fileList.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
//#define MAGIC 0x4C11DB7

//static const char* DEFAULT_ID = "TSUNABAN-LM";

typedef struct _ArchiveHeader {
	char          magic[4]; //{'K', 'I', 'F', '\0'}
	unsigned long entries;
} ArchiveHeader;

typedef struct _Entry {
	char          filename[64];
	unsigned long offset;
	unsigned long fileSize;
} Entry;

//unsigned long generateIndexSeed(const char* gameID);
//void unobfuscateFilename(char *obfuscated, unsigned long seed);
//void listInt(char* archiveName, char* gameID);
fileList *listDecryptedInt(char *filename);

#endif //EXTRACT_INT_H_

