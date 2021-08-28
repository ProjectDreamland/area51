###############################################################
#                                                             #
#  MAKEFILE for mss development                               #
#                                                             #
#  MSC 7.0 16-bit version                                     #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\win\mss16def.mak"

..\..\redist\win16\mp3dec.as6: mp3d16.def mp3api6.obj math_a16.obj math_i16.obj mp3dec6.obj ..\..\lib\win\mss16.lib
   $(LINK) /PACKC:60000 @mp3d16.rsp
   $(RC) -fomp3d16.res -dMSS_H ..\win\mss.rc mp3dec6.dll
   copy /b mp3dec6.dll ..\..\redist\win16\mp3dec.as6
   del mp3dec6.dll

#
# MPEG decoder files
#

mp3api6.obj: mp3dec\mp3api.cpp mp3dec\mp3dec.h mp3dec\datatbl.h ..\..\include\mss.h
   $(COMPILE) -Fomp3api6 mp3dec\mp3api.cpp

mp3dec6.obj: mp3dec\mp3dec.cpp mp3dec\mp3dec.h mp3dec\datatbl.h ..\..\include\mss.h
   $(COMPILE) -Fomp3dec6 mp3dec\mp3dec.cpp

math_i16.obj: mp3dec\math_16.asm
   $(ASSEMBLE) -I..\win -Fomath_i16.obj mp3dec\math_16.asm

math_a16.obj: mp3dec\math_16.asm
   $(ASSEMBLE) -I..\win -Fomath_a16.obj -DAMD mp3dec\math_16.asm
