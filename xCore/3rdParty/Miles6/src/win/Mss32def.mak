###############################################################
#                                                             #
#  MAKEFILE definitions                                       #
#                                                             #
###############################################################

#
#   *** Change these directory paths to your directory structure!
#

MSVCDIR=c:\devel\devstu6\vc98
RCDIR=c:\devel\devstu6\common\msdev98
MASMDIR=c:\devel\masm
DXDIR=c:\devel\libs\dx\sdk
DX5DIR=c:\devel\libs\dx5\sdk
DX7DIR=c:\devel\libs\dx7


#
#   *** Set the next variable to one or zero depending on whether you want debug info
#

DEBUG=0


!if $(DEBUG)

COMPILEBASE=$(MSVCDIR)\BIN\cl -c -Zi -Zl -Gs -Gr -Zp1 -W3 -I..\..\include -I..\shared /QIfist
ASSEMBLE=$(MASMDIR)\BIN\ml /Cp /Zm /c /W2 /Zi /Zd /coff
LINK=$(MSVCDIR)\BIN\link /MAP -debug:full -debugtype:both /NODEFAULTLIB /RELEASE -MACHINE:IX86 -LIBPATH:$(MSVCDIR)\LIB
LIB=$(MSVCDIR)\BIN\lib -debugtype:both /NODEFAULTLIB /RELEASE -MACHINE:IX86

!else

COMPILEBASE=$(MSVCDIR)\BIN\cl -c -Ox -O2 -Zl -G6 -Gs -Gr -GX- -GR- -Gy -GF -Zp1 -W3 -I..\..\include -I..\shared /QIfist
ASSEMBLE=$(MASMDIR)\BIN\ml /Cp /Zm /c /W2
LINK=$(MSVCDIR)\BIN\link /NODEFAULTLIB /RELEASE -MACHINE:IX86 -LIBPATH:$(MSVCDIR)\LIB /opt:ref,nowin98
LIB=$(MSVCDIR)\BIN\lib /NODEFAULTLIB /RELEASE -MACHINE:IX86

!endif

COMPILE=$(COMPILEBASE) -I$(DXDIR)\INC -I$(MSVCDIR)\INCLUDE -D_X86_=1 -DWIN32 -D_WIN32
COMPILEDX5=$(COMPILEBASE) -I$(DX5DIR)\INC -I$(MSVCDIR)\INCLUDE -D_X86_=1 -DWIN32 -D_WIN32
COMPILEDX7=$(COMPILEBASE) -I$(DX7DIR)\INCLUDE -I$(MSVCDIR)\INCLUDE -D_X86_=1 -DWIN32 -D_WIN32
RC=$(RCDIR)\BIN\rc -d_WIN32 -i$(MSVCDIR)\INCLUDE;$(MSVCDIR)\MFC\INCLUDE
LINKEXE=$(LINK) /FIXED
LINKDLL=$(LINK) /DLL /LINK50COMPAT

PATH=$(PATH);$(MSVCDIR)\bin;$(RCDIR)\bin;$(MSVCDIR)\lib

.c.obj::
  $(COMPILE) $< -DBUILD_MSS

.cpp.obj::
  $(COMPILE) $< -DBUILD_MSS
