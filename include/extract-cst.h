#ifndef EXTRACT_CST_H
#define EXTRACT_CST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include "basicIO.h"
	
unsigned int* makeOffsetList(unsigned char *src, unsigned int indexOff, unsigned int baseOff);
void dumpString(unsigned char* ret, unsigned char *src, unsigned int offset);
char **makeStringList(unsigned char *src, unsigned int *offsets);
void openCst(char *filename, char *output);
void openFes(char *filename, char *output);

#endif //EXTRACT_CST_H

