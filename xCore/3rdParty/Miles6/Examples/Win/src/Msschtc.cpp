//############################################################################
//##                                                                        ##
//##  CLIENT.CPP                                                            ##
//##                                                                        ##
//##  Chat server test bed                                                  ##
//##                                                                        ##
//##  V1.00 of 11-Feb-99: Initial                                           ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <assert.h>

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include "mss.h"

HDIGDRIVER dig;
char szAppName[]="MSSCHTC";
HWND hwnd;
HWND progress;

#define MONITOR_PORT 49778

#define HW_FORMAT   DIG_F_MONO_16
#define HW_RATE     8000
#define HW_BITS     16
#define HW_CHANNELS 1

SOCKET hs_socket = NULL;      // Handshaking socket
SOCKET data_socket = NULL;    // Data transfer socket

S32 input_source;             // SOURCE_VOICE for input, or SOURCE_FILE for file
S32 input_type;               // TYPE_ENCODED for ASI-encoded data transmission or TYPE_RAW for raw PCM transmission

S32 active;                   // Goes low when interruption or error occurs

#define SOURCE_FILE  0
#define SOURCE_VOICE 1

#define TYPE_RAW     0
#define TYPE_ENCODED 1

HSAMPLE   stream;

HDIGINPUT input  = NULL;

HANDLE transmit_thread;
HANDLE receive_thread;
S32    threads_active;

HPROVIDER  transmit_encoder;
HASISTREAM transmit_stream;

HPROVIDER  receive_decoder;

ASI_STREAM_OPEN           XMIT_stream_open;
ASI_STREAM_PROCESS        XMIT_stream_process;
ASI_STREAM_SEEK           XMIT_stream_seek;
ASI_STREAM_CLOSE          XMIT_stream_close;
//ASI_STREAM_ATTRIBUTE      XMIT_stream_attribute;
//ASI_STREAM_SET_PREFERENCE XMIT_stream_set_preference;

//
// Buffers used to acquire uncompressed voice data from input API
//

#define INPUT_BUFFER_SIZE 1024
#define N_INPUT_BUFFERS   4

volatile S32 input_buffer_head;
volatile S32 input_buffer_tail;
C8           input_buffer[N_INPUT_BUFFERS][INPUT_BUFFER_SIZE];

volatile S32 input_read_offset;

//
// Client transmission packet size
//
// This reflects the desired size of the compressed-data packets to be
// transmitted back to the clients, not including any overhead imposed
// by the transport layer
//
// Too large a packet size will require excessive latency time to accumulate;
// packets which are too small can be inefficient to transmit
//
// With the Voxware RT24 codec, 7 bytes of compressed data are generated every
// 22.05 milliseconds.  A transmit packet size of 32 bytes means that we'll
// send about 100 milliseconds' worth of voice data per packet.  This will
// result in about 1 packet per CLIENT_MIX_PERIOD.
//

#define TRANSMIT_PACKET_SIZE 32

C8 send_buffer[TRANSMIT_PACKET_SIZE];

//
// Input buffer used to receive compressed data from server
//
// Typically this buffer will be sized conservatively relative to the
// frame size of the codec in use.  We double the size of the buffer to
// allow for codecs that need to perform backward seeks (see comments
// in RECV_stream_CB() below).
//

#define RECEIVE_FRAME_SIZE 512

C8           RECV_frame[RECEIVE_FRAME_SIZE * 2];
volatile S32 RECV_read_cursor;
volatile S32 RECV_write_cursor;

//
// Buffers used to receive decompressed data from ASI codec, and
// to stream decompressed data to MSS
//
// Sized for reasonable latency on low-bandwidth voice data; for
// high-bandwidth transmission, the buffer size should be increased
// substantially and the Sleep() parameter that limits the speed of
// the main thread's execution should be reduced.
//
// We maintain three buffers for incoming decompressed data.  Once all of
// the buffers are initially filled, we set the "triggered" flag to allow
// the foreground loop to start copying the receive buffers into the two
// output stream buffers.  N_RECEIVE_BUFFERS may be increased to 4 or even
// higher for better immunity to dropouts, but at a significant cost in
// latency.
//
// Alternative buffering schemes may be implemented for lower latency.  In
// latency-critical applications it may make sense to write incoming data to
// a DirectSound secondary buffer, and/or use the Voxware comfort-noise and
// warping features to avoid dropouts due to late or missing packets.
//
// By default the size of each stream buffer is determined by calling
// AIL_minimum_sample_buffer_size() for the sample rate and format in use.
// Typically this value is around 4K.  The receive buffers are allocated
// at the same size.
//
// The fetch buffer is a temporary buffer used in 
// receive_ASI_thread_procedure(), whose size matches the receive and stream
// buffers.
//

