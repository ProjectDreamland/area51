//############################################################################
//##                                                                        ##
//##  MidiEchoW.CPP                                                         ##
//##                                                                        ##
//##  MIDI receiver / interpreter for Win32 MSS                             ##
//##                                                                        ##
//##  V1.00 of 4-Oct-97: Initial version                                    ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include "windows.h"

#include "mss.h"

#include <dsound.h>

#include <stdio.h>
#include <assert.h>


#include "MidiEchW.h"
#include "mssdls.h"



BOOL CALLBACK LauncherWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//
// Configuration equates
//

#define QSIZE       4096
#define SYSEX_QSIZE 4096

#define DLS_FORMAT DIG_F_STEREO_16
#define DLS_RATE   22050

#define HW_FORMAT  DIG_F_STEREO_16
#define HW_RATE    22050

//
// Misc. equates / structures
//

HWND hDlg;

HANDLE hStdOut;
HANDLE hStdIn;

//
// User preference file image
//

struct DAT_FILE
{
   S32 valid;
   C8  MIDI_in  [256];
   C8  MIDI_out [256];
   S32 DS_status;
   S32 WO_status;
   C8  WAVE_out [256];
   C8  DS_out   [256];
   C8  DLS_fn   [MAX_PATH];
   C8  WVL_fn   [MAX_PATH];
   C8  DLSDLL_fn[MAX_PATH];
   S32 GM_bank;
   S32 pitch_range;
};

DAT_FILE DAT;

//
// MM/DirectX device IDs
//

UINT   MIDI_in;   // -1 if invalid/unused
UINT   MIDI_out;  // -1 if invalid/unused
UINT   WAVE_out;  // -1 if invalid/unused
LPGUID DS_out;    // -1 if invalid/unused (NULL OK!)

//
// MM handles
//

HMIDIIN hMIDIIn = (HMIDIIN) INVALID_HANDLE_VALUE;

//
// MIDI receive queue
//

typedef struct _QUEUE
{
   U8 buffer[QSIZE];

   U32 head;
   U32 tail;
}
QUEUE;

QUEUE q;

//
// MIDI message queue
//

U8 sysex_message [SYSEX_QSIZE];
U32 sysex_bytecnt;

U8 sysex_output  [SYSEX_QSIZE+4];

U32 cv_message[8];
U32 cv_bytecnt;
U32 cv_status;

//
// AIL handles
//

HMDIDRIVER mdi = NULL;
HDIGDRIVER dig = NULL;

HWAVESYNTH W;
HTIMER     BAR;
S32        bar_refresh_request;

//
// MIDI statistics
//

U32 notes[16];
U32 total_notes;

U32 voice_limit;
U32 exceeded;

U32 timbre_bank [16];
U32 timbre_patch[16];

//
// DLS provider calls
//

S32       gdwDLSHandle;
S32       gdwDLSCollectionID;
HINSTANCE hDLSLibrary;

DLSMSSOPEN lpfnAIL_DLSOpen;
DLSOPEN lpfnDLSOpen;
DLSCLOSE lpfnDLSClose;
DLSLOADFILE lpfnDLSLoadFile;
DLSGETINFO lpfnDLSGetInfo;
DLSCOMPACTMEMORY lpfnDLSCompactMemory;
DLSUNLOADALL lpfnDLSUnloadAll;
DLSUNLOADFILE lpfnDLSUnloadFile;
DLSSETATTRIBUTE lpfnDLSSetAttribute;


//
// DLS support data
//

HSAMPLE DLS_sample;

S32   DLS_buffer_size;
void *DLS_buffer[2];
S32   reverb=0;

//
// MIDIECHO status variables
//

U32 trace_flag;
U32 beep_flag;

//
// MIDIECHO screen definitions and values
//

C8 *screen;

C8 save_screen[80*25*2];

U32 save_x,save_y;

U8 main_screen[] =
{"\
ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»\
º MIDIECHO - Version "MSS_VERSION"        "MSS_COPYRIGHT" º\
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶\
º                                                                              º\
º                                                                              º\
º                                                                              º\
º                                                                              º\
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶\
º                                                                              º\
º                                                                              º\
º                                                                              º\
º                                                                              º\
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶\
º      Voice limit:                                                            º\
º  Total overflows:                                                            º\
º ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ» º\
º ºúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúº º\
º ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ º\
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶\
º     +) Increase voice alert limit        -) Decrease voice alert limit       º\
º     T) MIDI trace on                     E) Error beep off                   º\
º   ESC) Quit                              A) All notes off                    º\
º                                                                              º\
º Select:                                                                      º\
ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼\
"};

C8 string[80];

#define TRACE  0
#define STATUS 1

U8 attribs[3][80];

#define FRAME_ATTR   0x1f
#define OK_ATTR      0x1a
#define BAD_ATTR     0x1c
#define WARN_ATTR    0x1e

#define BAR_SIZE     76

#define FRAME_CHAR   205
#define BAR_CHAR     219
#define PEAK_CHAR    7
#define BLANK_CHAR   250
#define T_MARK_CHAR  209
#define B_MARK_CHAR  207

U8 barscr[160*3];
U8 barcmp[160*3];

U32 bar_x;
U32 bar_y;
U8  *bar_addr;

/*
// ---------------------------------------------------------------------------
// debug_printf
// ---------------------------------------------------------------------------

void __cdecl debug_printf(char *fmt, ...)
{
   static char work_string[4096];

   if ((fmt == NULL) || (strlen(fmt) > sizeof(work_string)))
      {
      strcpy(work_string, "(String missing or too large)");
      }
   else
      {
      va_list ap;

      va_start(ap,
               fmt);

      vsprintf(work_string,
               fmt,
               ap);

      va_end  (ap);
      }

   OutputDebugString(work_string);
}
*/

// ---------------------------------------------------------------------------
// alert_box
// ---------------------------------------------------------------------------

void __cdecl alert_box(C8 *caption, C8 *fmt, ...)
{
   static char work_string[4096];

   if ((fmt == NULL) || (strlen(fmt) > sizeof(work_string)))
      {
      strcpy(work_string, "(String missing or too large)");
      }
   else
      {
      va_list ap;

      va_start(ap,
               fmt);

      vsprintf(work_string,
               fmt,
               ap);

      va_end  (ap);
      }

//   debug_printf("%s\n",work_string);

   if (caption == NULL)
      {
      MessageBox(NULL,
                 work_string,
                 "SAL Error",
                 MB_OK);
      }
   else
      {
      MessageBox(NULL,
                 work_string,
                 caption,
                 MB_OK);
      }
}


//****************************************************************************
//*                                                                          *
//*  Read all or part of a file into memory, returning memory location       *
//*  or NULL on error                                                        *
//*                                                                          *
//*  Memory will be allocated if dest==NULL                                  *
//*                                                                          *
//****************************************************************************

void * file_read (C8   *filename, //)
                  void *dest         = NULL,
                  S32   len          = -1,
                  S32   start_offset = 0)

