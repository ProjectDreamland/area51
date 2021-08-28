//############################################################################
//##                                                                        ##
//##  MSSSYS.C                                                              ##
//##                                                                        ##
//##  Windows MSS support routines                                          ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from DLLLOAD.C V1.12               ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include "stdlib.h"
#include "stdio.h"
#include "mss.h"
#include "imssapi.h"

#ifndef IS_WIN32
#include <stdarg.h>
#endif


#ifndef IS_WIN32

static int win31=0;

static int GetWin31()
{
  U32 ver;

  if (GetWinFlags()&0x4000)
    return(-1);

  ver=GetVersion();

  if ((LOBYTE(LOWORD(ver))>=3) && (HIBYTE(LOWORD(ver))>=95))
    return(-2);

  return(1);
}

BOOL DPMISetSelectorLimit (UINT selector, DWORD dwLimit)
{
   BOOL bRetVal=TRUE;

  // If the limit is >= 1MB, we need to make the limit a mulitple
  // of the page size or DPMISetSelectorLimit will fail.

     if( dwLimit >= 0x100000 )
         dwLimit |= 0x0FFF;

     __asm
     {
       mov  ax, 0008h
       mov  bx, selector
       mov  cx, word ptr [dwLimit+2]
       mov  dx, word ptr [dwLimit]
       int  31h
       jnc  success
       mov  bRetVal, FALSE
      success:
     }
     return bRetVal;

}

void AIL_apply_fixup_clone_ref(void FAR * FAR *src)
{
   U16 sel=HIWORD(*src);
   U32 limit=GetSelectorLimit(sel)-32768L;

   SetSelectorBase(sel,GetSelectorBase(sel) + 32768L);

   DPMISetSelectorLimit(HIWORD(*src),limit);

   *src = MAKELP(HIWORD(*src), LOWORD(*src) - 32768U);
}

// ---------------------------------------------------------------------------
// ptr_clone
//
// Make copy of selector:offset far pointer, where selector's base address
// may be freely modified without corrupting original pointer
//
// Cloned pointer is guaranteed to address at least 32K without further fixups
//
// Cloned pointer's selector must be freed with ptr_free()!
//
// ---------------------------------------------------------------------------

void FAR *AIL_ptr_alloc_clone(void const FAR *src)
{
   void FAR *ptr;

   ptr = (void FAR *) (((U32)(AllocSelector(HIWORD(src))) << 16) + LOWORD(src));

   AIL_ptr_fixup_clone(ptr);

   return ptr;
}

// ---------------------------------------------------------------------------
// ptr_adjust
//
// WARNING: Alters the base address of *ptr -- may be applied ONLY
// to a cloned pointer!
// ---------------------------------------------------------------------------

void AIL_ptr_inc_clone_ref(void FAR * FAR *ptr, U32 offset)
{
   void FAR *dest = *ptr;

   while (offset > 32767)
      {
      dest = (void FAR *) ((U8 FAR *) dest + 32768L);
      AIL_ptr_fixup_clone(dest);

      offset -= 32768;
      }

   dest = (void FAR *) ((U8 FAR *) dest + offset);
   AIL_ptr_fixup_clone(dest);

   *ptr = dest;

}

// ---------------------------------------------------------------------------
// AIL_ptr_dif
// ---------------------------------------------------------------------------

S32 AIL_ptr_dif(void const FAR *p1, void const FAR *p2)
{
  if (HIWORD(p1)==HIWORD(p2))
    return((S32)((U32)LOWORD(p1))-(S32)((U32)LOWORD(p2)));

   return (S32)(((S32)(GetSelectorBase(HIWORD(p1)) + LOWORD(p1))) -
                ((S32)(GetSelectorBase(HIWORD(p2)) + LOWORD(p2))));
}


extern void WINAPI AIL_memcpydb(void FAR*dst, void const FAR*src,S32 len);

void AIL_memmove(void FAR *d, void const FAR *s, S32 len)
{
   U32 saddr = AIL_ptr_lin_addr(s);
   U32 daddr = AIL_ptr_lin_addr(d);

   if (saddr==daddr)
     return;

   if (saddr > daddr)
      {
      AIL_memcpy(d,s,len);
      }
   else
      {
      if ((saddr+len)<daddr)
        {
        AIL_memcpy(d,s,len);
        }
      else
        {
        AIL_memcpydb(d,s,len);
        }

      }
}

