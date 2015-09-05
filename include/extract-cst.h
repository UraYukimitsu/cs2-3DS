#ifndef EXTRACT_CST_H
#define EXTRACT_CST_H
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <zlib.h>
	#include "basicIO.h"
	#define read_buf(inbuf, outbuf, length) memcpy(outbuf, inbuf, length)
	unsigned int* make_offset_list(unsigned char *src, unsigned int idx_off, unsigned int base_off);
	void dumpstr(unsigned char* ret, unsigned char *src, unsigned int offset);
	char **make_string_list(unsigned char *src, unsigned int *offsets);
	void open_cst(char *filename, char *output);
	void open_fes(char *filename, char *output);
#endif //EXTRACT_CST_H