{
   HANDLE handle;
   U32    n_bytes;
   U32    nbytes_read;
   S32    result;
   void  *buf;

   //
   // Open file
   //

   handle = CreateFile(filename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

   if (handle == INVALID_HANDLE_VALUE)
      {
      return NULL;
      }

   //
   // Set pointer to beginning of range
   //

   if (SetFilePointer(handle,
                      start_offset,
                      NULL,
                      FILE_BEGIN) == 0xffffffff)
      {
      CloseHandle(handle);
      return NULL;
      }

   //
   // Allocate memory for file range
   //

   n_bytes = len;

   if (n_bytes == 0xffffffff)
      {
      n_bytes = GetFileSize(handle, NULL) - start_offset;
      }

   buf = (dest == NULL) ? malloc(n_bytes) : dest;

   if (buf == NULL)
      {
      CloseHandle(handle);
      return NULL;
      }

   //
   // Read range
   //

   result = ReadFile(handle,
                     buf,
                     n_bytes,
                    &nbytes_read,
                     NULL);

   CloseHandle(handle);

   if ((!result) || (nbytes_read != n_bytes))
      {
      if (dest != buf)
         {
         free(buf);
         }

      return NULL;
      }

   return buf;
}

//############################################################################
//##                                                                        ##
//## Move text cursor to (x,y)                                              ##
//##                                                                        ##
//############################################################################

void TXT_locate(U32 x, U32 y)
{
   COORD c;

   c.X = (SHORT) x;
   c.Y = (SHORT) y;

   SetConsoleCursorPosition(hStdOut, c);
}

//############################################################################
//##                                                                        ##
//## Write cursor position to (*x,*y)                                       ##
//##                                                                        ##
//############################################################################

void TXT_curpos(U32 *x, U32 *y)
{
   CONSOLE_SCREEN_BUFFER_INFO c;

   GetConsoleScreenBufferInfo(hStdOut, &c);

   if (x != NULL)
      *x = (U32) c.dwCursorPosition.X;

   if (y != NULL)
      *y = (U32) c.dwCursorPosition.Y;
}

//############################################################################
//##                                                                        ##
//## Show screen                                                            ##
//##                                                                        ##
//############################################################################

void TXT_show_screen(U8 *image, U32 color)
{
   assert(strlen((char *) image) == 80*25);

   TXT_locate(0,0);

   SetConsoleTextAttribute(hStdOut, (U16) color);

   DWORD m;
   GetConsoleMode(hStdOut, &m);

   SetConsoleMode(hStdOut, ENABLE_WRAP_AT_EOL_OUTPUT |
                           ENABLE_PROCESSED_OUTPUT);
   DWORD written;

   WriteConsole(hStdOut,
                image,
                80*25-1,
               &written,
                NULL);

   SetConsoleMode(hStdOut, 0);

   WriteConsole(hStdOut,
               &image[80*25-1],
                1,
               &written,
                NULL);

   SetConsoleMode(hStdOut, m);

   TXT_locate(0,0);
}

//############################################################################
//##                                                                        ##
//## Set a line's color                                                     ##
//##                                                                        ##
//############################################################################

void TXT_line_color(U32 l, U32 r, U32 line, U32 color)
{
   U16 buffer[128];

   for (S32 i=0; i < 128; i++)
      {
      buffer[i] = (U16) color;
      }

   COORD c;

   c.X = (S16) l;
   c.Y = (S16) line;

   DWORD written;

   WriteConsoleOutputAttribute(hStdOut,
                               buffer,
                               r-l+1,
                               c,
                              &written);

   SetConsoleTextAttribute(hStdOut, (U16) color);
}

//############################################################################
//##                                                                        ##
//## Print a string on a line                                               ##
//##                                                                        ##
//############################################################################

void TXT_TTY(U32 h, U32 v)
{
   TXT_locate(h,v);

   DWORD written;

   WriteConsole(hStdOut,
                string,
                strlen((const char *) string),
               &written,
                NULL);
}

//############################################################################
//##                                                                        ##
//## Scroll a window up, filling new line with specified color              ##
//##                                                                        ##
//############################################################################

void TXT_scroll(U32 x0, U32 y0, U32 x1, U32 y1, U32 color)
{
   SMALL_RECT rect;

   rect.Left   = (S16) x0;
   rect.Top    = (S16) y0+1;
   rect.Right  = (S16) x1;
   rect.Bottom = (S16) y1;

   CHAR_INFO fill;

   fill.Char.AsciiChar = ' ';
   fill.Attributes     = (S16) color;

   COORD dest;

   dest.X = (S16) x0;
   dest.Y = (S16) y0;

   ScrollConsoleScreenBuffer(hStdOut,
                            &rect,
                             NULL,
                             dest,
                            &fill);

   SetConsoleTextAttribute(hStdOut, (U16) color);
}

//############################################################################
//##                                                                        ##
//## Write string to text window in specified color                         ##
//##                                                                        ##
//## Window 0 = status window                                               ##
//## Window 1 = trace window                                                ##
//##                                                                        ##
//############################################################################

void WND_write(U32 wnd, U32 color)
{
   U32 x0,y0,x1,y1;

   x0 = 2;
   x1 = 77;

   switch (wnd)
      {
      case TRACE:

         y0 = 3;
         y1 = 6;
         break;

      case STATUS:

         y0 = 8;
         y1 = 11;
         break;
      }

   TXT_scroll(x0,y0,x1,y1,color);
   TXT_TTY(x0,y1);

   TXT_locate(10,23);
}


//############################################################################
//##                                                                        ##
//## Initialize MIDI statistics                                             ##
//##                                                                        ##
//############################################################################

void MIDI_init(void)
{
   S32 i;

   for (i=0; i<16; i++)
      {
      notes[i] = 0;
      }

   total_notes = 0;
}

//############################################################################
//##                                                                        ##
//## Show MIDI trace                                                        ##
//##                                                                        ##
//############################################################################

void MIDI_trace(U32 status, U32 data_1, U32 data_2)
{
   C8             *out;
   static const U8 bs  [][4] = {"MSB","LSB"};
   static const U8 stat[][4] = {"off","on" };
   U32              color;
   U32              channel;
   U32              k,v;
   U32              con_num,con_val;
   U32              l,h,bn;

   channel = status & 0x0f;
   status  = status & 0xf0;

   sprintf(string,"Ch %.2u ",channel+1);

   out = &string[strlen(string)];

   switch (status)
      {
      case 0x80:        // note off
         if (channel == PERCUSS_CHAN)
            return;

         color = 0x04;

         k = data_1; v = data_2;

         if (v == 0)
            sprintf(out,"note %.3u off",k);
         else
            sprintf(out,"note %.3u off, release velocity %.3u",k,v);
         break;

      case 0x90:        // note on

         color = (channel == PERCUSS_CHAN) ? 0x0c : 0x04;

         k = data_1; v = data_2;
         if (v == 0)
            sprintf(out,"note %.3u off",k);
         else
            sprintf(out,"note %.3u on, attack velocity %.3u",k,v);
         break;

      case 0xa0:        // polyphonic key pressure
         color = 0x0d;

         k = data_1; v = data_2;
         sprintf(out,"polyphonic key pressure %.3u applied to note %.3u",v,k);
         break;

      case 0xb0:        // control change or channel mode message
         color = 0x0a;

         con_num = data_1;
         con_val = data_2;

         if (con_num < 64)
            {
            if (con_num < 32)
               bn = 0;
            else
               bn = 1;

            switch (con_num & 31)
               {
               case 1:
                  sprintf(out,"modulation controller %s = %.3u",bs[bn],con_val);
                  break;

               case 2:
                  sprintf(out,"breath controller %s = %.3u",bs[bn],con_val);
                  break;

               case 4:
                  sprintf(out,"foot controller %s = %.3u",bs[bn],con_val);
                  break;

               case 5:
                  sprintf(out,"portamento time %s = %.3u",bs[bn],con_val);
                  break;

               case 6:
                  sprintf(out,"data entry %s = %.3u",bs[bn],con_val);
                  break;

               case 7:
                  sprintf(out,"main volume %s = %.3u",bs[bn],con_val);
                  break;

               case 8:
                  sprintf(out,"balance controller %s = %.3u",bs[bn],con_val);
                  break;

               case 10:
                  sprintf(out,"pan controller %s = %.3u",bs[bn],con_val);
                  break;

               case 11:
                  sprintf(out,"expression controller %s = %.3u",bs[bn],con_val);
                  break;

               case 16:
                  sprintf(out,"general purpose controller #1 %s = %.3u",bs[bn],con_val);
                  break;

               case 17:
                  sprintf(out,"general purpose controller #2 %s = %.3u",bs[bn],con_val);
                  break;

               case 18:
                  sprintf(out,"general purpose controller #3 %s = %.3u",bs[bn],con_val);
                  break;

               case 19:
                  sprintf(out,"general purpose controller #4 %s = %.3u",bs[bn],con_val);
                  break;

               default:
                  sprintf(out,"undefined controller %s = %.3u",bs[bn],con_val);
                  break;
               }
            }
         else
            {
            switch (con_num)
               {
               case 64:
                  sprintf(out,"damper pedal (sustain) = %s",stat[con_val > 63]);
                  break;

               case 65:
                  sprintf(out,"portamento = %s",stat[con_val > 63]);
                  break;

               case 66:
                  sprintf(out,"sostenuto %s",stat[con_val > 63]);
                  break;

               case 67:
                  sprintf(out,"soft pedal = %s",stat[con_val > 63]);
                  break;

               case 69:
                  sprintf(out,"hold 2 = %s",stat[con_val > 63]);
                  break;

               case 80:
                  sprintf(out,"general purpose controller #5 = %.3u",con_val);
                  break;

               case 81:
                  sprintf(out,"general purpose controller #6 = %.3u",con_val);
                  break;

               case 82:
                  sprintf(out,"general purpose controller #7 = %.3u",con_val);
                  break;

               case 83:
                  sprintf(out,"general purpose controller #8 = %.3u",con_val);
                  break;

               case 91:
                  sprintf(out,"reverb (external effects depth) = %.3u",con_val);
                  break;

               case 92:
                  sprintf(out,"tremolo depth = %.3u",con_val);
                  break;

               case 93:
                  sprintf(out,"chorus depth = %.3u",con_val);
                  break;

               case 94:
                  sprintf(out,"celeste (detune) depth = %.3u",con_val);
                  break;

               case 95:
                  sprintf(out,"phaser depth = %.3u",con_val);
                  break;

               case 96:
                  sprintf(out,"data increment");
                  break;

               case 97:
                  sprintf(out,"data decrement");
                  break;


               case 98:
                  sprintf(out,"NRPN LSB %.3u selected",con_val);
                  break;

               case 99:
                  sprintf(out,"NRPN MSB %.3u selected",con_val);
                  break;

               case 100:
                  sprintf(out,"RPN LSB %.3u selected",con_val);
                  break;

               case 101:
                  sprintf(out,"RPN MSB %.3u selected",con_val);
                  break;

               case 112:
                  sprintf(out,"MSS voice protection %.3u",con_val);
                  break;

               case 113:
                  sprintf(out,"MSS timbre protection %.3u",con_val);
                  break;

               case 114:
                  sprintf(out,"MSS patch bank select, bank %.3u",con_val);
                  break;

               case 121:
                  sprintf(out,"reset all controllers");
                  break;

               case 122:
                  sprintf(out,"local control %s",stat[con_val > 63]);
                  break;

               case 123:
                  sprintf(out,"all notes off");
                  break;

               case 124:
                  sprintf(out,"omni mode off");
                  break;

               case 125:
                  sprintf(out,"omni mode on");
                  break;

               case 126:
                  sprintf(out,"mono mode on, %.3u channels",con_val);
                  break;

               case 127:
                  sprintf(out,"poly mode on");
                  break;

               default:
                  sprintf(out,"undefined controller #%.3u, value = %.3u",
                  con_num,con_val);
                  break;
               }
            }
         break;

      case 0xc0:        // program change
         color = 0x0e;

         sprintf(out,"program change to %.3u",data_1);
         break;

      case 0xd0:        // channel pressure
         color = 0x09;

         sprintf(out,"channel pressure %.3u applied",data_1);
         break;

      case 0xe0:        // pitch bend change
         color = 0x0b;

         l = data_1; h = data_2;
         sprintf(out,"pitch wheel set to %.5u",h*128+l);
         break;
      }

   WND_write(TRACE,color);
}

// ---------------------------------------------------------------------------
// MIDIReceiverProc
// ---------------------------------------------------------------------------

void CALLBACK midiInCallback(HANDLE hMidiIn, //)
                             WORD   wMsg,
                             DWORD  dwInstance,
                             DWORD  dwParam1,
                             DWORD  dwParam2)
{
   if (wMsg == MM_MIM_DATA)
      {
      U8 cmd,d1,d2;

      cmd = (U8) ((dwParam1 & 0xff)     >> 0);
      d1  = (U8) ((dwParam1 & 0xff00)   >> 8);
      d2  = (U8) ((dwParam1 & 0xff0000) >> 16);

      U8 op = cmd & 0xf0;

      if ((op >= 0x80) && (op < 0xf0))
         {
         q.buffer[q.head++] = cmd;

         if (q.head == QSIZE)
            {
            q.head=0;
            }

         q.buffer[q.head++] = d1;

         if (q.head == QSIZE)
            {
            q.head=0;
            }

         if ((op != 0xc0) && (op != 0xd0))
            {
            q.buffer[q.head++] = d2;

            if (q.head == QSIZE)
               {
               q.head=0;
               }
            }
         }

      }
}

// ---------------------------------------------------------------------------
// DSEnumProc
// ---------------------------------------------------------------------------

BOOL CALLBACK DSEnumProc(LPGUID  lpGUID, //)
                         LPCTSTR lpszDesc,
                         LPCTSTR lpszDrvName, 
                         LPVOID  lpContext)
{
   HWND   hCombo = *(HWND *) lpContext;
   LPGUID lpTemp = NULL;
   
   if (lpGUID != NULL)
      {
      if ((lpTemp = (LPGUID) malloc(sizeof(GUID))) == NULL)
         {
         return TRUE;
         }
    
      memcpy(lpTemp, lpGUID, sizeof(GUID));
      }
    
   S32 i = SendDlgItemMessage(hCombo,
                              IDC_DSOUND,
                              CB_ADDSTRING,
                              0,
                     (LPARAM) lpszDesc);

	SendDlgItemMessage(hCombo, 
                      IDC_DSOUND, 
                      CB_SETITEMDATA, 
             (WPARAM) i, 
             (LPARAM) lpTemp);

   return TRUE;
}
      
// ---------------------------------------------------------------------------
// DLG_init
// ---------------------------------------------------------------------------

void DLG_init(void)
{
   S32 i;

   //
   // Enumerate MIDI input devices
   //

   S32 n = midiInGetNumDevs();

   for (i=0; i < n; i++)
      {
      MIDIINCAPS in;

      if (midiInGetDevCaps(i, &in, sizeof(in)))
         {
         continue;
         }

      S32 index = SendDlgItemMessage(hDlg,
                                     IDC_MIDIIN,
                                     CB_ADDSTRING,
                                     0,
                                     (LPARAM) in.szPname);

	   SendDlgItemMessage(hDlg,
                         IDC_MIDIIN, 
                         CB_SETITEMDATA, 
                (WPARAM) index, 
                (LPARAM) i);
      }

	SendDlgItemMessage(hDlg, 
                      IDC_MIDIIN, 
                      CB_SETCURSEL, 
                      0, 
                      0);

   //
   // Enumerate MIDI output devices
   // 

   n = midiOutGetNumDevs();

   for (i=0; i < n; i++)
      {
      MIDIOUTCAPS out;

      if (midiOutGetDevCaps(i, &out, sizeof(out)))
         {
         continue;
         }

      S32 index = SendDlgItemMessage(hDlg,
                                     IDC_MIDIOUT,
                                     CB_ADDSTRING,
                                     0,
                                     (LPARAM) out.szPname);

	   SendDlgItemMessage(hDlg,
                         IDC_MIDIOUT,
                         CB_SETITEMDATA,
                (WPARAM) index,
                (LPARAM) i);
      }

     {  // add the NULL MIDI driver

       S32 index = SendDlgItemMessage(hDlg,
                                     IDC_MIDIOUT,
                                     CB_ADDSTRING,
                                     0,
                                     (LPARAM) "NULL MIDI Driver");

	     SendDlgItemMessage(hDlg,
                         IDC_MIDIOUT,
                         CB_SETITEMDATA,
                (WPARAM) index,
                (LPARAM) MIDI_NULL_DRIVER);
     }

     SendDlgItemMessage(hDlg,
                     IDC_MIDIOUT,
                     CB_SETCURSEL,
                     0,
                     0);

   //
   // Enumerate WAVE output devices
   //

   SendDlgItemMessage(hDlg,
                      IDC_WAVEOUT,
                      CB_ADDSTRING,
                      0,
            (LPARAM) "(None)");

	SendDlgItemMessage(hDlg,
                      IDC_WAVEOUT, 
                      CB_SETITEMDATA, 
             (WPARAM) 0,
             (LPARAM) -1);

   S32 WO_valid = 0;

   n = waveOutGetNumDevs();

   for (i=0; i < n; i++)
      {
      WAVEOUTCAPS out;

      if (waveOutGetDevCaps(i, &out, sizeof(out)))
         {
         continue;
         }

      WO_valid = 1;

      S32 index = SendDlgItemMessage(hDlg,
                                     IDC_WAVEOUT,
                                     CB_ADDSTRING,
                                     0,
                                     (LPARAM) out.szPname);

	   SendDlgItemMessage(hDlg,
                         IDC_WAVEOUT, 
                         CB_SETITEMDATA, 
                (WPARAM) index, 
                (LPARAM) i);
      }

   if (WO_valid)
      {
	   SendDlgItemMessage(hDlg, 
                        IDC_WORADIO, 
                        BM_SETCHECK, 
               (WPARAM) BST_CHECKED, 
                        0);

	   SendDlgItemMessage(hDlg, 
                        IDC_DSRADIO, 
                        BM_SETCHECK, 
               (WPARAM) BST_UNCHECKED, 
                        0);

	   SendDlgItemMessage(hDlg, 
                         IDC_WAVEOUT, 
                         CB_SETCURSEL, 
                (WPARAM) 1, 
                         0);
      }
   else
      {
	   EnableWindow(GetDlgItem(hDlg, IDC_WORADIO), 
                   FALSE);

      SendDlgItemMessage(hDlg,
                         IDC_WAVEOUT, 
                         CB_SETCURSEL, 
                (WPARAM) 0, 
                         0);
      }

   //
   // Enumerate DirectSound providers
   //

   S32 DS_valid = 0;

   SendDlgItemMessage(hDlg,
                      IDC_DSOUND,
                      CB_ADDSTRING,
                      0,
            (LPARAM) "(None)");

	SendDlgItemMessage(hDlg,
                      IDC_DSOUND, 
                      CB_SETITEMDATA,
             (WPARAM) 0, 
             (LPARAM) -1);

   if (DirectSoundEnumerate((LPDSENUMCALLBACK) DSEnumProc, &hDlg) == DS_OK)
      {
      DS_valid = 1;

	   SendDlgItemMessage(hDlg, 
                         IDC_DSRADIO, 
                         BM_SETCHECK, 
                (WPARAM) BST_CHECKED, 
                         0);

	   SendDlgItemMessage(hDlg, 
                         IDC_WORADIO, 
                         BM_SETCHECK, 
                (WPARAM) BST_UNCHECKED, 
                         0);

	   SendDlgItemMessage(hDlg, 
                         IDC_DSOUND,
                         CB_SETCURSEL, 
                (WPARAM) 1, 
                         0);
      }
   else
      {
	   EnableWindow(GetDlgItem(hDlg, IDC_DSRADIO), 
                   FALSE);

	   SendDlgItemMessage(hDlg, 
                         IDC_DSOUND,
                         CB_SETCURSEL,
                (WPARAM) 0, 
                         0);
      }

   //
   // Initialize DLS provider DLL filename
   // 

   SendDlgItemMessage(hDlg,
                     IDC_DLSDLL,
                     WM_SETTEXT,
                     0,
            (LPARAM) "(None)");

   //
   // Find first .DLS file in directory, if any
   //

   WIN32_FIND_DATA f;

   HANDLE h = FindFirstFile("*.dls", &f);

   if (h != INVALID_HANDLE_VALUE)
      {
      SendDlgItemMessage(hDlg,
                         IDC_DLSFN,
                         WM_SETTEXT,
                         0,
                (LPARAM) f.cFileName);

      FindClose(h);
      }
   else
      {
      SendDlgItemMessage(hDlg,
                        IDC_DLSFN,
                        WM_SETTEXT,
                        0,
               (LPARAM) "(None)");
      }

   //
   // Find first .WVL file in directory, if any
   //

   h = FindFirstFile("*.wvl", &f);

   if (h != INVALID_HANDLE_VALUE)
      {
      SendDlgItemMessage(hDlg,
                         IDC_WVLFN,
                         WM_SETTEXT,
                         0,
                (LPARAM) f.cFileName);

      FindClose(h);
      }
   else
      {
      SendDlgItemMessage(hDlg,
                        IDC_WVLFN,
                        WM_SETTEXT,
                        0,
               (LPARAM) "(None)");
      }

   //
   // Init default GM bank = 0
   //

   static C8 work_string[128];

   DAT.GM_bank = 0;

   SendDlgItemMessage(hDlg,
                      IDC_GMBANK,
                      WM_SETTEXT,
                      0,
             (LPARAM) _itoa(DAT.GM_bank, work_string, 128));

   //
   // Init default pitch range = 2
   //

   DAT.pitch_range = 2;

   SendDlgItemMessage(hDlg,
                      IDC_PRANGE,
                      WM_SETTEXT,
                      0,
             (LPARAM) _itoa(DAT.pitch_range, work_string, 128));

   //
   // Read .INI file, if present
   //

   DAT.valid = 0;

   file_read("MidiEchW.dat", &DAT);

   if (!DAT.valid)
      {
      return;
      }

   //
   // Change defaults to conform to .INI file settings, where possible
   //

   SendDlgItemMessage(hDlg,
                      IDC_MIDIIN,
                      CB_SELECTSTRING,
                     (U32)-1,
             (LPARAM) DAT.MIDI_in);

   SendDlgItemMessage(hDlg,
                      IDC_MIDIOUT,
                      CB_SELECTSTRING,
                     (U32)-1,
             (LPARAM) DAT.MIDI_out);

   SendDlgItemMessage(hDlg,
                      IDC_WAVEOUT,
                      CB_SELECTSTRING,
                     (U32)-1,
             (LPARAM) DAT.WAVE_out);

   SendDlgItemMessage(hDlg,
                      IDC_DSOUND,
                      CB_SELECTSTRING,
                     (U32)-1,
             (LPARAM) DAT.DS_out);

   SendDlgItemMessage(hDlg,
                      IDC_DLSDLL,
                      WM_SETTEXT,
                      0,
             (LPARAM) DAT.DLSDLL_fn);

   h = FindFirstFile(DAT.DLS_fn, &f);

   if (h != INVALID_HANDLE_VALUE)
      {
      FindClose(h);

      SendDlgItemMessage(hDlg,
                        IDC_DLSFN,
                        WM_SETTEXT,
                        0,
               (LPARAM) DAT.DLS_fn);
      }

   h = FindFirstFile(DAT.WVL_fn, &f);

   if (h != INVALID_HANDLE_VALUE)
      {
      FindClose(h);

      SendDlgItemMessage(hDlg,
                        IDC_WVLFN,
                        WM_SETTEXT,
                        0,
               (LPARAM) DAT.WVL_fn);
      }

   if (DS_valid && (DAT.DS_status))
      {
	   SendDlgItemMessage(hDlg, 
                         IDC_DSRADIO, 
                         BM_SETCHECK,
                (WPARAM) BST_CHECKED, 
                         0);

	   SendDlgItemMessage(hDlg, 
                         IDC_WORADIO, 
                         BM_SETCHECK, 
                (WPARAM) BST_UNCHECKED, 
                         0);
      }
   else if (WO_valid && (DAT.WO_status))
      {
	   SendDlgItemMessage(hDlg, 
                         IDC_WORADIO, 
                         BM_SETCHECK, 
                (WPARAM) BST_CHECKED, 
                         0);

	   SendDlgItemMessage(hDlg, 
                         IDC_DSRADIO, 
                         BM_SETCHECK, 
                (WPARAM) BST_UNCHECKED, 
                         0);
      }

   SendDlgItemMessage(hDlg,
                      IDC_GMBANK,
                      WM_SETTEXT,
                      0,
             (LPARAM) _itoa(DAT.GM_bank, work_string, 10));

   SendDlgItemMessage(hDlg,
                      IDC_PRANGE,
                      WM_SETTEXT,
                      0,
             (LPARAM) _itoa(DAT.pitch_range, work_string, 10));

}

// ---------------------------------------------------------------------------
// LauncherWndProc
// ---------------------------------------------------------------------------
// Description:             Message callback function for Launcher dialog.
// Arguments:
//  HWND                    [in] Dialog window handle.
//  UINT                    [in] Window message identifier.
//  WPARAM                  [in] Depends on message.
//  LPARAM                  [in] Depends on message.
// Returns:
//  BOOL                    TRUE if message was processed internally.

BOOL CALLBACK LauncherWndProc(HWND   hWnd, //)
                              UINT   uMsg, 
                              WPARAM wParam, 
                              LPARAM lParam)
{
   static HINSTANCE hInst;

   OPENFILENAME fn;
   C8           fn_buff[MAX_PATH];

   hDlg = hWnd;

   switch(uMsg)
      {
      case WM_INITDIALOG:

          hInst = (HINSTANCE) lParam;

          DLG_init();
          break;

      case WM_DESTROY:

          EndDialog(hDlg, FALSE);
          break;

       case WM_COMMAND:

         switch(LOWORD(wParam))
            {
            case IDC_DLSBROWSE:

               fn_buff[0] = 0;

		         SendDlgItemMessage(hWnd, 
                                  IDC_DLSFN,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(fn_buff),
                         (LPARAM) fn_buff);

               if (!_stricmp(fn_buff,"(None)"))
                  {
                  fn_buff[0] = 0;
                  }

               memset(&fn, 0, sizeof(fn));
               fn.lStructSize       = sizeof(fn);
               fn.hwndOwner         = hDlg;
               fn.hInstance         = NULL;
               fn.lpstrFilter       = "Downloadable sample files\0*.DLS\0All files\0*.*\0";
               fn.lpstrCustomFilter = NULL;
               fn.nMaxCustFilter    = 0;
               fn.nFilterIndex      = 1;
               fn.lpstrFile         = fn_buff;
               fn.nMaxFile          = sizeof(fn_buff);
               fn.lpstrFileTitle    = NULL;
               fn.nMaxFileTitle     = 0;
               fn.lpstrInitialDir   = NULL;
               fn.lpstrTitle        = "Select DLS Data File";
               fn.Flags             = OFN_EXPLORER |
                                      OFN_FILEMUSTEXIST |
                                      OFN_LONGNAMES |
                                      OFN_NOCHANGEDIR |
                                      OFN_PATHMUSTEXIST |
                                      OFN_HIDEREADONLY;
               fn.nFileOffset       = 0;
               fn.nFileExtension    = 0;
               fn.lpstrDefExt       = "dls";
               fn.lCustData         = NULL;
               fn.lpfnHook          = NULL;
               fn.lpTemplateName    = NULL;

               if (!GetOpenFileName(&fn))
                  {
                  break;
                  }

               SendDlgItemMessage(hDlg,
                                  IDC_DLSFN,
                                  WM_SETTEXT,
                                  0,
                         (LPARAM) fn_buff);
               break;

            case IDC_WVLBROWSE:

               fn_buff[0] = 0;

		         SendDlgItemMessage(hWnd, 
                                  IDC_WVLFN,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(fn_buff),
                         (LPARAM) fn_buff);

               if (!_stricmp(fn_buff,"(None)"))
                  {
                  fn_buff[0] = 0;
                  }

               memset(&fn, 0, sizeof(fn));
               fn.lStructSize       = sizeof(fn);
               fn.hwndOwner         = hDlg;
               fn.hInstance         = NULL;
               fn.lpstrFilter       = "Wave library files\0*.WVL\0All files\0*.*\0";
               fn.lpstrCustomFilter = NULL;
               fn.nMaxCustFilter    = 0;
               fn.nFilterIndex      = 1;
               fn.lpstrFile         = fn_buff;
               fn.nMaxFile          = sizeof(fn_buff);
               fn.lpstrFileTitle    = NULL;
               fn.nMaxFileTitle     = 0;
               fn.lpstrInitialDir   = NULL;
               fn.lpstrTitle        = "Select WVL Data File";
               fn.Flags             = OFN_EXPLORER |
                                      OFN_FILEMUSTEXIST |
                                      OFN_LONGNAMES |
                                      OFN_NOCHANGEDIR |
                                      OFN_PATHMUSTEXIST |
                                      OFN_HIDEREADONLY;
               fn.nFileOffset       = 0;
               fn.nFileExtension    = 0;
               fn.lpstrDefExt       = "dls";
               fn.lCustData         = NULL;
               fn.lpfnHook          = NULL;
               fn.lpTemplateName    = NULL;

               if (!GetOpenFileName(&fn))
                  {
                  break;
                  }

               SendDlgItemMessage(hDlg,
                                  IDC_WVLFN,
                                  WM_SETTEXT,
                                  0,
                         (LPARAM) fn_buff);
               break;

            case IDC_DLSDLLBROWSE:

               fn_buff[0] = 0;

		         SendDlgItemMessage(hWnd,
                                  IDC_DLSDLL,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(fn_buff),
                         (LPARAM) fn_buff);

               if (!_stricmp(fn_buff,"(None)"))
                  {
                  fn_buff[0] = 0;
                  }

               memset(&fn, 0, sizeof(fn));
               fn.lStructSize       = sizeof(fn);
               fn.hwndOwner         = hDlg;
               fn.hInstance         = NULL;
               fn.lpstrFilter       = "DLS providers\0*.DLL\0All files\0*.*\0";
               fn.lpstrCustomFilter = NULL;
               fn.nMaxCustFilter    = 0;
               fn.nFilterIndex      = 1;
               fn.lpstrFile         = fn_buff;
               fn.nMaxFile          = sizeof(fn_buff);
               fn.lpstrFileTitle    = NULL;
               fn.nMaxFileTitle     = 0;
               fn.lpstrInitialDir   = NULL;
               fn.lpstrTitle        = "Select DLS Provider DLL";
               fn.Flags             = OFN_EXPLORER |
                                      OFN_LONGNAMES |
                                      OFN_NOCHANGEDIR |
                                      OFN_HIDEREADONLY;
               fn.nFileOffset       = 0;
               fn.nFileExtension    = 0;
               fn.lpstrDefExt       = "dll";
               fn.lCustData         = NULL;
               fn.lpfnHook          = NULL;
               fn.lpTemplateName    = NULL;

               if (!GetOpenFileName(&fn))
                  {
                  break;
                  }

               SendDlgItemMessage(hDlg,
                                  IDC_DLSDLL,
                                  WM_SETTEXT,
                                  0,
                         (LPARAM) fn_buff);
               break;

            case IDCANCEL:
               EndDialog(hDlg, FALSE);
               break;

            case IDC_RUNAPPBUTTON:

               //
               // Save .INI file and return success
               //

               DAT.valid = 1;

               DAT.DS_status = SendDlgItemMessage(hDlg,
                                                  IDC_DSRADIO,
                                                  BM_GETSTATE,
                                                  0,
                                                  0);

               DAT.WO_status = SendDlgItemMessage(hDlg,
                                                  IDC_WORADIO,
                                                  BM_GETSTATE,
                                                  0,
                                                  0);
	            SendDlgItemMessage(hDlg, 
                                  IDC_MIDIIN,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.MIDI_in),
                         (LPARAM) DAT.MIDI_in);

	            SendDlgItemMessage(hDlg, 
                                  IDC_MIDIOUT,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.MIDI_out),
                         (LPARAM) DAT.MIDI_out);

	            SendDlgItemMessage(hDlg, 
                                  IDC_WAVEOUT,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.WAVE_out),
                         (LPARAM) DAT.WAVE_out);

	            SendDlgItemMessage(hDlg, 
                                  IDC_DSOUND,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.DS_out),
                         (LPARAM) DAT.DS_out);

	            SendDlgItemMessage(hDlg, 
                                  IDC_DLSDLL,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.DLSDLL_fn),
                         (LPARAM) DAT.DLSDLL_fn);

	            SendDlgItemMessage(hDlg, 
                                  IDC_DLSFN,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.DLS_fn),
                         (LPARAM) DAT.DLS_fn);

	            SendDlgItemMessage(hDlg, 
                                  IDC_WVLFN,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(DAT.WVL_fn),
                         (LPARAM) DAT.WVL_fn);

               static C8 work_string[128];

	            SendDlgItemMessage(hDlg, 
                                  IDC_GMBANK,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(work_string),
                         (LPARAM) work_string);

               DAT.GM_bank = atoi(work_string);

	            SendDlgItemMessage(hDlg, 
                                  IDC_PRANGE,
                                  WM_GETTEXT,
                         (WPARAM) sizeof(work_string),
                         (LPARAM) work_string);

               DAT.pitch_range = atoi(work_string);

               AIL_file_write("MidiEchW.dat",
                          &DAT,
                           sizeof(DAT));

               //
               // Retrieve and store IDs of selected devices
               //

               MIDI_in = SendDlgItemMessage(hDlg,
                                            IDC_MIDIIN,
                                            CB_GETITEMDATA,
                                            SendDlgItemMessage(hDlg,
                                                               IDC_MIDIIN,
                                                               CB_GETCURSEL,
                                                               0,
                                                               0),
                                            0);

               MIDI_out = SendDlgItemMessage(hDlg,
                                             IDC_MIDIOUT,
                                             CB_GETITEMDATA,
                                             SendDlgItemMessage(hDlg,
                                                                IDC_MIDIOUT,
                                                                CB_GETCURSEL,
                                                                0,
                                                                0),
                                             0);

               WAVE_out = SendDlgItemMessage(hDlg,
                                             IDC_WAVEOUT,
                                             CB_GETITEMDATA,
                                             SendDlgItemMessage(hDlg,
                                                                IDC_WAVEOUT,
                                                                CB_GETCURSEL,
                                                                0,
                                                                0),
                                             0);

               DS_out = (LPGUID) 
                        SendDlgItemMessage(hDlg,
                                           IDC_DSOUND,
                                           CB_GETITEMDATA,
                                           SendDlgItemMessage(hDlg,
                                                              IDC_DSOUND,
                                                              CB_GETCURSEL,
                                                              0,
                                                              0),
                                           0);

