#include <SNScript.h>	// for SN builtin functions
#include <stdio.h>		// for printf

//
// Locate the nearest label to address addr
//
void NearestLabel(unsigned int addr)
{
	char*	pname;
	unsigned int starta;

	if(SNFindNearestLabel(0,addr,&pname,&starta) )
		printf("0x%08x = %s + 0x%X\n", addr, pname, addr - starta);
	else
		printf("Not Found!\n");
}

//
// Tell me something about address addr
//
void Address(unsigned int addr)
{
	char*	pname;
	unsigned int starta, enda;

	if(SNAddr2Func(0,addr, &pname,&starta,&enda) )
		printf("%08X to %08X is %s\n", starta, enda, pname);
	else
		NearestLabel(addr);
}

int main(int argc, char** argv)
{
	char* pmsg =
	"This example has loaded two useful utility functions that you can\n"
	"call from immediate mode to display info about an address.\n"
	"\n"
	"Address(unsigned int addr)\n"
	"NearestLabel(unsigned int addr)\n";

	printf(pmsg);
	return 0;
}