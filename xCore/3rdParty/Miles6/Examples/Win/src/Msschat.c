//############################################################################
//##                                                                        ##
//##  MSSCHAT.C                                                             ##
//##                                                                        ##
//##  MSS Chat example                                                      ##
//##                                                                        ##
//##  V1.00 of 15-Sep-99: Initial release                                   ##
//##                                                                        ##
//##  C source compatible with Microsoft C v9.0 or later                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include <windows.h>
#include <commctrl.h>
#include "mss.h"
#include <string.h>
#include <stdlib.h>
#include <dos.h>

#define STARTSERV   101
#define CONNECT2900 102
#define CONNECT2400 103
#define CONNECT1200 104
#define CLOSE       106
#define ABOUT       107
#define IPSERV      500
#define IPCONNECT   200

char szAppName[]="MSSCHAT";
HWND hwnd;
char* codecs[]={".v29",".v24",".v12"};
char dir[128];
char ipnum[128];

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

//
// function to find only the filename from a complete pathname
//

static char FAR* get_filename(char FAR* pathname)
{
   char FAR* f=pathname+lstrlen(pathname)-1;
   while ((f>=pathname) && (*f!='\\') && (*f!=':')) --f;
   return(f+1);
}


static void call_client(HWND hwnd,S32 index)
{
  char ipn[128];
  char cmd[128];

  GetWindowText(GetDlgItem(hwnd,200),ipn,127);
  Trim(ipn);
  if (*ipn==0)
  {
    MessageBox(hwnd,"Enter an IP address first.","Error",MB_OK|MB_ICONSTOP);
    return;
  }

  wsprintf(cmd,"%sMSSChtC %s %s",dir,ipn,codecs[index]);
  WinExec(cmd,SW_SHOWNORMAL);
}


static void call_server(HWND hwnd)
{
  char cmd[128];

  SetWindowText(GetDlgItem(hwnd,200),ipnum);

  wsprintf(cmd,"%sMSSChtS",dir);
  WinExec(cmd,SW_SHOWNORMAL);
}


//############################################################################
//##                                                                        ##
//## Main window procedure                                                  ##
//##                                                                        ##
//############################################################################

LRESULT WINAPI Window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

           case STARTSERV:
             call_server(hwnd);
             break;

           case CONNECT2900:
           case CONNECT2400:
           case CONNECT1200:
             call_client(hwnd,wParam-CONNECT2900);
             break;

           case ABOUT:
             MessageBox(hwnd,
                "Miles Sound System Voice Chat Example - Version " MSS_VERSION
                "\n\nFor questions or comments, please contact RAD Game Tools at:\n\n"
                "\tRAD Game Tools\n"
                "\t335 Park Place - Suite G109\n"
                "\tKirkland, WA  98033\n"
                 "\t425-893-4300\n"
                "\tFAX: 425-893-9111\n\n"
                "\tWeb: http://www.radgametools.com\n"
                "\tE-mail: sales@radgametools.com",
                 "About the Miles Sound System Voice Chat Example...", MB_OK);
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
     strcpy(ipnum,inet_ntoa(inaddrIP));
     wsprintf(buf,"This machine's IP: %s",ipnum);
   }
   SetWindowText(GetDlgItem(hwnd,IPSERV),buf);
}

//############################################################################
//##                                                                        ##
//## WinMain()                                                              ##
//##                                                                        ##
//############################################################################

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
   MSG      msg;
   WNDCLASS wndclass;
   WSADATA wsadata;
   WORD    wVer = MAKEWORD(1,1);

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

   if (WSAStartup(wVer,&wsadata))
   {
     MessageBox(0,"Couldn't open Winsock.","Error",MB_OK);
     return(0);
   }

   InitCommonControls();

   hwnd = CreateDialog(hInstance,szAppName,0,NULL);

   if (hwnd==0) {
     MessageBox(0,"Couldn't create dialog box.","Error",MB_OK);
     return(0);
   }

   // get the chat directory
   GetModuleFileName(0,dir,127);
   *(get_filename(dir))=0;

   //
   // Initialize the Miles Sound System
   //

   set_server_ip(hwnd);

   //
   // Main message loop
   //

   ShowWindow(hwnd,nCmdShow);

   while (GetMessage(&msg, 0, 0, 0)) {

     if (!IsDialogMessage(hwnd,&msg)) {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
     }

   }

   WSACleanup();

   return msg.wParam;
}

