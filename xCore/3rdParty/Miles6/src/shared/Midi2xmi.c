//###########################################################################
//##                                                                       ##
//##   MIDI2XMI.CPP                                                        ##
//##                                                                       ##
//##   Converts standard MIDI file (.MID) image into equivalent Extended   ##
//##   MIDI (XMIDI) file (.XMI) image                                      ##
//##                                                                       ##
//##   V1.00 of 28-Nov-95: Initial, derived from MIDIFORM 1.05             ##
//##                                                                       ##
//##    Author: John Miles                                                 ##
//##                                                                       ##
//##   C source compatible with 32-bit ANSI C                              ##
//##                                                                       ##
//###########################################################################

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "mss.h"

#include "imssapi.h"

#define EVNT_AVAIL (256L*1024L)  // Allow up to 256K of output XMIDI event data
#define MAX_RBRN 128       // Max. # of unique branch targets in MIDI file
#define MAX_EVLEN 1536     // Max. length of MIDI event in bytes
#define MAX_TIMB 16384     // Max. # of timbre requests in MIDI file
#define MAX_TRKS 64        // Max. # of tracks in MIDI input file
#define CHAN_CNT 16        // # of MIDI channels

#define AIL_BRANCH_PT 120  // AIL XMIDI controller for branch points
#define AIL_TIMB_BNK  114  // AIL XMIDI controller: Timbre Bank Select
#define EV_INVALID 0x00

#define META_TRK_NAME 0x03
#define META_INS_NAME 0x04

#define MAX_ULONG 4294967295L


typedef struct
{
   U16 bnum;
   U32 offset;
}
RBRN_mx_entry;

typedef struct
{
   U8 t_num;
   U8 t_bank;
}
TIMB_mx_entry;

typedef struct
{
   U32 quantization;
}
AILH_block;

typedef struct
{
   U16        cnt;
   RBRN_mx_entry brn[MAX_RBRN];
}
RBRN_block;

typedef struct
{
   U16        cnt;
   TIMB_mx_entry tbr[MAX_TIMB];
}
TIMB_block;

typedef struct
{
   U32 len;
   U32 avail;
   U8 FAR*name;
   U8 FAR*base;
   U8 FAR*ptr;
}
IFF_block;

typedef struct _MIDI
{
   U8 FAR*base;
   U32 seq_len;

   U32 format;
   U32 ntrks;
   U32 division;
   U32 tick_time;
   U8 event[MAX_EVLEN];
   U32 event_len;
   U32 event_time;
   U32 event_chan;

   U32 event_trk;
   U8 FAR*trk_ptr[MAX_TRKS];
   U8 status[MAX_TRKS];
   U8 trk_active[MAX_TRKS];
   U32 pending_delta[MAX_TRKS];
}
MIDI;

typedef struct _XMIDI
{
   U32 q_int;
   F64 DDA_sum;
   U32 interval;
   U32 delta;
   U8 note_chan[MAX_NOTES];
   U8 note_num[MAX_NOTES];
   U32 note_intvl[MAX_NOTES];
   U8 FAR*note_next[MAX_NOTES];
   U8 timb_BNK[CHAN_CNT];
   U32 rbs_BNK[CHAN_CNT];
   AILH_block AILH;
   RBRN_block RBRN;
   TIMB_block TIMB;
   IFF_block EVNT;
}
XMIDI;

static jmp_buf errhand;
static S32 lenient;

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#define LOCK(x)   AIL_vmm_lock  (&(x),sizeof(x))
#define UNLOCK(x) AIL_vmm_unlock(&(x),sizeof(x))

static S32 locked = 0;

static void AIL_lock_end(void);

static void AIL_lock_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AIL_lock_start, AIL_lock_end);

      LOCK(errhand);
      LOCK(lenient);

      locked = 1;
      }
}

#define DOLOCK() AIL_lock_start()

#else

#define DOLOCK()

#endif

