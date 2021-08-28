/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library  Release 1.6
 */
/*
 *                      Emotion Engine Library
 *                          Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libkernel - app.cmd
 *                        kernel libraly
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.11            May.10.1999     horikawa    
 *      1.10            Oct.12.1999     horikawa   add .stack section
 *      1.20            Nov.09.1999     horikawa   add _heap_end
 *      1.50            Feb.29.2000     horikawa   remove .indata section
 *      1.60            May.08.2000     horikawa   modify start address
 */
_stack_size			= 0x00010000; /* 64KB stack size */
_stack				= 0xf0000;
_heap_size			= -1;


ENTRY(ENTRYPOINT)
SECTIONS {

	.text 0x000A0000: {
		crt0.obj(.text)
		*(.text)
		*(.text.*)
   		*(.gnu.linkonce.t*)
		QUAD(0)
	}

	
/*
 *	
 *	.ctors : {
 *		KEEP (*crtbegin.o(.ctors))
 *				KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
 *		KEEP (*(SORT(.ctors.*)))
 *		KEEP (*(.ctors))	
 *	}
 *	.dtors : {
 *		KEEP (*crtbegin.o(.dtors))
 *		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
 *		KEEP (*(SORT(.dtors.*)))
 *		KEEP (*(.dtors))
 *	}
 *	
 */

	.reginfo		  : { KEEP(*(.reginfo)) }
	
	.data		ALIGN(128): {
		*(.data)
	    *(.data.*)
    	*(.gnu.linkonce.d*)
	}
	
	.rodata		ALIGN(128): {
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
	}
	.rdata		ALIGN(128): { *(.rdata) }
	.gcc_except_table ALIGN(128): { *(.gcc_except_table) }
	_gp = ALIGN(128) + 0x7ff0;
	.lit8       	ALIGN(128): { *(.lit8) }
	.lit4       	ALIGN(128): { *(.lit4) }

	.sdata		ALIGN(128): {
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.s*)
	}
	.sbss		ALIGN(128): { _fbss = .; *(.sbss) *(.scommon) }
	.bss		ALIGN(128): { *(.bss) }

	end = .;
	_end = .;

	.spad		0x70000000: {
		 crt0.obj(.spad)
		 *(.spad)
	}

}