//               debug_printf("MidiEchoW: %d %d %d %X\n",
//                  MIDI_in, MIDI_out, WAVE_out, DS_out);

               //
               // End dialog and exit
               // 

               EndDialog(hDlg, TRUE);
               break;
            }
          break;
      }

    return FALSE;
}

//############################################################################
//##                                                                        ##
//## Update bar graph                                                       ##
//##                                                                        ##
//############################################################################

void BAR_update(void)
{
   U32 i;

   bar_refresh_request = FALSE;

   DWORD m;

   GetConsoleMode(hStdOut, &m);
   SetConsoleMode(hStdOut, 0);

   if (total_notes >= voice_limit)
      {
      TXT_line_color(bar_x+18,bar_x+24,bar_y-2,0x0e);
      sprintf(string,"%d   ",exceeded);
      TXT_TTY(bar_x+18,bar_y-2);
      }

   for (i=1; i <= total_notes; i++)
      {
      bar_addr[i*2] = BAR_CHAR;
      }

   for (;i < BAR_SIZE; i++)
      {
      if (bar_addr[i*2] == BAR_CHAR)
         {
         bar_addr[i*2] = PEAK_CHAR;
         }
      }

   //
   // Compare bar graph to shadow array, copying any differences to the
   // physical screen
   //

   S32 n;

   for (S32 y=0; y < 3; y++)
      {
      U8 *b = &barscr[y*160];
      U8 *c = &barcmp[y*160];

      for (S32 x=0; x < (BAR_SIZE * 2); x+=2)
         {
         if ((b[x] != c[x]) || (b[x+1] != c[x+1])) 
            {
            //
            // Char or attrib difference found, build string of 
            // characters that need to be updated
            //

            U8 attrib = b[x+1];

            SetConsoleTextAttribute(hStdOut, attrib);

            S32 h = (x/2)+bar_x;
            S32 v = (y-1)+bar_y;

            n = 0;

            for (; x < (BAR_SIZE * 2); x += 2)
               {
               string[n++] = b[x];

               c[x]   = b[x];
               c[x+1] = b[x+1];

               if (((b[x+2] == c[x+2]) && (b[x+3] == c[x+3]))
                    ||
                  (b[x+3] != attrib))
                  {
                  break;
                  }
               }

            string[n] = 0;

            TXT_TTY(h,v);
            }
         }
      }

   SetConsoleMode(hStdOut, m);
   TXT_locate(10,23);
}

