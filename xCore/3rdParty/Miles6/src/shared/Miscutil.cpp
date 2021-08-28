//############################################################################
//##                                                                        ##
//##  MISCUTIL.CPP                                                          ##
//##                                                                        ##
//##  Miscellaneous utility functions                                       ##
//##                                                                        ##
//##  V1.00 of 14-Sep-98: Initial version                                   ##
//##                                                                        ##
//##  Author: John Miles and Jeff Roberts                                   ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include "mss.h"
#include <stdlib.h>
#include <ctype.h>
#include "imssapi.h"

#include "mssdls.h"
#include "dls1.h"
#include "synth.h"

#ifndef IS_WIN32
#include <stdarg.h>
#endif

U8 FAR *ASI_mem_src_ptr;
S32     ASI_mem_src_len;
S32     ASI_mem_src_pos;

// ---------------------------------------------------------------------------
// Stream callback for ASI decoder/encoder
// ---------------------------------------------------------------------------

S32 AILLIBCALLBACK ASI_mem_stream_CB(U32       user, //)
                                  void FAR *dest,
                                  S32       bytes_requested,
                                  S32       offset)
{
   user = user; // for watcom

   if (offset != -1)
      {
      ASI_mem_src_pos = offset;
      }

   if ((ASI_mem_src_pos + bytes_requested) > ASI_mem_src_len)
      {
      bytes_requested = ASI_mem_src_len - ASI_mem_src_pos;
      }

   AIL_memcpy(dest, &ASI_mem_src_ptr[ASI_mem_src_pos], bytes_requested);

   ASI_mem_src_pos += bytes_requested;

   return bytes_requested;
}

//############################################################################
//##                                                                        ##
//## Read MIDI variable-length number                                       ##
//##                                                                        ##
//############################################################################

extern "C" U32 AILCALL XMI_read_VLN(U8 const FAR * FAR *ptr)
{
   U32 val,i,cnt;

   val = 0;
   cnt = 4;

   do
      {
      i = (U32) **ptr;

      *ptr=(U8 FAR*)AIL_ptr_add(*ptr,1);

      val = (val << 7) | (i & 0x7f);

      if (!(i & 0x80))
         {
         break;
         }
      }
   while (--cnt);

   return val;
}

//############################################################################
//##                                                                        ##
//## Return size in bytes of MIDI channel voice message, based on type      ##
//##                                                                        ##
//############################################################################

extern "C" S32 AILCALL XMI_message_size(S32 status)
{
   switch (status & 0xf0)
      {
      case EV_NOTE_OFF  :
      case EV_NOTE_ON   :
      case EV_POLY_PRESS:
      case EV_CONTROL   :
      case EV_PITCH     : return 3;

      case EV_PROGRAM   :
      case EV_CHAN_PRESS: return 2;
      }

   return 0;
}


//############################################################################
//##                                                                        ##
//## Find nth sequence in XMIDI IFF file                                    ##
//##                                                                        ##
//## Returns NULL if not found, else pointer to FORM XMID                   ##
//##                                                                        ##
//############################################################################

extern "C" void const FAR * AILCALL XMI_find_sequence(U8 const FAR *image, S32 sequence)
{
   U32     len;
   U8 const FAR *end;

   len = 0;

   do
      {
      //
      // Skip previous block, if any
      //

      image += len;

      //
      // Exit if not FORM or CAT block
      //

      if ((AIL_strnicmp((char FAR*)image,"FORM",4)) &&
          (AIL_strnicmp((char FAR*)image,"CAT ",4)))
         {
         return NULL;
         }

      //
      // Continue searching if not FORM XMID or CAT XMID
      //
      // XMIDI files always have even FORM lengths; therefore, no
      // odd-byte compensation is needed
      //

      len = 8 + BE_SWAP32( (U32 FAR *) ((U8 FAR *) (image+4)));
      }
   while (AIL_strnicmp((char FAR*)image+8,"XMID",4));

   //
   // If outer header was a FORM, return successfully if first sequence
   // requested
   //
   // If second or higher sequence requested, return NULL if
   // single-sequence FORM encountered
   //

   if (!AIL_strnicmp((char FAR*)image,"FORM",4))
      {
      if (sequence == 0)
         {
         return image;
         }
      else
         {
         return NULL;
         }
      }

   //
   // Calculate ending address of image, and skip 'CAT_nnnnXMID'
   // header to index first FORM chunk in CAT
   //

   end    = image + len;
   image += 12;

   //
   // Find nth FORM XMID chunk in outer CAT XMID block
   //

   while (image < end)
      {
      if (!AIL_strnicmp((char FAR*)image+8,"XMID",4))
         {
         if (!sequence--)
            {
            return image;
            }
         }

      image += 8 + BE_SWAP32( (U32 FAR *) ((U8 FAR *) (image+4)));
      }

   return NULL;
}

