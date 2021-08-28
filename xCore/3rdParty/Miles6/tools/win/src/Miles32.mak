###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System Sound Player               #
#                                                             #
#  MSVC 5.0                                                   #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\..\..\src\win\mss32def.mak"


all: ..\milesstu.exe ..\midiechw.exe ..\milesp32.exe

milesstu.res: milesstu.rc
  $(RC) -i..\..\..\include -fomilesstu.res milesstu.rc

milesstu.obj: milesstu.c ..\..\..\include\mss.h
  $(COMPILE) milesstu.c -DBUILD_MSS -I..\..\..\include

milesply.obj: milesply.c ..\..\..\include\mss.h
  $(COMPILE) milesply.c -DBUILD_MSS -I..\..\..\include

midiechw.res: midiechw.rc
  $(RC) -i..\..\..\include -I\devel\msvc40\mfc\include -fomidiechw.res midiechw.rc

midiechw.obj: midiechw.cpp ..\..\..\include\mss.h
  $(COMPILE) midiechw.cpp -DBUILD_MSS -I..\..\..\include -I..\..\..\src\shared

milesp32.res: milesply.rc
  $(RC) -i..\..\..\include -fomilesp32.res milesply.rc

..\milesstu.exe: milesstu.obj milesstu.res ..\..\..\lib\win\mss32.lib ..\mss32.dll
  $(LINKEXE) /OUT:..\milesstu.exe milesstu.obj milesstu.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib comdlg32.lib shell32.lib

..\midiechw.exe: midiechw.obj midiechw.res ..\..\..\lib\win\mss32.lib ..\mss32.dll
  $(LINKEXE) /OUT:..\midiechW.exe midiechw.obj midiechw.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib comdlg32.lib dsound.lib

..\milesp32.exe: milesply.obj milesp32.res ..\..\..\lib\win\mss32.lib ..\mss32.dll
  $(LINKEXE) /OUT:..\milesp32.exe milesply.obj milesp32.res ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib comctl32.lib comdlg32.lib