//############################################################################
//##                                                                        ##
//## Reset peak bar graph indication                                        ##
//##                                                                        ##
//############################################################################

void AILCALLBACK BAR_timer(U32 junk)
{
   U32 i;

   junk=junk;

   for (i=0;i<BAR_SIZE;i++)
      {
      if (bar_addr[i*2] == PEAK_CHAR)
         {
         bar_addr[i*2] = BLANK_CHAR;
         bar_refresh_request = TRUE;
         }
      }
}

//############################################################################
//##                                                                        ##
//## Initialize bar graph                                                   ##
//##                                                                        ##
//############################################################################

void BAR_init(void)
{
   U32 j,i;
   U8 *l1;
   U8 *l2;
   U8 *l3;

   bar_addr = &barscr[160];

   exceeded = 0;

   bar_x    = 2;
   bar_y    = 16;

   TXT_line_color(bar_x+18,bar_x+24,bar_y-3,0x0e);
   sprintf(string,"%d ",voice_limit);
   TXT_TTY(bar_x+18,bar_y-3);

   TXT_line_color(bar_x+18,bar_x+24,bar_y-2,0x0e);
   sprintf(string,"%d   ",exceeded);
   TXT_TTY(bar_x+18,bar_y-2);

   l1 = bar_addr - 160;
   l2 = bar_addr;
   l3 = bar_addr + 160;

   for (i=0; i < BAR_SIZE; i++)
      {
      j=2*i;

      l1[j+1] = FRAME_ATTR;

      if (i == voice_limit)
         {
         l2[j+1] = WARN_ATTR;
         }
      else
         l2[j+1] = (i > voice_limit) ? BAD_ATTR : OK_ATTR;

      l3[j+1] = FRAME_ATTR;
      }

   for (i=1; i < BAR_SIZE-1; i++)
      {
      j=2*i;

      l1[j]   = (i == voice_limit) ? T_MARK_CHAR : FRAME_CHAR;
      l3[j]   = (i == voice_limit) ? B_MARK_CHAR : FRAME_CHAR;
      }

   l2[0*2+1]            = FRAME_ATTR;
   l2[(BAR_SIZE-1)*2+1] = FRAME_ATTR;

   //
   // Initialize bar graph character array
   //

   U8 *barchr = barscr;

   for (U32 y=bar_y-1; y < bar_y+2; y++)
      {
      for (U32 x=0; x < BAR_SIZE; x++)
         {
         barchr[x*2] = main_screen[(80*y)+(x+bar_x)];
         }

      barchr += 160;
      }

   //
   // Invalidate shadow array
   //

   memset(barcmp, -1, sizeof(barcmp));

   BAR_update();
}