//###########################################################################
static void abend(S32 err, U32 info_1)
{
   if (!err) return;

   switch (err)
      {
      case 1:
         switch (info_1)
            {
            case 2:
               AIL_set_error("Out of memory.");
               break;
            default:
               AIL_set_error("Undefined file error.");
               break;
            }
         break;
      case 2:
         AIL_set_error("File not a standard MIDI file.");
         break;
      case 3:
         AIL_set_error("MIDI event too long in file.");
         break;
      case 4:
         AIL_set_error("Illegal MIDI status byte in file.");
         break;
      case 5:
         AIL_set_error("Illegal MIDI event in file.");
         break;
      case 6:
         AIL_set_error("Too many tracks in MIDI file.");
         break;
      case 7:
         AIL_set_error("Out of memory.");
         break;
      case 8:
         AIL_set_error("Unpaired MIDI note-on event in file.");
         if (lenient) return;
         break;
      case 9:
         AIL_set_error("Too many simultaneous notes in file.");
         if (lenient) return;
         break;
      case 10:
         AIL_set_error("Too many branch point controllers in file.");
         if (lenient) return;
         break;
      case 11:
         AIL_set_error("Too many timbre request controller pairs in file.");
         break;
      case 12:
         AIL_set_error("Duplicate branch point controller in file.");
         if (lenient) return;
         break;
      }

   longjmp(errhand,1);
}



#ifdef IS_MAC

#define read32(ptr) (*((U32*)ptr))
#define read16(ptr) (*((U16*)ptr))

#else

#ifdef IS_32

#define read32(ptr) BE_SWAP32(((U32*)ptr))
#define read16(ptr) BE_SWAP16(((U16*)ptr))

#else

static U32 read32(void FAR* ptr)
{
  char buf[4];

  if ((((U32)ptr)&0xffff)<=0xfffc)
    return( (*((U32 FAR*)ptr)) );

  buf[3]=*((U8 FAR*)ptr);
  ptr=AIL_ptr_add(ptr,1);
  buf[2]=*((U8 FAR*)ptr);
  ptr=AIL_ptr_add(ptr,1);
  buf[1]=*((U8 FAR*)ptr);
  ptr=AIL_ptr_add(ptr,1);
  buf[0]=*((U8 FAR*)ptr);

  return(*((U32 FAR*)buf));
}

static U16 read16(void FAR* ptr)
{
  char buf[2];

  if ((((U32)ptr)&0xffff)<=0xfffe)
    return( (*((U16 FAR*)ptr)) );

  buf[1]=*((U8 FAR*)ptr);
  ptr=AIL_ptr_add(ptr,1);
  buf[0]=*((U8 FAR*)ptr);

  return(*((U16 FAR*)buf));
}

#endif
#endif

//#############################################################################
static S32 IFF_append_CAT(HMEMDUMP out, S8 FAR*CAT_type)
{
   static U32 len = 0L;

   AIL_mem_prints(out,"CAT ");
   AIL_mem_write(out,&len,4);
   AIL_mem_prints(out,(char*)CAT_type);

   return (!AIL_mem_error(out));
}

//#############################################################################
static void IFF_close_CAT(HMEMDUMP out, U32 len_off)
{
   static U32 len;

   len = AIL_mem_pos(out)-len_off+4L;
   MEM_BE_SWAP32( &len );
   AIL_mem_seek(out,len_off-8L);
   AIL_mem_write(out,&len,4);
}

//#############################################################################
static U32 IFF_construct(IFF_block FAR*BLK)
{
   BLK->ptr = BLK->base;
   BLK->len = 0L;

   return 0L;
}

static S32 IFF_write_block(HMEMDUMP out, IFF_block FAR*BLK)
{
   U32 len,blen;
   U8 FAR*ptr;

   ptr = BLK->base;
   len = BLK->len;

   blen = len + (len & 1L);
   MEM_BE_SWAP32( &blen );

   AIL_mem_prints(out,(char FAR*) BLK->name);
   AIL_mem_write(out,&blen,4);

   AIL_mem_write(out,ptr,len);

   if (blen & 1L) AIL_mem_printc(out,0);

   return (!AIL_mem_error(out));
}

//######################################################################
static S32 IFF_append_FORM(HMEMDUMP out, S8 FAR*FORM_type, HMEMDUMP in)
{
   static S8 buff[512];
   U32 len,blen;

   AIL_mem_seek(in,0L);

   len = AIL_mem_size(in);

   blen = len + (len & 1L) + 4L;
   MEM_BE_SWAP32( &blen );

   AIL_mem_prints(out,"FORM");
   AIL_mem_write(out,&blen,4);

   AIL_mem_prints(out,(char*)FORM_type);

   blen = len;
   while (len > 512L)
      {
      AIL_mem_read(in,buff,512);
      AIL_mem_write(out,buff,512);
      len -= 512L;
      }
   AIL_mem_read(in,buff,len);
   AIL_mem_write(out,buff,len);

   if (blen & 1L) AIL_mem_printc(out,0);

   return (!AIL_mem_error(out));
}

