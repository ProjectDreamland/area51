//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: procvif.c gives an example of the sort of thing you can do with scripts.
// This sample script processes a set of VIF and VPU data and displays it in a graphical
// overlay on the script pane.
//                                                                          
//****************************************************************************
//
// There is a sample data file "igacapture.bin" in the Testscripts directory.
// You can copy this to "c:\igacapture.bin" before running the script.
//
// You could add an event callback hooked to target-update messages,
// then the example "VPU/VIF pane could auto-update when the target
// CPU halts in suitable conditions (e.g. at a breakpoint).
//
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <SNScript.h>

#ifndef UINT
#define UINT unsigned int
#endif

//#define max(a,b)		(((a)>=(b))?(a):(b))
//#define timediff(a,b)	(((b)>=(a))?((b)-(a)):((b)+(0-(a)))	// Yuck - I think I'll stick with a function

//
// Structure of the data records from EE memory.
// One quadword per time interval.
//
struct cap
{
	unsigned int vpustat;	// Snapshot of VPU_STAT register.
	unsigned int vif1stat;	// Snapshot of VIF_STAT register.
	unsigned int vif1mark;	// Snapshot of VIF1_MARK register.
	unsigned int time;		// Snapshot of CPU COP0 count register (CPU clocks).
};
typedef struct cap CAP;

unsigned int lastoffset = 0;
void* pcapbuff = 0;
unsigned int pcaplen = 0;

//
// Return the difference in two 32-bit counters (allowing for wrap).
//
unsigned int timediff(unsigned int t0, unsigned int t1)
{
	if(t1 >= t0)
		return t1 - t0;
	else
		return t1 + (0 - t0);
}