#if defined(IS_WINDOWS) || defined(IS_MAC)

//############################################################################
//##                                                                        ##
//## Far strncmp() substitute                                               ##
//##                                                                        ##
//############################################################################

extern "C" S32 AIL_strnicmp(char const FAR *s1, char const FAR *s2, U32 count)
{
   char c1,c2;
   U8 const FAR *st1=(U8 const FAR *)s1;
   U8 const FAR *st2=(U8 const FAR *)s2;
   S32 c=count;

   for(;c;c--) {
     c1=*st1;
     c1=toupper(c1);
     c2=*st2;
     c2=toupper(c2);
     if (c1<c2)
       return(-1);
     if (c1>c2)
       return(1);
     if (c1==0)
       break;
     st1=(U8 const FAR*)AIL_ptr_add(st1,1);
     st2=(U8 const FAR*)AIL_ptr_add(st2,1);
   }

   return(0);
}

//############################################################################
//##                                                                        ##
//## Far strncmp() substitute                                               ##
//##                                                                        ##
//############################################################################

#endif

#ifdef IS_MAC

extern "C" S32 AIL_stricmp(char const FAR *s1, char const FAR *s2)
{
   char c1,c2;
   U8 const FAR *st1=(U8 const FAR *)s1;
   U8 const FAR *st2=(U8 const FAR *)s2;

   while (1) {
     c1=*st1;
     c1=toupper(c1);
     c2=*st2;
     c2=toupper(c2);
     if (c1<c2)
       return(-1);
     if (c1>c2)
       return(1);
     if (c1==0)
       break;
     st1=(U8 const FAR*)AIL_ptr_add(st1,1);
     st2=(U8 const FAR*)AIL_ptr_add(st2,1);
   }

   return(0);
}

#endif


//
// memory file routines
//

#define EACHMEMBUF (63L*1024L)

extern "C" HMEMDUMP AIL_mem_open(void FAR* addr, U32 size)
{
  HMEMDUMP m;

  if ((!addr) || (!size))
    return(0);

  m=(struct _MEMDUMP FAR *) AIL_mem_alloc_lock( sizeof(MEMDUMP) );
  if (m==0)
    return(0);

  m->buffer[0]=addr;

  m->current=(U8 FAR *) addr;
  m->size=0;
  m->error=0;
  m->curbufnum=0;
  m->lastbufnum=0;
  m->totalsize=size;
  m->curpos=0;
  m->expandable=0;
  m->eachbuf=size;

  return(m);
}

extern "C" HMEMDUMP AIL_mem_create()
{
  HMEMDUMP m;

  m=(struct _MEMDUMP FAR *) AIL_mem_alloc_lock(sizeof(MEMDUMP) );
  if (m==0)
    return(0);

  m->buffer[0]=AIL_mem_alloc_lock(EACHMEMBUF);
  if (m->buffer[0]==0) {
    AIL_mem_free_lock(m->buffer[0]);
    return(0);
  }

  m->current=(U8 FAR *) m->buffer[0];
  m->size=0;
  m->error=0;
  m->curbufnum=0;
  m->lastbufnum=0;
  m->totalsize=0;
  m->curpos=0;
  m->expandable=1;
  m->eachbuf=EACHMEMBUF;

  return(m);
}