//######################################################################
static void IFF_put_byte(U32 val, IFF_block FAR*BLK)
{
   *BLK->ptr = (U8) val;
   BLK->ptr=( U8 FAR* )AIL_ptr_add(BLK->ptr,1);
   BLK->len++;
}

//######################################################################
static U32 MIDI_get_chr(MIDI FAR*MIDI, U32 trk)
{
   U32 val;
   U8 FAR*ptr = MIDI->trk_ptr[trk];

   val = (U32) *ptr;

   ptr=( U8 FAR* ) AIL_ptr_add(ptr,1);

   MIDI->trk_ptr[trk] = ptr;

   return val;
}

static U32 MIDI_next_chr(MIDI FAR*MIDI, U32 trk)
{
   U32 val;

   val = (U32) *MIDI->trk_ptr[trk];

   return val;
}

static U32 MIDI_get_vln(MIDI FAR*MIDI, U32 trk)
{
   U32 val=0L;
   U32 i,cnt=4;

   do
      {
      i = MIDI_get_chr(MIDI,trk);
      val = (val << 7) | (U32) (i & 0x7f);
      if (!(i & 0x80))
         cnt = 0;
      else
         --cnt;
      }
   while (cnt);

   return val;
}

static U32 MIDI_vln_size(U32 val)
{
   U32 cnt=0;

   do
      {
      cnt++;
      val >>= 7;
      }
   while (val);

   return cnt;
}

static U32 MIDI_put_vln(U32 val, U8 FAR*ptr)
{
   U32 i,n,cnt;
   U8 bytefield[4];

   bytefield[3] = (U8)((val & 0x7fL));
   bytefield[2] = (U8)(((val >> 7) & 0x7fL) | 0x80);
   bytefield[1] = (U8)(((val >> 14) & 0x7fL) | 0x80);
   bytefield[0] = (U8)(((val >> 21) & 0x7fL) | 0x80);

   n=3;
   for (i=0;i<=3;i++)
      if (bytefield[i] & 0x7f)
         {
         n=i;
         break;
         }

   cnt=0;
   for (i=n;i<=3;i++)
      {
      *ptr = bytefield[i];
      ptr=( U8 FAR* ) AIL_ptr_add(ptr,1);
      ++cnt;
      }

   return cnt;
}

static U32 MIDI_get_24(MIDI FAR*MIDI, U32 trk)
{
   U32 val;

   val = (U32) MIDI_get_chr(MIDI,trk);
   val = (val << 8) | (U32) MIDI_get_chr(MIDI,trk);
   val = (val << 8) | (U32) MIDI_get_chr(MIDI,trk);

   return val;
}

//######################################################################
static U32 MIDI_construct(MIDI FAR*MIDI)
{
   U32 chunk_len;
   U32 trk,bad;
   U8 FAR*src;
   U32 len;

   src = MIDI->base;
   len = MIDI->seq_len;
   bad=1;
   while (len-- >= 4L)
      {
      if (!AIL_strnicmp((char FAR*) src,"MThd",4))
         {
         bad=0;
         break;
         }
      src=( U8 FAR* ) AIL_ptr_add(src,1);
      };
   if (bad) return 2;

   chunk_len = read32(AIL_ptr_add(src,4));

   MIDI->ntrks = read16(AIL_ptr_add(src,10));

   if (MIDI->ntrks > MAX_TRKS) return 6;

   MIDI->format = read16(AIL_ptr_add(src,8));
   MIDI->division = read16(AIL_ptr_add(src,12));

   MIDI->tick_time = (50000000L) / (U32) MIDI->division;

   MIDI->event_time = 0L;
   MIDI->event_trk = MIDI->ntrks - 1;

   src=( U8 FAR* ) AIL_ptr_add(src,chunk_len+8L);

   trk = 0;
   do
      {
      chunk_len = read32( AIL_ptr_add(src,4));

      if (!AIL_strnicmp((char FAR*) src,"MTrk",4))
         {
         MIDI->trk_ptr[trk] = ( U8 FAR* ) AIL_ptr_add(src , 8);
         MIDI->status[trk] = 0;
         MIDI->trk_active[trk] = 1;
         MIDI->pending_delta[trk] = MIDI_get_vln(MIDI,trk);
         trk++;
         }
      src=( U8 FAR* ) AIL_ptr_add(src,chunk_len+8L);
      }
   while (trk < MIDI->ntrks);

   return 0;
}