#define N_RECEIVE_BUFFERS 3

C8          *receive_buffer     [N_RECEIVE_BUFFERS];
volatile S32 receive_buffer_full[N_RECEIVE_BUFFERS];
S32          receive_buffer_size;
volatile S32 receive_read_buffer;
volatile S32 receive_write_buffer;

C8          *fetch_buffer;

C8          *stream_buffer[2];
volatile S32 triggered;

typedef struct
   {
   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_ATTRIBUTE      ASI_stream_attribute;
   ASI_STREAM_SET_PREFERENCE ASI_stream_set_preference;

   HATTRIB OUTPUT_SAMPLE_RATE;
   HATTRIB OUTPUT_BITS;
   HATTRIB OUTPUT_CHANNELS;
   HATTRIB REQUESTED_RATE;

   HASISTREAM stream;
   }
ASISTRUCT;

ASISTRUCT RECV;

static update_graph(void const *data, S32 len)
{
  S32 i;
  F64 a=0;
  S16 const* d=(S16*)data;
  S32 ave;

  len/=2;
  for(i=0;i<len;i++)
  {
    S32 ab=abs(*d++);
    a+=ab;
  }
  ave=(S32)(a/len);

  PostMessage(progress,PBM_SETPOS,ave,0);
}

//############################################################################
//##                                                                        ##
//## Input buffer service routine                                           ##
//##                                                                        ##
//############################################################################

void AILCALLBACK serve_input(void const FAR *data, //)
                             S32       len,
                             U32       user)
{
   assert(len <= INPUT_BUFFER_SIZE);

   C8 FAR *dest = &input_buffer [input_buffer_head] [0];

   update_graph(data,len);

   if (len < INPUT_BUFFER_SIZE)
      {
      //
      // Less than an entire input buffer was available, so flush the dest
      // buffer before copying
      //

      memset(dest,
             0,
             INPUT_BUFFER_SIZE);
      }

   memcpy(dest,
          data,
          len);

   //
   // Add buffer to head of queue, assuming there's room
   //

   if (input_buffer_head == N_INPUT_BUFFERS-1)
      {
      input_buffer_head = 0;
      }
   else
      {
      ++input_buffer_head;
      }

   //
   // Enable reading from queue as soon as first buffer arrives
   //

   if (input_buffer_tail == -1)
      {
      input_buffer_tail = 0;
      input_read_offset = 0;
      }
}

// ---------------------------------------------------------------------------
// NET_poll_for_data
// ---------------------------------------------------------------------------

S32 NET_poll_for_data(C8     *dest,  //)
                      S32     size)
{
   S32 index = 0;

   //
   // Get as much data as is currently available, up to specified size
   //

   while (index < size)
      {
      S32 amount = recv(data_socket,
                       &dest[index],
                        size - index,
                        0);

      if (amount == 0)
         {
         //
         // Connection has been closed, return failure
         //

         return -1;
         }

      if (amount == SOCKET_ERROR)
         {
         //
         // No (more) data available
         //

         if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
            return index;
            }

         return -1;
         }

      index += amount;

      if (index < size)
         {
         //
         // If more data expected, continue polling at 1-millisecond 
         // intervals
         //

         Sleep(1);
         }
      }

   //
   // Return success
   //

   return index;
}

// ---------------------------------------------------------------------------
// RECV_stream_CB()
// Runs from client receive thread
// ---------------------------------------------------------------------------

