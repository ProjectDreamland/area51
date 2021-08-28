###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System Miles Player program       #
#                                                             #
#  MSVC 1.52                                                  #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\..\..\src\win\mss16def.mak"



..\milesp16.exe: milesp16.obj milesply.rc ..\..\..\lib\win\mss16.lib
  set lib=$(MSVCDIR)\LIB
  set path=$(MSVCDIR)\BIN
  $(LINK) milesp16,..\milesp16 /align:16,nul,/nod slibcew libw ..\..\..\lib\win\mss16.lib mmsystem commdlg, milesp16
  $(RC) -fomilesp16.res -dMSS_H -I..\..\..\include milesply.rc ..\milesp16.exe


milesp16.obj: milesply.c ..\..\..\include\mss.h
  $(COMPILEEXE) -Fomilesp16.obj -I..\..\..\include milesply.c