//######################################################################
static U32 MIDI_event_type(MIDI FAR*MIDI)
{
   switch (MIDI->event[0] & 0xf0)
      {
      case EV_NOTE_OFF:
         return EV_NOTE_OFF;
      case EV_NOTE_ON:
         return (MIDI->event[2]) ? EV_NOTE_ON : EV_NOTE_OFF;
      case EV_POLY_PRESS:
         return EV_POLY_PRESS;
      case EV_CONTROL:
         return EV_CONTROL;
      case EV_PROGRAM:
         return EV_PROGRAM;
      case EV_CHAN_PRESS:
         return EV_CHAN_PRESS;
      case EV_PITCH:
         return EV_PITCH;
      case EV_SYSEX:
         switch (MIDI->event[0])
            {
            case EV_SYSEX: return EV_SYSEX;
            case EV_ESC: return EV_SYSEX;
            case EV_META: return EV_META;
            }
      default:
         return EV_INVALID;
      }
}

//######################################################################
static U32 MIDI_get_event(MIDI FAR*MIDI, S32 trk)
{
   S32 type;
   U32 cnt,len;
   U8 FAR*temp;

   if (MIDI_next_chr(MIDI,trk) >= 0x80)
      MIDI->status[trk] = (U8)MIDI_get_chr(MIDI,trk);

   if (MIDI->status[trk] < 0x80)
      return 5;

   MIDI->event_len = 0;
   MIDI->event[MIDI->event_len++] = MIDI->status[trk];

   switch (MIDI->status[trk] & 0xf0)
      {
      case EV_NOTE_OFF:
      case EV_NOTE_ON:
      case EV_POLY_PRESS:
      case EV_CONTROL:
      case EV_PITCH:
         MIDI->event[MIDI->event_len++] = (U8)MIDI_get_chr(MIDI,trk);
      case EV_PROGRAM:
      case EV_CHAN_PRESS:
         MIDI->event[MIDI->event_len++] = (U8)MIDI_get_chr(MIDI,trk);
         break;

      case EV_SYSEX:
         switch (MIDI->status[trk])
            {
            case EV_META:
               MIDI->event[MIDI->event_len++] = (U8)(type = MIDI_get_chr(MIDI,trk));
               switch (type)
                  {
                  case META_EOT:
                     MIDI->trk_active[trk] = 0;
                     break;
                  case META_TEMPO:
                     temp = MIDI->trk_ptr[trk];
                     MIDI_get_vln(MIDI,trk);
                     MIDI->tick_time = (100L * MIDI_get_24(MIDI,trk)) /
                        (U32) MIDI->division;
                     MIDI->trk_ptr[trk] = temp;
                     break;
                  }
            case EV_SYSEX:
            case EV_ESC:
               len = MIDI_get_vln(MIDI,trk);
               MIDI->event_len += MIDI_put_vln(len,
                  &(MIDI->event[MIDI->event_len]));

               for (cnt=0L;cnt<len;cnt++)
                  {
                  if (MIDI->event_len >= MAX_EVLEN)
                     return 3;
                  MIDI->event[MIDI->event_len++] = (U8)MIDI_get_chr(MIDI,trk);
                  }
               break;

            default:
               return 4;
            }
      }

   MIDI->pending_delta[trk] = MIDI_get_vln(MIDI,trk);
   MIDI->event_chan = MIDI->event[0] & 0x0f;
   return 0;
}

#ifdef IS_WIN16
   #pragma optimize( "l", off ) // Disable loop optimizations (bug in 16-bit)
   #pragma optimize( "l", on )  // Re-enable loop optimizations (bug in 16-bit)
#endif

