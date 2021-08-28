//############################################################################
//##                                                                        ##
//##  SERVER.CPP                                                            ##
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

char szAppName[]="MSSCHTS";
HWND hwnd;
HWND status;
HWND hear;

//
// Server listens for connection requests on port 49778
//

#define MONITOR_PORT 49778

//
// Hardware settings for monitor audio
//

#define HW_FORMAT DIG_F_MONO_16
#define HW_RATE   8000

//
// Up to 16 clients supported
//

#define N_CLIENTS 16

//
// Wait until sufficient data is buffered in each client's queue before
// allowing that client to contribute data to the output streams
//
// Example: 2K = approx. 1/8 second at 8-kHz 16-bit mono
//

#define CLIENT_LATENCY 3072

//
// Mix and send blocks of data to the clients at 100-millisecond intervals
// In practice, this will often be governed by the codec's frame buffer size;
// the Voxware 2400bps codec uses 22-millisecond frames so 100 milliseconds
// is conservative
//

#define CLIENT_MIX_PERIOD 100

#define CLIENT_MIX_BUFFER_SIZE (HW_RATE / (1000 / (2 * CLIENT_MIX_PERIOD)))

//
// Client receive queue size
//
// This queue feeds the foreground mixer thread, and isolates it from
// the background ASI decoder thread
//
// It should be large enough to comfortably hold a few mixing intervals'
// worth of source data in the uncompressed PCM format
//

#define RECEIVE_QUEUE_SIZE 8192

//
// Client transmit queue size
//
// This queue feeds the ASI encoder thread, and isolates it from
// the foreground mixer thread
//
// Typically this will be the same size as the receive queue, since the
// total amount of data received from and transmitted to a
// client will be the same
//

#define TRANSMIT_QUEUE_SIZE 8192

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

//
// Our ASI descriptor structure
//

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


//****************************************************************************
//
// CQueue
//
// Template used to represent circular queue
//
// Use CQueue when:
//
//    - You want to maintain a circular buffer of arbitrary size
//
//    - You want to insulate memory- or message-based communication
//      between threads with a critsec-protected layer
//
//    - You want to retrieve inserted entries in FIFO (first-in, first-out)
//      order
//
// The # of slots in the queue available for data insertion equals size-1
//
//****************************************************************************

template <class T, S32 size=256> class CQueue
{
   T                queue[size];
   S32              head;
   S32              tail;
   CRITICAL_SECTION cs;

public:

   //
   // Constructor and destructor
   //

   CQueue()
      {
      InitializeCriticalSection(&cs);
      reset();
      }

  ~CQueue()
      {
      DeleteCriticalSection(&cs);
      }

   //
   // Reset queue to empty state by abandoning contents
   //

   void reset(void)
      {
      EnterCriticalSection(&cs);

      head = 0;
      tail = 0;

      LeaveCriticalSection(&cs);
      }

   //
   // Return # of free entry slots in queue
   //
   // This value is guaranteed to remain conservatively valid until more
   // data is added
   //

   S32 space_available(void)
      {
      EnterCriticalSection(&cs);

      S32 result;

      if (head > tail)
         {
         result = (head - tail) - 1;
         }
      else
         {
         result = (size - (tail - head)) - 1;
         }

      LeaveCriticalSection(&cs);
      return result;
      }

   //
   // Return # of occupied entry slots in queue
   //
   // This value is guaranteed to remain conservatively valid until data
   // is fetched
   //

   S32 data_available(void)
      {
      EnterCriticalSection(&cs);

      S32 result;

      if (tail >= head)
         {
         result = tail - head;
         }
      else
         {
         result = size - (head - tail);
         }

      LeaveCriticalSection(&cs);

      return result;
      }

   //
   // Fetch data from queue
   //
   // IMPORTANT: Does not verify the requested amount of data is actually
   // in the queue -- always check data_available() first!
   //

   void fetch_data(T   *dest,
                   S32  amount)
      {
      EnterCriticalSection(&cs);

      if ((head + amount) >= size)
         {
         memcpy(dest,
               &queue[head],
                size - head);

         dest   += (size - head);
         amount -= (size - head);

         head = 0;
         }

      if (amount)
         {
         memcpy(dest,
               &queue[head],
                amount);

         head += amount;
         }

      LeaveCriticalSection(&cs);
      }

   //
   // Put data into queue
   //
   // IMPORTANT: Does not verify the necessary amount of space is actually
   // available in the queue -- always check space_available() first!
   //

   void put_data(T  *src,
                 S32 amount)
      {
      EnterCriticalSection(&cs);

      if ((tail + amount) >= size)
         {
         memcpy(&queue[tail],
                 src,
                 size - tail);

         src    += (size - tail);
         amount -= (size - tail);

         tail = 0;
         }

      if (amount)
         {
         memcpy(&queue[tail],
                 src,
                 amount);

         tail += amount;
         }

      LeaveCriticalSection(&cs);
      }
};


//
// Client thread state
//

enum CLIENTSTATE
{
   UNCONNECTED,         // Not connected
   WAITING_FOR_DATA_ID, // Waiting for 8-byte data type (file suffix) identifier
   ACTIVE_ASI,          // Actively streaming ASI-encoded data
};

//
// Client data structure
//

