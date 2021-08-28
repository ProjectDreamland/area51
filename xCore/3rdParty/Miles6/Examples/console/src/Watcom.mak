###############################################################
#                                                             #
#  MAKEFILE for MSS development                               #
#  6-Jun-96 Jeff Roberts                                      #
#                                                             #
#  Watcom                                                     #
#                                                             #
#  Execute with wmake                                         #
#                                                             #
###############################################################


DEBUG=0
REGISTER=1
CAUSEWAY=1

CAUSEWAYPATH=\devel\libs\cause\

!ifneq REGISTER 0
CALL=r
!else
CALL=s
!endif

!ifneq CAUSEWAY 0
DPMITYPE=DPMI
BIND1=$(CAUSEWAYPATH)le23p
BIND2=$(CAUSEWAYPATH)cwc
!else
BIND1=rem
BIND2=rem
DPMITYPE=DPMI
!endif

!ifneq DEBUG 0
CMP=wcc386 /w9 /s /zq /we /zp1 /zm /bt=dos /d2 /D$(DPMITYPE) /5$(CALL) /I..\..\..\include -fpi87 -fp5
LINK=wlink op q op el d all system dos4g
!else
CMP=wcc386 /w9 /s /zq /we /zp1 /zm /bt=dos /D$(DPMITYPE) /5$(CALL) /otexan /I..\..\..\include -fpi87 -fp5
LINK=wlink op q op el system dos4g
!endif




all: ..\sstest.exe ..\digplay.exe ..\xmiplay.exe ..\play.exe ..\audiocd.exe  ..\stream.exe ..\dlsplay.exe

..\sstest.exe: sstest.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\sstest f sstest l ..\..\..\lib\dos\msswat.lib
  $(bind1) ..\sstest
  $(bind2) ..\sstest

..\digplay.exe: digplay.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\digplay f digplay l ..\..\..\lib\dos\msswat.lib
  $(bind1) ..\digplay
  $(bind2) ..\digplay

..\xmiplay.exe: xmiplay.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\xmiplay f xmiplay l ..\..\..\lib\dos\msswatnm.lib
  $(bind1) ..\xmiplay
  $(bind2) ..\xmiplay

..\play.exe: play.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\play f play l ..\..\..\lib\dos\msswat.lib
  $(bind1) ..\play
  $(bind2) ..\play

..\dlsplay.exe: dlsplay.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\dlsplay f dlsplay l ..\..\..\lib\dos\msswat.lib
  $(bind1) ..\dlsplay
  $(bind2) ..\dlsplay

..\audiocd.exe: audiocd.obj ..\..\..\lib\dos\msswatnm.lib
  $(link) n ..\audiocd f audiocd l ..\..\..\lib\dos\msswatnm.lib
  $(bind1) ..\audiocd
  $(bind2) ..\audiocd

..\stream.exe: stream.obj ..\..\..\lib\dos\msswat.lib
  $(link) n ..\stream f stream l ..\..\..\lib\dos\msswat.lib
  $(bind1) ..\stream
  $(bind2) ..\stream


sstest.obj: sstest.c ..\..\..\include\mss.h
  $(cmp) sstest.c

digplay.obj: digplay.c ..\..\..\include\mss.h
  $(cmp) digplay.c

xmiplay.obj: xmiplay.c ..\..\..\include\mss.h
  $(cmp) xmiplay.c

play.obj: play.c ..\..\..\include\mss.h
  $(cmp) play.c

dlsplay.obj: dlsplay.c ..\..\..\include\mss.h
  $(cmp) dlsplay.c

audiocd.obj: audiocd.c ..\..\..\include\mss.h
  $(cmp) audiocd.c

stream.obj: stream.c ..\..\..\include\mss.h
  $(cmp) stream.c