//############################################################################
//##                                                                        ##
//## Check queue for incoming MIDI data                                     ##
//##                                                                        ##
//## Returns 0 if no data available, else 1                                 ##
//##                                                                        ##
//############################################################################

S32 QUEUE_check(void)
{
   return (q.head != q.tail);
}

//############################################################################
//##                                                                        ##
//## Read byte from queue                                                   ##
//##                                                                        ##
//## Returns -1 if no data available                                        ##
//##                                                                        ##
//############################################################################

S32 QUEUE_read(void)
{
   U32 n;

   if (q.tail == q.head)
      {
      return -1;
      }

   n = q.buffer[q.tail++];

   if (q.tail == QSIZE)
      {
      q.tail = 0;
      }

   return (S32) n;
}

//############################################################################
//##                                                                        ##
//## Initialize MIDI receiver queue                                         ##
//##                                                                        ##
//############################################################################

void QUEUE_init(void)
{
   q.head   = 0;
   q.tail   = 0;
}

//############################################################################
//##                                                                        ##
//## Initialize MIDI message queue                                          ##
//##                                                                        ##
//############################################################################

void MSG_init(void)
{
   cv_status  = 0;
   cv_bytecnt = 0;
   sysex_bytecnt  = 0;
}

//############################################################################
//##                                                                        ##
//## Transmit MIDI Channel Voice message to driver                          ##
//##                                                                        ##
//############################################################################