//######################################################################
static U32 MIDI_get_next_event(MIDI FAR*MIDI)
{
   U32 event_delta,min_delta;
   S32 trk,new_trk,trk_cnt;

   new_trk = -1;
   trk = MIDI->event_trk;
   trk_cnt = MIDI->ntrks;
   min_delta = MAX_ULONG;

   do
      {
      if (MIDI->trk_active[trk])
         {
         event_delta = MIDI->pending_delta[trk];
         if (event_delta <= min_delta)
            {
            min_delta = event_delta;
            new_trk = trk;
            }
         }
      if (trk-- == 0)
         trk = MIDI->ntrks-1;
      }
   while (--trk_cnt);

   if (new_trk == -1) return 0;

   MIDI->event_trk = new_trk;
   MIDI->event_time = min_delta;

   for (trk=0;trk<(S32)MIDI->ntrks;trk++)
      if (MIDI->trk_active[trk])
         MIDI->pending_delta[trk] -= min_delta;

   abend(MIDI_get_event(MIDI,new_trk),0L);

   return 1;
}

//######################################################################
static U32 XMIDI_construct(XMIDI FAR*XMIDI)
{
   U32 i;

   XMIDI->RBRN.cnt = 0;
   XMIDI->TIMB.cnt = 0;

   XMIDI->q_int = (100000000L / (U32) XMIDI->AILH.quantization);
   XMIDI->DDA_sum = 0L;
   XMIDI->interval = 0L;
   XMIDI->delta = 0L;

   for (i=0;i<MAX_NOTES;i++)
      XMIDI->note_chan[i] = 255;

   for (i=0;i<CHAN_CNT;i++)
   {
      XMIDI->rbs_BNK[i] = 0;
      XMIDI->timb_BNK[i] = 0;
      }

   XMIDI->rbs_BNK[9] = 127;

   XMIDI->EVNT.base = (U8 FAR*) AIL_mem_alloc_lock(XMIDI->EVNT.avail);
   abend(XMIDI->EVNT.base?0:1,2);

   IFF_construct(&XMIDI->EVNT);

   return 0;
}

static void XMIDI_destroy(XMIDI FAR*XMIDI)
{
   AIL_mem_free_lock(XMIDI->EVNT.base);
}

//######################################################################
static void XMIDI_accum_interval(XMIDI FAR*XMIDI,MIDI FAR*MIDI)
{
   XMIDI->DDA_sum += (F64) MIDI->event_time *
                     (F64) MIDI->tick_time;

   while (XMIDI->DDA_sum >= (F64) XMIDI->q_int)
      {
      XMIDI->DDA_sum -= (F64) XMIDI->q_int;
      XMIDI->interval++;
      XMIDI->delta++;
      }
}

//######################################################################
static void XMIDI_write_interval(XMIDI FAR*XMIDI)
{
   while (XMIDI->delta > 127L)
      {
      IFF_put_byte(127,&XMIDI->EVNT);
      XMIDI->delta -= 127L;
      }

   if (XMIDI->delta)
      IFF_put_byte((U32) XMIDI->delta, &XMIDI->EVNT);

   XMIDI->delta = 0L;
}

//######################################################################
static void XMIDI_put_MIDI_event(XMIDI FAR*XMIDI, MIDI FAR*MIDI)
{
   U32 i;

   if (XMIDI->delta) XMIDI_write_interval(XMIDI);

   for (i=0;i<MIDI->event_len;i++)
      IFF_put_byte(MIDI->event[i],&XMIDI->EVNT);
}

//######################################################################
static U32 XMIDI_log_branch(XMIDI FAR*XMIDI, MIDI FAR*MIDI)
{
   U32 i,b=XMIDI->RBRN.cnt;

   if (b >= MAX_RBRN) return 10;

   for (i=0;i<b;i++)
      if (XMIDI->RBRN.brn[i].bnum == MIDI->event[2]) return 12;

   XMIDI->RBRN.brn[b].offset = AIL_ptr_dif(XMIDI->EVNT.ptr, XMIDI->EVNT.base);
   XMIDI->RBRN.brn[b].bnum = MIDI->event[2];

   XMIDI->RBRN.cnt++;
   return 0;
}

