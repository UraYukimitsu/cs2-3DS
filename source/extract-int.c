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
        printf("REV = %c\nPTR = %c\n", reversed[index2 % 52], (*ptr));
		while(reversed[index2 % 52] != (*ptr))
		{
		    printf("REV = %c\nPTR = %c\n", reversed[index2 % 52], (*ptr));
			if(reversed[(shift + index + 1) % 52] == (*ptr))
			{
			    printf("1 : %c\np : %c\n\n", reversed[(shift + index + 1) % 52], (*ptr));
			    index += 1;
				break;
			}
			if(reversed[(shift + index + 2) % 52] == (*ptr))
			{
			    printf("2 : %c\np : %c\n\n", reversed[(shift + index + 2) % 52], (*ptr));
			    index += 2;
				break;
			}
			if(reversed[(shift + index + 3) % 52] == (*ptr))
			{
			    printf("3 : %c\np : %c\n\n", reversed[(shift + index + 3) % 52], (*ptr));
			    index += 3;
				break;
			}

			index  += 4;
			index2 += 4;

			if(index > 52){
                printf("Index = %d\n", index);
                break;
            }
		}

		if(index < 52)
			*ptr = alphabet[index];
		shift++;
	}
}

void file_list(char* archive_name, char* game_id)
{
	int file_handle, do_decrypt = 0, write_handle;
	unsigned long i, index_seed, len, file_key = 0, j;
	unsigned char *buff = NULL, *debug = NULL;
	char outStr[512];
	ArchiveHeader hdr;
	Blowfish bf;
	Entry *entries = NULL;
	printf("Listing files contained in %s using the id %s...\n", archive_name, game_id);
	fflush(stdout);

	debug = malloc(sizeof(Entry) + 16);

	file_handle = open(archive_name, O_RDONLY | O_BINARY);
	if(!file_handle)
	{
		fprintf(stderr, "Could not open %s.\n", archive_name);
		exit(1);
	}

	read(file_handle, &hdr, sizeof(hdr));
	entries = malloc(sizeof(Entry) * hdr.entries);
	printf("File contains %u entries", hdr.entries);
	read(file_handle, entries, sizeof(Entry) * hdr.entries);

	index_seed = generate_index_seed(game_id);

	for(i = 0; i < hdr.entries; i++)
	{
		printf("%s : %u\n", entries[i].filename, entries[i].file_sz);
		if(!strcmp(entries[i].filename, "__key__.dat"))
		{
			mt_sgenrand(entries[i].file_sz);
			file_key = mt_genrand();
			printf("mt seed: %08x\nfile_key: %08x\n", entries[i].file_sz, file_key);
			do_decrypt = 1;
			break;
		}
	}

	if(!do_decrypt)
	{
		printf("%s: no key information found, assuming not encrypted.\n", archive_name);
		fflush(stdout);
	}

	for(i = 0; i < hdr.entries; i++)
	{
        printf("%s\n", entries[i].filename);

	    memcpy(debug, &entries[i], sizeof(Entry) + 16);
	    for(j = 0; j < sizeof(Entry) + 16; j += 16)
            printf("%04x | %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x | %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", j,
                   debug[j], debug[j+1], debug[j+2], debug[j+3],
                   debug[j+4], debug[j+5], debug[j+6], debug[j+7],
                   debug[j+8], debug[j+9], debug[j+10], debug[j+11],
                   debug[j+12], debug[j+13], debug[j+14], debug[j+15],
                   debug[j], debug[j+1], debug[j+2], debug[j+3],
                   debug[j+4], debug[j+5], debug[j+6], debug[j+7],
                   debug[j+8], debug[j+9], debug[j+10], debug[j+11],
                   debug[j+12], debug[j+13], debug[j+14], debug[j+15]);

		if(!strcmp(entries[i].filename, "__key__.dat"))
			continue;

		if(do_decrypt)
		{
		    printf("Filename at %08x\n", &entries[i].filename);
			unobfuscate_filename(entries[i].filename, index_seed + i);
			entries[i].offset += i;
			printf("%s\n", entries[i].filename);
			bf_initialize(&bf, (unsigned char*) &file_key, 4);
			bf_decrypt(&bf, (unsigned char*) &entries[i].offset, 8);
		}

		len = entries[i].file_sz;
		printf("Allocating %u (%08x) bytes.\n", len, len);
		buff = malloc(len * sizeof(unsigned char));

        if(buff == NULL)
        {
            printf("Couldn't allocate buffer.");
            exit(1);
        }

		lseek(file_handle, entries[i].offset, SEEK_SET);

		read(file_handle, buff, len);

		if(do_decrypt)
        {
            bf_initialize(&bf, (unsigned char*) &file_key, 4);
			bf_decrypt(&bf, buff, (len / 8) * 8);
        }

        sprintf(outStr, "out/%s", entries[i].filename);
        write_handle = open(outStr , O_WRONLY | O_TRUNC | O_CREAT | O_BINARY);
        write(write_handle, buff, len);

		free(buff);
		buff = NULL;
		printf("Freed buff : %d\n", buff);
		//getc(stdin);
	}

	free(entries);
}
