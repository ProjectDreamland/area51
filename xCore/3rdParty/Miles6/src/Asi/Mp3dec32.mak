###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System Demo32 example program     #
#                                                             #
#  MSVC 4.0                                                   #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\win\mss32def.mak"

#
# MPEG Layer 3 decoder
#

..\..\redist\win32\mp3dec.asi: ..\..\lib\win\mss32.lib mp3api.obj mp3dec.obj math_x86.obj math_amd.obj mss32.res
  $(LINKDLL) -base:0x26f00000 -out:..\..\redist\win32\mp3dec.asi mp3api.obj mp3dec.obj math_x86.obj math_amd.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res

mp3api.obj: mp3dec\mp3api.cpp ..\..\include\mss.h mssasi.h ..\shared\rib.h ..\shared\imssapi.h
  $(COMPILE) mp3dec\mp3api.cpp -DBUILD_MSS

mp3dec.obj: mp3dec\mp3dec.cpp ..\..\include\mss.h mssasi.h ..\shared\rib.h ..\shared\imssapi.h
  $(COMPILE) mp3dec\mp3dec.cpp -DBUILD_MSS

math_amd.obj: mp3dec\math_a.asm
  $(ASSEMBLE) -DAMD -Fomath_amd.obj mp3dec\math_a.asm

math_x86.obj: mp3dec\math_a.asm
  $(ASSEMBLE) -Fomath_x86.obj mp3dec\math_a.asm

mss32.res: ..\win\mss.rc ..\..\include\mss.h
  $(RC) -fomss32.res ..\win\mss.rc


