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


all: ..\sstest.exe ..\digplay.exe ..\xmiplay.exe ..\play.exe ..\audiocd.exe ..\stream.exe ..\dlsplay.exe  ..\filter.exe ..\asifile.exe ..\con3d.exe

..\sstest.exe: sstest.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\sstest.exe sstest ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

sstest.obj: sstest.c ..\..\..\include\mss.h
  $(COMPILE) sstest.c -I..\..\..\include

..\digplay.exe: digplay.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\digplay.exe digplay ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

digplay.obj: digplay.c ..\..\..\include\mss.h
  $(COMPILE) digplay.c -I..\..\..\include

..\xmiplay.exe: xmiplay.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\xmiplay.exe xmiplay ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

xmiplay.obj: xmiplay.c ..\..\..\include\mss.h
  $(COMPILE) xmiplay.c -I..\..\..\include

..\play.exe: play.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\play.exe play ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

play.obj: play.c ..\..\..\include\mss.h
  $(COMPILE) play.c -I..\..\..\include

..\audiocd.exe: audiocd.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\audiocd.exe audiocd ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

audiocd.obj: audiocd.c ..\..\..\include\mss.h
  $(COMPILE) audiocd.c -I..\..\..\include

..\stream.exe: stream.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\stream.exe stream ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

stream.obj: stream.c ..\..\..\include\mss.h
  $(COMPILE) stream.c -I..\..\..\include

..\dlsplay.exe: dlsplay.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\dlsplay.exe dlsplay ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

dlsplay.obj: dlsplay.c ..\..\..\include\mss.h
  $(COMPILE) dlsplay.c -I..\..\..\include

..\filter.exe: filter.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\filter.exe filter ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

filter.obj: filter.cpp ..\..\..\include\mss.h
  $(COMPILE) filter.cpp -I..\..\..\include

..\asifile.exe: asifile.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\asifile.exe asifile ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

asifile.obj: asifile.cpp ..\..\..\include\mss.h
  $(COMPILE) asifile.cpp -I..\..\..\include

..\con3d.exe: con3d.obj ..\..\..\lib\win\mss32.lib
  $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE /OUT:..\con3d.exe con3d ..\..\..\lib\win\mss32.lib libc.lib kernel32.lib user32.lib winmm.lib gdi32.lib oldnames.lib

con3d.obj: con3d.cpp ..\..\..\include\mss.h
  $(COMPILE) con3d.cpp -I..\..\..\include