static void FAR* freelist=0;

static void addtobackfree(void FAR* ptr)
{
  *((void FAR * FAR*)ptr)=freelist;
  freelist=ptr;
}

//only call when you know we're in foreground
static void checkforbackfrees()
{
  void FAR* fl=freelist;
  freelist=0;

  while (fl)
  {
    void FAR* next=*((void FAR* FAR*)fl);
    AIL_API_mem_free_lock(fl);
    fl=next;
  }
}

#endif


#ifdef IS_WIN32

//#define DEBUGALLOC

#ifdef DEBUGALLOC
static U32 count = 0;
#endif

void * AILCALLBACK w32_alloc(U32 size)
{
   void FAR* p;

#ifndef DEBUGALLOC
   HGLOBAL h;

   h = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size);

   p=GlobalLock(h);

   if (p==0)
     AIL_set_error("Out of memory.");

#else

   U32 s=(size+4+4095)&~4095;

   p=VirtualAlloc(0,size+4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

   if (p==0)
     AIL_set_error("Out of memory.");
   else
   {
     U32 old;
     void* o=p;

     ++count;

     VirtualProtect(((U8*)p)+s,4096,PAGE_NOACCESS,&old);
     p=((U8*)p)+(s-size);
     ((void**)p)[-1]=o;
   }

   //
   // Fill the buffer with junk
   //

   {
     U32 i;
     for(i=0;i<(size/8);i++)
     {
       ((U32*)p)[i*2]=0x76541234;
       ((U32*)p)[i*2+1]=0x24689753;
     }
     for(i=0;i<(size&7);i++)
       ((U8*)p)[(size&(~7))+i]=(U8)(127+(i*33));
   }

#endif
  return( p );
}

void AILCALLBACK w32_free( void * ptr )
{
#ifndef DEBUGALLOC

      HANDLE h = (HANDLE) GlobalHandle(ptr);

      if (h==0)
      {
        return;
      }

      GlobalUnlock(h);   //jkr: prevents warnings from debug kernel

      GlobalFree(h);

#else

      void* p=((void**)ptr)[-1];
      ((void**)ptr)[-1]=0;

      if (p==0)
      {
        MessageBox(0,"Double freed memory.","Miles memory error",MB_OK|MB_ICONSTOP);
      }
      else
      {
        --count;

        VirtualFree(p,0,MEM_RELEASE);
      }

#endif
}

static AILMEMALLOCCB mss_alloc = w32_alloc;
static AILMEMFREECB mss_free = w32_free;

DXDEC void * AILCALL AIL_mem_use_malloc(AILMEMALLOCCB fn)
{
  void * ret = mss_alloc;
  mss_alloc = (fn)?fn:w32_alloc;
  return( ret );
}

DXDEC void * AILCALL AIL_mem_use_free  (AILMEMFREECB fn)
{
  void * ret = mss_free;
  mss_free = (fn)?fn:w32_free;
  return( ret );
}

#endif

//############################################################################
//##                                                                        ##
//## Allocate and free page-locked global memory for AIL resources          ##
//##                                                                        ##
//## These routines should not be used for allocation of numerous small     ##
//## objects, due to limited LDT handle space                               ##
//##                                                                        ##
//## Memory allocated is owned by DLL, and is allocated with                ##
//## GMEM_SHARE and GMEM_ZEROINIT attributes (MOVEABLE attribute is         ##
//## disabled by GlobalPageLock())                                          ##
//##                                                                        ##
//############################################################################