extern "C" S32 AIL_mem_close(HMEMDUMP m, void FAR* FAR* buf, U32 FAR* size)
{
  S32 ret=1;

  if (m) {
    U32 i;

    if (size)
      *size=m->totalsize;

    if (buf) {
      char FAR* b;
      b=(C8 FAR *) AIL_mem_alloc_lock(m->totalsize);
      *buf=(void FAR*)b;
      if (b) {

        i=0;
        while (m->totalsize) {
          U32 thissize=(m->totalsize>=m->eachbuf)?m->eachbuf:m->totalsize;
          AIL_memcpy(b,m->buffer[i++],thissize);
          b=(C8 FAR *)AIL_ptr_add(b,thissize);
          m->totalsize-=thissize;
        }

      } else {
        ret=0;
      }

    }

    for(i=0;i<=m->lastbufnum;i++)
      AIL_mem_free_lock(m->buffer[i]);

    AIL_mem_free_lock(m);
  }
  return(ret);
}

static void memcopydata(HMEMDUMP m,void const FAR* s, U32 bytes)
{
  AIL_memcpy(m->current,s,bytes);

  m->current=(U8 FAR *) AIL_ptr_add(m->current,bytes);
  m->size+=bytes;

  m->curpos+=bytes;
  if (m->curpos>m->totalsize)
    m->totalsize=m->curpos;
}

extern "C" U32 AIL_mem_write(HMEMDUMP m, void const FAR* s,U32 bytes)
{
  if ((m) && (bytes) && (m->error==0)) {

    U32 amt;
    U32 wrote=0;

   again:

    amt=(m->eachbuf-m->size);
    amt=(bytes>=amt)?amt:bytes;

    if (amt) {
      memcopydata(m,s,amt);
      bytes-=amt;
      wrote+=amt;
    }

    if (bytes) {

      // we need to bump a buffer

      s=AIL_ptr_add(s,amt);

      if (m->curbufnum==m->lastbufnum) {

        // we at the end of the file

        if ((m->curbufnum>=(sizeof(m->buffer)/4)) || (!m->expandable)) {
          m->error=1;
          return(0);
        }

        m->buffer[m->curbufnum+1]=AIL_mem_alloc_lock(EACHMEMBUF);
        if (m->buffer[m->curbufnum+1]==0) {
          m->error=1;
          return(0);
        }

        ++m->lastbufnum;
      }

      ++m->curbufnum;
      m->current=(U8 FAR*)m->buffer[m->curbufnum];
      m->size=0;

      goto again;

    }

    return(wrote);
  }
  return(0);
}

static void memcopytodata(HMEMDUMP m,void FAR* d, U32 bytes)
{
  AIL_memcpy(d,m->current,bytes);

  m->current=(U8 FAR *)AIL_ptr_add(m->current,bytes);
  m->size+=bytes;

  m->curpos+=bytes;
}

extern "C" U32 AIL_mem_read(HMEMDUMP m, void FAR* s,U32 bytes)
{
  if ((m) && (bytes) && (m->error==0)) {
    U32 amt;
    U32 read=0;

   again:

    amt=(m->eachbuf-m->size);
    amt=(bytes>=amt)?amt:bytes;

    if (amt) {
      memcopytodata(m,s,amt);
      bytes-=amt;
      read+=amt;
    }

    if (bytes) {

      // we're at the end

      s=AIL_ptr_add(s,amt);

      if (m->curbufnum==m->lastbufnum)
        return(amt);

      ++m->curbufnum;
      m->current=(U8 FAR*)m->buffer[m->curbufnum];
      m->size=0;

      goto again;
    }

    return(read);
  }
  return(0);
}

extern "C" U32
#ifndef IS_MAC
_cdecl
#endif
AIL_mem_printf(HMEMDUMP m, char const FAR* fmt, ...)
{
  char work_string[1024];
  va_list ap;

  va_start(ap,
           fmt);

  vsprintf(work_string,
            fmt,
            ap);

  va_end  (ap);

  return( AIL_mem_write(m,work_string,AIL_strlen(work_string)) );
}