//
// Process a pile of captured data that is already in our allocated buffer.
//
void ProcCap(unsigned int offset, int scale)
{
	int	ixpos = 50;	// Starting horizontal position.
	int	iypos = 132;	// Starting vertical position.
	int	my = 18;	// Max height of a single trace.
	int	dy = 24;	// Vertical distance between traces.
	int	fo = 16;	// Font size.

	int	hsf;
	int width, height;
	int x0, x1, ypos;
	unsigned int w, st, temp;
	unsigned int index, ms;
	CAP* rec = (CAP*) pcapbuff;

	// hsf = horizontal scale factor is CPU clocks per pixel width.
	// 0x54 is approx 1:1 with our sample data rate.
	if(scale>0 && scale < 10)
		hsf = 0x54/scale;
	else
		hsf = 0x28;	// Default scale if passed zero.

	if(offset == 1)
		offset = lastoffset;
	st = rec[offset].time;	// Record start time for display.

	//SNTxtHome();				// Keep the screen clear and tidy.
	SNGetWindowSize(&width, &height);	// Request the size of the script pane.
	SNCreateBmp(width,height);		// Create a BITMAP overlay the same size.
	// Default is not-stretched and overlayed with text layer.

	SNSetFont(fo, 0, "Comic Sans MS");

	ypos = iypos;
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "VPS");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + (timediff(st, rec[index+1].time) / hsf);
		temp = rec[index].vif1stat & 3;
		switch(temp)
		{
		case 0:						// Nothing => VIF is idle.
			break;
		case 1:
			SNSetColor(255,0,0);	// red => stalled waiting for data from EE.
			SNRectangle(x0, ypos - my, x1, ypos);	
			break;
		case 2:
			SNSetColor(0,255,0);	// green => decoding vifcode.
			SNRectangle(x0, ypos - my, x1, ypos);
			break;
		case 3:
			SNSetColor(0,0,255);	// blue => decomp/xfer of the data after VIFCODE (mpg or unpack)
			SNRectangle(x0, ypos - my, x1, ypos);
			break;
		}
		x0 = x1;	index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.


	ypos += dy;	// VEW
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "VEW");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if( rec[index].vif1stat & 4)
		{
			SNSetColor(255,0,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;	index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy;	// VGW
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "VGW");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if( rec[index].vif1stat & 8)
		{
			SNSetColor(255,0,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy;	// DBF
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "DBF");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if( rec[index].vif1stat & 128)
		{
			SNSetColor(0,0,255);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += 34;	// RQC
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "RQC");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		temp = (rec[index].vif1stat >> 24) & 0x1F;
		if(temp)
		{
			SNSetColor(0,255,0);
			SNRectangle(x0, ypos - temp*2, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy*2;
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "VBS1");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if(rec[index].vpustat & 0x0100)
		{
			SNSetColor(0,255,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy;
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "VGW1");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if(rec[index].vpustat & 0x1000)
		{
			SNSetColor(0,255,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy;
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "DIV1");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if(rec[index].vpustat & 0x2000)
		{
			SNSetColor(0,255,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
 	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.

	ypos += dy;
	index = offset;
	x0 = ixpos + timediff(st, rec[index].time) / hsf;
	SNSetColor(0,0,0);		SNTextOut(12, ypos-fo, "EFU1");	SNSetLine(x0, ypos+1, width, ypos+1);
	while(x0 < width && index < pcaplen-1 )
	{
		x1 = ixpos + timediff(st, rec[index+1].time) / hsf;
		if(rec[index].vpustat & 0x4000)
		{
			SNSetColor(0,255,0);
			SNRectangle(x0, ypos - my, x1, ypos);
		}
		x0 = x1;		index++;
	}
	SNUpdateWindows(0,0);	// Force a repaint of this pane.
	lastoffset = index;

	ms = timediff(st, rec[index].time);
	printf("Display Width = %d CPU clocks (%3g microseconds)\n", ms, (double)ms/300.0);

	ms = timediff(st, rec[pcaplen-1].time);
	printf("Total DMA is 0x%08X samples = %3g milliseconds\n", pcaplen, (double)ms/300000.0);
}

//
// Load data from file.
// set pcapbuff = ptr to buffer.
// set pcaplen = number of quadwords in the buffer.
//
int LoadBufferFromFile(char* fname)
{
	UINT	len;
	FILE*	file;

	file = fopen(fname, "rb");
	if(file)
	{
		if( fseek(file, 0, 2) == 0)
		{
			len = ftell(file);
			if(pcapbuff)
			{
				free(pcapbuff);
				pcapbuff = 0;
			}

			pcapbuff = malloc(len);
			if(pcapbuff)
			{
				fseek(file, 0, 0);
				fread(pcapbuff, 1, len, file);
				fclose(file);
				pcaplen = len >> 4;
				return 1;
			}
		}
	}
	else
		printf("Could not open data file %s\n", fname);

	return 0;
}

//
// Try to load a dataset from target memory.
//
int LoadBufferFromTarget(UINT address, UINT len)
{
	if(pcapbuff)
	{
		free(pcapbuff);
		pcapbuff = 0;
	}
	printf("Allocating 0x%08X bytes...\n", len);
	pcapbuff = malloc(len);
	if(pcapbuff)
	{
		SNTxtHome();
		printf("Fetching 0x%08X bytes from 0x%08X...\n", len, address);

		if(SNGetMemory(pcapbuff, 0, address, len))
		{
			pcaplen = len >> 4;
			return 1;
		}
	}
	return 0;
}

//
// Look at symbols in target context to see if there is data to process in PS2 memory.
//
int GetPS2buffer()
{
	snval_t	result1, result2;
	char* pexperr1 = 0;
	char* pexperr2 = 0;

	// These will only evaluate OK if the target is halted at a breakpoint
	// just after the buffer has been collected.
	pexperr1 = SNEvaluate(&result1, 0, "pbuffer");	// ptr to buffer.
	pexperr2 = SNEvaluate(&result2, 0, "buffgot");	// number of quadwords in it.

	if(!pexperr1 && !pexperr2)
		return LoadBufferFromTarget(result1.val.u32, result2.val.u32*16);
	else
		return 0;	// fail
}

//
// Something handy to call from Imm mode on subsequent frames so we don't have to keep 
// running the script.
//
int ProcPS2()
{
	SNTxtHome(); // leave out whilst debugging because it will hide warnings
	if(GetPS2buffer() && pcapbuff)
	{
		ProcCap(0,1);		// Display the data from start at 1:1 resolution.
		return 1;
	}
	return 0;
}

//
// First look for data to process in memory
// but if not found then try to load it from a binary file, then process the first 
// screenful.
//
int main(int argc, char**argv)
{
	char*	fname = "c:\\igacapture.bin";

	SNTxtHome(); // Leave out whilst debugging because it will hide warnings.

	if( ! GetPS2buffer() )
	{
		puts("Could not locate in PS2 memory so loading sample data from file.\n");
		LoadBufferFromFile(fname);
	}

	if(pcapbuff)
	{
		ProcCap(0,1);		// display the data from start at 1:1 resolution

		puts("Enter ProcCap(1,n) to advance display\n");
		puts(" or ProcCap(offset,n) to see specific data (offset = quadword index)\n");
		puts(" where n = 1 to 9 display resolution (1 = finest)\n");
	}

	return 0;
}