volatile struct CLIENT
{
   SOCKET      data_socket;         // Socket for data communication with client
   SOCKET      hs_socket;           // Socket for handshaking with client
   sockaddr_in address;             // Address and port of client

   CQueue <C8,RECEIVE_QUEUE_SIZE>  *receive;   // Circular buffer used to receive data
   CQueue <C8,TRANSMIT_QUEUE_SIZE> *transmit;  // Circular buffer used to transmit data

   S32         in_use;            // Goes high when client structure is allocated
   S32         terminate_request; // Set to 1 to force thread to exit
   CLIENTSTATE state;             // Determines client thread behavior

   HANDLE  receive_thread;        // RECV_stream_CB() blocks in this thread
   HANDLE  transmit_thread;       // XMIT_stream_CB() blocks in this thread

   HPROVIDER HPRECV;              // ASI provider used to decode data received from client
   ASISTRUCT RECV;                // ASI interface
   C8       *RECV_frame;          // ASI frame buffer
   S32       RECV_frame_size;     // ASI frame buffer size in bytes
   S32       RECV_read_cursor;    // ASI frame buffer read position
   S32       RECV_write_cursor;   // ASI frame buffer write position

   HPROVIDER HPXMIT;              // ASI provider used to encode data transmitted to client
   ASISTRUCT XMIT;                // ASI interface
   C8       *XMIT_frame;          // ASI frame buffer
   S32       XMIT_frame_size;     // ASI frame buffer size in bytes
   S32       XMIT_read_cursor;    // ASI frame buffer read position
   S32       XMIT_write_cursor;   // ASI frame buffer write position

   S32       triggered;          // Client is ready to contribute data to output streams
   S32       samples_to_mix;     // # of samples in fetch buffer
   S16       fetch_buffer  [CLIENT_MIX_BUFFER_SIZE];
   S32       mixer_buffer  [CLIENT_MIX_BUFFER_SIZE];
};

static char scroll[4096+256];
static U32 scrolllen=0;

static show_string(char* buf)
{
  U32 i;
  if (scrolllen>4096)
  {
    i=0;
    while (scroll[i]!=10)
      i++;
    memcpy(scroll,scroll+i+1,scrolllen-i);
    scrolllen-=i+1;
  }
  i=strlen(buf);
  memcpy(scroll+scrolllen,buf,i);
  memcpy(scroll+scrolllen+i,"\r\n",3);
  scrolllen+=i+2;
  SetWindowText(status,scroll);
  SendMessage(status,EM_SETSEL,scrolllen,scrolllen);
  SendMessage(status,EM_SCROLLCARET,0,0);
}

//
// Globals
//

HDIGDRIVER dig;

CLIENT clients[N_CLIENTS];     // Client list

SOCKET monitor_socket = NULL;
volatile S32    monitor_active = 0;

// ---------------------------------------------------------------------------
// NET_wait_for_data
// ---------------------------------------------------------------------------

S32 NET_wait_for_data(CLIENT *C, //)
                         C8     *dest,
                         S32     size)
{
   S32 index = 0;

   while (index < size)
      {
      S32 amount = recv(C->hs_socket,
                       &dest[index],
                        size - index,
                        0);

      if (amount == 0)
         {
         //
         // Connection has been closed, return failure
         //

         return 0;
         }

      if (amount == SOCKET_ERROR)
         {
         //
         // No data available yet, retry
         //

         if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
            return 0;
            }
         }
      else
         {
         index += amount;
         }

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

   return 1;
}

// ---------------------------------------------------------------------------
// NET_poll_for_data
// ---------------------------------------------------------------------------

