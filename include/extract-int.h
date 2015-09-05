#ifndef EXTRACT_INT_H
#define EXTRACT_INT_H
	#include "mt.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <fcntl.h>
	#include <unistd.h>
	unsigned long generate_index_seed(const char* game_id);
	void unobfuscate_filename(char *obfuscated, unsigned long seed);
	void file_list(char* archive_name, char* game_id);
#endif //EXTRACT_INT_H