extern "C" U32 AIL_mem_printc(HMEMDUMP m, char c)
{
  return( AIL_mem_write(m,&c,1) );
}

extern "C" U32 AIL_mem_prints(HMEMDUMP m, char const FAR* s)
{
  return( AIL_mem_write(m,s,AIL_strlen(s)) );
}

extern "C" U32 AIL_mem_seek(HMEMDUMP m, U32 pos)
{
  if (m) {

    if (pos>m->totalsize)
      pos=m->totalsize;

    m->curbufnum=pos/EACHMEMBUF;
    m->size=pos%EACHMEMBUF;
    m->current=(U8 FAR*)AIL_ptr_add(m->buffer[m->curbufnum],m->size);
    m->curpos=pos;

    return(m->curpos);

  }
  return(0);
}

extern "C" U32 AIL_mem_size(HMEMDUMP m)
{
  return( m?m->totalsize:0);
}

extern "C" U32 AIL_mem_pos(HMEMDUMP m)
{
  return( m?m->curpos:0);
}

extern "C" S32 AIL_mem_error(HMEMDUMP m)
{
  return( m?m->error:0);
}

typedef struct FHVALUES
{
  S32 hand;
  S32 length;
} FHVALUES;

static FHVALUES get_file_handle_values(char const FAR* str)
{
  FHVALUES v;
  char const FAR* s;

  v.hand=0;
  s=str;
  if ((*s)=='-')
    ++s;
  while (((*s)>='0') && ((*s)<='9'))
    v.hand=(v.hand*10)+((*s++)-'0');
  if ((*str)=='-')
    v.hand=-v.hand;

  if ((*s)==',')
  {
    v.length=0;
    str=++s;
    if ((*s)=='-')
      ++s;
    while (((*s)>='0') && ((*s)<='9'))
      v.length=(v.length*10)+((*s++)-'0');
    if ((*str)=='-')
      v.length=-v.length;
  }
  else
  {
    v.length=-1;
  }
  return(v);
}

#ifdef IS_MAC

#include "devices.h"

#define MAX_ASYNCS 4

typedef struct MACASYNC
{
  IOParam stream_param;
  S32 in_use;
} MACASYNC;

typedef struct PLATFILE
{
  U32 offset;
  U32 file_length;
  U32 start_pos;
  MACASYNC asyncs[MAX_ASYNCS];
  S32 dont_close;
  short hand;
} PLATFILE;

#define BADFILEHANDLE 0

static U32 AIL_lowopen( S32 frn, S32 len )
{
  PLATFILE FAR* pf;

  if (frn!=BADFILEHANDLE)
  {
    pf=(PLATFILE FAR*)AIL_mem_alloc_lock(sizeof(PLATFILE));
    if (pf==0)
    {
      return( 0 );
    }
    AIL_memset(pf,0,sizeof(PLATFILE));

    pf->hand=(short)frn;

    ::GetFPos(pf->hand,(long FAR*)&pf->start_pos);

    if (len==-1)
    {
      ::SetFPos(pf->hand,fsFromLEOF,0);
      ::GetFPos(pf->hand,(long FAR*)&pf->file_length);
      ::SetFPos(pf->hand,fsFromStart,pf->start_pos);
      pf->file_length-=pf->start_pos;
    }
    else
    {
      pf->file_length=len;
    }

    return( (U32)pf );
  }

  return( 0 );
}

static U32 AIL_fhopen( FHVALUES vals )
{
  U32 fh=AIL_lowopen( vals.hand,vals.length );
  
  if (fh)
  {
    ((PLATFILE FAR FAR*)fh)->dont_close=1;
  }
  
  return( fh );
}

static U32 AIL_fopen( FSSpec const FAR* name,U32 acc )
{
  U32 fh;

  short frn = BADFILEHANDLE;
  ::FSpOpenDF(name,0,&frn);

  fh=AIL_lowopen( frn, -1 );
  
  if ((fh==0) && (frn!=BADFILEHANDLE))
    ::FSClose( frn );

  return( fh );
}

