/****************************************************************************************************/
/*                                                                                                  */
/* app.cmd for ProView                                                                              */
/*                                                                                                  */
/* Copyright 2003 SN Systems Ltd                                                                    */
/*                                                                                                  */
/*     Note: For ProView, the entire app should sit below 1MB including the stack, so set it        */
/*           to just below 1MB and set the stack size to 64K.                                       */
/*                                                                                                  */
/*           The application is org'd at 1MB - 384K (0xA0000) and is about 220K in size, so this    */
/*           should be an ok size for the stack allowing extra room for expansion of app if         */
/*           required.                                                                              */
/*                                                                                                  */
/****************************************************************************************************/

_stack_size = 0x0010000;
_stack      = 0xf0000;
_heap_size  = 0xffffffff;

GROUP(-lc -lkernl -lgcc)
ENTRY(ENTRYPOINT)
SECTIONS {
	.text		0x000A0000: {
		crt0.o(.text)
		*(.text)
	}
	.reginfo		  : { KEEP(*(.reginfo)) }
	.data		ALIGN(128): { *(.data) }
	.rodata		ALIGN(128): { *(.rodata) }
	.rdata		ALIGN(128): { *(.rdata) }
	.gcc_except_table ALIGN(128): { *(.gcc_except_table) }
	_gp = ALIGN(128) + 0x7ff0;
	.lit8       	ALIGN(128): { *(.lit8) }
	.lit4       	ALIGN(128): { *(.lit4) }
	.sdata		ALIGN(128): { *(.sdata) }
	.sbss		ALIGN(128): { _fbss = .; *(.sbss) *(.scommon) }
	.bss		ALIGN(128): { *(.bss) }
	end = .;
	_end = .;
	.spad		0x70000000: {
		 crt0.o(.spad)
		 *(.spad)
	}
}