S32 NET_poll_for_data(CLIENT *C, //)
                         C8     *dest, 
                         S32     size)
{
   S32 index = 0;

   //
   // Get as much data as is currently available, up to specified size
   //

   while (index < size)
      {
      S32 amount = recv(C->data_socket,
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

S32 CALLBACK XMIT_stream_CB(U32       user, //)
                            void FAR *dest,
                            S32       bytes_requested,
                            S32       offset)
{
   CLIENT *C = (CLIENT *) user;

   //
   // Compression encoders typically don't attempt to perform seeks, so 
   // we can safely ignore the "offset" parameter
   //
   // Data will typically be requested one frame at a time by the ASI encoder.
   // A return value of less than bytes_requested indicates the end of the 
   // stream has been reached.  We don't support fragmented frames, so we'll 
   // return 0 to the codec if the client disconnects while we're waiting for
   // data to become available for it.  Any ASI codec designed for 
   // compatibility with IP streaming MUST accept 0-byte return values at 
   // any stage.
   //

   S32 needed = bytes_requested;

   while (1)
      {
      //
      // As soon as the required amount of mixed output data is available,
      // return it to the ASI encoder
      //

      S32 avail = C->transmit->data_available();

      if (avail)
         {
         S32 amount = min(avail, needed);

         C->transmit->fetch_data((C8 *) dest,
                                        amount);

         dest    = ((C8 *) dest) + amount;
         needed -= amount;
         }

      //
      // If all requested data has been obtained, return
      //

      if (needed <= 0)
         {
         return bytes_requested;
         }

      //
      // Otherwise, allow other threads to run while this one is blocked
      //

      Sleep(5);

      //
      // Block until more data comes in, client disconnects, or
      // keyboard hit
      //

      if (C->state == UNCONNECTED)
         {
         return 0;
         }

//      if (kbhit() && getch())
//         {
//         return 0;
//         }

      }
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
   CLIENT *C = (CLIENT *) user;

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
      C->RECV_read_cursor = offset;
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

      if (C->RECV_read_cursor < C->RECV_write_cursor)
         {
         S32 n = min(needed, 
                     C->RECV_write_cursor - C->RECV_read_cursor);

         memcpy(dest,
               &C->RECV_frame[C->RECV_read_cursor],
                n);

         dest                = ((C8 *) dest) + n;
         C->RECV_read_cursor += n;
         needed             -= n;
         }

      //
      // If all requested data has been read, return
      //

      if (needed <= 0)
         {
         //
         // Keep read/write cursors in first half of frame buffer
         //

         if (C->RECV_read_cursor >= C->RECV_frame_size)
            {
            C->RECV_read_cursor  -= C->RECV_frame_size;
            C->RECV_write_cursor -= C->RECV_frame_size;
            }

         return bytes_requested;
         }

      //
      // We need more data -- read it into the frame buffer
      //

      S32 result = NET_poll_for_data(C,
                                    &C->RECV_frame[C->RECV_write_cursor],
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

         C->RECV_write_cursor += result;
         }

      //
      // Block until more data comes in, client disconnects, or
      // keyboard hit
      //

//      if (kbhit() && getch())
//         {
//         return 0;
//         }
      }
}

// ---------------------------------------------------------------------------
// CLIENT_address
// ---------------------------------------------------------------------------

C8 *CLIENT_address(CLIENT *C)
{
   static C8 result[256];

   wsprintf(result,"%s:%d",
      inet_ntoa(C->address.sin_addr),
          ntohs(C->address.sin_port));

   return result;
}

// ---------------------------------------------------------------------------
// CLIENT_transmit_thread
//
// Manages transmission of compressed data to client
// ---------------------------------------------------------------------------

unsigned int WINAPI CLIENT_transmit_thread(LPVOID pParam)
{
   CLIENT *C = (CLIENT *) pParam;

   C8  packet_buffer[TRANSMIT_PACKET_SIZE];
   S32 current_send_offset = TRANSMIT_PACKET_SIZE;

   while (1)
      {
      //
      // Allow other threads to run
      //

      Sleep(5);

      //
      // Exit if thread killed
      //

      if ((C->terminate_request) ||
          (C->state != ACTIVE_ASI))
         {
         return 0;
         }

      //
      // See if more transmit data is needed
      //

      if (current_send_offset == TRANSMIT_PACKET_SIZE)
         {
         //
         // Request data from ASI encoder to send to client
         //
         // This will block in XMIT_stream_CB() until enough input
         // data is available to satisfy the request.  Because we've tried
         // to match the TRANSMIT_PACKET_SIZE and CLIENT_MIX_PERIOD parameters
         // to a reasonable extent, we will generally send about one packet
         // per mixing interval.  Our FIFO-based buffering mechanism allows 
         // for a large amount of 'slop' in this process without risking data 
         // loss.
         //

         S32 amount = C->XMIT.ASI_stream_process(C->XMIT.stream,
                                                 packet_buffer,
                                                 TRANSMIT_PACKET_SIZE);
         if (amount != TRANSMIT_PACKET_SIZE)
            {
            //
            // ASI process failed -- connection must have been lost
            //

            C->state = UNCONNECTED;
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

         S32 result = send(C->data_socket,
                          &packet_buffer[current_send_offset],
                           send_amount,
                           0);

         if (result == SOCKET_ERROR)
            {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
               C->state = UNCONNECTED;
               return 0;
               }

            result = 0;
            }

         current_send_offset += result;
         }
      }
}

// ---------------------------------------------------------------------------
// CLIENT_receive_thread
//
// Manages client connection state, and reception of compressed data
// from client
// ---------------------------------------------------------------------------

unsigned int WINAPI CLIENT_receive_thread(LPVOID pParam)
{
   char buf[128];

   CLIENT *C = (CLIENT *) pParam;

   while (1)
      {
      //
      // Allow other threads to run
      //

      Sleep(2);

      //
      // Exit if thread killed
      //

      if (C->terminate_request)
         {
         C->state = UNCONNECTED;
         return 0;
         }

      //
      // Otherwise, process thread state
      //

      switch (C->state)
         {
         //
         // UNCONNECTED: Run at low priority until terminated by foreground
         // process
         //

         case UNCONNECTED:
            {
            Sleep(10);
            break;
            }

         //
         // WAITING_FOR_DATA_ID: Block until 8-byte file suffix received,
         // then load required ASI codec and enable streaming
         // 

         case WAITING_FOR_DATA_ID:
            {
            //
            // Receive 8-byte data ID field from client
            //

            C8 suffix[8];
            C8 status = 0;

            if (!NET_wait_for_data(C, suffix, sizeof(suffix)))
               {
               C->state = UNCONNECTED;
               break;
               }

            //
            // Search for ASI codec capable of processing this input file type
            //
            // This ASI provider will be used to decode data sent from the client
            // to the server
            //

            C->HPRECV = RIB_find_files_provider("ASI codec",
                                               "Input file types",
                                                suffix,
                                               "Output file types",
                                               ".RAW");

            if (C->HPRECV == NULL)
               {
               wsprintf(buf,"Error: No ASI provider available for data type '%s'",
                  suffix);
               show_string(buf);

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            wsprintf(buf,"Using ASI provider %X to receive data type '%s'",C->HPRECV,
               suffix);
            show_string(buf);

            //
            // Search for ASI codec capable of encoding raw PCM data into
            // specified file type
            //
            // This ASI provider will be used to encode data sent from the server to
            // the client
            //

            C->HPXMIT = RIB_find_files_provider("ASI codec",
                                               "Output file types",
                                                suffix,
                                               "Input file types",
                                                ".RAW");

            if (C->HPXMIT == NULL)
               {
               wsprintf(buf,"Error: No ASI provider available for data type '%s'",
                  suffix);
               show_string(buf);

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            wsprintf(buf,"Using ASI provider %X to transmit data type '%s'",C->HPXMIT,
               suffix);
            show_string(buf);

            //
            // Get worst-case frame size for receive decoder
            //
            // Our ASI frame buffer must match this size, to ensure that
            // we can seek back to the beginning of the data stream after
            // ASI_stream_open() parses the first frame to determine the
            // data format
            //

            C->RECV_frame_size = 0;

            PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute;
            HATTRIB                  FSIZE;

            RIB_INTERFACE_ENTRY ASICODEC[] =
               {
               FN(PROVIDER_query_attribute),
               AT("Maximum frame size", FSIZE),
               };

            if (RIB_request(C->HPRECV,"ASI codec",ASICODEC) != RIB_NOERR)
               {
               show_string("Frame size attribute not available, upgrade your ASI codec!");

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            C->RECV_frame_size = PROVIDER_query_attribute(FSIZE);

            C->RECV_frame = (C8 *) AIL_mem_alloc_lock(C->RECV_frame_size * 2);

            if (C->RECV_frame == NULL)
               {
               show_string("Out of memory");

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            //
            // Get worst-case input frame size for transmit encoder
            //
            // This represents the minimum amount of data needed by
            // the transmission encoder to emit a frame
            //

            C->XMIT_frame_size = 0;

            if (RIB_request(C->HPXMIT,"ASI codec",ASICODEC) != RIB_NOERR)
               {
               show_string("Frame size attribute not available, upgrade your ASI codec!");

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            C->XMIT_frame_size = PROVIDER_query_attribute(FSIZE);

            C->XMIT_frame = (C8 *) AIL_mem_alloc_lock(C->XMIT_frame_size * 2);

            if (C->XMIT_frame == NULL)
               {
               show_string("Out of memory");

               send(C->hs_socket,
                   &status,
                    1,
                    0);

               C->state = UNCONNECTED;
               break;
               }

            wsprintf(buf,"XMIT frame size %d bytes, RECV frame size %d bytes",C->XMIT_frame_size, C->RECV_frame_size);
            show_string(buf);

            //
            // Get ASI stream interface for receive decoder
            //

            ASISTRUCT FAR *RECV = &C->RECV;

            RIB_INTERFACE_ENTRY RECVSTR[] =
               {
               { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &RECV->ASI_stream_attribute,      RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_open",           (U32) &RECV->ASI_stream_open,           RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &RECV->ASI_stream_seek,           RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_close",          (U32) &RECV->ASI_stream_close,          RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_process",        (U32) &RECV->ASI_stream_process,        RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &RECV->ASI_stream_set_preference, RIB_NONE },
               { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &RECV->OUTPUT_SAMPLE_RATE,        RIB_NONE },
               { RIB_ATTRIBUTE,  "Output sample width",       (U32) &RECV->OUTPUT_BITS,               RIB_NONE },
               { RIB_ATTRIBUTE,  "Output channels",           (U32) &RECV->OUTPUT_CHANNELS,           RIB_NONE },
               { RIB_PREFERENCE, "Requested sample rate",     (U32) &RECV->REQUESTED_RATE,            RIB_NONE },
               };

            RIB_request(C->HPRECV,"ASI stream",RECVSTR);

            //
            // Get ASI stream interface for transmit encoder
            //

            ASISTRUCT FAR *XMIT = &C->XMIT;

            RIB_INTERFACE_ENTRY XMITSTR[] = 
               {
               { RIB_FUNCTION,   "ASI_stream_attribute",      (U32) &XMIT->ASI_stream_attribute,      RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_open",           (U32) &XMIT->ASI_stream_open,           RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_seek",           (U32) &XMIT->ASI_stream_seek,           RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_close",          (U32) &XMIT->ASI_stream_close,          RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_process",        (U32) &XMIT->ASI_stream_process,        RIB_NONE },
               { RIB_FUNCTION,   "ASI_stream_set_preference", (U32) &XMIT->ASI_stream_set_preference, RIB_NONE },
               { RIB_ATTRIBUTE,  "Output sample rate",        (U32) &XMIT->OUTPUT_SAMPLE_RATE,        RIB_NONE },
               { RIB_ATTRIBUTE,  "Output sample width",       (U32) &XMIT->OUTPUT_BITS,               RIB_NONE },
               { RIB_ATTRIBUTE,  "Output channels",           (U32) &XMIT->OUTPUT_CHANNELS,           RIB_NONE },
               { RIB_PREFERENCE, "Requested sample rate",     (U32) &XMIT->REQUESTED_RATE,            RIB_NONE },
               };

            RIB_request(C->HPXMIT,"ASI stream",XMITSTR);

            //
            // Signal successful negotiation with client
            // This enables the client to begin sending data which is
            // necessary to open the stream
            //

            status = 1;

            send(C->hs_socket,
                &status,
                 1,
                 0);

            //
            // Establish data socket connection with client
            //

            while (1)
               {
               SOCKET      requestor;
               sockaddr_in request_addr;
               int         request_addr_size;

               memset(&request_addr,0,sizeof(request_addr));
               request_addr_size = sizeof(request_addr);

               requestor = accept(monitor_socket,
                    (sockaddr *) &request_addr,
                                 &request_addr_size);

               if (requestor != INVALID_SOCKET)
                  {
                  C->data_socket = requestor;

                  //
                  // Mark data socket non-blocking, and disable Nagling
                  //

                  U32 dwVal = 1;

                  ioctlsocket(C->data_socket,
                              FIONBIO,
                             &dwVal);

                  dwVal = 1;

                  setsockopt(C->data_socket,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                   (char *) &dwVal,
                             sizeof(dwVal));

                  dwVal = C->RECV_frame_size*2;
                  setsockopt(C->data_socket,
                             IPPROTO_TCP,
                             SO_RCVBUF,
                   (char *) &dwVal,
                             sizeof(dwVal));

                  dwVal = C->XMIT_frame_size*2;
                  setsockopt(C->data_socket,
                             IPPROTO_TCP,
                             SO_SNDBUF,
                   (char *) &dwVal,
                             sizeof(dwVal));

                  break;
                  }

               if (WSAGetLastError() != WSAEWOULDBLOCK)
                  {
                  C->state = UNCONNECTED;
                  break;
                  }

               Sleep(10);
               }

            if (C->state == UNCONNECTED)
               {
               break;
               }

            //
            // Open the receiver stream
            //
            // If the codec needs to inspect the stream data to
            // configure itself, this call will block in the RECV_stream_CB()
            // handler until the requested amount of data (typically only
            // the first few bytes of the source stream) is received from the
            // client.
            //

            C->RECV_read_cursor = 0;
            C->RECV_write_cursor = 0;

            RECV->stream = RECV->ASI_stream_open((U32) C,
                                                       RECV_stream_CB,
                                                       0);

            if (RECV->stream == NULL)
               {
               //
               // Client bailed out before finishing the logon, cancel
               //

               C->state = UNCONNECTED;
               break;
               }

            //
            // Request codec output rate which matches hardware rate
            //

            U32 req_rate = HW_RATE;

            RECV->ASI_stream_set_preference(RECV->stream,
                                            RECV->REQUESTED_RATE,
                                           &req_rate);

            //
            // Stream is now open -- get its attributes and set output
            // sample attributes accordingly
            //

            U32 nch  = RECV->ASI_stream_attribute(RECV->stream, RECV->OUTPUT_CHANNELS);
            U32 rate = RECV->ASI_stream_attribute(RECV->stream, RECV->OUTPUT_SAMPLE_RATE);
            U32 bits = RECV->ASI_stream_attribute(RECV->stream, RECV->OUTPUT_BITS);

            wsprintf(buf,"Client receive stream format: %d channels, %d Hz, %d bits",nch,rate,bits);
            show_string(buf);

            //
            // Open the transmitter stream
            //

            C->XMIT_read_cursor = 0;
            C->XMIT_write_cursor = 0;

            XMIT->stream = XMIT->ASI_stream_open((U32) C,
                                                       XMIT_stream_CB,
                                                       0);

            if (XMIT->stream == NULL)
               {
               //
               // Client bailed out before finishing the logon, cancel
               //

               C->state = UNCONNECTED;
               break;
               }

            //
            // Request codec output rate which matches hardware rate
            //

            req_rate = HW_RATE;

            XMIT->ASI_stream_set_preference(XMIT->stream,
                                            XMIT->REQUESTED_RATE,
                                           &req_rate);
            C->state = ACTIVE_ASI;
            break;
            }

         //
         // ACTIVE_ASI: Call ASI decoder to fetch data for output buffers
         // This will block in the ASI callback 
         // 

         case ACTIVE_ASI:
            {
            //
            // Skip if no room in client's receiver buffer
            //

            C8 buffer[2048];

            if (C->receive->space_available() < sizeof(buffer))
               {
               break;
               }

            //
            // Get data from ASI decoder
            // This will cause the current thread to block in RECV_stream_CB()
            //

            S32 amount = C->RECV.ASI_stream_process(C->RECV.stream, 
                                                    buffer,
                                                    sizeof(buffer));

            if (amount != sizeof(buffer))
               {
               //
               // Bad read -- client must have disconnected, or keyboard
               // interrupt received
               //

               C->state = UNCONNECTED;
               break;
               }

            //
            // Data was received, add to client buffer
            //

            C->receive->put_data(buffer,
                                 amount);
            break;
            }
         }
      }
}

// ---------------------------------------------------------------------------
// CLIENT_init_list
// ---------------------------------------------------------------------------

void CLIENT_init_list(void)
{
   S32 i;

   for (i=0; i < N_CLIENTS; i++)
      {
      CLIENT *C = &clients[i];

      memset(C, 
             0, 
             sizeof(CLIENT));

      C->in_use    = 0;
      C->state     = UNCONNECTED;

      //
      // Create FIFO objects
      //

      C->receive  = new CQueue <C8,RECEIVE_QUEUE_SIZE>;
      C->transmit = new CQueue <C8,TRANSMIT_QUEUE_SIZE>;
      }
}

// ---------------------------------------------------------------------------
// CLIENT_cleanup
// ---------------------------------------------------------------------------

void CLIENT_cleanup(CLIENT *C)
{
   //
   // Kill I/O threads for this client
   //

   C->terminate_request = 1;

   if (C->receive_thread != NULL)
      {
      WaitForSingleObject(C->receive_thread,
                          INFINITE);

      CloseHandle(C->receive_thread);
      C->receive_thread = NULL;
      }

   if (C->transmit_thread != NULL)
      {
      WaitForSingleObject(C->transmit_thread, 
                          INFINITE);

      CloseHandle(C->transmit_thread);
      C->transmit_thread = NULL;
      }

   C->terminate_request = 0;

   //
   // Terminate 
   //

   if (C->data_socket != NULL)
      {
      closesocket(C->data_socket);
      C->data_socket = NULL;
      }

   if (C->hs_socket != NULL)
      {
      closesocket(C->hs_socket);
      C->hs_socket = NULL;
      }

   //
   // Allow client structure to be reused
   //

   C->in_use = 0;
}

// ---------------------------------------------------------------------------
// CLIENT_shutdown_list
// ---------------------------------------------------------------------------

void CLIENT_shutdown_list(void)
{
   S32 i;

   for (i=0; i < N_CLIENTS; i++)
      {
      CLIENT *C = &clients[i];

      CLIENT_cleanup(C);

      delete C->receive;
      delete C->transmit;
      }
}

// ---------------------------------------------------------------------------
// CLIENT_connect
// ---------------------------------------------------------------------------

S32 CLIENT_connect(sockaddr_in *who, SOCKET requestor)
{
   // 
   // Find an available client slot
   //

   S32 i;

   CLIENT *C;

   for (i=0; i < N_CLIENTS; i++)
      {
      C = &clients[i];

      if (!C->in_use)
         {
         break;
         }
      }

   //
   // If no slots available, close the connection and exit
   //

   if (i == N_CLIENTS)
      {
      closesocket(requestor);
      return 0;
      }

   //
   // Assign socket to client as "handshaking" socket, and mark it
   // non-blocking
   //

   C->hs_socket = requestor;
   C->address = *who;

   U32 dwVal = 1;

   ioctlsocket(C->hs_socket,
               FIONBIO,
              &dwVal);

   //
   // Init stream maintenance threads
   //

   C->receive_thread  = NULL;
   C->transmit_thread = NULL;

   //
   // Init FIFOs
   //

   C->receive->reset();
   C->transmit->reset();

   //
   // Start thread to maintain connection state and service reception of
   // compressed data
   //

   C->triggered = 0;

   C->state = WAITING_FOR_DATA_ID;

   DWORD stupId;

   C->receive_thread = (HANDLE) _beginthreadex(NULL,
                                               0,
                                               CLIENT_receive_thread,
                                       (PVOID) C,
                                               0,
                             (unsigned int *) &stupId);

   //
   // Wait for data ID handshaking to complete (otherwise, the monitor thread
   // may intercept the data socket connection request before the client
   // thread does)
   //

   while (C->state == WAITING_FOR_DATA_ID)
      {
      Sleep(10);
      }

   if (C->state != ACTIVE_ASI)
      {
      return 0;
      }

   //
   // Start thread to maintain outgoing stream of mixed, compressed data
   //

   C->transmit_thread = (HANDLE) _beginthreadex(NULL,
                                                0,
                                                CLIENT_transmit_thread,
                                        (PVOID) C,
                                                0,
                              (unsigned int *) &stupId);

   //
   // Mark client structure in use, and return success
   //

   C->in_use = 1;

   return 1;
}

// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------

void __cdecl shutdown(void)
{
   if (dig != NULL)
      {
      AIL_waveOutClose(dig);
      dig = NULL;
      }

   AIL_shutdown();

   if (monitor_socket != NULL)
      {
      closesocket(monitor_socket);
      monitor_socket = NULL;
      }

   WSACleanup();   

   //
   // Kill any WINOLDAPP residuals, so console app can be launched from
   // another program without eventually filling up the task list
   //

   HWND scan = GetTopWindow(0);

   while (scan)
      {
      C8 string[256];

      string[0] = 0;

      S32 result = GetWindowText(scan, string, sizeof(string));

      if (result)
         {
         if (strstr(string, "Finished -") != NULL)
            {
            PostMessage(scan, WM_CLOSE, 0, 0);
            }
         }

      scan = GetWindow(scan, GW_HWNDNEXT);
      }
}

// ---------------------------------------------------------------------------
// MONITOR_thread_procedure
// ---------------------------------------------------------------------------

unsigned int WINAPI MONITOR_thread_procedure(LPVOID pParam)
{
   char buf[128];

   //
   // Loop until thread terminated
   //

   while (1)
      {
      //
      // Exit if thread killed by main thread
      //

      if (!monitor_active)
         {
         return 0;
         }

      // --------------------------------------
      // Check for new connection request
      // --------------------------------------

      SOCKET      requestor;
      sockaddr_in request_addr;
      int         request_addr_size;

      memset(&request_addr,0,sizeof(request_addr));
      request_addr_size = sizeof(request_addr);

      requestor = accept(monitor_socket,
           (sockaddr *) &request_addr,
                        &request_addr_size);

      if (requestor != INVALID_SOCKET)
         {
         //
         // Connection request received -- add connection to client list
         // and continue polling
         //

         S32 result = CLIENT_connect(&request_addr, requestor);

         if (result)
            {
            wsprintf(buf,"Client %s:%d connection active",
                  inet_ntoa(request_addr.sin_addr),
                  ntohs    (request_addr.sin_port));
            }
         else
            {
            wsprintf(buf,"Client %s:%d connection refused",
                  inet_ntoa(request_addr.sin_addr),
                  ntohs    (request_addr.sin_port));
            }
           show_string(buf);
         }
      else
         {
         if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
            wsprintf(buf,"Received error %d",WSAGetLastError());
            show_string(buf);
            }
         }

      // --------------------------------------
      // Wait 10 milliseconds before checking again
      // --------------------------------------

      Sleep(10);
      }
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
                "Miles Sound System Voice Chat Server - Version " MSS_VERSION
                "\n\nFor questions or comments, please contact RAD Game Tools at:\n\n"
                "\tRAD Game Tools\n"
                "\t335 Park Place - Suite G109\n"
                "\tKirkland, WA  98033\n"
                 "\t425-893-4300\n"
                "\tFAX: 425-893-9111\n\n"
                "\tWeb: http://www.radgametools.com\n"
                "\tE-mail: sales@radgametools.com",
                 "About the Miles Sound System Voice Chat Server...", MB_OK|MB_ICONSTOP);
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

static void set_server_ip(HWND hwnd)
{
   char buf[255];

   if (gethostname(buf,255)==SOCKET_ERROR)
     strcpy(buf,"Couldn't detect this machine's IP.");
   else
   {
     LPHOSTENT lphp;
     struct in_addr inaddrIP;
     lphp=gethostbyname(buf);
     inaddrIP=*(struct in_addr*)(lphp->h_addr);
     wsprintf(buf,"This machine's IP: %s",inet_ntoa(inaddrIP));
   }
   SetWindowText(GetDlgItem(hwnd,500),buf);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
   MSG      msg;
   WNDCLASS wndclass;
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

   InitCommonControls();

   hwnd = CreateDialog(hInstance,szAppName,0,NULL);

   if (hwnd==0) {
     MessageBox(0,"Couldn't create dialog box.","Error",MB_OK|MB_ICONSTOP);
     return(0);
   }

   status=GetDlgItem(hwnd,200);
   hear=GetDlgItem(hwnd,300);

   //
   // Start MSS and WinSock
   //

   WSADATA wsadata;
   WORD    wVer = MAKEWORD(1,1);

   WSAStartup(wVer, &wsadata);

   wsprintf(buf,"Started Winsock v%X",wsadata.wVersion);
   show_string(buf);

   AIL_set_redist_directory("..\\..\\redist\\" MSS_REDIST_DIR_NAME);
   AIL_startup();
   atexit(shutdown);

   set_server_ip(hwnd);

   ShowWindow(hwnd, nCmdShow);

   //
   // Initialize client list
   //

   CLIENT_init_list();

   //
   // Create monitor socket and mark it non-blocking
   //

   monitor_socket = socket(PF_INET, SOCK_STREAM, 0);

   if (monitor_socket == INVALID_SOCKET)
      {
      MessageBox(hwnd,"Bad socket!","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   U32 dwNoBlock = 1;

   ioctlsocket(monitor_socket,
               FIONBIO,
              &dwNoBlock);

   //
   // Assign socket's IP address (for single-homed machine) and port
   //

   sockaddr_in monitor_addr;

   memset(&monitor_addr,0,sizeof(monitor_addr));

   monitor_addr.sin_family      = PF_INET;
   monitor_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   monitor_addr.sin_port        = htons(MONITOR_PORT);

   S32 result = bind(monitor_socket,
       (sockaddr *) &monitor_addr,
                     sizeof(monitor_addr));

   if (result)
      {
      MessageBox(hwnd,"Bad bind (is the server already running?)!","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Set up to listen for connection requests from clients
   //

   result = listen(monitor_socket,
                   5);

   if (result)
      {
      MessageBox(hwnd,"Bad listen!","Error",MB_OK|MB_ICONSTOP);
      return(0);
      }

   //
   // Start thread to watch for incoming connection requests...
   //

   monitor_active = 1;

   DWORD stupId;

   HANDLE monitor_thread = (HANDLE) _beginthreadex(NULL,
                                                   0,
                                                   MONITOR_thread_procedure,
                                                   NULL,
                                                   0,
                                 (unsigned int *) &stupId);

   //
   // Watch for incoming data from clients, and play it
   //

   U32 last_interval_us_count = AIL_us_count();

   S32 extra_cnt = 0;

   while (1)
      {
      S32 i;

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
      // Give other threads a chance to run
      //

      Sleep(10);

      // ------------------------------------------------------------
      // Check for disconnections
      // ------------------------------------------------------------

      for (i=0; i < N_CLIENTS; i++)
         {
         CLIENT *C = &clients[i];

         //
         // Handle client disconnections
         //

         if ((C->in_use) && (C->state == UNCONNECTED))
            {
            wsprintf(buf,"Client %s disconnected",CLIENT_address(C));
            show_string(buf);

            CLIENT_cleanup(C);
            }
         }

      // ------------------------------------------------------------
      // See if it is time for a new mixing interval
      // ------------------------------------------------------------

      U32 start_us = AIL_us_count();

      S32 elapsed_us = start_us - last_interval_us_count;

      S32 elapsed_ms;

      if (elapsed_us < 0)
         {
         elapsed_ms = ((start_us + (0xffffffff - last_interval_us_count)) + 500) / 1000;
         }
      else
         {
         elapsed_ms = (elapsed_us + 500) / 1000;
         }

      if (elapsed_ms < CLIENT_MIX_PERIOD)
         {
         continue;
         }

      last_interval_us_count = start_us;

      //
      // Get amount of data to fetch from (and send to) each client, assuming
      // 16-bit audio
      //

      S32 samples = S32((F32(HW_RATE) * F32(elapsed_ms)) / 1000.0F);

      if (samples > CLIENT_MIX_BUFFER_SIZE)
         {
         samples = CLIENT_MIX_BUFFER_SIZE;
         }

      S32 bytes = (samples * sizeof(S16));

      // ------------------------------------------------------------
      // If so, acquire data to be mixed from each client
      // ------------------------------------------------------------

      for (i=0; i < N_CLIENTS; i++)
         {
         CLIENT *C = &clients[i];

         C->samples_to_mix = 0;

         //
         // If not actively streaming, skip further foreground processing
         //

         if (!C->in_use)
            {
            continue;
            }

         //
         // If at least CLIENT_LATENCY bytes have been buffered, enable
         // output contributions from this client
         //

         if ((!C->triggered) && (C->receive->data_available() >= CLIENT_LATENCY))
            {
            C->triggered = 1;
            }

         if (!C->triggered)
            {
            continue;
            }

         //
         // Skip starved clients
         //

         if (C->receive->data_available() < bytes)
            {
            continue;
            }

         //
         // Fetch data from client's output FIFO
         //

         C->receive->fetch_data((C8 *) C->fetch_buffer,
                                       bytes);

         C->samples_to_mix = samples;
         }

      // ------------------------------------------------------------
      // Merge data from all active clients into each client's
      // mixer buffer
      // ------------------------------------------------------------

      for (i=0; i < N_CLIENTS; i++)
         {
         CLIENT *C = &clients[i];

         //
         // If not actively streaming, skip further foreground processing
         //

         if (!C->in_use)
            {
            continue;
            }

         //
         // Clear client's mixer buffer to signed-PCM 0 value (0x0000)
         //

         memset(C->mixer_buffer,
                0,
                sizeof(C->mixer_buffer));

         //
         // Add samples from each contributing client's decode buffer to
         // target's mixing buffer
         //

         for (S32 j=0; j < N_CLIENTS; j++)
            {
            CLIENT *SRC = &clients[j];

            //
            // Avoid echoing each client's own traffic back to it, unless
            // we've enabled local echo as a diagnostic option
            //

            if (SendMessage(hear,BM_GETCHECK,0,0)==0)
              if (i == j)
                 {
                 continue;
                 }

            //
            // Skip inactive or newly-logged-on clients that don't have
            // enough data buffered yet
            //

            if (SRC->samples_to_mix == 0)
               {
               continue;
               }

            //
            // Add 16-bit data to 32-bit output buffer
            //

            S16 *src  = SRC->fetch_buffer;
            S32 *dest = C->mixer_buffer;

            for (S32 m=0; m < samples; m++)
               {
               dest[m] += src[m];
               }
            }

         //
         // Clip and copy contents of client's mixer buffer to output buffer
         //

         C8 output_buffer[TRANSMIT_QUEUE_SIZE];

         S32 *src  =         C->mixer_buffer;
         S16 *dest = (S16 *) output_buffer;

         for (S32 m=0; m < samples; m++)
            {
            S32 s = src[m];

            if (s < -32768)
               {
               *dest++ = -32768;
               }
            else if (s > 32767)
               {
               *dest++ = 32767;
               }
            else
               {
               *dest++ = S16(s);
               }
            }

         //
         // Add buffer to transmit queue
         //

         C->transmit->put_data(output_buffer,
                               samples * sizeof(S16));
         }
      }

   //
   // Tell the monitor thread to exit, and wait for it to terminate
   //

   monitor_active = 0;

   WaitForSingleObject(monitor_thread,
                       INFINITE);

   CloseHandle(monitor_thread);

   //
   // Terminate client threads
   //

   CLIENT_shutdown_list();

  return(1);
}
