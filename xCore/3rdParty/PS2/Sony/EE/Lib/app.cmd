/*
 * SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *           		Emotion Engine Library
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                            app.cmd
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.11            May.10.1999     horikawa    
 *      1.10            Oct.12.1999     horikawa   add .stack section
 *      1.20            Nov.09.1999     horikawa   add _heap_end
 *      1.50            Feb.29.2000     horikawa   remove .indata section
 *      1.60            May.08.2000     horikawa   modify start address
 *      1.70            Jun.08.2000     horikawa   modify .text section
 *      1.80            Oct.09.2002     shibata    add .init, .fini, and
 *                                                 gnu.linkonce.r*
 *      1.90            Oct.09.2002     shibata    eraze lines which order 
 *                                                 linker to link crt0.o .
 */

_stack_size = 0x00100000;
_stack = -1;
_heap_size = -1;

GROUP(-lc -lkernl -lgcc)
ENTRY(ENTRYPOINT)
SECTIONS {
	.text		0x00100000: {
		*(.text)
		*(.text.*)
    		*(.gnu.linkonce.t*)
		KEEP (*(.init))
		KEEP (*(.fini))
		QUAD(0)
	}
	.ctors 			  : {
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
	    	KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))	
	}
  	.dtors    : 	{
    		KEEP (*crtbegin.o(.dtors))
    		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    		KEEP (*(SORT(.dtors.*)))
    		KEEP (*(.dtors))
  	}

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
	/*
        .stack          ALIGN(128)(NOLOAD): {
		 _stack = .;
                 . += _stack_size;
        }
	*/
	end = .;
	_end = .;
	/* _heap_end = . + _heap_size */
	.spad		0x70000000: {
		 *(.spad)
	}
}