S32 CALLBACK RECV_stream_CB(U32       user, //)
                            void FAR *dest,
                            S32       bytes_requested,
                            S32       offset)
{
   //
   // A seek offset of 0 occurs only when the stream is opened (initial seek to 
   // beginning to determine stream characteristics) and when the first frame
   // of the stream is read after opening (when actually beginning to stream
   // data).  Since we are dealing with a non-seekable input stream, we 
   // must handle this second case by maintaining a separate read and write
   // cursor in the frame buffer.  The second 0-offset seek is guaranteed 
   // to occur while the initial frame data is still in the buffer.
   //
   // Most voice codecs won't perform any seek operations at all -- this 
   // functionality is here primarily to support MP3 streaming and similar
   // operations.  A chat server designed strictly for use with the Voxware
   // codecs can safely ignore the offset parameter altogether.
   //
   //

   if (offset != -1)
      {
      RECV_read_cursor = offset;
      }

   //
   // Data will typically be requested one frame at a time by the ASI codec
   // (although frame header components may be requested separately, a few words
   // at a time).  A return value of less than bytes_requested indicates the 
   // end of the stream has been reached.  We don't support fragmented frames, 
   // so we'll return 0 to the codec if the client disconnects while we're 
   // polling it for data.  Any ASI codec designed for compatibility with 
   // IP streaming MUST accept 0-byte return values at any stage.
   //

   S32 needed = bytes_requested;

   while (1)
      {
      //
      // Get as much data as possible from frame buffer
      //

      if (RECV_read_cursor < RECV_write_cursor)
         {
         S32 n = min(needed, 
                     RECV_write_cursor - RECV_read_cursor);

         memcpy(dest,
               &RECV_frame[RECV_read_cursor],
                n);

         dest              = ((C8 *) dest) + n;
         RECV_read_cursor += n;
         needed           -= n;
         }

      //
      // If all requested data has been read, return
      //

      if (needed <= 0)
         {
         //
         // Keep read/write cursors in first half of frame buffer
         //

         if (RECV_read_cursor >= RECEIVE_FRAME_SIZE)
            {
            RECV_read_cursor  -= RECEIVE_FRAME_SIZE;
            RECV_write_cursor -= RECEIVE_FRAME_SIZE;
            }

         return bytes_requested;
         }

      //
      // We need more data -- read it into the frame buffer
      //

      S32 result = NET_poll_for_data(&RECV_frame[RECV_write_cursor],
                                      needed);

      if (result == -1)
         {
         //
         // Client disconnected, return 0 to abort current frame
         //

         return 0;
         }

      if (result == 0)
         {
         //
         // No data available yet
         //

         Sleep(10);
         }
      else
         {
         //
         // Advance the write cursor
         //

         RECV_write_cursor += result;
         }

      //
      // Block until more data comes in, server disconnects, or
      // keyboard hit
      //

//      if (kbhit() && getch())
//         {
//         return 0;
//         }
      }
}

// ---------------------------------------------------------------------------
// XMIT_stream_CB()
// ---------------------------------------------------------------------------

S32 CALLBACK XMIT_stream_CB(U32       user, //)
                            void FAR *dest,
                            S32       bytes_requested,
                            S32       offset)
{
   //
   // Block transmission thread in this routine until specified
   // number of bytes available from input, or disconnection occurs
   //

   S32 bytes_sent = 0;

   while (bytes_requested > 0)
      {
      //
      // Allow other threads to run
      //

      Sleep(3);

      //
      // Exit if transmit thread killed by main thread
      //
      // Returning 0 here will cause transmit_thread_procedure() to exit
      // when its ASI_stream_process() call fails
      //

      if (!threads_active)
         {
         return 0;
         }

      //
      // Get input buffer to send
      //

      if ((input_buffer_tail == -1) ||
          (input_buffer_tail == input_buffer_head))
         {
         //
         // No input buffers available
         //

         continue;
         }

      //
      // Transmit data from input buffer at queue tail
      //

      C8 FAR *src       = &input_buffer [input_buffer_tail] [input_read_offset];
      S32     src_avail = INPUT_BUFFER_SIZE - input_read_offset;

      S32 send_amount = min(bytes_requested,
                            src_avail);

      memcpy(dest,
             src,
             send_amount);

      dest               = ((C8 *) dest) + send_amount;
      input_read_offset += send_amount;
      bytes_requested   -= send_amount;
      bytes_sent        += send_amount;

      if (input_read_offset == INPUT_BUFFER_SIZE)
         {
         //
         // Entire buffer has been sent; advance tail pointer
         //

         if (input_buffer_tail == N_INPUT_BUFFERS-1)
            {
            input_buffer_tail = 0;
            }
         else
            {
            ++input_buffer_tail;
            }

         input_read_offset = 0;
         }
      }

   return bytes_sent;
}

// ---------------------------------------------------------------------------
// receive_ASI_thread_procedure
// ---------------------------------------------------------------------------

unsigned int WINAPI receive_ASI_thread_procedure(LPVOID pParam)
{
   S32 fetch_buffer_full = 0;

   while (1)
      {
      //
      // Exit if receive thread killed by main thread
      //

      if (!threads_active)
         {
         return 0;
         }

      //
      // If fetch buffer empty, call ASI decoder to acquire decompressed data
      // from server
      //
      // This will block in RECV_stream_CB() until enough data is received
      // from the server to fill the fetch buffer
      //
      // Using the temporary fetch buffer allows us to read and decompress 
      // data while all of the receive buffers are full -- otherwise, we'd
      // have to spin until an empty receive buffer becomes available
      //

      if (!fetch_buffer_full)
         {
         S32 amount = RECV.ASI_stream_process(RECV.stream, 
                                              fetch_buffer,
                                              receive_buffer_size);

         if (amount != receive_buffer_size)
            {
            //
            // Bad read
            //

            active = 0;
            return 0;
            }
//         else
//            {
//            printf(".");
//            }

         fetch_buffer_full = 1;
         }

      //
      // Decompressed PCM data is now available in fetch buffer
      //
      // See if there is room in the receive buffer chain for the fetched data
      //

      if (receive_buffer_full[receive_write_buffer])
         {
         //
         // No room -- we must wait for output stream to free up a buffer
         //

         Sleep(3);
         continue;
         }

      //
      // If so, copy contents of fetch buffer to empty receive buffer, and
      // mark fetch buffer empty
      //

      memcpy(receive_buffer[receive_write_buffer],
             fetch_buffer,
             receive_buffer_size);

      fetch_buffer_full = 0;

      //
      // Mark buffer "full" and advance write buffer counter
      //
      // If a full set of buffers has been received, enable output streaming
      // so that buffers will start to be emptied by the foreground thread
      //

      receive_buffer_full[receive_write_buffer] = 1;

      if (receive_write_buffer == N_RECEIVE_BUFFERS-1)
         {
         receive_write_buffer = 0;
         triggered = 1;
         }
      else
         {
         ++receive_write_buffer;
         }
      }
}

