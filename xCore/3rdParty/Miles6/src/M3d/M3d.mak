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

all: ..\..\redist\win32\msseax2.m3d \
     ..\..\redist\win32\mssa3d.m3d ..\..\redist\win32\mssds3dh.m3d ..\..\redist\win32\mssds3ds.m3d   \
     ..\..\redist\win32\mssrsx.m3d ..\..\redist\win32\msseax.m3d ..\..\redist\win32\mssfast.m3d      \
     ..\..\redist\win32\mssdolby.m3d ..\..\redist\win32\mssa3d2.m3d  ..\..\examples\win\mssqsnd.m3d  \
     ..\..\redist\win32\mssdx7sn.m3d ..\..\redist\win32\mssdx7sl.m3d ..\..\redist\win32\mssdx7sh.m3d

#
# 3D providers
#

rsx.obj: rsx\rsx.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) rsx\rsx.cpp -Forsx.obj -DSTANDALONE -DBUILD_MSS

a3d.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) ds3dbase\ds3dbase.cpp -Foa3d.obj -DHWARE -DBUILD_MSS -DAUREAL

a3d2.obj: a3d2\a3d2.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) a3d2\a3d2.cpp -Foa3d2.obj -DHWARE -DBUILD_MSS -DAUREAL

ds3ds.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) ds3dbase\ds3dbase.cpp -Fods3ds.obj -DSWARE -DBUILD_MSS

ds3dh.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) ds3dbase\ds3dbase.cpp -Fods3dh.obj -DHWARE -DBUILD_MSS

dx7sn.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILEDX7) ds3dbase\ds3dbase.cpp -Fodx7sn.obj -DDX7SN -DBUILD_MSS -DSWARE -DDX7

dx7sl.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILEDX7) ds3dbase\ds3dbase.cpp -Fodx7sl.obj -DDX7SL -DBUILD_MSS -DSWARE -DDX7

dx7sh.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILEDX7) ds3dbase\ds3dbase.cpp -Fodx7sh.obj -DDX7SH -DBUILD_MSS -DSWARE -DDX7

eax.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILEDX5) ds3dbase\ds3dbase.cpp -Foeax.obj -DHWARE -DBUILD_MSS -DEAX3D

eax2.obj: ds3dbase\ds3dbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILEDX5) ds3dbase\ds3dbase.cpp -Foeax2.obj -DHWARE -DBUILD_MSS -DEAX3D -DEAX2

fast.obj: mssbase\mssbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) mssbase\mssbase.cpp -Fofast.obj -DBUILD_MSS

dolby.obj: mssbase\mssbase.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) mssbase\mssbase.cpp -Fodolby.obj -DUSE_REAR -DBUILD_MSS

qsound.obj: qsound\qsound.cpp ..\..\include\mss.h ..\shared\imssapi.h
  $(COMPILE) qsound\qsound.cpp -DBUILD_MSS -DQMIXMSS

..\..\redist\win32\mssa3d.m3d: ..\..\lib\win\mss32.lib a3d.obj mss32.res
  $(LINKDLL) -base:0x22100000 -out:..\..\redist\win32\mssa3d.m3d a3d.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DXDIR)\lib\dxguid.lib user32.lib winmm.lib advapi32.lib

..\..\redist\win32\mssa3d2.m3d: ..\..\lib\win\mss32.lib a3d2.obj mss32.res
  $(LINKDLL) -base:0x22200000 -out:..\..\redist\win32\mssa3d2.m3d a3d2.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res user32.lib winmm.lib advapi32.lib a3d2\ia3dutil.lib ole32.lib

..\..\redist\win32\mssds3ds.m3d: ..\..\lib\win\mss32.lib ds3ds.obj mss32.res
  $(LINKDLL) -base:0x22300000 -out:..\..\redist\win32\mssds3ds.m3d ds3ds.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DXDIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\mssds3dh.m3d: ..\..\lib\win\mss32.lib ds3dh.obj mss32.res
  $(LINKDLL) -base:0x22400000 -out:..\..\redist\win32\mssds3dh.m3d ds3dh.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DXDIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\msseax.m3d: ..\..\lib\win\mss32.lib eax.obj mss32.res
  $(LINKDLL) -base:0x22500000 -out:..\..\redist\win32\msseax.m3d eax.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DX5DIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\mssfast.m3d: ..\..\lib\win\mss32.lib fast.obj mss32.res
  $(LINKDLL) -base:0x22600000 -out:..\..\redist\win32\mssfast.m3d fast.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res user32.lib winmm.lib

..\..\redist\win32\mssdolby.m3d: ..\..\lib\win\mss32.lib dolby.obj mss32.res
  $(LINKDLL) -base:0x22700000 -out:..\..\redist\win32\mssdolby.m3d dolby.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res user32.lib winmm.lib

..\..\examples\win\mssqsnd.m3d: ..\..\lib\win\mss32.lib qsound.obj mss32.res
  $(LINKDLL) -base:0x22800000 -out:..\..\examples\win\mssqsnd.m3d qsound.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res user32.lib winmm.lib qsound\qmixmss.lib

..\..\redist\win32\mssdx7sl.m3d: ..\..\lib\win\mss32.lib dx7sl.obj mss32.res
  $(LINKDLL) -base:0x22900000 -out:..\..\redist\win32\mssdx7sl.m3d dx7sl.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DX7DIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\mssdx7sh.m3d: ..\..\lib\win\mss32.lib dx7sh.obj mss32.res
  $(LINKDLL) -base:0x22a00000 -out:..\..\redist\win32\mssdx7sh.m3d dx7sh.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DX7DIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\mssdx7sn.m3d: ..\..\lib\win\mss32.lib dx7sn.obj mss32.res
  $(LINKDLL) -base:0x22b00000 -out:..\..\redist\win32\mssdx7sn.m3d dx7sn.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DX7DIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\msseax2.m3d: ..\..\lib\win\mss32.lib eax2.obj mss32.res
  $(LINKDLL) -base:0x22c00000 -out:..\..\redist\win32\msseax2.m3d eax2.obj libc.lib kernel32.lib ..\..\lib\win\mss32.lib mss32.res $(DX5DIR)\lib\dxguid.lib user32.lib winmm.lib

..\..\redist\win32\mssrsx.m3d: ..\..\lib\win\mss32.lib rsx.obj mss32.res rsx\rsxcore.lib
  $(LINKDLL) -base:0x22d00000 -out:..\..\redist\win32\mssrsx.m3d rsx.obj kernel32.lib ..\..\lib\win\mss32.lib mss32.res ole32.lib user32.lib advapi32.lib winmm.lib rsx\rsxcore.lib libcmt.lib msacm32.lib uuid.lib

mss32.res: ..\win\mss.rc ..\..\include\mss.h
  $(RC) -fomss32.res ..\win\mss.rc