static void AIL_fclose( U32 hand )
{
  if (hand != 0)
  {
    PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;
    if (!pf->dont_close)
      ::FSClose( pf->hand );
    AIL_mem_free_lock( pf );
  }
}

static S32 AIL_fseekbegin(U32 hand,S32 offset)
{
  PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;

  pf->offset = offset;

  if (pf->offset>pf->file_length)
    pf->offset=pf->file_length;

  return( pf->offset );
}

static S32 AIL_fseekcur(U32 hand,S32 offset)
{
  PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;

  pf->offset += offset;

  if (pf->offset>pf->file_length)
    pf->offset=pf->file_length;

  return( pf->offset );
}

static S32 AIL_fseekend(U32 hand,S32 offset)
{
  PLATFILE FAR * pf = ( PLATFILE FAR* ) hand;

  pf->offset = pf->file_length + offset;

  if (pf->offset>pf->file_length)
    pf->offset=pf->file_length;

  return( pf->offset );
}

static void FAR* AILLIBCALLBACK def_async_read (U32 FileHandle,
                                                void FAR* Buffer,
                                                U32 Bytes)
{
  int i;
  PLATFILE FAR* pf = ( PLATFILE FAR* ) FileHandle;

  for (i = 0 ; i < MAX_ASYNCS ; i++ )
  {
    if ( pf->asyncs[i].in_use == 0 )
    {
      OSErr err;

      if ((pf->file_length-pf->offset)<Bytes)
        Bytes=pf->file_length-pf->offset;

      pf->asyncs[i].in_use = 1;
      pf->asyncs[i].stream_param.ioRefNum = pf->hand;
      pf->asyncs[i].stream_param.ioCompletion = 0;
      pf->asyncs[i].stream_param.ioBuffer = (char FAR*)Buffer;
      pf->asyncs[i].stream_param.ioReqCount = Bytes;
      pf->asyncs[i].stream_param.ioPosMode = fsFromStart|noCacheMask;
  	  pf->asyncs[i].stream_param.ioPosOffset = pf->offset+pf->start_pos;
  	  pf->asyncs[i].stream_param.ioActCount = 0;
  	  pf->asyncs[i].stream_param.ioResult = 0;
      pf->offset += Bytes;

      err = PBRead((ParamBlockRec FAR*)&(pf->asyncs[i]),1);

      if ( err != noErr )
      {
        pf->asyncs[i].in_use = 0;
        return( 0 );
      }
      else
      {
        return( &(pf->asyncs[i]) );
      }
    }
  }

  return( 0 );
}

static S32 AILLIBCALLBACK def_async_status  (void FAR* async,
                                             S32 wait,
                                             U32 FAR* BytesRead)
{
  if (async == 0)
  {
    return( 1 );
  }
  else
  {
  	MACASYNC FAR* ma = (MACASYNC FAR*)async;

    if (wait)
    {
      while (ma->stream_param.ioResult > noErr ) {};
    }
    else
    {
      if (ma->stream_param.ioResult > noErr )
        return(0);
    }

    if (BytesRead)
      *BytesRead = ma->stream_param.ioActCount;

    ma->in_use = 0;
    return( 1 );
  }
}


#else

#ifdef IS_DOS

#define BADFILEHANDLE 0
#define AIL_doread(file,addr,size,ar)  ar=AIL_fread(file,addr,size)

#else

#ifdef IS_WIN32

#define BADFILEHANDLE INVALID_HANDLE_VALUE
#define AIL_lowfopen(name,acc)           ((U32)CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,0))
#define AIL_lowfclose(hand)              CloseHandle((HANDLE)hand)
#define AIL_lowfseekbegin(hand,offset)   SetFilePointer((HANDLE)(hand),offset,0,FILE_BEGIN)
#define AIL_lowfseekcur(hand,offset)     SetFilePointer((HANDLE)(hand),offset,0,FILE_CURRENT)
#define AIL_lowfseekend(hand,offset)     SetFilePointer((HANDLE)(hand),offset,0,FILE_END)
#define AIL_doread(hand,addr,size,ar)    ReadFile((HANDLE)(hand),addr,size,&ar,0)