//######################################################################
static U32 XMIDI_log_timbre_request(XMIDI FAR*XMIDI, MIDI FAR*MIDI)
{
   U32 i,ch,val,t=XMIDI->TIMB.cnt;

   ch = MIDI->event_chan;

   if (t >= MAX_TIMB) return 11;

   switch (MIDI_event_type(MIDI))
      {
      case EV_NOTE_ON:
         val = XMIDI->rbs_BNK[ch];
         for (i=0;i<t;i++)
            if ((XMIDI->TIMB.tbr[i].t_bank == val) &&
               (XMIDI->TIMB.tbr[i].t_num == MIDI->event[1])) break;
         if (i == t)
            {
            XMIDI->TIMB.tbr[t].t_bank = (U8)val;
            XMIDI->TIMB.tbr[t].t_num = MIDI->event[1];
            XMIDI->TIMB.cnt++;
            }
         break;
      case EV_CONTROL:
         switch (MIDI->event[1])
            {
            case AIL_TIMB_BNK:
               XMIDI->timb_BNK[ch] = MIDI->event[2];
               break;
            }
         break;
      case EV_PROGRAM:
         for (i=0;i<t;i++)
            if ((XMIDI->TIMB.tbr[i].t_bank == XMIDI->timb_BNK[ch]) &&
               (XMIDI->TIMB.tbr[i].t_num == MIDI->event[1])) break;
         if (i == t)
            {
            XMIDI->TIMB.tbr[t].t_bank = XMIDI->timb_BNK[ch];
            XMIDI->TIMB.tbr[t].t_num = MIDI->event[1];
            XMIDI->TIMB.cnt++;
            }
         break;

      }

   return 0;
}

//######################################################################
static void XMIDI_note_off(XMIDI FAR*XMIDI,MIDI FAR*MIDI)
{
   U32 i,j;
   U32 duration,offset,len;
   U8 FAR*src, FAR*dest;
   U32 channel,note;

   channel = MIDI->event_chan;
   note = MIDI->event[1];

   for (i=0;i<MAX_NOTES;i++)
      {
      if ((XMIDI->note_chan[i] == channel) && (XMIDI->note_num[i] == note))
         {
         XMIDI->note_chan[i] = 255;

         duration = XMIDI->interval - XMIDI->note_intvl[i];
         offset = (U32) MIDI_vln_size(duration) - 1L;

         if (offset)
            {
            dest = XMIDI->note_next[i] + offset;
            src = XMIDI->note_next[i];
            len = AIL_ptr_dif( XMIDI->EVNT.ptr, XMIDI->note_next[i]);

            AIL_memmove(dest,src,len);

            XMIDI->EVNT.ptr = ( U8 FAR* ) AIL_ptr_add(XMIDI->EVNT.ptr , offset);
            XMIDI->EVNT.len += offset;

            for (j=0;j<MAX_NOTES;j++)
               {
               if (XMIDI->note_chan[j] != 255)
                  {
                  if (((S32) XMIDI->note_next[j]) - ((S32) src) >= 0L)
                     {
                     XMIDI->note_next[j] += offset;
                     }
                  }
               }

            for (j=0;j<XMIDI->RBRN.cnt;j++)
               {
               if (AIL_ptr_dif(AIL_ptr_add(XMIDI->EVNT.base , XMIDI->RBRN.brn[j].offset),src)>=0l)
                  {
                  XMIDI->RBRN.brn[j].offset += offset;
                  }
               }
            }

         MIDI_put_vln(duration,XMIDI->note_next[i] - 1);
         }
      }
}

//######################################################################
static U32 XMIDI_note_on(XMIDI FAR*XMIDI, MIDI FAR*MIDI)
{
   U32 i;

   for (i=0;i<MAX_NOTES;i++)
      if (XMIDI->note_chan[i] == 255)
         {
         XMIDI_put_MIDI_event(XMIDI,MIDI);
         IFF_put_byte(0x00,&XMIDI->EVNT);

         XMIDI->note_chan[i] = (U8)MIDI->event_chan;
         XMIDI->note_num[i] = MIDI->event[1];
         XMIDI->note_intvl[i] = XMIDI->interval;
         XMIDI->note_next[i] = XMIDI->EVNT.ptr;
         return(0);
         }


   return (1);
}