// ---------------------------------------------------------------------------
// transmit_ASI_thread_procedure
// ---------------------------------------------------------------------------

unsigned int WINAPI transmit_ASI_thread_procedure(LPVOID pParam)
{
   //
   // Loop until thread terminated
   //

   S32 current_send_offset = TRANSMIT_PACKET_SIZE;

   while (1)
      {
      //
      // Allow other threads to run
      //

      Sleep(3);

      //
      // Exit if transmit thread killed by main thread
      //

      if (!threads_active)
         {
         return 0;
         }

      //
      // See if more transmit data is needed
      //

      if (current_send_offset == TRANSMIT_PACKET_SIZE)
         {
         //
         // Request data from ASI encoder to send to server
         //
         // This will block in XMIT_stream_CB() until enough input
         // data is available to satisfy the request
         //

         S32 amount = XMIT_stream_process(transmit_stream,
                                          send_buffer,
                                          TRANSMIT_PACKET_SIZE);

         if (amount != TRANSMIT_PACKET_SIZE)
            {
            //
            // Bad read
            //

            active = 0;
            return 0;
            }

         current_send_offset = 0;
         }

      //
      // If data is ready to be sent, try to send it
      //

      if (current_send_offset != TRANSMIT_PACKET_SIZE)
         {
         S32 send_amount = TRANSMIT_PACKET_SIZE - current_send_offset;

         S32 result = send(data_socket,
                          &send_buffer[current_send_offset],
                           send_amount,
                           0);

         if (result == SOCKET_ERROR)
            {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
               active = 0;
               return 0;
               }

            result = 0;
            }

         current_send_offset += result;
         }
      }
}

// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------

void __cdecl shutdown(void)
{
   threads_active = 0;

   if (transmit_thread != NULL)
      {
      WaitForSingleObject(transmit_thread,
                          INFINITE);

      CloseHandle(transmit_thread);
      transmit_thread = NULL;
      }

   if (receive_thread != NULL)
      {
      WaitForSingleObject(receive_thread,
                          INFINITE);

      CloseHandle(receive_thread);
      receive_thread = NULL;
      }

   if (transmit_stream != NULL)
      {
      XMIT_stream_close(transmit_stream);
      transmit_stream = NULL;
      }

   if (RECV.stream != NULL)
      {
      RECV.ASI_stream_close(RECV.stream);
      RECV.stream = NULL;
      }

   if (input != NULL)
      {
      AIL_close_input(input);
      input = NULL;
      }

   if (dig != NULL)
      {
      AIL_waveOutClose(dig);
      dig = NULL;
      }

   AIL_shutdown();

   if (hs_socket != NULL)
      {
      closesocket(hs_socket);
      hs_socket = NULL;
      }

   if (data_socket != NULL)
      {
      closesocket(data_socket);
      data_socket = NULL;
      }

   WSACleanup();

}

#define CLOSE 106
#define ABOUT 107
#define LEVELS 108
#define PROGRESS 200

//############################################################################
//##                                                                        ##
//## Main window procedure                                                  ##
//##                                                                        ##
//############################################################################

