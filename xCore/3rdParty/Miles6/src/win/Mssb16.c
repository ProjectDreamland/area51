//############################################################################
//##                                                                        ##
//##  MSSB16.C                                                              ##
//##                                                                        ##
//##  MSS 3.5 Background processor for Win16                                ##
//##                                                                        ##
//##  V1.00 of 18-Mar-96: Initial version                                   ##
//##                                                                        ##
//##  C source compatible with Microsoft C v9.0 or later                    ##
//##                                                                        ##
//##   Author: Jeff Roberts                                                 ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################
                                                                         
#include <windows.h>
#include <stdlib.h>

//############################################################################
//##                                                                        ##
//## WinMain() - Calls into the 16-bit DLL                                  ##
//##                                                                        ##
//############################################################################

typedef void (WINAPI FAR* func) ();

int PASCAL FAR WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
   func f;
   LPSTR c=lpszCmdLine;
   if (*c=='!') {
     f=(func)atol(c+1);
     if (IsBadCodePtr((FARPROC)f)) 
       MessageBox(0,"Bad MSS code pointer.","Error",MB_OK);
     else
       (*f)();
   }
   return(0);
}

