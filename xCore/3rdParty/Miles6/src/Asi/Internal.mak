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

all: ..\..\tools\win\mp3enc.asi ..\..\redist\win32\mssv29.asi ..\..\redist\win32\mssv24.asi ..\..\redist\win32\mssv12.asi

..\..\tools\win\mp3enc.asi: ..\..\lib\win\mss32.lib mp3enc.obj musicin.obj common.obj encode.obj huffman.obj ieee.obj l3bs.obj portable.obj l3psy.obj loop.obj mdct.obj reserv.obj psy.obj subs.obj tonal.obj format.obj
  $(LINKDLL) -base:0x26100000 -out:..\..\tools\win\mp3enc.asi mp3enc.obj common.obj encode.obj huffman.obj ieee.obj l3bs.obj portable.obj l3psy.obj loop.obj mdct.obj reserv.obj psy.obj subs.obj tonal.obj format.obj musicin.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res


HDRS=mp3enc\common.h mp3enc\encoder.h mp3enc\l3bsp.h mp3enc\ieee.h mp3enc\huffman.h mp3enc\l3bs.h mp3enc\portable.h mp3enc\reserv.h mp3enc\l3psy.h mp3enc\l3side.h mp3enc\loop-pvt.h mp3enc\loop.h mp3enc\mdct.h mp3enc\format.h mp3enc\tables.h mssasi.h

MP3OPTS=/DBS_FORMAT=BINARY /DMSC60 /DINTERNAL_TABLES

mp3enc.obj: mp3enc\mp3enc.cpp $(HDRS)
  $(COMPILE) mp3enc\mp3enc.cpp -DBUILD_MSS $(MP3OPTS)

musicin.obj: mp3enc\musicin.c $(HDRS)
  $(COMPILE) mp3enc\musicin.c -DBUILD_MSS $(MP3OPTS)

common.obj: mp3enc\common.c $(HDRS)
  $(COMPILE) mp3enc\common.c -DBUILD_MSS $(MP3OPTS)

encode.obj: mp3enc\encode.c $(HDRS)
  $(COMPILE) mp3enc\encode.c -DBUILD_MSS $(MP3OPTS)

huffman.obj: mp3enc\huffman.c $(HDRS)
  $(COMPILE) mp3enc\huffman.c -DBUILD_MSS $(MP3OPTS)

ieee.obj: mp3enc\ieee.c $(HDRS)
  $(COMPILE) mp3enc\ieee.c -DBUILD_MSS $(MP3OPTS)

l3bs.obj: mp3enc\l3bs.c $(HDRS)
  $(COMPILE) mp3enc\l3bs.c -DBUILD_MSS $(MP3OPTS)

portable.obj: mp3enc\portable.c $(HDRS)
  $(COMPILE) mp3enc\portable.c -DBUILD_MSS $(MP3OPTS)

l3psy.obj: mp3enc\l3psy.c $(HDRS)
  $(COMPILE) mp3enc\l3psy.c -DBUILD_MSS $(MP3OPTS)

loop.obj: mp3enc\loop.c $(HDRS)
  $(COMPILE) mp3enc\loop.c -DBUILD_MSS $(MP3OPTS)

mdct.obj: mp3enc\mdct.c $(HDRS)
  $(COMPILE) mp3enc\mdct.c -DBUILD_MSS $(MP3OPTS)

reserv.obj: mp3enc\reserv.c $(HDRS)
  $(COMPILE) mp3enc\reserv.c -DBUILD_MSS $(MP3OPTS)

psy.obj: mp3enc\psy.c $(HDRS)
  $(COMPILE) mp3enc\psy.c -DBUILD_MSS $(MP3OPTS)

subs.obj: mp3enc\subs.c $(HDRS)
  $(COMPILE) mp3enc\subs.c -DBUILD_MSS $(MP3OPTS)

tonal.obj: mp3enc\tonal.c $(HDRS)
  $(COMPILE) mp3enc\tonal.c -DBUILD_MSS $(MP3OPTS)

format.obj: mp3enc\format.c $(HDRS)
  $(COMPILE) mp3enc\format.c -DBUILD_MSS $(MP3OPTS)

#
# voice codec
#

..\..\redist\win32\mssv29.asi: ..\..\lib\win\mss32.lib v29api.obj mss32.res
  $(LINKDLL) -base:0x26400000 -out:..\..\redist\win32\mssv29.asi v29api.obj libc.lib voice\voxware\rt.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res

v29api.obj: voice\voice.cpp ..\..\include\mss.h mssasi.h ..\shared\rib.h ..\shared\imssapi.h
  $(COMPILE) voice\voice.cpp -Fov29api.obj -Ivoice\voxware -DBUILD_MSS -DV29

..\..\redist\win32\mssv24.asi: ..\..\lib\win\mss32.lib v24api.obj mss32.res
  $(LINKDLL) -base:0x26500000 -out:..\..\redist\win32\mssv24.asi v24api.obj libc.lib voice\voxware\rt.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res

v24api.obj: voice\voice.cpp ..\..\include\mss.h mssasi.h ..\shared\rib.h ..\shared\imssapi.h
  $(COMPILE) voice\voice.cpp -Fov24api.obj -Ivoice\voxware -DBUILD_MSS -DV24

..\..\redist\win32\mssv12.asi: ..\..\lib\win\mss32.lib v12api.obj mss32.res
  $(LINKDLL) -base:0x26600000 -out:..\..\redist\win32\mssv12.asi v12api.obj libc.lib voice\voxware\rt.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res

v12api.obj: voice\voice.cpp ..\..\include\mss.h mssasi.h ..\shared\rib.h ..\shared\imssapi.h
  $(COMPILE) voice\voice.cpp -Fov12api.obj -Ivoice\voxware -DBUILD_MSS -DV12