void MSG_send(S32 status, S32 data_1, S32 data_2)
{
   U32 channel;

   channel = status & 0x0f;

   //
   // Process the Channel Voice message
   //

   switch (status & 0xf0)
      {
      //
      // Note On: Increment global and channel note counts; if in
      // MIDI channel 10, load and play drum timbre
      //

      case EV_NOTE_ON:

         if (data_2 != 0)
            {
            ++notes[channel];
            ++total_notes;

            if (total_notes > voice_limit)
               {
               sprintf(string,"Voice limit exceeded in channel %d",
                  channel+1);

               WND_write(STATUS,0x0e);

               if (beep_flag)
                  {
                  MessageBeep(0);
                  }

               exceeded++;
               }

            bar_refresh_request = TRUE;
            }
         else
            {
            if (notes[channel]) --notes[channel];
            if (total_notes)    --total_notes;

            bar_refresh_request = TRUE;
            break;
            }

         if (channel == PERCUSS_CHAN)
            {
            timbre_patch[channel] = data_1;
            }
         break;

      //
      // Note Off: Decrement global and channel note counts
      //

      case EV_NOTE_OFF:
         
         if (notes[channel]) --notes[channel];
         if (total_notes)    --total_notes;

         bar_refresh_request = TRUE;
         break;

      //
      // Program Change: Log timbre patch for channel, and
      // install timbre if not already present
      //

      case EV_PROGRAM:

         timbre_patch[channel] = data_1;
         break;

      //
      // Control Change: If XMIDI Patch Bank Select (114),
      // set new timbre bank for channel
      //

      case EV_CONTROL:

         if (data_1 == PATCH_BANK_SEL)
            {
            timbre_bank[channel] = data_2;
            }
         break;
      }

   //
   // Finally, transmit message to driver
   //

   AIL_send_channel_voice_message(mdi,
                                  NULL,
                                  status,
                                  data_1,
                                  data_2);
}

//############################################################################
//##                                                                        ##
//## Write byte to MIDI message queue                                       ##
//##                                                                        ##
//############################################################################

void MSG_queue(S32 n)
{
   U32 i,j,m,status,channel;
   U32 bytefield[3];

   //
   // F0 = start of system exclusive message
   //

   if (n == 0xf0)
      {
      sysex_message[0] = 0xf0;
      sysex_bytecnt    = 1;

      return;
      }

   //
   // If transmitting sysex message, wait until F7 terminator reached,
   // then transmit entire message at once
   //

   if (sysex_bytecnt)
      {
      if (n == 0xf7)
         {
         //
         // Transmit queued sysex message
         // 

         sprintf(string,"System Exclusive message, %d bytes ",
            sysex_bytecnt+1);

         WND_write(TRACE,0x07);

         sysex_message[sysex_bytecnt] = 0xf7;

         j = 0;

         sysex_output[j++] = sysex_message[0];

         bytefield[2] =  (sysex_bytecnt        & 0x7f);
         bytefield[1] = ((sysex_bytecnt >> 7)  & 0x7f) | 0x80;
         bytefield[0] = ((sysex_bytecnt >> 14) & 0x7f) | 0x80;
   
         m=2;
         for (i=0;i<=2;i++)
            if (bytefield[i] & 0x7f)
               {
               m=i;
               break;
               }

         for (i=m;i<=2;i++)
            {
            sysex_output[j++] = (U8) (bytefield[i] & 0xff);
            }

         memmove(&sysex_output[j],&sysex_message[1],sysex_bytecnt);

         AIL_send_sysex_message(mdi,sysex_output);

         sysex_bytecnt = 0;
         }
      else
         {
         //
         // Record sysex byte in queue
         //

         sysex_message[sysex_bytecnt++] = (U8) n;
         }

      return;
      }

   //
   // Prepare for new Channel Voice message if status byte received
   //

   if (n >= 0x80)
      {
      if (n < 0xf8)
         {
         //
         // Status byte received -- initialize message
         //

         cv_bytecnt = 0;

         status = n & 0xf0;

         if (status >= 0xf0)
            {
            status = 0;
            }

         channel = n & 0x0f;

         cv_status = status | channel;
         }

      return;
      }
   
   if (cv_bytecnt == 8)
      {
      cv_bytecnt = 0;
      }

   //
   // If valid Channel Voice status, log data byte
   //

   if (cv_status >= 0x80)
      {
      cv_message[cv_bytecnt++] = n;
      }

   //
   // If complete Channel Voice message is now available, 
   // send it to the AIL driver and prepare for next message 
   // (possibly with running status)
   //
 
   if (cv_bytecnt >= (U32) (2 - ((cv_status & 0xe0) == 0xc0)))
      {
      cv_bytecnt = 0;

      //
      // Show MIDI trace
      //

      if (trace_flag)
         {
         MIDI_trace(cv_status, cv_message[0], cv_message[1]);
         }

      //
      // Transmit the message
      //

      MSG_send(cv_status,cv_message[0],cv_message[1]);
      }
}

//############################################################################
//##                                                                        ##
//## Evaluate ASCII string in specified numeric base                        ##
//##                                                                        ##
//############################################################################

S32 ASC_val(C8 *str, S32 base)
{
   S32 i,j;
   S32 total = 0L;

   for (i=0; i < (S32) strlen(str); i++)
      {
      for (j=0; j < base; j++)
         {
         if (toupper(str[i]) == "0123456789ABCDEF"[j])
            {
            total = (total * base) + j;
            break;
            }
         }

      if (j == base) return -1;
      }

   return total;
}

// ---------------------------------------------------------------------------
// DLS_poll_callback
// ---------------------------------------------------------------------------

S32 CALLBACK DLS_poll_callback(U32 user)
{
   return AIL_sample_buffer_ready((HSAMPLE) user);
}

// ---------------------------------------------------------------------------
// DLS_lock_callback
// ---------------------------------------------------------------------------

void FAR * CALLBACK DLS_lock_callback(U32 user, S32 buffer_section)
{
   return DLS_buffer[buffer_section];
}

// ---------------------------------------------------------------------------
// DLS_unlock_callback
// ---------------------------------------------------------------------------

void CALLBACK DLS_unlock_callback(U32 user, S32 buffer_section)
{
   AIL_load_sample_buffer((HSAMPLE) user,
                                    buffer_section,
                                    DLS_buffer[buffer_section],
                                    DLS_buffer_size);
}

