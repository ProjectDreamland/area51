//############################################################################
//##                                                                        ##
//##  QUICK.C                                                               ##
//##                                                                        ##
//##  MSS 3.5 Quick API example program                                     ##
//##                                                                        ##
//##  V1.00 of 18-Mar-96: Initial version                                   ##
//##  V1.01 of 11-Nov-00: Cleaned up version                                ##
//##                                                                        ##
//##   Author: Jeff Roberts                                                 ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include <windows.h>
#include "mss.h"


#define SOUNDBUTTON 0x0080
#define CLOSEBUTTON 0x0081

static char szAppName[] = "QUICK";

static HWND Window=0;

static HAUDIO Audio=0;
static S32 Which=0;


//############################################################################
//##                                                                        ##
//## Handle a button being clicked                                          ##
//##                                                                        ##
//############################################################################

static void Handle_button_click( HWND wnd, U32 button )
{
  switch (button)
  {
    case SOUNDBUTTON:
      //
      // do we already have an audio handle? if so, close it
      //
      if (Audio)                  // if audio is playingoing, hose it
      {
        AIL_quick_unload( Audio );
      }
      Audio = AIL_quick_load_and_play( (Which^=1)?
                                        "..\\MEDIA\\GLASS.WAV":
                                        "..\\MEDIA\\DEMO.XMI", 1, 0 );
      break;

   case CLOSEBUTTON:
   case IDCANCEL:
     DestroyWindow( wnd );
     break;
 }
}

//############################################################################
//##                                                                        ##
//## Main window procedure                                                  ##
//##                                                                        ##
//############################################################################

LRESULT AILEXPORT Window_proc( HWND wnd,
                               UINT message,
                               WPARAM wparam,
                               LPARAM lparam )
{
  switch (message)
  {
    case WM_COMMAND:
      Handle_button_click( wnd, LOWORD( wparam ) );
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_ENDSESSION:   // for 16-bit windows
      AIL_shutdown();
      break;

   }

   return DefWindowProc( wnd, message, wparam, lparam );
}



//############################################################################
//##                                                                        ##
//## Create the application window                                          ##
//##                                                                        ##
//############################################################################


static HWND Create_window( HINSTANCE inst, HINSTANCE previnst )
{

  //
  // if this is the first instance (always true under Win32), create the class
  //

  if (!previnst)
  {
    WNDCLASS wndclass;

    wndclass.lpszClassName = szAppName;
    wndclass.lpfnWndProc   = (WNDPROC) Window_proc;
    wndclass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndclass.hInstance     = inst;
    wndclass.hIcon         = LoadIcon( inst, szAppName );
    wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );

#ifdef _WIN32
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
#else
    wndclass.hbrBackground = GetStockObject( LTGRAY_BRUSH );
#endif

    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = DLGWINDOWEXTRA;
    wndclass.lpszMenuName  = NULL;

    RegisterClass( &wndclass );
  }

  //
  // now attempt to create the window
  //

  return( CreateDialog( inst, szAppName, 0, NULL ) );
}


//############################################################################
//##                                                                        ##
//## WinMain()                                                              ##
//##                                                                        ##
//############################################################################

int PASCAL WinMain( HINSTANCE instance,
                    HINSTANCE previnstance,
                    LPSTR cmdline,
                    int cmdshow )
{
   MSG      msg;
   
   //
   // Create the main window
   //

   Window=Create_window( instance, previnstance );
   if (!Window)
   {
     MessageBox( 0, "Unable to create window.", "Error", MB_OK|MB_ICONSTOP );
     return( 0 );
   }

   //
   // Main message loop
   //

   ShowWindow( Window, cmdshow );

   //
   // Try to open MSS, if it fails, close the Window
   //

   AIL_set_redist_directory("..\\..\\redist\\" MSS_REDIST_DIR_NAME);

   if (AIL_quick_startup(1,1,44100,8,1) == 0) // open digital and MIDI
   {
     MessageBox( Window, AIL_last_error(), "Couldn't open MSS.", MB_OK );
     DestroyWindow( Window );
   }

   while ( GetMessage( &msg, 0, 0, 0 ) ) 
   {
     if (!IsDialogMessage( Window, &msg ))
     {
       TranslateMessage( &msg );
       DispatchMessage( &msg );
     }
   }

   //
   // shutdown Miles
   //

   AIL_quick_shutdown();

   return msg.wParam;
}