//######################################################################
static U32 XMIDI_verify_notes(XMIDI FAR*XMIDI)
{
   U32 i;

   for (i=0;i<MAX_NOTES;i++)
      if (XMIDI->note_chan[i] != 255)
         return 8;

   return 0;
}

//######################################################################
static U32 XMIDI_IFF_write_blocks(XMIDI FAR*XMIDI, HMEMDUMP out)
{
   IFF_block IFF_TIMB;
   IFF_block IFF_RBRN;

   IFF_TIMB.name = (U8 FAR*) "TIMB";
   IFF_TIMB.base = (U8 FAR*) &XMIDI->TIMB;
   IFF_TIMB.len = sizeof(TIMB_block) -
      ((MAX_TIMB - XMIDI->TIMB.cnt) * sizeof(TIMB_mx_entry));

#ifdef IS_BE
   {
     S32 i;
     for(i = 0; i < XMIDI->TIMB.cnt; i++)
     {
       MEM_LE_SWAP16( &(XMIDI->TIMB.tbr[i].t_num) );
     }
     MEM_LE_SWAP16( &XMIDI->TIMB.cnt );
   }
#endif

   IFF_RBRN.name = (U8 FAR*) "RBRN";
   IFF_RBRN.base = (U8 FAR*) &XMIDI->RBRN;
   IFF_RBRN.len = sizeof(RBRN_block) -
      ((MAX_RBRN - XMIDI->RBRN.cnt) * sizeof(RBRN_mx_entry));

#ifdef IS_BE
  {
    S32 i;
    for(i=0; i < XMIDI->RBRN.cnt; i++)
    {
      MEM_LE_SWAP16( &XMIDI->RBRN.brn[i].bnum );
      MEM_LE_SWAP16( &XMIDI->RBRN.brn[i].offset );
    }
    MEM_LE_SWAP16( &XMIDI->RBRN.cnt );
  }
#endif

   if (XMIDI->TIMB.cnt)
      if (!IFF_write_block(out,&IFF_TIMB)) return 1;

   if (XMIDI->RBRN.cnt)
      if (!IFF_write_block(out,&IFF_RBRN)) return 1;

   if (!IFF_write_block(out,&XMIDI->EVNT)) return 1;

   return 0;
}

