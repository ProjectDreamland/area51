###############################################################
#                                                             #
#  MAKEFILE for mss development                               #
#                                                             #
#  MSC 7.0 16-bit version                                     #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################


!include "mss16def.mak"

..\..\redist\win16\mssb16.tsk: mssb16.obj ..\..\lib\win\mss16.lib
  $(LINK) mssb16,mssb16.exe /align:16,nul,/nod llibcew libw, mssb16
  $(RC) mssb16.exe
  copy mssb16.exe ..\..\redist\win16\mssb16.tsk
  del mssb16.exe

..\..\lib\win\mss16.lib: ..\..\redist\win16\mss16.dll
  $(IMPLIB) ..\..\lib\win\mss16.lib ..\..\redist\win16\mss16.dll

..\..\redist\win16\mss16.dll: mss16.def mss6.obj msssys6.obj mssxmid6.obj mssdig6.obj mssa16.obj mssdbg6.obj mssxdig6.obj wavefil6.obj rib6.obj msscd6.obj mssqapi6.obj mssstrm6.obj mss.rc dlshl6.obj synth6.obj dls1par6.obj dls1syn6.obj dlsfile6.obj midi2xm6.obj miscuti6.obj mssadpc6.obj midilis6.obj dlslist6.obj mssinpu6.obj
   $(LINK) /PACKC:60000 @mss16.rsp
   $(RC) -fomss16.res -dMSS_H mss.rc ..\..\redist\win16\mss16.dll
   copy /b ..\..\redist\win16\mss16.dll ..\..\tools\win
   copy /b ..\..\redist\win16\mss16.dll ..\..\examples\win

mssb16.obj: mssb16.c
   $(COMPILE) mssb16.c

mssa16.obj: mssa16.asm
   $(ASSEMBLE) -I..\shared mssa16.asm

mss6.obj: mss.cpp ..\..\include\mss.h
   $(COMPILE) -Fomss6 mss.cpp

mssqapi6.obj: ..\shared\mssqapi.c ..\..\include\mss.h
   $(COMPILE) -Fomssqapi6 ..\shared\mssqapi.c

msssys6.obj: msssys.c ..\..\include\mss.h
   $(COMPILE) -Fomsssys6 msssys.c

mssdig6.obj: mssdig.cpp ..\..\include\mss.h
   $(COMPILE) -Fomssdig6 mssdig.cpp

mssinpu6.obj: mssinput.cpp ..\..\include\mss.h
   $(COMPILE) -Fomssinpu6 mssinput.cpp

wavefil6.obj: ..\shared\wavefile.cpp ..\..\include\mss.h
   $(COMPILE) -Fowavefil6 ..\shared\wavefile.cpp

rib6.obj: ..\shared\rib.cpp ..\..\include\mss.h
   $(COMPILE) -Forib6 ..\shared\rib.cpp

mssxmid6.obj: mssxmidi.c ..\..\include\mss.h
   $(COMPILE) -Fomssxmid6 mssxmidi.c

mssxdig6.obj: ..\shared\mssxdig.c ..\..\include\mss.h
   $(COMPILE) -Fomssxdig6 ..\shared\mssxdig.c

mssdbg6.obj: ..\shared\mssdbg.c ..\..\include\mss.h ..\shared\mssdbg.inc
   $(COMPILE) -Fomssdbg6 ..\shared\mssdbg.c

msscd6.obj: msscd.c ..\..\include\mss.h
   $(COMPILE) -Fomsscd6 msscd.c

mssstrm6.obj: ..\shared\mssstrm.cpp ..\..\include\mss.h
   $(COMPILE) -Fomssstrm6 ..\shared\mssstrm.cpp

dlshl6.obj: ..\shared\dlshl.c ..\..\include\mss.h
   $(COMPILE) -Fodlshl6 ..\shared\dlshl.c

midi2xm6.obj: ..\shared\midi2xmi.c ..\..\include\mss.h
   $(COMPILE) -Fomidi2xm6 ..\shared\midi2xmi.c

miscuti6.obj: ..\shared\miscutil.cpp ..\..\include\mss.h
   $(COMPILE) -Fomiscuti6 ..\shared\miscutil.cpp

mssadpc6.obj: ..\shared\mssadpcm.cpp ..\..\include\mss.h
   $(COMPILE) -Fomssadpc6 ..\shared\mssadpcm.cpp

midilis6.obj: ..\shared\midilist.c ..\..\include\mss.h
   $(COMPILE) -Fomidilis6 ..\shared\midilist.c

dlslist6.obj: ..\shared\dlslist.cpp ..\..\include\mss.h
   $(COMPILE) -Fodlslist6 ..\shared\dlslist.cpp



#
# DLS synthesizer files
#

synth6.obj: ..\shared\synth.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h
    $(COMPILE) -Fosynth6 ..\shared\synth.cpp

dls1par6.obj: ..\shared\dls1pars.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h
    $(COMPILE) -Fodls1par6 ..\shared\dls1pars.cpp

dls1syn6.obj: ..\shared\dls1syn.cpp ..\shared\mssdls.h ..\shared\dls1.h ..\shared\synth.h
    $(COMPILE) -Fodls1syn6 ..\shared\dls1syn.cpp

dlsfile6.obj: ..\shared\dlsfile.cpp ..\shared\mssdls.h
    $(COMPILE) -Fodlsfile6 ..\shared\dlsfile.cpp