void FAR * AILCALL AIL_API_mem_alloc_lock(U32 size)
{
#ifdef IS_WIN32

   return( mss_alloc( size ) );

#else

   HANDLE    h;
   void FAR *ptr;

   typedef struct _LINK
      {
      UINT              selector;
      struct _LINK FAR *next;
      }
   LINK;

   LINK FAR       *first;
   LINK FAR       *cur;
   LINK FAR       *temp;
   LINK FAR * FAR *next;

   UINT  sel;
   U32   blk_size;
   U32   allocsize;

   // fail all background mallocs
   if (AIL_background())
     return(0);

   checkforbackfrees();

   //
   // Fail if 0 bytes requested
   //

   if (size == 0)
      {
      return NULL;
      }

   //
   // Allocate all available memory from GlobalDosAlloc() to
   // ensure that GlobalPageLock() does not move block to lower 1MB
   //
   // Otherwise, Windows would quickly run out of lower memory for
   // program descriptors, causing out-of-memory errors and/or GP faults
   //

   first = NULL;

   if (win31==0)
     win31=GetWin31();

   if (win31==1) {

     allocsize=(size>sizeof(LINK))?size:sizeof(LINK);

     next  = &first;
     blk_size=allocsize;
     while (blk_size<32768)
       blk_size<<=1;

     do
        {
        while ((sel = LOWORD(GlobalDosAlloc(blk_size))) != 0)
           {
           cur = (LINK FAR *) MAKELONG(0, sel);

           *next = cur;

           cur->selector = sel;
           cur->next     = NULL;
           next          = &cur->next;
           }

        blk_size >>= 1;
        }
     while (blk_size >= allocsize);
  }

   if (size<4)
     size=4;

   //
   // Now allocate requested memory block and GlobalPageLock() it
   //

   h = GlobalAlloc(GMEM_FIXED | GMEM_SHARE | GMEM_ZEROINIT, size+16);  //jkr: alloc 16 bytes extra for Win32s support
   GlobalFix(h);

   if (!GlobalPageLock(h))
      {
      ptr = NULL;
      }
   else
      {
      ptr = GlobalLock(h);
      }

   //
   // Finally, walk through list of GlobalDosAlloc() blocks, returning
   // them to the lower-1MB heap
   //

   cur = first;

   while (cur != NULL)
      {
      temp = cur->next;

      GlobalDosFree(cur->selector);

      cur = temp;
      }

   //
   // Return the pointer to the allocated block
   //

   if (ptr) {

     *((U32 FAR*)ptr)=((U32)ptr)+16;     // jkr: hide 16:16 pointer so Win32s can find it later
     *(((U32 FAR*)ptr)+1)=((U32)ptr)+16; // jkr: hide it twice
     *(((U32 FAR*)ptr)+3)=0x5753534d;    // jkr: hide a marker
     return(((U8 FAR*)ptr)+16);          // jkr: return adjusted pointer

   } else

     return(0);

#endif
}

void AILCALL AIL_API_mem_free_lock(void FAR *ptr)
{
   if (ptr != NULL)
      {
#ifdef IS_WIN32
      mss_free( ptr );
#else
      HANDLE h;

      if (AIL_background())
        addtobackfree(ptr);
      else
      {
        checkforbackfrees();

        if (*(((U32 FAR*)ptr)-1)!=0x5753534d) {  // jkr: validate pointer
          MessageBox(0,"Attempted to free a bad pointer.","Error from MSS",MB_OK);
          return;
        }

        *(((U32 FAR*)ptr)-1)=0;                      //jkr: clear marker
        h = (HANDLE) GlobalHandle(SELECTOROF(ptr));  //jkr: don't need to subtract 16 because the selector is the same
        GlobalUnlock(h);
        GlobalPageUnlock(h);
        GlobalUnfix(h);
        GlobalFree(h);
      }

#endif

   }
}

