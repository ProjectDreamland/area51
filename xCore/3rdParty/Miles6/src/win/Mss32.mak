###############################################################
#                                                             #
#  MAKEFILE for Miles Sound System Demo32 example program     #
#                                                             #
#  MSVC 4.0                                                   #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

!include "mss32def.mak"

{..\shared}.c.obj::
  $(COMPILE) $< -DBUILD_MSS

{..\shared}.cpp.obj::
  $(COMPILE) $< -DBUILD_MSS

all: ..\..\redist\win32\mss32.dll ..\..\tools\win\vort_dls.dll

..\..\redist\win32\mss32.dll: mssa32.obj \
            mssdig.obj   \
            mss.obj     \
            msssys.obj  \
            wavefile.obj \
            mssxmidi.obj \
            midilist.obj \
            mssutil.obj \
            dlslist.obj \
            mssxdig.obj \
            mssdbg.obj  \
            mssqapi.obj \
            msscd.obj   \
            mssadpcm.obj \
            mssstrm.obj \
            miscutil.obj \
            rib.obj \
            m3d.obj \
            flt.obj \
            mssadpcm.obj \
            dlshl.obj  \
            mssinput.obj \
            synth.obj dls1pars.obj dls1syn.obj dlsfile.obj \
            midi2xmi.obj \
            cleanup.obj  \
            mss32.res
    $(LINKDLL) /OUT:..\..\redist\win32\mss32.dll /DEF:mss32.def -base:0x21100000 mss32.res mss.obj mssdbg.obj msssys.obj mssdig.obj wavefile.obj \
                                                                 mssxmidi.obj mssxdig.obj mssa32.obj msscd.obj miscutil.obj mssqapi.obj mssstrm.obj \
                                                                 dlshl.obj synth.obj dls1pars.obj dls1syn.obj dlsfile.obj midi2xmi.obj mssadpcm.obj \
                                                                 rib.obj m3d.obj flt.obj midilist.obj mssutil.obj dlslist.obj mssinput.obj \
                                                                 libc.lib kernel32.lib user32.lib winmm.lib
    $(LIB) /OUT:..\..\lib\win\mss32.lib cleanup.obj ..\..\redist\win32\mss32.lib
    del ..\..\redist\win32\mss32.lib
    del ..\..\redist\win32\mss32.exp
    $(MSVCDIR)\bin\dumpbin /exports ..\..\redist\win32\mss32.dll >tmp.def
    dodefs tmp.def nul ..\..\lib\win\mss32bc.def
    del tmp.def
    copy /b ..\..\redist\win32\mss32.dll ..\..\tools\win
    copy /b ..\..\redist\win32\mss32.dll ..\..\examples\win
    copy /b ..\..\redist\win32\mss32.dll ..\..\examples\console

mss32.res: mss.rc ..\..\include\mss.h
  $(RC) -fomss32.res mss.rc

mssa32.obj: ..\shared\mssa32.asm ..\shared\mssmixer.asm
   $(ASSEMBLE) ..\shared\mssa32.asm

..\shared\mssmixer.asm: ..\shared\mixbuild.exe
  ..\shared\mixbuild.exe ..\shared\mssmixer.asm

..\shared\mixbuild.exe: ..\shared\mixbuild.c
    $(COMPILE) ..\shared\mixbuild.c
    $(LINK) /SUBSYSTEM:CONSOLE /FIXED /RELEASE mixbuild.obj libc.lib kernel32.lib -out:..\shared\mixbuild.exe

mss.obj: mss.cpp ..\..\include\mss.h ..\shared\imssapi.h

rib.obj: ..\shared\rib.cpp ..\..\include\mss.h ..\shared\imssapi.h

m3d.obj: ..\shared\m3d.cpp ..\..\include\mss.h ..\shared\imssapi.h

flt.obj: ..\shared\flt.cpp ..\..\include\mss.h ..\shared\imssapi.h

mssqapi.obj: ..\shared\mssqapi.c ..\..\include\mss.h ..\shared\imssapi.h

cleanup.obj: cleanup.c ..\..\include\mss.h

msssys.obj: msssys.c ..\..\include\mss.h ..\shared\imssapi.h

mssdig.obj: mssdig.cpp ..\..\include\mss.h ..\shared\imssapi.h

mssinput.obj: mssinput.cpp ..\..\include\mss.h ..\shared\imssapi.h

wavefile.obj: ..\shared\wavefile.cpp ..\..\include\mss.h ..\shared\imssapi.h

mssutil.obj: ..\shared\mssutil.cpp ..\..\include\mss.h ..\shared\imssapi.h

mssxmidi.obj: mssxmidi.c ..\..\include\mss.h ..\shared\imssapi.h

mssxdig.obj: ..\shared\mssxdig.c ..\..\include\mss.h ..\shared\imssapi.h

mssdbg.obj: ..\shared\mssdbg.c ..\..\include\mss.h ..\shared\mssdbg.inc ..\shared\imssapi.h

msscd.obj: msscd.c ..\..\include\mss.h ..\shared\imssapi.h

mssstrm.obj: ..\shared\mssstrm.cpp ..\..\include\mss.h ..\shared\imssapi.h

dlshl.obj: ..\shared\dlshl.c ..\..\include\mss.h ..\shared\imssapi.h

midi2xmi.obj: ..\shared\midi2xmi.c ..\..\include\mss.h ..\shared\imssapi.h

midilist.obj: ..\shared\midilist.c ..\..\include\mss.h ..\shared\imssapi.h

dlslist.obj: ..\shared\dlslist.cpp ..\..\include\mss.h ..\shared\imssapi.h

mssadpcm.obj: ..\shared\mssadpcm.cpp ..\..\include\mss.h ..\shared\imssapi.h

miscutil.obj: ..\shared\miscutil.cpp ..\..\include\mss.h ..\shared\imssapi.h

#
# DLS synthesizer files
#

synth.obj: ..\shared\synth.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h ..\..\include\mss.h ..\shared\imssapi.h

dls1pars.obj: ..\shared\dls1pars.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h ..\..\include\mss.h ..\shared\imssapi.h

dls1syn.obj: ..\shared\dls1syn.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h ..\..\include\mss.h ..\shared\imssapi.h

dls1mix.obj: ..\shared\dls1mix.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h ..\..\include\mss.h ..\shared\imssapi.h

dlsfile.obj: ..\shared\dlsfile.cpp ..\shared\mssdls.h ..\..\include\mss.h ..\shared\imssapi.h

#
# Vortex DLS downloader
#

..\..\tools\win\vort_dls.dll: vortex.obj
  $(LINK) -dll -MACHINE:IX86 -base:0x21200000 /RELEASE /NODEFAULTLIB -out:..\..\tools\win\vort_dls.dll vortex.obj libc.lib kernel32.lib /def:vortex.def
  del ..\..\tools\win\vort_dls.exp
  del ..\..\tools\win\vort_dls.lib

vortex.obj: vortex.c