#else

#define BADFILEHANDLE HFILE_ERROR
#define AIL_lowfopen(name,acc)           ((U32)_lopen(name,OF_READ))
#define AIL_lowfclose(file)              _lclose((HFILE)(file))
#define AIL_lowfseekbegin(file,offset)   _llseek((HFILE)(file),offset,SEEK_SET)
#define AIL_lowfseekcur(file,offset)     _llseek((HFILE)(file),offset,SEEK_CUR)
#define AIL_lowfseekend(file,offset)     _llseek((HFILE)(file),offset,SEEK_END)
#define AIL_doread(file,addr,size,ar)  ar=_hread((HFILE)(file),addr,size)

#endif

#endif

typedef struct PLATFILE
{
  U32 hand;
  U32 file_length;
  U32 start_pos;
  S32 dont_close;
} PLATFILE;

static U32 AIL_lowopen( S32 hand, S32 len )
{
  PLATFILE FAR* pf;

  if (hand!=(S32)BADFILEHANDLE)
  {
    pf=(PLATFILE FAR*)AIL_mem_alloc_lock(sizeof(PLATFILE));
    if (pf==0)
    {
      return( 0 );
    }
    AIL_memset(pf,0,sizeof(PLATFILE));

    pf->hand=hand;

    pf->start_pos=AIL_lowfseekcur(hand,0);

    if (len==-1)
    {
      pf->file_length=AIL_lowfseekend(hand,0);
      AIL_lowfseekbegin(hand,pf->start_pos);
      pf->file_length-=pf->start_pos;
    }
    else
    {
      pf->file_length=len;
    }

    return( (U32)pf );
  }

  return( 0 );
}


static U32 AIL_fopen( char const FAR* name,U32 acc )
{
  U32 hand;
  U32 fh;

  hand=AIL_lowfopen( name, acc );

  fh=AIL_lowopen( hand, -1 );

  if ((fh==0) && ( hand != (U32)BADFILEHANDLE ))
    AIL_lowfclose( hand );

  return( fh );
}

static U32 AIL_fhopen( FHVALUES vals )
{
  U32 fh=AIL_lowopen( vals.hand,vals.length );
  if (fh)
  {
    ((PLATFILE FAR*)fh)->dont_close=1;
  }
  return( fh );
}

static S32 AIL_fseekbegin(U32 hand,S32 offset)
{
  PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;

  if ((U32)offset>pf->file_length)
    offset=pf->file_length;

  return( AIL_lowfseekbegin(pf->hand,offset+pf->start_pos ) - pf->start_pos );
}

static S32 AIL_fseekcur(U32 hand,S32 offset)
{
  PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;

  U32 new_pos = (AIL_lowfseekcur(pf->hand,offset ) - pf->start_pos) ;

  if (new_pos > pf->file_length)
    return( AIL_lowfseekend(pf->hand,0) );

  return( new_pos );
}

static S32 AIL_fseekend(U32 hand,S32 offset)
{
  PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;

  offset = pf->file_length + offset;

  if ((U32)offset>pf->file_length)
    offset=pf->file_length;

  return( AIL_lowfseekbegin(pf->hand,offset+pf->start_pos ) - pf->start_pos );
}

static void AIL_fclose( U32 hand )
{
  if (hand!=0)
  {
    PLATFILE FAR* pf = ( PLATFILE FAR* ) hand;
    if (!pf->dont_close)
      AIL_lowfclose( pf->hand );
    AIL_mem_free_lock( pf );
  }
}

#endif