//############################################################################
//##                                                                        ##
//##  Write file at *buf of length len                                      ##
//##                                                                        ##
//##  Overwrites any existing file                                          ##
//##                                                                        ##
//##  Returns 0 on error, else 1                                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_file_write(char const FAR *filename, void const FAR *buf, U32 len)
{
#ifdef IS_WIN32

   HANDLE handle;
   U32    nbytes;

   disk_err = 0;

   handle = CreateFile(filename,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

   if (handle == INVALID_HANDLE_VALUE)
      {
      disk_err = AIL_CANT_WRITE_FILE;
      AIL_set_error("Unable to create file.");
      return 0;
      }

   if ((!WriteFile(handle,
                   buf,
                   len,
                  &nbytes,
                   NULL))
        || (nbytes != len))
      {

      disk_err = AIL_DISK_FULL;
      AIL_set_error("Unable to write to file (disk full?).");
      return 0;
      }

   CloseHandle(handle);

   return 1;

#else

   U32 i;
   S16 handle;
   U16 readamt;

   disk_err = 0;

   handle = _lcreat(filename, 0);

   if (handle==-1)
      {
      disk_err = AIL_CANT_WRITE_FILE;
      AIL_set_error("Unable to create file.");
      return 0;
      }

   while (len)
      {
      readamt=(U16) ((len >= (32768-512)) ? (32768-512) : len);

      i = _lwrite(handle,buf,readamt);

      if (i == -1)
         {
         disk_err = AIL_CANT_WRITE_FILE;
         _lclose(handle);
         return 0;
         }

      if (i != readamt)
         {
         disk_err = AIL_DISK_FULL;
         AIL_set_error("Unable to write to file (disk full?).");
         _lclose(handle);
         return 0;
         }

      len -= readamt;
      buf = AIL_ptr_add(buf,readamt);
      }

   _lclose(handle);

   return 1;

#endif
}


//############################################################################
//##                                                                        ##
//##  Write wave file at *buf of length len                                 ##
//##                                                                        ##
//##  Overwrites any existing file                                          ##
//##                                                                        ##
//##  Returns 0 on error, else 1                                            ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_WAV_file_write(char const FAR *filename, void const FAR *buf, U32 len, S32 rate, S32 format)
{
   WAVEOUT wo;

   AIL_memcpy(&wo.riffmark,"RIFF",4);
   wo.rifflen=len+sizeof(WAVEOUT)-8;
   AIL_memcpy(&wo.wavemark,"WAVE",4);
   AIL_memcpy(&wo.fmtmark,"fmt ",4);
   wo.fmtlen=16;
   wo.fmttag=WAVE_FORMAT_PCM;
   wo.channels=(S16)((format&DIG_F_STEREO_MASK)?2:1);
   wo.sampersec=rate;
   wo.bitspersam=(S16)((format&DIG_F_16BITS_MASK)?16:8);
   wo.blockalign=(S16)(((S32)wo.bitspersam*(S32)wo.channels) / 8);
   wo.avepersec=(rate *(S32)wo.bitspersam*(S32)wo.channels) / 8;
   AIL_memcpy(&wo.datamark,"data",4);
   wo.datalen=len;

   {

#ifdef IS_WIN32

   HANDLE handle;
   U32    nbytes;

   disk_err = 0;

   handle = CreateFile(filename,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

   if (handle == INVALID_HANDLE_VALUE)
      {
      disk_err = AIL_CANT_WRITE_FILE;
      AIL_set_error("Unable to create file.");
      return 0;
      }

   WriteFile(handle,&wo,sizeof(wo),&nbytes,0);

   if ((!WriteFile(handle,
                   buf,
                   len,
                  &nbytes,
                   NULL))
        || (nbytes != len))
      {

      disk_err = AIL_DISK_FULL;
      AIL_set_error("Unable to write to file (disk full?).");
      return 0;
      }

   CloseHandle(handle);

   return 1;

#else

   U32 i;
   S16 handle;
   U16 readamt;

   disk_err = 0;

   handle = _lcreat(filename, 0);

   if (handle==-1)
      {
      disk_err = AIL_CANT_WRITE_FILE;
      AIL_set_error("Unable to create file.");
      return 0;
      }

   _lwrite(handle,&wo,sizeof(wo));

   while (len)
      {
      readamt=(U16) ((len >= (32768-512)) ? (32768-512) : len);

      i = _lwrite(handle,buf,readamt);

      if (i == -1)
         {
         disk_err = AIL_CANT_WRITE_FILE;
         _lclose(handle);
         return 0;
         }

      if (i != readamt)
         {
         disk_err = AIL_DISK_FULL;
         AIL_set_error("Unable to write to file (disk full?).");
         _lclose(handle);
         return 0;
         }

      len -= readamt;
      buf = AIL_ptr_add(buf,readamt);
      }

   _lclose(handle);

   return 1;

#endif
  }
}


S32 _cdecl AIL_sprintf            (char FAR *dest,
                                   char const FAR *fmt, ...)
{
  S32 len;

  va_list ap;

  va_start(ap,
           fmt);

  len=vsprintf(dest,
               fmt,
               ap);

  va_end  (ap);

  return( len );

}
