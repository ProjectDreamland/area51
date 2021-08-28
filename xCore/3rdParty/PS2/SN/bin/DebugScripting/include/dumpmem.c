//
// Example debugger script
//
// How to use scripts to fetch memory from the target console
//
#include <stdlib.h>
#include <stdio.h>
#include <SNscript.h>

//
// dump a single number in hexadecimal and advance the pointer
// size = 1,2,4 for byte, short, word etc.
// note: this example is little-endian only
//
void dumpnum(char** ppbuff, int size)
{
	int	i;
	switch(size)
	{
	case 1:	// default is to handle it as bytes
		printf("%02X", **ppbuff);
		(*ppbuff)++;
		break;
	case 2:
		printf("%04X", *(unsigned short*)(*ppbuff) );
		(*ppbuff)+=2;
		break;
	case 4:
		printf("%08X", *(int*)(*ppbuff) );
		(*ppbuff)+=4;
		break;
	default:	// anything else just prints as a list of bytes
		for(i=0; i<size; i++)
		{
			printf("%02X ", **ppbuff);
			(*ppbuff)++;
		}
		putchar(' ');
		break;
	}
}

int dumpmemline(SNADDR addr, int num, int size)
{
	char	ch;
	int		sizereq;
	static char* pbuff = NULL;
	static int	bufflen= 0;

	if(size > 4)
		size = 4;

	sizereq = num * size;

	if(bufflen < sizereq)
	{
		if(pbuff)
			free(pbuff);
		pbuff = malloc(sizereq);
		if(pbuff)
			bufflen = sizereq;
		else
			bufflen = 0;
	}

	if(pbuff)
	{
		if( SNGetMemory(pbuff, 0, addr, sizereq) )
		{
			int	i;
			char* ptemp = pbuff;
			printf("%08X: ", addr);
			for(i=0; i<num; i++)
			{
				dumpnum(&ptemp, size);
				putchar(' ');
			}
			ptemp = pbuff;
			for(i=0; i<sizereq; i++)
			{
				ch = *ptemp++;
				if(ch < 32)
					ch = '.';
				putchar(ch);
			}
			putchar('\n');
		}
	}
	return sizereq;
}

//
// startaddr	address to start dumping at
// endaddr		address to finish at
// format		1,2,4 for bytes, shorts, words
//
void dumpmem(SNADDR startaddr, SNADDR endaddr, int size)
{
	int 	width = 16;		// going to do 16 bytes per line
	int		modulo = 16;	// and add 16 to get to next line
	SNADDR	temp;
	SNADDR	smalladdr = 0x100;

	// if endaddr is a small number then assume it is a length, not an address
	if(endaddr < smalladdr && startaddr >= smalladdr)
		endaddr = startaddr + endaddr;

	if(endaddr < startaddr)
		printf("START address must be < END address!\n");
	else
	{
		if(endaddr - startaddr > 1024)	 // sanity check the range
			endaddr = startaddr + 1024; // and limit it to something that won't take too long
		
		while( startaddr < endaddr)
		{
			startaddr+=dumpmemline(startaddr, width/size, size);
		}
	}
}

//
// This condition is here to avoid redefining main()
// if this file is #included from autoexec.eic
//
#ifndef __NO_MAIN
int main(int argc, char** argv)
{
	SNTxtHome();
	printf("This is just an example.\n");
	printf("But you can also enter dumpmem(start,end,wordsize)\n");
	printf("in immediate mode.\n\n");

	dumpmem(0xBFC00100, 0xBFC00180, 4); // dump 128 bytes of ROM as words
	return 0;
}
#endif