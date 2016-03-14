#include "stringList.h"

stringList *addString(stringList *next, stringList *prev, ulong type, char *str)
{
	stringList *ret = NULL;
	ret = malloc(sizeof(stringList));
	if(!ret)
	{
		printf("Couldn't malloc stringList.\n");
		return NULL;
	}
	ret->next = next;
	ret->prev = prev;
	ret->type = type;
	ret->str  = strdup(str);

	return ret;
}

stringList *catStringList(stringList *prev, stringList *next)
{
	fwdStringList(prev);
	rwdStringList(next);
	prev->next = next;
	next->prev = prev;
	rwdStringList(prev);
	return prev;
}

stringList *stringListSeek(stringList *list, long offset, int whence)
{
	long i;
	switch(whence)
	{
		case SEEK_CUR:
			break;
		case SEEK_SET:
			rwdStringList(list);
			break;
		case SEEK_END:
			fwdStringList(list);
			break;
		default:
			break;
	}
	if(offset > 0)
		for(i = 0; i < offset; i++)
		{
			if(!list->next)
				return list;
			list = list->next;
		}
	if(offset < 0)
		for(i = 0; i > offset; i--)
		{
			if(!list->prev)
				return list;
			list = list->prev;
		}
	return list;
}

void freeStringList(stringList *list)
{
	stringList *next;
	while(list)
	{
		next = list->next;
		free(list->str);
		free(list);
		list = next;
	}
}

