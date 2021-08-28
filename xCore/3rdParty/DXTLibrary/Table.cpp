/*

	Table.cpp - Support Functions for Table template class

*/

#include "x_files.hpp"
#include <assert.h>
#include <memory.h>
#include <string.h>

#include "Table.h"

//  #define MIN(a,b) ((a)<(b)?(a):(b))

static bool Resize(TableHeader **ppTabHdr, int num, int elsize)
{
	if(num == 0)
	{
		if(*ppTabHdr)
			delete [] (char *)(*ppTabHdr);
		*ppTabHdr = 0;
		return true;
	}

	if(*ppTabHdr == NULL)
	{
		*ppTabHdr = (TableHeader *)new char[sizeof(TableHeader) + elsize*num];
		if(*ppTabHdr == 0) return false;

		(*ppTabHdr)->count = 0;
		memset((*ppTabHdr),0,sizeof(TableHeader) + elsize*num);
	}
	else
	{
		TableHeader* tmp = (TableHeader *)new char[sizeof(TableHeader)+elsize*num];
		if(tmp == 0) return false;

		memset(tmp, 0, sizeof(TableHeader) + elsize*num);
		memcpy(tmp, *ppTabHdr, sizeof(TableHeader) + elsize*(MIN((*ppTabHdr)->nalloc, num)));

		delete [] (char *)(*ppTabHdr);
		*ppTabHdr = tmp;
	}
	return true;
}

long TableMakeSize(TableHeader **ppTabHdr, int num, int elsize)
{ 
	if(!Resize(ppTabHdr, num, elsize))
		return 0;

	TableHeader *tb = *ppTabHdr;
	if(tb)
	{
		tb->nalloc = num;
		tb->count = MIN(tb->count, tb->nalloc);
	}
	return(1);
}

void TableSetCount(TableHeader **ppTabHdr, int n, int elsize)
{
TableHeader *tb = *ppTabHdr;

	if(n == 0)
	{
		if (tb) tb->count = 0;
		return;
	}

	if(tb == NULL || tb->nalloc < n)
	{
		TableMakeSize(ppTabHdr, n, elsize);
		tb = *ppTabHdr;
		tb->count = n;
	}
	else 
		tb->count = n;
}

#define PTR(i) ((char *)tb+sizeof(TableHeader)+(i)*elsize)

long TableInsertAt(TableHeader **ppTabHdr, int at, int num, void *el, int elsize, int extra)
{
TableHeader *tb = *ppTabHdr;
int oldCount;

	if(num == 0)
		return(at);

	if(tb == NULL)
	{
		assert(at == 0);
		oldCount=0;
		Resize(ppTabHdr, num+extra, elsize);
		if (ppTabHdr == NULL) return(-1);
		tb = *ppTabHdr;
		tb->nalloc = num+extra;
	}
	else if ((tb->count+num) > tb->nalloc)
	{
		oldCount = tb->count;
		assert(at <= oldCount);
		int newnum = tb->count + num + extra;
		Resize(ppTabHdr, newnum, elsize);
		if (ppTabHdr == 0) return(-1);
		tb = *ppTabHdr;
		tb->nalloc = newnum;
	}
	else
	{
		oldCount = tb->count;
		assert(at <= oldCount);
	}

	if (at < oldCount)
	{
		int nmove = oldCount-at;
		memmove(PTR(at+num), PTR(at), nmove*elsize);
	}

	memmove(PTR(at), el, num*elsize);
	tb->count += num;

	return(at);
}

long TableCopy(TableHeader **ppTabHdr, int at, int num, void *el, int elsize)
{ 
	TableMakeSize(ppTabHdr, num, elsize);
	(*ppTabHdr)->count = 0;
	return(TableInsertAt(ppTabHdr, 0, num, el, elsize,0));
}

long TableDelete(TableHeader **ppTabHdr, int starting, int num, int elsize)
{
TableHeader *tb = *ppTabHdr;

	if(tb == NULL) return 0;

	if(starting < tb->count)
	{
		if ((starting + num) > tb->count)
			num = tb->count - starting;
		else 
			memmove(PTR(starting), PTR(starting + num), (tb->count - starting - num)*elsize);
		tb->count -= num;
	}	
	return(tb->count);
} 