//######################################################################
static void XMIDI_compile(char const FAR*src, U32 src_len, HMEMDUMP out, U32 quant)
{

   MIDI FAR* midi;
   XMIDI FAR* xmidi;
   jmp_buf saveerr;

   midi=AIL_mem_alloc_lock(sizeof(struct _MIDI));
   if (midi==0)
     abend(1,2);

   xmidi=AIL_mem_alloc_lock(sizeof(struct _XMIDI));
   if (xmidi==0) {
     AIL_mem_free_lock(midi);
     abend(1,2);
   }

   AIL_memcpy(&saveerr,&errhand,sizeof(errhand));

#if defined(__BORLANDC__) || defined(IS_MAC)
   if (setjmp(errhand)) {
#else
   if (_setjmp(errhand)) {
#endif
     AIL_mem_free_lock(midi);
     AIL_mem_free_lock(xmidi);
     AIL_memcpy(&errhand,&saveerr,sizeof(errhand));
     longjmp(errhand,1);
   }


   midi->seq_len = src_len;
   midi->base    = (U8 FAR*) src;

   abend(MIDI_construct(midi),0L);

   xmidi->EVNT.avail = EVNT_AVAIL;
   xmidi->EVNT.name = (U8 FAR*) "EVNT";
   xmidi->AILH.quantization = quant;

   XMIDI_construct(xmidi);

   while (MIDI_get_next_event(midi))
      {
      if ((xmidi->EVNT.avail - xmidi->EVNT.len) < (MAX_EVLEN + 16))
         abend(7,0L);

      XMIDI_accum_interval(xmidi,midi);

      switch (MIDI_event_type(midi))
         {
         case EV_NOTE_ON:
            if (xmidi->rbs_BNK[midi->event_chan])
               abend(XMIDI_log_timbre_request(xmidi,midi),0L);
            if (XMIDI_note_on(xmidi,midi))
               abend(9,0L);
            break;

         case EV_NOTE_OFF:
            XMIDI_note_off(xmidi,midi);
            break;

         case EV_CONTROL:
            XMIDI_put_MIDI_event(xmidi,midi);
            switch (midi->event[1])
               {
               case AIL_BRANCH_PT:
                  abend(XMIDI_log_branch(xmidi,midi),0L);
                  break;
               case AIL_TIMB_BNK:
                  abend(XMIDI_log_timbre_request(xmidi,midi),0L);
                  break;
               }
            break;

         case EV_PROGRAM:
            abend(XMIDI_log_timbre_request(xmidi,midi),0L);
            XMIDI_put_MIDI_event(xmidi,midi);
            break;

         case EV_META:
            switch (midi->event[1])
               {
               case META_EOT:
               case META_TRK_NAME:
               case META_INS_NAME:
                  break;
               default:
                  XMIDI_put_MIDI_event(xmidi,midi);
               }
            break;

         case EV_INVALID:
            break;

         default:
            XMIDI_put_MIDI_event(xmidi,midi);
         }
      }

   XMIDI_write_interval(xmidi);
   IFF_put_byte(EV_META,&xmidi->EVNT);
   IFF_put_byte(META_EOT,&xmidi->EVNT);
   IFF_put_byte(0x00,&xmidi->EVNT);

   abend(XMIDI_verify_notes(xmidi),0L);

   abend(XMIDI_IFF_write_blocks(xmidi,out),6L);

   XMIDI_destroy(xmidi);

   AIL_mem_free_lock(midi);
   AIL_mem_free_lock(xmidi);
   AIL_memcpy(&errhand,&saveerr,sizeof(errhand));
}

S32 AILCALL AIL_API_MIDI_to_XMI(void const FAR* MIDI,
                        U32       MIDI_size,
                        void FAR* FAR*XMIDI,
                        U32  FAR* XMIDI_size,
                        S32       flags )
{
   U32      catlen_off;
   HMEMDUMP tmp=0;
   HMEMDUMP XMID=0;

   DOLOCK();

   //
   // setup error handler
   //

#if defined(__BORLANDC__) || defined(IS_MAC)
   if (setjmp(errhand)) {
#else
   if (_setjmp(errhand)) {
#endif
     if (tmp) {
       AIL_mem_close(tmp,0,0);
       tmp=0;
     }
     if (XMID) {
       AIL_mem_close(XMID,0,0);
       XMID=0;
     }
     return(0);
   }

   lenient=(flags&AILMIDITOXMI_TOLERANT)?1:0;

   //
   // Create XMIDI output file buffer
   //

   XMID = AIL_mem_create();

   //
   // Open temporary file, and create INFO chunk of length 2
   // with a sequence count of 1
   //

   tmp = AIL_mem_create();

   AIL_mem_prints(tmp,"INFO");

   AIL_mem_printc(tmp,0);  // DWORD 2 in Motorola format for IFF reader
   AIL_mem_printc(tmp,0);
   AIL_mem_printc(tmp,0);
   AIL_mem_printc(tmp,2);

   AIL_mem_printc(tmp,1);  // WORD 1 in Intel format for application
   AIL_mem_printc(tmp,0);

   //
   // Append INFO chunk to XMIDI file as FORM XDIR, and close temporary
   // file
   //

   IFF_append_FORM(XMID,(S8 FAR*)"XDIR",tmp);

   AIL_mem_close(tmp,0,0);

   //
   // Append CAT XMID to XMIDI file, recording length offset for later
   // validation
   //

   IFF_append_CAT(XMID,(S8 FAR*)"XMID");

   catlen_off = AIL_mem_size(XMID);

   //
   // Open temporary file, and compile the input MIDI data
   // into a FORM XMID file chunk
   //

   tmp = AIL_mem_create();

   XMIDI_compile((char const FAR *)MIDI, MIDI_size, tmp, AIL_get_preference(MDI_SERVICE_RATE));

   //
   // Append XMIDI chunk to XMIDI file as FORM XMID, and close
   // temporary file
   //

   IFF_append_FORM(XMID,(S8 FAR*)"XMID",tmp);

   AIL_mem_close(tmp,0,0);

   //
   // Close CAT chunk to update its length
   //

   IFF_close_CAT(XMID,catlen_off);

   return( AIL_mem_close( XMID, XMIDI, XMIDI_size) );
}

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

static void AIL_lock_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AIL_lock_start, AIL_lock_end);

      locked = 0;
      }
}

#endif

