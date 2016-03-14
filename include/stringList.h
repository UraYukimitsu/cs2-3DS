#ifndef _STRINGLIST_H
#define _STRINGLIST_H

#include "cs2.h"

typedef struct stringList_ {
	ulong type;
	char *str;
	struct stringList_ *prev;
	struct stringList_ *next;
} stringList;

stringList *addString(stringList *next, stringList *prev, ulong type, char *str);
stringList *catStringList(stringList *prev, stringList *next);
stringList *stringListSeek(stringList *list, long offset, int whence);
void freeStringList(stringList *list);

#define rwdStringList(l) while(l->prev) l = l->prev;
#define fwdStringList(l) while(l->next) l = l->next;

#endif //_STRINGLIST_H

