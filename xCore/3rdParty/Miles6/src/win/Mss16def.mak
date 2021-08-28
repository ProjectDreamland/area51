###############################################################
#                                                             #
#  MAKEFILE for mss development                               #
#                                                             #
#  MSC 7.0 16-bit version                                     #
#                                                             #
#  Execute with Microsoft (or compatible) NMAKE               #
#                                                             #
###############################################################

#
#   *** Change this directory path to your directory structure!
#

MSVCDIR=c:\devel\msvc
MASMDIR=c:\devel\masm


#
#   *** Set the next variable to one or zero depending on whether you want debug info
#

DEBUG=0


!if $(DEBUG)

ASSEMBLE=$(MASMDIR)\BIN\ml /Zm /c /W2 /Zi
COMPILE=$(MSVCDIR)\BIN\cl -c -Zi -ALw -GDs -Ow -W3 -Zp -Zp1 -I$(MSVCDIR)\INCLUDE -I..\shared -I..\..\include
COMPILEEXE=$(MSVCDIR)\BIN\cl -c -Zi -GAsr -Ow -W2 -Zp -I$(MSVCDIR)\INCLUDE -D_X86_=1 -I..\shared -I..\..\include
LINK=$(MSVCDIR)\BIN\link /LINENUMBERS /CODEVIEW

!else

ASSEMBLE=$(MASMDIR)\BIN\ml /Zm /c /W2
COMPILE=$(MSVCDIR)\BIN\cl -FPi87 -c -Ox -G3 -Gs -ALw -GDs -Ow -W3 -Zp -Zp1 -I$(MSVCDIR)\INCLUDE -I..\shared -I..\..\include
COMPILEEXE=$(MSVCDIR)\BIN\cl -c -Ox -G3 -GAsr -Ow -W2 -Zp -I$(MSVCDIR)\INCLUDE -D_X86_=1 -I..\shared -I..\..\include
LINK=$(MSVCDIR)\BIN\link

!endif

RC=$(MSVCDIR)\BIN\rc -i$(MSVCDIR)\INCLUDE;$(MSVCDIR)\MFC\INCLUDE

PATH=$(PATH);$(MSVCDIR)\bin;$(MSVCDIR)\lib
LIB=$(MSVCDIR)\lib

IMPLIB=$(MSVCDIR)\BIN\implib