// ---------------------------------------------------------------------------
// WinMain
// ---------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, //)
                    LPSTR     lpCmdLine, int       nCmdShow )
{
   //
   // Init dialog box
   //

   if (!DialogBoxParam(hInstance, 
                       MAKEINTRESOURCE(IDD_LAUNCHERDIALOG), 
                       NULL,
             (DLGPROC) LauncherWndProc, 
              (LPARAM) hInstance))
      {
      return 0;
      }

   //
   // Bring up MIDIECHO console
   //

   AllocConsole();
   
   SetConsoleTitle("MIDI Echo for Windows");

   CONSOLE_SCREEN_BUFFER_INFO csbi;

   hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   hStdIn  = GetStdHandle(STD_INPUT_HANDLE);

   GetConsoleScreenBufferInfo(hStdOut,
                             &csbi);

   //
   // Initialize AIL
   //

   AIL_set_redist_directory("..\\..\\redist\\" MSS_REDIST_DIR_NAME);
   AIL_startup();

   //
   // Initialize MIDIECHO status variables
   //

   trace_flag  = 1;
   beep_flag   = 0;

   AIL_set_preference(MDI_DEFAULT_BEND_RANGE, DAT.pitch_range);

   voice_limit = 24;

   //
   // Initialize MIDI input device
   //

   S32 result;

   result = midiInOpen(&hMIDIIn,
                        MIDI_in,
                (DWORD) midiInCallback,
                        0,
                        CALLBACK_FUNCTION);

   if (result != MMSYSERR_NOERROR)
      {
      alert_box("Error","midiInOpen failed, code = %X\n",result);
      return 0;
      }

   midiInStart(hMIDIIn);

   //
   // Initialize MIDI output device, if any
   //

   mdi = NULL;

   if (MIDI_out != -1)
      {
      result = AIL_midiOutOpen(&mdi,
                                NULL,
                                MIDI_out);

      if (result)
         {
         alert_box("Error","AIL_midiOutOpen failed: %s\n",AIL_last_error());
         return 0;
         }
      }

   //
   // Initialize digital output device, if any
   //

   dig = NULL;

   PCMWAVEFORMAT format;

   format.wf.wFormatTag      = WAVE_FORMAT_PCM;
   format.wf.nChannels       = (HW_FORMAT & DIG_F_STEREO_MASK) ? 2 : 1;
   format.wf.nSamplesPerSec  = HW_RATE;
   format.wf.nBlockAlign     = ((HW_FORMAT & DIG_F_16BITS_MASK) ? 2 : 1) *
                               ((HW_FORMAT & DIG_F_STEREO_MASK) ? 2 : 1);
   format.wf.nAvgBytesPerSec = HW_RATE * format.wf.nBlockAlign;
   format.wBitsPerSample     = (HW_FORMAT & DIG_F_16BITS_MASK) ? 16 : 8;

   result = 0;

   //
   // give the reverb system 256K to buffer with
   //

   AIL_set_preference( DIG_REVERB_BUFFER_SIZE, 65536L*4L);

   if ((DAT.WO_status) && (WAVE_out != -1))
      {
//      debug_printf("MidiEchoW: Using waveOut\n");

      AIL_set_preference(DIG_USE_WAVEOUT, YES);

      result = AIL_waveOutOpen(&dig,
                                NULL,
                                WAVE_out,
                (LPWAVEFORMAT) &format);
      }
   else if ((DAT.DS_status) && ((S32) DS_out != -1))
      {
//      debug_printf("MidiEchoW: Using DirectSound\n");

      AIL_set_preference(DIG_USE_WAVEOUT, NO);

      result = AIL_waveOutOpen(&dig,
                                NULL,
                       (DWORD)  DS_out,
                (LPWAVEFORMAT) &format);
      }

   if (result)
      {
      alert_box("Error","AIL_waveOutOpen failed: %s\n",AIL_last_error());
      return 0;
      }

   //
   // Read .WVL file, if any
   //

   W = NULL;

   if ((strlen(DAT.WVL_fn))          &&
       (strcmp(DAT.WVL_fn,"(None)")) &&
       (dig != NULL))
      {
      void *wave = file_read(DAT.WVL_fn, NULL);

      if (wave == NULL)
         {
         alert_box("Warning","Couldn't read wave library file '%s'\n",
            DAT.WVL_fn);
         }
      else
         {
         W = AIL_create_wave_synthesizer(dig, 
                                         mdi, 
                                         wave, 
                                         MAX_W_VOICES-1);
         }
      }

   //
   // Load DLS library and bankfile, if requested
   //

   hDLSLibrary        = NULL;
   gdwDLSHandle       = -1;
   gdwDLSCollectionID = -1;

   DLS_sample = NULL;
   DLS_buffer[0] = DLS_buffer[1] = NULL;

   if ((strlen(DAT.DLSDLL_fn))          && 
       (strcmp(DAT.DLSDLL_fn,"(None)")) && 
       (dig != NULL))
      {
      hDLSLibrary = LoadLibrary(DAT.DLSDLL_fn);

      if (hDLSLibrary == NULL)
         {
         alert_box("Warning","Couldn't load DLS provider library '%s'\n",
            DAT.DLSDLL_fn);
         }
      else
         {
         *(U32 *) &lpfnAIL_DLSOpen      = (U32) GetProcAddress(hDLSLibrary,"DLSMSSOpen");
         *(U32 *) &lpfnDLSOpen          = (U32) GetProcAddress(hDLSLibrary,"DLSOpen");
         *(U32 *) &lpfnDLSClose         = (U32) GetProcAddress(hDLSLibrary,"DLSClose");
         *(U32 *) &lpfnDLSLoadFile      = (U32) GetProcAddress(hDLSLibrary,"DLSLoadFile");
         *(U32 *) &lpfnDLSUnloadFile    = (U32) GetProcAddress(hDLSLibrary,"DLSUnloadFile");
         *(U32 *) &lpfnDLSUnloadAll     = (U32) GetProcAddress(hDLSLibrary,"DLSUnloadAll");
         *(U32 *) &lpfnDLSGetInfo       = (U32) GetProcAddress(hDLSLibrary,"DLSGetInfo");
         *(U32 *) &lpfnDLSCompactMemory = (U32) GetProcAddress(hDLSLibrary,"DLSCompactMemory");
         *(U32 *) &lpfnDLSSetAttribute  = (U32) GetProcAddress(hDLSLibrary,"DLSSetAttribute");

         if (lpfnAIL_DLSOpen != NULL)
            {
            //
            // AIL initialization function found -- this is a software
            // wave synthesizer implementation which requires valid
            // digital and XMIDI output drivers
            //

            if ((dig == NULL) || (mdi == NULL))
               {
               alert_box("Warning","Software DLS synthesizer requires valid digital and XMIDI drivers\n");
               FreeLibrary(hDLSLibrary);
               hDLSLibrary = NULL;
               }
            else
               {
               //
               // Allocate streaming buffers for DLS output and pass size to
               // initialization function
               //

               DLS_buffer_size = AIL_minimum_sample_buffer_size(dig, 
                                                                DLS_RATE,
                                                                DLS_FORMAT);

               DLS_buffer[0]   = AIL_mem_alloc_lock(DLS_buffer_size);
               DLS_buffer[1]   = AIL_mem_alloc_lock(DLS_buffer_size);

               DLS_sample      = AIL_allocate_sample_handle(dig);

               AIL_init_sample(DLS_sample);

               AIL_set_sample_type(DLS_sample,
                                   DLS_FORMAT, 
                                  (DLS_FORMAT & DIG_F_16BITS_MASK) ? DIG_PCM_SIGN : 0);

               AIL_set_sample_playback_rate(DLS_sample, 
                                            DLS_RATE);
 
               //
               // Call AIL DLS initialization function to initialize 
               // software synthesizer
               //

               S32 result = lpfnAIL_DLSOpen(&gdwDLSHandle, 
                                             0,
                                             mdi,
                                             DLS_FORMAT,
                                             DLS_RATE,
                                             DLS_buffer_size,
                                       (U32) DLS_sample,
                                             DLS_poll_callback,
                                             DLS_lock_callback,
                                             DLS_unlock_callback);

               if (result != DLS_NOERR)
                  {
                  alert_box("Warning","AIL_DLSOpen() failed, code %X\n",
                     result);

                  FreeLibrary(hDLSLibrary);
                  hDLSLibrary = NULL;
                  }

               }
            }
         else if (lpfnDLSOpen != NULL)
            {
            //
            // No AIL-specific initialization needed
            //

            S32 result = lpfnDLSOpen(&gdwDLSHandle, 0);

            if (result != DLS_NOERR)
               {
               alert_box("Warning","DLSOpen() failed, code %X\n",
                  result);

               FreeLibrary(hDLSLibrary);
               hDLSLibrary = NULL;
               }
            }
         else
            {
            //
            // No valid initialization function found
            //

            alert_box("Warning","%s does not appear to be a valid DLS provider DLL\n",
               DAT.DLSDLL_fn);

            FreeLibrary(hDLSLibrary);
            hDLSLibrary = NULL;
            }
         }
      }

   if ((strlen(DAT.DLS_fn))          && 
       (strcmp(DAT.DLS_fn,"(None)")) && 
       (hDLSLibrary != NULL))
      {
      S32 size = AIL_file_size(DAT.DLS_fn);

      if (size == -1)
         {
         alert_box("Warning","DLS data file '%s' not found\n",
            DAT.DLS_fn);
         }
      else
         {
         //
         // Unload all current DLS banks
         //

         lpfnDLSUnloadAll(gdwDLSHandle, DLS_REMOVE_ALL);

         //
         // Get largest block size and make sure it's enough
         //

         DLS_INFO_STRUC DlsInfo;               // storage for DLS info
         DlsInfo.dwSize = sizeof(DLS_INFO_STRUC);

         lpfnDLSGetInfo(&DlsInfo);

//         debug_printf("MidiEchoW: %d bytes in largest available block, %d required\n",
//            DlsInfo.dwLargestBuf, size);

//         debug_printf("MidiEchoW: Hardware error status = %d\n",DlsInfo.dwHwStatus);

         if (size > DlsInfo.dwLargestBuf)
            {
            alert_box("Warning","%d bytes in largest available block, %d required to load %s",
               DlsInfo.dwLargestBuf, size, DAT.DLS_fn);
            }
         else
            {
            S32 result = lpfnDLSLoadFile(gdwDLSHandle,
                                         DLS_OVERWRITE_EXISTING_PATCH,
                                        &gdwDLSCollectionID,
                                         DAT.DLS_fn);

            if (result != DLS_NOERR)
               {
               alert_box("Warning","Unable to load DLS data bank '%s', code %X",
                  DAT.DLS_fn, result);

               gdwDLSCollectionID = -1;
               }
            }
         }
      }


   //
   // Initialize other program sections
   //

   QUEUE_init();
   MSG_init();
   MIDI_init();

   BAR = AIL_register_timer(BAR_timer);
   AIL_set_timer_period(BAR,2000000);
   AIL_start_timer(BAR);

   //
   // Initialize MIDIECHO screen
   //

   TXT_show_screen(main_screen,0x0f);

   TXT_line_color(1,78,23,0x0a);
   TXT_line_color(1,78,22,0x0a);
   TXT_line_color(1,78,21,0x0a);
   TXT_line_color(1,78,20,0x0a);
   TXT_line_color(1,78,19,0x0a);
   TXT_line_color(1,78,1, 0x0a);

   BAR_init();

   TXT_locate(10,23);

   if (DAT.pitch_range != 2)
      {
      sprintf(string,"Default pitch bend range set to ñ%d semitones",
         DAT.pitch_range);
      WND_write(STATUS,0x0d);
      }

   //
   // Install default instrument for program change 0
   // 

   S32 i;

   for (i=0; i < NUM_CHANS; i++)
      {
      if (i != PERCUSS_CHAN)
         {
         MSG_send(EV_CONTROL | i, PATCH_BANK_SEL, 0);
         MSG_send(EV_PROGRAM | i, 0,              0);
         }
      else
         {
         MSG_send(EV_CONTROL | i, PATCH_BANK_SEL, 127);
         }
      }

   //
   // Set requested default GM bank (controllers 0/32)
   //

   S32 LSB = DAT.GM_bank & 0x7f;
   S32 MSB = DAT.GM_bank >> 7;

   for (i=0; i < NUM_CHANS; i++)
      {
      MSG_send(EV_CONTROL | i, 32, LSB);
      MSG_send(EV_CONTROL | i, 0,  MSB);
      }

   sprintf(string,"Default GM bank set to %d:%d (%d)", MSB,LSB, DAT.GM_bank);
   WND_write(STATUS, 0x0d);

   sprintf(string,"MIDI performance in progress");
   WND_write(STATUS,0x0d);

   //
   // Main input loop
   //

   S32 done = 0;

   while (!done)
      {
      //
      // Check for MIDI input
      //

      if (QUEUE_check())
         {
         MSG_queue(QUEUE_read());
         }

      //
      // Update bar graph
      //

      if (bar_refresh_request)
         {
         BAR_update();
         }

      //
      // Check for user input
      //

      INPUT_RECORD inputBuffer;
      DWORD        dwInputEvents;

      GetNumberOfConsoleInputEvents(hStdIn,
                                   &dwInputEvents);

      if (dwInputEvents)
         {
         ReadConsoleInput(hStdIn,
                         &inputBuffer,
                          1,
                         &dwInputEvents);

         switch (inputBuffer.EventType)
            {
            case KEY_EVENT:

               if (inputBuffer.Event.KeyEvent.bKeyDown)
                  {
                  //had to cast to eight - some weird problem with alignment
                  switch (((U8*)&inputBuffer.Event.KeyEvent)[8])
                     {
                     //
                     // ESC: Shut down and exit
                     //
                     case 27:
                        sprintf(string,"Quit");
                        WND_write(STATUS,0x0d);

                        done = 1;
                        break;

                     //
                     // +/-: Adjust voice alert limit
                     //

                     case '+':
                     case '=':

                        if (voice_limit < BAR_SIZE-2)
                           {
                           voice_limit++;

                           sprintf(string,"Voice limit set to %d",voice_limit);
                           WND_write(STATUS,0x0d);
                           }
                        BAR_init();
                        break;

                     case '-':
                     case '_':

                        if (voice_limit > 1)
                           {
                           voice_limit--;

                           sprintf(string,"Voice limit set to %d",voice_limit);
                           WND_write(STATUS,0x0d);
                           }
                        BAR_init();
                        break;

                     //
                     // T)race on/off
                     //

                     case 't':
                     case 'T':

                        trace_flag = !trace_flag;

                        if (trace_flag)
                           {
                           sprintf(string,"MIDI trace enabled");
                           WND_write(STATUS,0x0d);

                           sprintf(string,"on ");
                           }
                        else
                           {
                           sprintf(string,"MIDI trace disabled");
                           WND_write(STATUS,0x0d);

                           sprintf(string,"off");
                           }

                        TXT_TTY(20,20);
                        TXT_locate(10,23);
                        break;

                     //
                     // E)rror beep on/off
                     //

                     case 'e':
                     case 'E':

                        beep_flag = !beep_flag;

                        if (beep_flag)
                           {
                           sprintf(string,"Error beep enabled");
                           WND_write(STATUS,0x0d);

                           sprintf(string,"on ");
                           }
                        else
                           {
                           sprintf(string,"Error beep disabled");
                           WND_write(STATUS,0x0d);

                           sprintf(string,"off");
                           }

                        TXT_TTY(57,20);
                        TXT_locate(10,23);
                        break;

                     //
                     // F)iltering on/off (for DLS)
                     //

                     case 'f':
                     case 'F':

                         AIL_set_preference(DLS_ENABLE_FILTERING,
                                            !AIL_get_preference(DLS_ENABLE_FILTERING));

                         sprintf(string,"Filtering %s\n",
                            AIL_get_preference(DLS_ENABLE_FILTERING) ? "On" : "Off");
                         WND_write(STATUS, 0x0d);

                        break;

                     //
                     // R)everb on/off (for DLS)
                     //

                     case 'r':
                     case 'R':

                         reverb=!reverb;
                         if (reverb)
                           AIL_set_sample_reverb(DLS_sample,0.5F,0.06F,0.25F);
                         else
                           // no reverb
                           AIL_set_sample_reverb(DLS_sample,0.0F,0.0F,0.0F);

                         sprintf(string,"Global reverb: %d\n",
                            reverb?1:0);
                         WND_write(STATUS, 0x0d);

                        break;

                     //
                     // A)ll notes off
                     //

                     case 'a':
                     case 'A':

                        for (i=0; i < NUM_CHANS; i++)
                           {
                           MSG_send(EV_CONTROL | i, SUSTAIN      , 0);
                           MSG_send(EV_CONTROL | i, ALL_NOTES_OFF, 127);

                           notes[i] = 0;
                           }

                        total_notes = 0;

                        BAR_update();

                        sprintf(string,"All Notes Off transmitted");
                        WND_write(STATUS,0x0d);

                        break;
                     }
                  }
               break;
            }
         }
      }

   //
   // Shut down and exit
   //

   if (gdwDLSHandle != -1)
      {
      lpfnDLSClose(gdwDLSHandle, RETAIN_DLS_COLLECTION);
      gdwDLSHandle = -1;
      }

   if (hDLSLibrary != NULL)
      {
      FreeLibrary(hDLSLibrary);
      hDLSLibrary = NULL;
      }

   if (DLS_sample != NULL)
      {
      AIL_release_sample_handle(DLS_sample);
      DLS_sample = NULL;
      }

   if (DLS_buffer[0] != NULL)
      {
      AIL_mem_free_lock(DLS_buffer[0]);
      DLS_buffer[0] = NULL;
      }

   if (DLS_buffer[1] != NULL)
      {
      AIL_mem_free_lock(DLS_buffer[1]);
      DLS_buffer[1] = NULL;
      }

   if (W != NULL)
      {
      AIL_destroy_wave_synthesizer(W);
      W = NULL;
      }

   midiInStop(hMIDIIn);
   midiInReset(hMIDIIn);
   midiInClose(hMIDIIn);

   if (mdi != NULL)
      {
      AIL_midiOutClose(mdi);
      mdi = NULL;
      }

   if (dig != NULL)
      {
      AIL_waveOutClose(dig);
      dig = NULL;
      }

   AIL_shutdown();

   FreeConsole();

   return 1;
}