LRESULT AILEXPORT Window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWND h;

   switch (message)
      {

      case WM_SETFOCUS:    // deal with the focus in this weird dialog-window
          h=GetWindow(hwnd,GW_CHILD);
          while (h) {
            if ((GetWindowLong(h,GWL_STYLE)&0x2f)==BS_DEFPUSHBUTTON) {
              SetFocus(h);
              goto found;
            }
            h=GetNextWindow(h,GW_HWNDNEXT);
          }
          SetFocus(GetWindow(hwnd,GW_CHILD));
       found:
          break;

      case WM_COMMAND:

         switch (wParam)
         {
           case LEVELS:
             WinExec("Control mmsys.cpl",SW_SHOWNORMAL);
             break;

           case ABOUT:
             MessageBox(hwnd,
                "Miles Sound System Voice Chat Client - Version " MSS_VERSION
                "\n\nFor questions or comments, please contact RAD Game Tools at:\n\n"
                "\tRAD Game Tools\n"
                "\t335 Park Place - Suite G109\n"
                "\tKirkland, WA  98033\n"
                 "\t425-893-4300\n"
                "\tFAX: 425-893-9111\n\n"
                "\tWeb: http://www.radgametools.com\n"
                "\tE-mail: sales@radgametools.com",
                 "About the Miles Sound System Voice Chat Client...", MB_OK|MB_ICONSTOP);
             break;

           case IDCANCEL:
           case CLOSE:
             DestroyWindow(hwnd);
             break;
         }
         return 0;

      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

      }

   return DefWindowProc(hwnd,message,wParam,lParam);
}

//
// a few command line parsing functions
//

static void Trim(char* str)
{
  char * s=str;
  while ((*s<=' ') && (*s!=0))
    s++;
  strcpy(str,s);
  s=str+strlen(str)-1;
  while ((s>=str) && ((*s<=' ') && (*s!=0)))
    s--;
  *(s+1)=0;
}

static void TrimL(char *str)
{
  char *s=str;
  while ((*s<=' ') && (*s!=0))
    s++;
  if (str!=s)
    strcpy(str,s);
}

