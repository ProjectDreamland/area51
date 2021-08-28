#include <SNscript.h>

typedef unsigned int WORD;
#define FBSIZE 1024		// 4KB buffer

WORD fmbuff[FBSIZE];

void fillmemw(WORD starta, WORD enda, WORD val)
{
	int	addr = starta;
	int i, len;

	for(i=0; i<FBSIZE; i++)
		fmbuff[i] = val;

	while(addr < enda)
	{
		len = enda-addr;
		if(len > (FBSIZE*sizeof(int)))
			len = FBSIZE*sizeof(int);
		if(SNSetMemory(fmbuff, 0, addr, len)==0)
			break;
		addr+=len;
	}
}

void test()
{
	WORD patch[] = {0x3c0207ff,0x03e00008,0,0,0,0,0,0,0,0,0};

	SNSetMemory(patch, 0, 0x80000C40, sizeof(patch));
	fillmemw(0x07FF0000, 0x08000000, 0xDEADBABE);
	puts("GetMemorySize is now patched to leave top 64K alone\n");
}