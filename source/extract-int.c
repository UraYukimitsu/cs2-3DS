#include "extract-int.h"

/*******************************
 * Constants definitions start *
 ******************************/

#define MAGIC 0x4C11DB7
static const char* DEFAULT_ID = "TSUNABAN-LM";

/*****************************
 * Constants definitions end *
 ****************************/


/*************************************
 * Structure types definitions start *
 ************************************/
 
typedef struct ArchiveHeader {
	unsigned char sig[4]; //{'K', 'I', 'F', '\0'}
	unsigned long entries;
} ArchiveHeader;
	
typedef struct Entry {
	char          filename[64];
	unsigned long offset;
	unsigned long file_sz;
} Entry;

/***********************************
 * Structure types definitions end *
 **********************************/
 
 
/*******************************
 * Functions definitions start *
 ******************************/

unsigned long generate_index_seed(const char* game_id)
{
	unsigned long ret = -1, i;
	unsigned char *ptr = NULL;
	
	for(ptr = (unsigned char*) game_id; *ptr; ptr++) //Parsing the string
	{
		ret ^= ((unsigned long) *ptr) << 24;
		
		for(i = 0; i < 8; i++)
		{
			if(ret & 0x80000000)
			{
				ret *= 2;
				ret ^= MAGIC;
			} else {
				ret *= 2;
			}
		}
		ret = ~ret;
	}
	
	return ret;
}

void unobfuscate_filename(char *obfuscated, unsigned long seed)
{
	static unsigned char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	static unsigned char reversed[] = "zyxwvutsrqponmlkjihgfedcbaZYXWVUTSRQPONMLKJIHGFEDCBA";
	unsigned long skey, shift, index, index2;
	unsigned char *ptr = NULL;
	
	mt_sgenrand(seed);
	skey = mt_genrand();
	shift = (unsigned char) ((skey >> 24) + (skey >> 16) + (skey >> 8) + skey);
	
	for(ptr = (unsigned char*) obfuscated; *ptr; ptr++) //Parsing the string
	{
		index = 0;
		index2 = shift;
		
		while(reversed[index2 % 52] != *ptr)
		{
			if(reversed[(shift + index + 1) % 52] == *ptr)
				index += 1; break;
			if(reversed[(shift + index + 2) % 52] == *ptr)
				index += 2; break;
			if(reversed[(shift + index + 3) % 52] == *ptr)
				index += 3; break;
				
			index  += 4;
			index2 += 4;
			
			if(index > 52)
				break;
		}
		
		if(index > 52)
			*ptr = alphabet[index];
		shift++;
	}
}

void file_list(char* archive_name, char* game_id)
{
	int file_handle, do_decrypt = 0;
	unsigned long i, index_seed, len, file_key = 0;
	unsigned char *buff = NULL;
	ArchiveHeader hdr;
	Blowfish *bf = NULL;
	Entry* entries = NULL;
	printf("Listing files contained in %s using the id %s...\n", archive_name, game_id);
	fflush(stdout);
	
	file_handle = open(archive_name, O_RDONLY | O_BINARY);
	if(!file_handle)
	{
		fprintf(stderr, "Could not open %s.\n", archive_name);
		exit(1);
	}
	
	read(file_handle, &hdr, sizeof(hdr));
	entries = malloc(sizeof(Entry) * hdr.entries);
	read(file_handle, entries, sizeof(Entry) * hdr.entries);
	
	index_seed = generate_index_seed(game_id);
	
	for(i = 0; i < hdr.entries; i++)
	{
		//printf("%s\n", entries[i].filename);
		if(!strcmp(entries[i].filename, "__key__.dat"))
		{
			mt_sgenrand(entries[i].file_sz);
			file_key = mt_genrand();
			do_decrypt = 1;
			break;
		}
	}
	
	if(!do_decrypt)
	{
		printf("%s: no key information found, assuming not encrypted.\n", archive_name);
		fflush(stdout);
	} else
		bf_initialize(bf, (unsigned char*) &file_key, 4);
	
	for(i = 0; i < hdr.entries; i++)
	{
		if(!strcmp(entries[i].filename, "__key__.dat"))
			continue;
		
		if(do_decrypt)
		{
			unobfuscate_filename(entries[i].filename, index_seed + i);
			entries[i].offset += i;
			printf("%s\n", entries[i].filename);
			bf_decrypt(bf, (unsigned char*) &entries[i].offset, 8);
		}
		
		len = entries[i].file_sz;
		buff = malloc(len * sizeof(unsigned char));
		
		lseek(file_handle, entries[i].offset, SEEK_SET);
		
		read(file_handle, buff, len);
		
		if(do_decrypt)
			bf_decrypt(bf, buff, (len / 8) * 8);
		
		free(buff);
	}
	
	free(entries);
}






























