/*
 *
 *  +++++++++++++++++++++++++++++++++++++++++++
 *  +++ User configeration of this file for +++
 *  +++ different Harware settings.         +++
 *  +++ Please look for the __ArenaHi       +++
 *  +++ setting below                       +++
 *  +++++++++++++++++++++++++++++++++++++++++++
 *
 * Description : 
 *
 *  SN Systems Ltd : File Revision History
 *
 *  $Archive: /Linker/ngc/ngc.ld $
 *
 *  Last Modified 
 *  $Date: 3/07/01 15:21 $ by $Author: Bill $
 *
 *  History:
 *
 *  $History: ngc.ld $
 * 
 * *****************  Version 14  *****************
 * User: Bill         Date: 3/07/01    Time: 15:21
 * Updated in $/Linker/ngc
 * Add support for lit sections etc.
 * 
 * *****************  Version 13  *****************
 * User: Bill         Date: 19/06/01   Time: 13:19
 * Updated in $/Linker/ngc
 * Tidy up. Add extab stuff. add .comment section.
 * 
 * *****************  Version 12  *****************
 * User: Bill         Date: 11/05/01   Time: 16:41
 * Updated in $/Linker/ngc
 * Remove GNU Heap. 128K small data support.
 * 
 * *****************  Version 11  *****************
 * User: Bill         Date: 27/04/01   Time: 15:50
 * Updated in $/Linker/ngc
 * 
 * *****************  Version 10  *****************
 * User: Bill         Date: 23/04/01   Time: 9:47
 * Updated in $/Linker/ngc
 * QUAD should be LONG (Constructor table)
 * 
 * *****************  Version 9  *****************
 * User: Bill         Date: 20/04/01   Time: 15:57
 * Updated in $/Linker/ngc
 * Add __CTORS list back in the script.
 * 
 * *****************  Version 8  *****************
 * User: Bill         Date: 20/04/01   Time: 14:35
 * Updated in $/Linker/ngc
 * Remove __CTOR_LIST__
 * 
 * *****************  Version 7  *****************
 * User: Bill         Date: 3/04/01    Time: 14:02
 * Updated in $/Linker/ngc
 * 
 *  $NoKeywords: $
 *
 */

 ENTRY(__start) 
 SEARCH_DIR(/dolphin/gcc/powerpc-x750-elf/lib);
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
PROVIDE (__stack = 0);
SECTIONS
{
	.comment :{ *(.comment)}
	.init	0x80003100	 : { *(.init)		} =0

	. =  ALIGN(32);
	.text      :
	{
		*(.text)
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
	} =0
	. =  ALIGN(32);
	.ctors	  : 
	{ 
		__CTOR_LIST__ = .;
		LONG(-1);
		*(.ctors)
		LONG(0); 
		/* PROVIDE (__CTOR_END__ = .); */
	}
	. =  ALIGN(32);		
	.dtors	  :
	{ 
		 __DTOR_LIST__ = .;
		 LONG(-1);
		*(.dtors)
		LONG(0);
		/* PROVIDE (__DTOR_END__ = .); */
	}


	. =  ALIGN(32);
	extabindex        :
	{
		*(extabindex)
	}

	. =  ALIGN(32);
	extab     :
	{
		*(extab)
	}
	 . =  ALIGN(32);
	.rodata	  : 
	{
		 *(.rodata) *(.gnu.linkonce.r*) *(.rodata.*)
	}
	. =  ALIGN(32);
	.data    :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
	}
	. =  ALIGN(32);
	.bss       :
	{
		_f_bss = .;
		__bss_start = .;
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__bss_end = .;
		_e_bss = .;
	}

	. =  ALIGN(32);

	.sdata	  :
	{ 

		 __SDATA_START__ = .;
		 _SDA_BASE_ = . + 0x8000; 
		 _SDA2_BASE_ = . + 0x18000;

		*(.sdata) 
		*(.sdata.*)
	}

	. =  ALIGN(32);
	.sbss      :
	{
		_f_sbss = .;
		__sbss_start = .;
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.dynsbss)
		__sbss_end = .;
		_e_sbss = .;
		__SBSS_END__ = .;
	}
	. =  ALIGN(32);
	.sdata2   :
	{
		__SDATA2_START__ = .;
		 *(.sdata2)
		*(.sdata2.*)
	}
	.sbss2   :
	{
		*(.sbss2)
		*(.sbss2.*)
		 __SBSS2_END__ = .;
	}


  .got2		  :  { *(.got2) }
		PROVIDE (_FIXUP_START_ = .);
  .fixup	  : { *(.fixup) }
		PROVIDE (_FIXUP_END_ = .);
		PROVIDE (_GOT2_END_ = .);
		PROVIDE (_GOT_START_ = .);
  .got		  : { *(.got) }
  .got.plt	  : { *(.got.plt) }
		PROVIDE (_GOT_END_ = .);

 
  .plt   : { *(.plt) }

	. = . + 16;		
	. = ALIGN(32);		
	_stack_end = .;	 /* Used by thread managment */	
	. = . + 0x10000;	


	_stack_addr = .;
	. = . + 16;

	. = ALIGN(32) ;
	__ArenaLo = .;
/*
 *  +++++++++++++++++++++++++++++++++++++++++++
 *  +++ User configeration of this file for +++
 *  +++ different Dolphin Harware.          +++
 *  +++ Please coose one of the following   +++
 *  +++ lines for your hardware.            +++
 *  +++ Comment out others.                 +++
 *  +++++++++++++++++++++++++++++++++++++++++++
 */

	 /* . = 0x80380000;  HW1_DRIP */
	 . = 0x81780000; /*   HW1 or HW2 (Others ?)  */
	__ArenaHi = .;
	

  /* These are needed for ELF backends which have not yet been
     converted to the new style linker.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* These must appear regardless of  .  */
}
