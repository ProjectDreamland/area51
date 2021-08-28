###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System Demo16 example program     #
#                                                             #
#  MSVC 1.52                                                  #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\..\..\src\win\mss16def.mak"


all: ..\demo16.exe ..\quick16.exe

..\demo16.exe: demo16.obj demo.rc ..\..\..\lib\win\mss16.lib
  $(LINK) demo16,..\demo16 /align:16,nul,/nod slibcew libw ..\..\..\lib\win\mss16.lib mmsystem, demo16
  $(RC) -fodemo16.res demo.rc ..\demo16.exe

demo16.obj: demo.c ..\..\..\include\mss.h
  $(COMPILEEXE) -Fodemo16 -I..\..\..\include demo.c

..\quick16.exe: quick16.obj quick.rc ..\..\..\lib\win\mss16.lib
  $(LINK) quick16,..\quick16 /align:16,nul,/nod slibcew libw ..\..\..\lib\win\mss16.lib mmsystem, quick16
  $(RC) -foquick16.res quick.rc ..\quick16.exe

quick16.obj: quick.c ..\..\..\include\mss.h
  $(COMPILEEXE) -Foquick16 -I..\..\..\include quick.c
