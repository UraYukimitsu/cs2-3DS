#include "extract-cst.h"

unsigned int* make_offset_list(unsigned char *src, unsigned int idx_off, unsigned int base_off)
{
	unsigned int count, i, ptr, *ret;

	ptr = idx_off;
	count = (base_off - idx_off) / 4;
	ret = malloc((count + 1) * sizeof(unsigned int));
    //printf("%d\n%08x\n", count, ret);
	for(i = 0; i < count; i++)
	{
		read_buf((src + ptr), &ret[i], 4);
		ret[i] += base_off;
		//printf("Offset %d: %08x\n", i, ret[i]);
		ptr += 4;
	}

	ret[count] = 0;
	return ret;
}

void dumpstr(unsigned char* ret, unsigned char *src, unsigned int offset)
{
	int i;
	i = 0;
	while(src[i + offset])
	{
		*(ret + i) = src[i + offset];
		i++;
	}
	*(ret + i + 1) = 0;
	*(ret + i) = '\n';
}

char **make_string_list(unsigned char *src, unsigned int *offsets)
{
	unsigned int i;
	char **ret;
	i = 0;
	while(offsets[i])
		i++;
	//printf("%d\n", i);
	ret = malloc((i + 2) * sizeof(char*));
	i = 0;
	while(offsets[i])
	{
		ret[i] = malloc(2048 * sizeof(char));
		dumpstr(ret[i], src, offsets[i] + 2);
		//printf("%05d:%s\n", i, ret[i]);
		i++;
	}
	ret[i] = malloc(2048 * sizeof(char));
	ret[i][0] = 0;
	return ret;
}

void open_cst(char *filename, char *output) //CST files are strings with an offset index compressed using zlib.
{
	int file_handle, index_off, base_off;
	unsigned int compressed_len, decompressed_len, *offset_list, i, j, tempStorage;
	char magic[9], **string_list;
	unsigned char *comp_buff, *decomp_buff;

	string_list = NULL;
	offset_list = NULL;

	file_handle = open(filename, O_RDONLY | O_BINARY);
	if(!file_handle)
	{
		printf("Could not open %s.\n", filename);
		exit(1);
	}
	magic[8] = 0;
	read(file_handle, &magic, 8);
	if(strcmp(magic, "CatScene"))
	{
		printf("Invalid cst file, magic does not match.\nExpecting CatScene, was %s", magic);
		return;
	}

	read(file_handle, &compressed_len, 4);
	read(file_handle, &decompressed_len, 4);

	comp_buff = malloc(compressed_len);
	decomp_buff = malloc(decompressed_len);
	read(file_handle, comp_buff, compressed_len);
	j = decompressed_len;
	i = uncompress(decomp_buff, &decompressed_len, comp_buff, compressed_len);
	if(i != Z_OK)
	{
		printf("Error: decompression error (%d)", i);
		return;
	}
	if(decompressed_len != j)
	{
		printf("Error: decompression error.\nExpected decompressed size: %d\nActual decompressed size: %d\n", j, decompressed_len);
		return;
	}

	read_buf((decomp_buff + 8), &tempStorage, 4);
	index_off = 0x10 + tempStorage;
	read_buf((decomp_buff + 12), &tempStorage, 4);
	base_off = 0x10 + tempStorage;

	offset_list = make_offset_list(decomp_buff, index_off, base_off);

	string_list = make_string_list(decomp_buff, offset_list);

	close(file_handle);

    i = 0;
    file_handle = open(output, O_WRONLY | O_TRUNC | O_CREAT);
    while(string_list[i][0])
    {
        write(file_handle, string_list[i], strlen(string_list[i]));
        i++;
    }
    close(file_handle);

	free(comp_buff);
	free(decomp_buff);
	free(offset_list);
	free(string_list);
}

void open_fes(char *filename, char *output) //FES files are just plain compressed text with a small header.
{
	int file_handle;
	unsigned int compressed_len, decompressed_len, i, j, tempStorage;
	char magic[4];
	unsigned char *comp_buff, *decomp_buff;

	file_handle = open(filename, O_RDONLY | O_BINARY);
	if(!file_handle)
	{
		printf("Could not open %s.\n", filename);
		exit(1);
	}

	read(file_handle, &magic, 4);
	if(strcmp(magic, "FES"))
	{
		printf("Invalid fes file, magic does not match.\nExpecting FES, was %s", magic);
		return;
	}

	read(file_handle, &compressed_len, 4);
	read(file_handle, &decompressed_len, 4);
	read(file_handle, &tempStorage, 4); //4 useless bytes, data starts at 0x00000010

	comp_buff = malloc(compressed_len);
	decomp_buff = malloc(decompressed_len);
	read(file_handle, comp_buff, compressed_len);
	j = decompressed_len;
	i = uncompress(decomp_buff, &decompressed_len, comp_buff, compressed_len);
	if(i != Z_OK)
	{
		printf("Error: decompression error (%d)", i);
		return;
	}
	if(decompressed_len != j)
	{
		printf("Error: decompression error.\nExpected decompressed size: %d\nActual decompressed size: %d\n", j, decompressed_len);
		return;
	}
	close(file_handle);

    i = 0;
    file_handle = open(output, O_WRONLY | O_TRUNC | O_CREAT);
    while(decomp_buff[i])
    {
        write(file_handle, &decomp_buff[i], 1);
        i++;
    }
    close(file_handle);

    free(comp_buff);
    free(decomp_buff);
}
