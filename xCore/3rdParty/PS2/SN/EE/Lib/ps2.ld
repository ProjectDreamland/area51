/* SCEI CONFIDENTIAL
 $Revision: 3 $
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

/* Only require a small amount of stackspace since the 'main' thread, as created by the PS2 system,
 * will be used only as an 'idle' state thread.
 */

_stack_size = 0x00004000;
_stack          = -1;
_heap_size      = -1;
_memory_size    = 0x2000000;

/*GROUP(-lc -lkernl -lgcc)*/
ENTRY(ENTRYPOINT)
SECTIONS {
    .text		0x00100000: {
    __text_obj = .;
		*(.boot)
        *(.text)
        *(.gnu.linkonce.t*)
        *(.render)
    __text_objend = .;
    }
    .reginfo                    : { KEEP(*(.reginfo)) }
    .data       ALIGN(128)      : { *(.data) *(.vutext) *(.vudata) *(.gnu.linkonce.d*) }
    .sndata     ALIGN(128)      : { sn_dll_header_root = .; . += 16384; }
    .rodata     ALIGN(128)      : { *(.rodata) *(.gnu.linkonce.r*) }
    .rdata      ALIGN(128)      : { *(.rdata) }
    .gcc_except_table ALIGN(128): { *(.gcc_except_table) }
    _gp = ALIGN(128) + 0x7ff0;
    .lit8       ALIGN(128)      : { *(.lit8) }
    .lit4       ALIGN(128)      : { *(.lit4) }
    .sdata      ALIGN(128)      : { *(.sdata) *(.gnu.linkonce.s.*) }
    .sbss       ALIGN(128)      : { _fbss = .; *(.sbss) *(.scommon) }
    .bss        ALIGN(128)      : { *(.bss) *(.vubss) }
    /*
        .stack          ALIGN(128)(NOLOAD): {
	     _stack = .;
                 . += _stack_size;
        }
    */
	end = .;
	_end = .;
	/* _heap_end = . + _heap_size */
	.spad		0x70000000 (NOLOAD): {
		 *(.spad)
	}
}
