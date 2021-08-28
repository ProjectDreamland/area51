###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System example programs           #
#                                                             #
#  MSVC 5.0                                                   #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\..\..\src\win\mss32def.mak"


all:  ..\demo32.exe ..\quick32.exe ..\exam3d.exe ..\msschat.exe ..\msschtc.exe ..\msschts.exe

..\demo32.exe: demo32.obj demo32.res ..\..\..\lib\win\mss32.lib
  $(LINK) /FIXED /RELEASE /OUT:..\demo32.exe demo32.obj demo32.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib

demo32.res: demo.rc
  $(RC) -fodemo32.res demo.rc

demo32.obj: demo.c ..\..\..\include\mss.h
  $(COMPILE) demo.c /Fodemo32 /I..\..\..\include

..\quick32.exe: quick32.obj quick32.res ..\..\..\lib\win\mss32.lib
  $(LINK) /FIXED /RELEASE /OUT:..\quick32.exe quick32.obj quick32.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib

quick32.res: quick.rc
  $(RC) -foquick32.res quick.rc

quick32.obj: quick.c ..\..\..\include\mss.h
  $(COMPILE) quick.c /Foquick32 /I..\..\..\include

..\exam3d.exe: exam3d.obj exam3d.res ..\..\..\lib\win\mss32.lib
  $(LINK) /FIXED /RELEASE /OUT:..\exam3d.exe exam3d.obj exam3d.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib comctl32.lib comdlg32.lib

exam3d.res: exam3d.rc
  $(RC) -foexam3d.res -i..\..\..\include exam3d.rc

exam3d.obj: exam3d.c ..\..\..\include\mss.h
  $(COMPILE) exam3d.c /Foexam3d /I..\..\..\include

..\msschat.exe: msschat.obj msschat.res
  $(LINK) /FIXED /RELEASE /OUT:..\msschat.exe msschat.obj msschat.res libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib comctl32.lib comdlg32.lib wsock32.lib

msschat.res: msschat.rc
  $(RC) -fomsschat.res -i..\..\..\include msschat.rc

msschat.obj: msschat.c ..\..\..\include\mss.h
  $(COMPILE) msschat.c /Fomsschat /I..\..\..\include

..\msschtc.exe: msschtc.obj msschtc.res ..\..\..\lib\win\mss32.lib
  $(LINK) /FIXED /RELEASE /OUT:..\msschtc.exe msschtc.obj msschtc.res ..\..\..\lib\win\mss32.lib libcmt.lib kernel32.lib user32.lib winmm.lib gdi32.lib comctl32.lib comdlg32.lib wsock32.lib

msschtc.res: msschtc.rc
  $(RC) -fomsschtc.res -i..\..\..\include msschtc.rc

msschtc.obj: msschtc.cpp ..\..\..\include\mss.h
  $(COMPILE) msschtc.cpp /MT /Fomsschtc /I..\..\..\include

..\msschts.exe: msschts.obj msschts.res ..\..\..\lib\win\mss32.lib
  $(LINK) /FIXED /RELEASE /OUT:..\msschts.exe msschts.obj msschts.res ..\..\..\lib\win\mss32.lib libcmt.lib kernel32.lib user32.lib winmm.lib gdi32.lib comctl32.lib comdlg32.lib wsock32.lib

msschts.res: msschts.rc
  $(RC) -fomsschts.res -i..\..\..\include msschts.rc

msschts.obj: msschts.cpp ..\..\..\include\mss.h
  $(COMPILE) msschts.cpp /MT /Fomsschts /I..\..\..\include