static void GetAndRemoveFirst(char *dest,char *source)
{
  TrimL(source);
  if (*source==0)
    *dest=0;
  else {
    char*s=source;
    while (*s>' ')
      s++;
    while ((*s!=0) && (*s<=' '))
      s++;
    if (*s==',')
      s++;
    memcpy(dest,source,s-source);
    dest[s-source]=0;
    strcpy(source,s);
    Trim(dest);
    TrimL(source);
  }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
   MSG      msg;
   WNDCLASS wndclass;
   char cmd[512];
   char ipnum[512];
   char buf[512];

   if (!hPrevInstance)
      {
      wndclass.lpszClassName = szAppName;
      wndclass.lpfnWndProc   = (WNDPROC) Window_proc;
      wndclass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
      wndclass.hInstance     = hInstance;
      wndclass.hIcon         = LoadIcon(hInstance,"Demo");
      wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = DLGWINDOWEXTRA;
      wndclass.lpszMenuName  = NULL;

      RegisterClass(&wndclass);
      }

   strcpy(cmd,lpszCmdLine);
   GetAndRemoveFirst(ipnum,cmd);

   InitCommonControls();

   hwnd = CreateDialog(hInstance,szAppName,0,NULL);

   if (hwnd==0) {
     MessageBox(0,"Couldn't create dialog box.","Error",MB_OK|MB_ICONSTOP);
     return(0);
   }

   progress=GetDlgItem(hwnd,PROGRESS);
   SendMessage(progress,PBM_SETRANGE,0,MAKELPARAM(0,32767));

   //
   // Fail if no server IP addr given
   //

   if (*cmd==0)
      {
      MessageBox(hwnd,"Usage: MSSChtC server_IP <stream_filename.xxx | .xxx>","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   C8 suffix[8] = ".raw";

   C8 *str = strrchr(cmd,'.');

   if (str != NULL)
      {
      strncpy(suffix,
              str,
              4);
      }

   //
   // If suffix alone supplied, use voice input with specified ASI codec
   //

   if (str == cmd)
      {
      input_source = SOURCE_VOICE;
      }
   else
      {
      input_source = SOURCE_FILE;
      }

   //
   // Start MSS and WinSock
   //

   WSADATA wsadata;
   WORD    wVer = MAKEWORD(1,1);

   WSAStartup(wVer, &wsadata);

   AIL_set_redist_directory("..\\..\\redist\\" MSS_REDIST_DIR_NAME);
   AIL_startup();

   ShowWindow(hwnd,nCmdShow);

   //
   // Register shutdown handler for resource cleanup
   //

   transmit_stream = NULL;
   transmit_thread = NULL;
   RECV.stream  = NULL;
   receive_thread  = NULL;
   dig             = NULL;
   hs_socket       = NULL;
   data_socket     = NULL;

   atexit(shutdown);

   //
   // Init digital driver and output stream sample
   //

   S32 result = 0;

   dig = AIL_open_digital_driver( HW_RATE, HW_BITS, HW_CHANNELS, 0 );

   if (dig==0)
   {
      MessageBox(hwnd,"Unable to open sound output device","Error",MB_OK|MB_ICONSTOP);
      return(0);
   }

   stream = AIL_allocate_sample_handle(dig);

   //
   // Create handshaking and data sockets
   //

   hs_socket = socket(PF_INET, SOCK_STREAM, 0);

   if (hs_socket == INVALID_SOCKET)
      {
      MessageBox(hwnd,"Bad Socket!","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   data_socket = socket(PF_INET, SOCK_STREAM, 0);

   if (hs_socket == INVALID_SOCKET)
      {
      MessageBox(hwnd,"Bad Socket!","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Submit connection request to server for handshaking socket
   //

   struct sockaddr_in server_addr;

   memset(&server_addr,0,sizeof(server_addr));

   server_addr.sin_family      = PF_INET;
   server_addr.sin_addr.s_addr = inet_addr(ipnum);
   server_addr.sin_port        = htons(MONITOR_PORT);

   result = connect(hs_socket,
      (sockaddr *) &server_addr,
                    sizeof(server_addr));

   if (result)
      {
      wsprintf(buf,"Error: Unable to connect to server, code %d",WSAGetLastError());
      MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Send 8-byte data type specifier to server on handshaking socket, so it
   // can search for an ASI codec to deal with it
   //

   wsprintf(buf,"Connected to server: %s",ipnum);
   SetWindowText(GetDlgItem(hwnd,500),buf);

   result = send(hs_socket,
                 suffix,
                 8,
                 0);

   if (result != 8)
      {
      wsprintf(buf,"Error: Sent %d bytes, error = %d",result,WSAGetLastError());
      MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Wait for server acknowledgement
   //

   C8 status = 0;

   recv(hs_socket,
       &status,
        1,
        0);

   if (!status)
      {
      wsprintf(buf,"Error: Server unable to process data type \"%s\"\n",
         suffix);
      MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Submit connection request to server for data socket
   //

   memset(&server_addr,0,sizeof(server_addr));

   server_addr.sin_family      = PF_INET;
   server_addr.sin_addr.s_addr = inet_addr(ipnum);
   server_addr.sin_port        = htons(MONITOR_PORT);

   result = connect(data_socket,
      (sockaddr *) &server_addr,
                    sizeof(server_addr));

   if (result)
      {
      wsprintf(buf,"Error: Unable to connect to server, code %d\n",WSAGetLastError());
      MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Mark data socket non-blocking
   //
   // Handshake socket remains in blocking mode, since we don't expect it
   // to be blocked for long
   //
   // Also disable Nagling to minimize transmission buffering of small packets
   //

   U32 dwVal = 1;

   ioctlsocket(data_socket,
               FIONBIO,
              &dwVal);

   dwVal = 1;

   setsockopt(data_socket,
              IPPROTO_TCP,
              TCP_NODELAY,
    (char *) &dwVal,
              sizeof(dwVal));

   threads_active = 1;

   //
   // Open data source
   //

   FILE *src_file = NULL;
   S32   src_len = 0;

   if (input_source == SOURCE_FILE)
      {
      //
      // Open source file
      //

      src_file = fopen(cmd,"rb");

      if (src_file == NULL)
         {
         MessageBox(hwnd,"File not found!","Error",MB_OK|MB_ICONSTOP);
         return(0);
         }

      src_len = AIL_file_size(cmd);
      }
   else
      {
      //
      // Start up input subsystem
      //

      AIL_INPUT_INFO info;

      S32 in_bits = 16;
      S32 in_rate = 8000;

      info.device_ID       = WAVE_MAPPER;
      info.hardware_format = (in_bits == 16) ? DIG_F_MONO_16 : DIG_F_MONO_8;
      info.hardware_rate   = in_rate;
      info.callback        = serve_input;
      info.buffer_size     = INPUT_BUFFER_SIZE;

      input = AIL_open_input(&info);

      if (input == NULL)
         {
         wsprintf(buf,"Error: %s",AIL_last_error());
         MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
         return(0);
         }

      input_buffer_head = 0;
      input_buffer_tail = -1;

      AIL_set_input_state(input, 1);

      //
      // Find ASI provider to encode voice data for transmission
      //

      transmit_encoder = RIB_find_files_provider("ASI codec",
                                                 "Output file types",
                                                  suffix,
                                                 "Input file types",
                                                 ".RAW"
                                                  );

      if (transmit_encoder == NULL)
         {
         wsprintf(buf,"Error: No ASI provider available to encode data type '%s'",
            suffix);
         MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
         return(0);
         }

      wsprintf(buf,"Using ASI provider %X to transmit data type '%s'",transmit_encoder,
         suffix);
      SetWindowText(GetDlgItem(hwnd,501),buf);

      //
      // Get ASI stream interface for transmit encoder
      //

      RIB_INTERFACE_ENTRY ASISTR[] =
         {
         { RIB_FUNCTION, "ASI_stream_open",           (U32) &XMIT_stream_open,           RIB_NONE },
         { RIB_FUNCTION, "ASI_stream_close",          (U32) &XMIT_stream_close,          RIB_NONE },
         { RIB_FUNCTION, "ASI_stream_process",        (U32) &XMIT_stream_process,        RIB_NONE },
         };

      RIB_request(transmit_encoder,"ASI stream",ASISTR);

      //
      // Open stream with codec, registering callback function
      //

      transmit_stream = XMIT_stream_open(0,
                                         XMIT_stream_CB,
                                         0);

      if (transmit_stream == NULL)
         {
         MessageBox(hwnd,"Could not open stream encoder","Error",MB_OK|MB_ICONSTOP);
         return(0);
         }

      //
      // Create thread to monitor input and compress the output data
      //

      DWORD stupId;

      transmit_thread = (HANDLE) _beginthreadex(NULL,
                                                0,
                                                transmit_ASI_thread_procedure,
                                                NULL,
                                                0,
                              (unsigned int *) &stupId);
      }

   //
   // Search for ASI codec capable of processing specified input file type
   //
   // This ASI provider will be used to decode data sent from the server
   // to the client
   //

   receive_decoder = RIB_find_files_provider("ASI codec",
                                            "Output file types",
                                            ".RAW",
                                            "Input file types",
                                             suffix);


   if (receive_decoder == NULL)
      {
      wsprintf(buf,"Error: No ASI provider available to decode data type '%s'",
         suffix);
      MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Get ASI stream interface for receive decoder
   //

   RIB_INTERFACE_ENTRY RECVSTR[] =
      {
      { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &RECV.ASI_stream_attribute,      RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_open",           (U32) &RECV.ASI_stream_open,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &RECV.ASI_stream_seek,           RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_close",          (U32) &RECV.ASI_stream_close,          RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_process",        (U32) &RECV.ASI_stream_process,        RIB_NONE },
      { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &RECV.ASI_stream_set_preference, RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &RECV.OUTPUT_SAMPLE_RATE,        RIB_NONE },
      { RIB_ATTRIBUTE,  "Output sample width",       (U32) &RECV.OUTPUT_BITS,               RIB_NONE },
      { RIB_ATTRIBUTE,  "Output channels",           (U32) &RECV.OUTPUT_CHANNELS,           RIB_NONE },
      { RIB_PREFERENCE, "Requested sample rate",     (U32) &RECV.REQUESTED_RATE,            RIB_NONE },
      };

   RIB_request(receive_decoder,"ASI stream",RECVSTR);

   //
   // Open the receiver stream
   //
   // If the codec needs to inspect the stream data to
   // configure itself, this call will block in the RECV_stream_CB() 
   // handler until the requested amount of data (typically only
   // the first few bytes of the source stream) is received from the
   // client.
   //

   RECV_read_cursor = 0;
   RECV_write_cursor = 0;

   RECV.stream = RECV.ASI_stream_open(0,
                                         RECV_stream_CB,
                                         0);

   if (RECV.stream == NULL)
      {
       MessageBox(hwnd,"Could not open stream decoder","Error",MB_OK|MB_ICONSTOP);
       return(0);
      }

   //
   // Request codec output rate which matches hardware rate
   //

   U32 req_rate = HW_RATE;

   RECV.ASI_stream_set_preference(RECV.stream,
                                  RECV.REQUESTED_RATE,
                                 &req_rate);

   //
   // Stream is now open -- get its attributes and set output
   // sample attributes accordingly
   //

   U32 nch  = RECV.ASI_stream_attribute(RECV.stream, RECV.OUTPUT_CHANNELS);
   U32 rate = RECV.ASI_stream_attribute(RECV.stream, RECV.OUTPUT_SAMPLE_RATE);
   U32 bits = RECV.ASI_stream_attribute(RECV.stream, RECV.OUTPUT_BITS);

   S32 type;

   if (nch == 2)
      {
      type = ( (bits == 16) ? DIG_F_STEREO_16 : DIG_F_STEREO_8 ) |
             ( (bits == 16) ? DIG_PCM_SIGN    : 0 );
      }
   else
      {
      type = ( (bits == 16) ? DIG_F_MONO_16   : DIG_F_MONO_8) |
             ( (bits == 16) ? DIG_PCM_SIGN    : 0 );
      }

   AIL_init_sample             (stream);
   AIL_set_sample_playback_rate(stream, rate);
   AIL_set_sample_type         (stream, type, 0);

   receive_buffer_size = AIL_minimum_sample_buffer_size(dig, rate, type);

   wsprintf(buf,"Receive stream format: %d channels, %d Hz, %d bits",nch,rate,bits);
   SetWindowText(GetDlgItem(hwnd,502),buf);

   printf("Receive buffer size=%d\n",receive_buffer_size);

   //
   // Allocate receiver buffers
   //

   receive_read_buffer = 0;
   receive_write_buffer = 0;

   S32 i;

   for (i=0; i < N_RECEIVE_BUFFERS; i++)
      {
      receive_buffer_full[i] = 0;

      receive_buffer[i] = (C8 *) malloc(receive_buffer_size);

      if (receive_buffer[i] == NULL)
         {
         MessageBox(hwnd,"Out of memory!","Error",MB_OK|MB_ICONSTOP);
         return(0);
         }
      }

   for (i=0; i < 2; i++)
      {
      stream_buffer[i] = (C8 *) malloc(receive_buffer_size);

      if (stream_buffer[i] == NULL)
         {
         MessageBox(hwnd,"Out of memory!","Error",MB_OK|MB_ICONSTOP);
         return(0);
         }
      }

   fetch_buffer = (C8 *) malloc(receive_buffer_size);

   if (fetch_buffer == NULL)
      {
       MessageBox(hwnd,"Out of memory!","Error",MB_OK|MB_ICONSTOP);
       return(0);
      }

   //
   // Create thread to receive compressed data from server and decompress
   // it to output buffers
   //

   DWORD stupId;

   receive_thread = (HANDLE) _beginthreadex(NULL,
                                            0,
                                            receive_ASI_thread_procedure,
                                            NULL,
                                            0,
                          (unsigned int *) &stupId);

   //
   // Event loop
   //

   triggered = 0;

   S32 current_send_offset = TRANSMIT_PACKET_SIZE;

   active = 1;
   S32 file_active = 1;

   while (active)
      {
      if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT)
          break;
        if (!IsDialogMessage(hwnd,&msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }

      //
      // Allow other threads to run
      //

      Sleep(3);

      //
      // Perform file transmission
      //
      // This is handled from a background thread if voice input is used
      //

      if ((input_source == SOURCE_FILE) && (file_active))
         {
         //
         // See if more transmit data is needed
         //

         if (current_send_offset == TRANSMIT_PACKET_SIZE)
            {
            memset(send_buffer, 0, sizeof(send_buffer));

            current_send_offset = 0;

            S32 fill = 0;

            while (fill < TRANSMIT_PACKET_SIZE)
               {
               S32 result = fread(&send_buffer[fill], 1, TRANSMIT_PACKET_SIZE - fill, src_file);

               fill += result;

               if (result < TRANSMIT_PACKET_SIZE - fill)
                  {
                  fseek(src_file, 0, SEEK_SET);
                  file_active = 0;
                  }
               }
            }

         //
         // If data is ready to be sent, try to send it
         //

         if (current_send_offset != TRANSMIT_PACKET_SIZE)
            {
            S32 send_amount = TRANSMIT_PACKET_SIZE - current_send_offset;

            S32 result = send(data_socket,
                             &send_buffer[current_send_offset],
                              send_amount,
                              0);

            if (result == SOCKET_ERROR)
               {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
                  {
                  wsprintf(buf,"Error: Sent %d bytes, code = %d",result,WSAGetLastError());
                  MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
                  break;
                  }

               result = 0;
               }

            current_send_offset += result;
            }
         }

      //
      // Service stream buffers
      //
      // If we have not yet received a full set of buffers from the server,
      // don't submit any to the output stream
      //

      if (!triggered)
         {
         continue;
         }

      //
      // If no new stream buffer is needed yet, continue
      //

      S32 n = AIL_sample_buffer_ready(stream);

      if (n == -1)
         {
         continue;
         }

      //
      // If output stream is starving because insufficient decoded data
      // is available, print warning message, then flush stream buffer
      // with silence
      //
      // This is conservative -- because the MSS stream is double-buffered
      // itself, we don't actually need to feed it the moment AIL_sample_buffer_ready()
      // returns a valid buffer.
      //

      if (!receive_buffer_full[receive_read_buffer])
         {
//         printf("WARNING: Wanted data but none ready yet!\n");

         memset(stream_buffer[n],
                0,
                receive_buffer_size);
         }
      else
         {

         //
         // Copy next receive buffer into stream buffer, and mark it "empty"
         //

         memcpy(stream_buffer[n],
                receive_buffer[receive_read_buffer],
                receive_buffer_size);

         receive_buffer_full[receive_read_buffer] = 0;

         //
         // Advance receive buffer counter
         //

         if (receive_read_buffer == N_RECEIVE_BUFFERS-1)
            {
            receive_read_buffer = 0;
            }
         else
            {
            ++receive_read_buffer;
            }
         }

      //
      // Submit stream buffer to MSS
      //

      AIL_load_sample_buffer(stream,
                             n,
                             stream_buffer[n],
                             receive_buffer_size);
      }

  if (active==0)
    MessageBox(hwnd,"Server was shutdown or a link error occurred.","Connection down",MB_OK|MB_ICONINFORMATION);

  return(1);
}
