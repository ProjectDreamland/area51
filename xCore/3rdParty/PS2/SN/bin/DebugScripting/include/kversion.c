//
// Example debugger script that attempts to find and print
// the date and version number of the Kernel of the connected PS2 devkit
//
// Nothing clever, it just searches the PS2 ROM for the ROMconf text
// and extracts the version number from it to print
//
// example use: you could embed this in your autoexec.eic and get it to
// issue a warning if your devkit has been flashed to the wrong kernel version
//
#include <string.h>		// for string ops
#include <stdlib.h>		// for malloc
#include <stdio.h>		// for stdout
#include <SNscript.h>	// for SN builtin functions

#define ROMTOGET 16384

int bcd2int(int bcd)
{
	int	i;
	int	f=1;
	int	result = 0;

	for(i=0; i<8; i++)
	{
		result+=(bcd & 0x0F)*f;
		f*=10;
		bcd >>= 4;
	}
	return result;
}

//
// Attempts to pull the date out of the ROM
// returns year (eg. 2002) if successful
// returns 0 if it cannot find a sensible date
//
int GetRomDate(int* pday, int* pmonth, int* pyear)
{
	unsigned char	buff[4];
	int		day,month,year;

	// PS2 Kernel seems to have the date here in BCD
	if(SNGetMemory(buff,0,0xBFC00100,4))
	{
		day = bcd2int(buff[0]);
		month = bcd2int(buff[1]);
		year = bcd2int((buff[3]<<8)+buff[2]);

		if(day > 0 && day < 32 && month > 0 && month < 13 && year > 1995 && year < 2006)
		{
			*pday = day;
			*pmonth = month;
			*pyear = year;
			return year;
		}
	}
	return 0;
}

//
// Attempts to pull the kernel version number out of the ROM
// returns ptr to version string if successful
// returns NULL if it cannot find a sensible date
//
char* GetRomVersion()
{
	static char  strbuff[34];

	char* sptr;
	int	  i,j;
	char* matchstr = "ROMconf,";
	int	  matchlen = strlen(matchstr);

	char* buff = (char*)malloc(ROMTOGET);
	if(buff && SNGetMemory(buff,0,0xBFC02000,ROMTOGET))
	{
		for(i=0; i<ROMTOGET-32; i++)
		{
			if(strncmp(buff+i, matchstr, matchlen)==0)
			{
				sptr = buff+i+matchlen;
				for(j=0;j<32;j++)
				{
					if(strncmp(sptr+j,".bin",4)==0 ||  sptr[j]==',')
						break;
				}
				strncpy(strbuff, sptr, j);
				strbuff[j]=0;
				return strbuff;
			}
		}
	}
	return NULL;
}

//
// Wrapper to call GetRomDate and GetRomVersion and print results
//
void RomVer()
{
	char* pver;
	int	  day,month,year;
	char* punk="unknown\n";

	printf("ROM date (dd-mm-yyyy) = ");
	if(GetRomDate(&day,&month,&year))
		printf("%02d-%02d-%04d\n",day,month,year);
	else
		puts(punk);

	printf("ROM version = ");
	pver = GetRomVersion();
	if(pver)
		printf("%s\n", pver);
	else
		printf(punk);
}

//
// This condition is here to avoid redefining main()
// if this file is #included from autoexec.eic
//
#ifndef __NO_MAIN
int main(int argc, char** argv)
{
	RomVer();
	return 0;
}
#endif