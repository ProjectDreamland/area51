/* Copyright © 2002 SN Systems Software Limited
 * All Rights Reserved
 * $Revision: 4 $
 * $NoKeywords: $
 */
 /* We *may* need _gp and _end, I don't know! */
_gp = 1;
_end = -1;

SECTIONS {
    .text        ALIGN(128): { *(.text) *(.text.*) }
    .data        ALIGN(128): { *(.data) *(.data.*) *(.vutext) *(.vutext.*) *(.vudata) *(.vudata.*)}
    .rodata        ALIGN(128): { *(.rodata) *(.rodata.*) }
    .rdata        ALIGN(128): { *(.rdata) *(.rdata.*) }
    .gcc_except_table ALIGN(128): { *(.gcc_except_table) }
    .lit8         ALIGN(128): { *(.lit8) }
    .lit4          ALIGN(128): { *(.lit4) }
/*    .sdata        ALIGN(128): { *(.sdata) *(.sdata.*) }
    .sbss        ALIGN(128): { *(.sbss) *(.sbss.*) *(.scommon) }*/
    .bss        ALIGN(128): { *(.bss) *(.bss.*) *(.vubss) *(.vubss.*) }
}
