#include "extract-int.h"

unsigned long generateIndexSeed(const char* gameID)
{
	unsigned long ret = -1, i;
	unsigned char *ptr = NULL;

	for(ptr = (unsigned char*) gameID; *ptr; ptr++) //Parsing the string
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

void unobfuscateFilename(char *obfuscated, unsigned long seed)
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

void listInt(char* archiveName, char* gameID)
{
	int fd, doDecrypt = 0, fdOut;
	unsigned long i, indexSeed, len, fileKey = 0, j;
	unsigned char *buff = NULL;
	char outStr[512];
	ArchiveHeader hdr;
	Blowfish bf;
	Entry *entries = NULL;
	printf("Listing files contained in %s using the id %s...\n", archiveName, gameID);
	fflush(stdout);

	fd = open(archiveName, O_RDONLY | O_BINARY);
	if(!fd)
	{
		fprintf(stderr, "Could not open %s.\n", archiveName);
		exit(1);
	}

	read(fd, &hdr, sizeof(hdr));
	entries = malloc(sizeof(Entry) * hdr.entries);
	printf("File contains %u entries", hdr.entries);
	read(fd, entries, sizeof(Entry) * hdr.entries);

	indexSeed = generateIndexSeed(gameID);

	for(i = 0; i < hdr.entries; i++)
	{
		printf("%s : %u\n", entries[i].filename, entries[i].fileSize);
		if(!strcmp(entries[i].filename, "__key__.dat"))
		{
			mt_sgenrand(entries[i].fileSize);
			fileKey = mt_genrand();
			printf("mt seed: %08x\nfile_key: %08x\n", entries[i].fileSize, fileKey);
			doDecrypt = 1;
			break;
		}
	}

	if(!doDecrypt)
	{
		printf("%s: no key information found, assuming not encrypted.\n", archiveName);
		fflush(stdout);
	}

	for(i = 0; i < hdr.entries; i++)
	{
		printf("%s\n", entries[i].filename);

		
		if(!strcmp(entries[i].filename, "__key__.dat"))
			continue;

		if(doDecrypt)
		{
			printf("Filename at %08x\n", &entries[i].filename);
			unobfuscateFilename(entries[i].filename, indexSeed + i);
			entries[i].offset += i;
			printf("%s\n", entries[i].filename);
			bf_initialize(&bf, (unsigned char*) &fileKey, 4);
			bf_decrypt(&bf, (unsigned char*) &entries[i].offset, 8);
		}

		len = entries[i].fileSize;
		printf("Allocating %u (%08x) bytes.\n", len, len);
		buff = malloc(len * sizeof(unsigned char));

		if(buff == NULL)
		{
			printf("Couldn't allocate buffer.");
			exit(1);
		}

		lseek(fd, entries[i].offset, SEEK_SET);

		read(fd, buff, len);

		if(doDecrypt)
		{
			bf_initialize(&bf, (unsigned char*) &fileKey, 4);
			bf_decrypt(&bf, buff, (len / 8) * 8);
		}

		sprintf(outStr, "out/%s", entries[i].filename);
		fdOut = open(outStr , O_WRONLY | O_TRUNC | O_CREAT | O_BINARY);
		write(fdOut, buff, len);

		free(buff);
		buff = NULL;
		printf("Freed buff : %d\n", buff);
	}

	free(entries);
}

fileList *listDecryptedInt(char *filename)
{
	fileList *ret = NULL;
	ArchiveHeader hdr;
	Entry entry;
	ulong i;
	int fd;

	fd = open(filename, O_RDONLY | O_BINARY);
	read(fd, &hdr, sizeof(hdr));
	if(strcmp(hdr.magic, "KIF"))
	{
		printf("Incorrect INT archive. Expected magic 'KIF', was %s\n.", hdr.magic);
		return NULL;
	}
	for(i = 0; i < hdr.entries; i++)
	{
		read(fd, &entry, sizeof(entry));
		if(!strcmp(entry.filename, "__key__.dat"))
		{
			printf("Encrypted INT archive. Aborting...\n");
			freeFileList(ret); //In case there were files before the key entry
			return NULL;
		}
		ret = addFile(ret, entry.filename, filename, entry.offset, entry.fileSize);
	}
	return ret;
}

