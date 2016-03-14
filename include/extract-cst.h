#ifndef EXTRACT_CST_H
#define EXTRACT_CST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include "basicIO.h"
	
unsigned long *makeOffsetList(unsigned char *src, unsigned long indexOff, unsigned long baseOff);
char *dumpString(unsigned char *src, unsigned long offset);
char **makeStringList(unsigned char *src, unsigned long *offsets);
void openCst(char *filename, char *output);
void openFes(char *filename, char *output);

#endif //EXTRACT_CST_H

