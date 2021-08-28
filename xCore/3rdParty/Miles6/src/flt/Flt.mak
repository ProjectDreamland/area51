###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System pipeline filters           #
#                                                             #
#  MSVC 6.0                                                   #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "..\win\mss32def.mak"

all: ..\..\redist\win32\capture.flt \
     ..\..\redist\win32\lowpass.flt \
     ..\..\redist\win32\highpass.flt \
     ..\..\redist\win32\bandpass.flt \
     ..\..\redist\win32\mdelay.flt \
     ..\..\redist\win32\sdelay.flt \
     ..\..\redist\win32\parmeq.flt \
     ..\..\redist\win32\phaser.flt \
     ..\..\redist\win32\reson.flt \
     ..\..\redist\win32\reverb1.flt \
     ..\..\redist\win32\reverb2.flt \
     ..\..\redist\win32\reverb3.flt \
     ..\..\redist\win32\ringmod.flt \
     ..\..\redist\win32\flange.flt \
     ..\..\redist\win32\chorus.flt \
     ..\..\redist\win32\shelfeq.flt \
     ..\..\redist\win32\compress.flt \
     ..\..\redist\win32\autopan.flt \
     ..\..\redist\win32\laginter.flt \


#
# Filter providers
#

lowpass.obj: lowpass.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) lowpass.cpp -DBUILD_MSS

..\..\redist\win32\lowpass.flt: lowpass.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24100000 -out:..\..\redist\win32\lowpass.flt lowpass.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


highpass.obj: highpass.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) highpass.cpp -DBUILD_MSS

..\..\redist\win32\highpass.flt: highpass.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24200000 -out:..\..\redist\win32\highpass.flt highpass.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


bandpass.obj: bandpass.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) bandpass.cpp -DBUILD_MSS

..\..\redist\win32\bandpass.flt: bandpass.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24300000 -out:..\..\redist\win32\bandpass.flt bandpass.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


reverb1.obj: reverb1.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) reverb1.cpp -DBUILD_MSS

..\..\redist\win32\reverb1.flt: reverb1.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24400000 -out:..\..\redist\win32\reverb1.flt reverb1.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


reverb2.obj: reverb2.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) reverb2.cpp -DBUILD_MSS

..\..\redist\win32\reverb2.flt: reverb2.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24500000 -out:..\..\redist\win32\reverb2.flt reverb2.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


reverb3.obj: reverb3.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) reverb3.cpp -DBUILD_MSS

..\..\redist\win32\reverb3.flt: reverb3.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24600000 -out:..\..\redist\win32\reverb3.flt reverb3.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


reson.obj: reson.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) reson.cpp -DBUILD_MSS

..\..\redist\win32\reson.flt: reson.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24700000 -out:..\..\redist\win32\reson.flt reson.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


phaser.obj: phaser.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) phaser.cpp -DBUILD_MSS

..\..\redist\win32\phaser.flt: phaser.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24800000 -out:..\..\redist\win32\phaser.flt phaser.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


parmeq.obj: parmeq.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) parmeq.cpp -DBUILD_MSS

..\..\redist\win32\parmeq.flt: parmeq.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24900000 -out:..\..\redist\win32\parmeq.flt parmeq.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


mdelay.obj: mdelay.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) mdelay.cpp -DBUILD_MSS

..\..\redist\win32\mdelay.flt: mdelay.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24a00000 -out:..\..\redist\win32\mdelay.flt mdelay.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


sdelay.obj: sdelay.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) sdelay.cpp -DBUILD_MSS

..\..\redist\win32\sdelay.flt: sdelay.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24b00000 -out:..\..\redist\win32\sdelay.flt sdelay.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


ringmod.obj: ringmod.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) ringmod.cpp -DBUILD_MSS

..\..\redist\win32\ringmod.flt: ringmod.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24c00000 -out:..\..\redist\win32\ringmod.flt ringmod.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


flange.obj: flange.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) flange.cpp -DBUILD_MSS

..\..\redist\win32\flange.flt: flange.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24d00000 -out:..\..\redist\win32\flange.flt flange.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib

chorus.obj: chorus.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) chorus.cpp -DBUILD_MSS

..\..\redist\win32\chorus.flt: chorus.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24e00000 -out:..\..\redist\win32\chorus.flt chorus.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


shelfeq.obj: shelfeq.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) shelfeq.cpp -DBUILD_MSS

..\..\redist\win32\shelfeq.flt: shelfeq.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x24f00000 -out:..\..\redist\win32\shelfeq.flt shelfeq.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


compress.obj: compress.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) compress.cpp -DBUILD_MSS

..\..\redist\win32\compress.flt: compress.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x25100000 -out:..\..\redist\win32\compress.flt compress.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


autopan.obj: autopan.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) autopan.cpp -DBUILD_MSS

..\..\redist\win32\autopan.flt: autopan.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x25200000 -out:..\..\redist\win32\autopan.flt autopan.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


laginter.obj: laginter.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) laginter.cpp -DBUILD_MSS

..\..\redist\win32\laginter.flt: laginter.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x25300000 -out:..\..\redist\win32\laginter.flt laginter.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


capture.obj: capture.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) capture.cpp -DBUILD_MSS

..\..\redist\win32\capture.flt: capture.obj mss32.res ..\..\lib\win\mss32.lib
  $(LINKDLL) -base:0x25400000 -out:..\..\redist\win32\capture.flt capture.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


mss32.res: ..\win\mss.rc ..\..\include\mss.h
  $(RC) -fomss32.res ..\win\mss.rc

#
# sample reverb provider - not compiled, just supplied as source
#

#reverb.obj: reverb.cpp ..\..\include\mss.h ..\shared\imssapi.h
#  $(COMPILE) reverb.cpp -DBUILD_MSS

#..\..\examples\console\reverb.flt: reverb.obj mss32.res ..\..\lib\win\mss32.lib
#  $(LINKDLL) -base:0x50000000 -out:..\..\examples\console\reverb.flt reverb.obj libc.lib ..\..\lib\win\mss32.lib mss32.res kernel32.lib