extern "C"
{

static U32 AILLIBCALLBACK def_open(MSS_FILE const FAR* File,
                                   U32 FAR* FileHandle)
{
  U32 fh;

#ifdef IS_MAC
  if (File->file_type==0)
  {
    Str255 str;
    OSErr err;
    FSSpec spec;

    if (AIL_strnicmp((char FAR*)File->file,"\\\\\\\\",4)==0)
    {
      fh=AIL_fhopen(get_file_handle_values(((char FAR *)File->file)+4));
    }
    else
    {
      str[ 0 ] = AIL_strlen( (char FAR*)File->file );
      AIL_memcpy( str+1, File->file, str[ 0 ] );
      err = FSMakeFSSpec(0,0,str,&spec);
      if( err != noErr )
      {
        disk_err = AIL_FILE_NOT_FOUND;
        return(0);
      }
      fh=AIL_fopen(&spec,0);
    }
  }
  else
    fh=AIL_fopen(( FSSpec const FAR* )File->file,0);
#else
    if (AIL_strnicmp(File,"\\\\\\\\",4)==0)
    {
      fh=AIL_fhopen(get_file_handle_values(File+4));
    }
    else
    {
      fh=AIL_fopen(File,0);
    }
#endif

  if (fh==(U32)0)
    return(0);

  *FileHandle=fh;
  return(1);
}

static void AILLIBCALLBACK def_close(U32 FileHandle)
{
  AIL_fclose(FileHandle);
}


static S32 AILLIBCALLBACK def_seek(U32 FileHandle,
                         S32 Offset,
                         U32 Type)
{
  if (Type==AIL_FILE_SEEK_END)
    return(AIL_fseekend(FileHandle,Offset));
  else if (Type==AIL_FILE_SEEK_CURRENT)
    return(AIL_fseekcur(FileHandle,Offset));
  else
    return(AIL_fseekbegin(FileHandle,Offset));
}


AIL_file_open_callback MSS_open=def_open;
AIL_file_close_callback MSS_close=def_close;
AIL_file_seek_callback MSS_seek=def_seek;

S32 disk_err = 0;


#if defined(IS_WIN32) || defined(IS_MAC)

static U32 AILLIBCALLBACK def_read_with_async( U32 FileHandle,
                                     void FAR* Buffer,
                                     U32 Bytes )
{
  void FAR* a = MSS_async_read( FileHandle, Buffer, Bytes );
  if ( a )
  {
    U32 bytes = 0;
    MSS_async_status( a, 1, &bytes );
    return( bytes );
  }
  return( 0 );
}

#endif

#ifdef IS_MAC

AIL_file_read_callback MSS_read=def_read_with_async;
AIL_file_async_read_callback MSS_async_read=def_async_read;
AIL_file_async_status_callback MSS_async_status=def_async_status;

#else

#ifdef IS_WIN32

AIL_file_async_read_callback MSS_async_read=0;
AIL_file_async_status_callback MSS_async_status=0;

#endif

static U32 AILLIBCALLBACK def_read(U32 FileHandle,
                         void FAR* Buffer,
                         U32 Bytes)
{
  U32 amt=0;
  AIL_doread(((PLATFILE FAR*)FileHandle)->hand,Buffer,Bytes,amt);
  return(amt);
}

AIL_file_read_callback MSS_read=def_read;

#endif

#if defined(IS_WIN32) || defined(IS_MAC)

DXDEC  void  AILEXPORT AIL_set_file_async_callbacks (AIL_file_open_callback opencb,
                                                   AIL_file_close_callback closecb,
                                                   AIL_file_seek_callback seekcb,
                                                   AIL_file_async_read_callback areadcb,
                                                   AIL_file_async_status_callback statuscb)
{
  MSS_open=(opencb)?opencb:def_open;
  MSS_close=(closecb)?closecb:def_close;
  MSS_seek=(seekcb)?seekcb:def_seek;
  if ((areadcb) && (statuscb))
  {
    MSS_async_read=areadcb;
    MSS_async_status=statuscb;
    MSS_read=def_read_with_async;
  }
  else
  {
    MSS_async_read=0;
    MSS_async_status=0;
    MSS_read=0;
  }
}


#endif


DXDEC  void  AILEXPORT AIL_set_file_callbacks  (AIL_file_open_callback opencb,
                                                AIL_file_close_callback closecb,
                                                AIL_file_seek_callback seekcb,
                                                AIL_file_read_callback readcb)
{
  MSS_open=(opencb)?opencb:def_open;
  MSS_close=(closecb)?closecb:def_close;
  MSS_seek=(seekcb)?seekcb:def_seek;
  MSS_read=(readcb)?readcb:
#ifdef IS_MAC
    def_read_with_async;
#else
    def_read;
#endif    
}


//############################################################################
//##                                                                        ##
//##  Return most recent system error code                                  ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_file_error(void)
{
   return disk_err;
}

//############################################################################
//##                                                                        ##
//##  Return size of file, or -1 if error                                   ##
//##                                                                        ##
//############################################################################

#ifdef IS_MAC
static S32 file_size(MSS_FILE const FAR *filename)
#else
S32 AILCALL AIL_API_file_size(char const FAR *filename)
#endif
{
   U32    handle;
   U32    len;

   disk_err = 0;


   if (!MSS_open(filename,&handle))
   {
      AIL_set_error("Unable to open file.");
      disk_err = AIL_FILE_NOT_FOUND;
      return -1;
   }

   len = MSS_seek(handle, 0, AIL_FILE_SEEK_END);

   MSS_close(handle);

   return len;
}


//############################################################################
//##                                                                        ##
//##  Read file into memory at *dest, allocating memory first if dest=NULL  ##
//##                                                                        ##
//############################################################################

#ifdef IS_MAC
static void FAR * file_read(MSS_FILE const FAR *filename, void FAR *dest)
#else
void FAR * AILCALL AIL_API_file_read(char const FAR *filename, void FAR *dest)
#endif
{
   U32 handle;
   U32 i;
   U32 len;
   U32 readamt;
   U8 FAR *buf, FAR *mem;

   disk_err = 0;

   if (!MSS_open(filename, &handle))
   {
      disk_err = AIL_FILE_NOT_FOUND;
      AIL_set_error("Unable to open file.");
      return NULL;
   }

   len = MSS_seek(handle,0,AIL_FILE_SEEK_END);
   MSS_seek(handle,0,AIL_FILE_SEEK_BEGIN);

   buf = mem =(U8 FAR*)( ((dest == NULL) || (dest==FILE_READ_WITH_SIZE))? AIL_mem_alloc_lock(len+4) : dest);

   if (buf==NULL)
   {
      disk_err = AIL_OUT_OF_MEMORY;
      AIL_set_error("Out of memory.");
      MSS_close(handle);
      return NULL;
   }

   if (dest==FILE_READ_WITH_SIZE)
   {
      *((S32 FAR*)buf)=len;
      buf+=4;
   }

   while (len)
   {
      readamt=(U16) ((len >= 32768) ? 32768 : len);

      i = MSS_read(handle,buf,readamt);

      if (i != readamt)
      {
         if (dest!=mem)
           AIL_mem_free_lock(mem);
         MSS_close(handle);
         disk_err = AIL_CANT_READ_FILE;
         AIL_set_error("Unable to read file.");

         return NULL;
      }

      len -= readamt;
      buf = (U8 FAR*)AIL_ptr_add(buf,readamt);
   }

   MSS_close(handle);

   return mem;
}

#ifdef IS_MAC

//############################################################################
//##                                                                        ##
//##  Return size of file, or -1 if error                                   ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_API_file_size(char const FAR *filename)
{
  MSS_FILE file;
  file.file_type=0;
  file.file=filename;

  return( file_size( &file ) );
}

S32 AILCALL AIL_API_file_fss_size(FSSpec const FAR *filespec)
{
  MSS_FILE file;
  file.file_type=1;
  file.file=filespec;

  return( file_size( &file ) );
}

//############################################################################
//##                                                                        ##
//##  Read file into memory at *dest, allocating memory first if dest=NULL  ##
//##                                                                        ##
//############################################################################

void FAR * AILCALL AIL_API_file_read(char const FAR *filename, void FAR *dest)
{
  MSS_FILE file;
  file.file_type=0;
  file.file=filename;

  return( file_read( &file, dest) );
}

void FAR * AILCALL AIL_API_file_fss_read(FSSpec const FAR *filespec, void FAR *dest)
{
  MSS_FILE file;
  file.file_type=1;
  file.file=filespec;

  return( file_read( &file, dest) );
}

#endif

}
