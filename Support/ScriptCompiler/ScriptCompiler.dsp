# Microsoft Developer Studio Project File - Name="ScriptCompiler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ScriptCompiler - Win32 PS2 DevKit Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ScriptCompiler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ScriptCompiler.mak" CFG="ScriptCompiler - Win32 PS2 DevKit Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ScriptCompiler - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 DVD Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 DVD Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 Client Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 Client Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 DevKit Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ScriptCompiler - Win32 PS2 DevKit Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/A51/Support/ScriptCompiler", WXHAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ScriptCompiler - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(X)\x_files" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "$(X)\x_files" /I "..\ScriptVM" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(X)\x_files" /I "..\ScriptVM" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "X_DEBUG" /D "X_ASSERT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 DVD Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DVD-Release"
# PROP BASE Intermediate_Dir "PS2-DVD-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DVD-Release"
# PROP Intermediate_Dir "PS2-DVD-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 DVD Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DVD-Debug"
# PROP BASE Intermediate_Dir "PS2-DVD-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DVD-Debug"
# PROP Intermediate_Dir "PS2-DVD-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_DVD" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DVD /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-Client-Release"
# PROP BASE Intermediate_Dir "PS2-Client-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-Client-Release"
# PROP Intermediate_Dir "PS2-Client-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-Client-Debug"
# PROP BASE Intermediate_Dir "PS2-Client-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-Client-Debug"
# PROP Intermediate_Dir "PS2-Client-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_CLIENT" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_CLIENT /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 DevKit Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PS2-DevKit-Release"
# PROP BASE Intermediate_Dir "PS2-DevKit-Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PS2-DevKit-Release"
# PROP Intermediate_Dir "PS2-DevKit-Release"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /O2 /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD CPP /O2 /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /FD
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ELSEIF  "$(CFG)" == "ScriptCompiler - Win32 PS2 DevKit Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2-DevKit-Debug"
# PROP BASE Intermediate_Dir "PS2-DevKit-Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2-DevKit-Debug"
# PROP Intermediate_Dir "PS2-DevKit-Debug"
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE CPP /Zi /Od /I "$(X)\x_files" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD CPP /Zi /Od /I "$(X)\x_files" /I "..\ScriptVM" /D "TARGET_PS2_DEV" /D "VENDOR_SN" /D "X_DEBUG" /D "X_ASSERT" /FD
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN
# ADD LIB32 /D:TARGET_PS2_DEV /D:VENDOR_SN

!ENDIF 

# Begin Target

# Name "ScriptCompiler - Win32 Release"
# Name "ScriptCompiler - Win32 Debug"
# Name "ScriptCompiler - Win32 PS2 DVD Release"
# Name "ScriptCompiler - Win32 PS2 DVD Debug"
# Name "ScriptCompiler - Win32 PS2 Client Release"
# Name "ScriptCompiler - Win32 PS2 Client Debug"
# Name "ScriptCompiler - Win32 PS2 DevKit Release"
# Name "ScriptCompiler - Win32 PS2 DevKit Debug"
# Begin Group "Parser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_parse_class.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_compound_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_continue_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_expression.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_for_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_if_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_import.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_return_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_typedef.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parse_while_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_parser.hpp
# End Source File
# End Group
# Begin Group "Tokenizer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_tokenizer.hpp
# End Source File
# End Group
# Begin Group "CodeGenerator"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_codegen.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen.hpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_compound_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_continue_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_expression.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_for_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_if_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_module.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_return_statement.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_utility.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_codegen_while_statement.cpp
# End Source File
# End Group
# Begin Group "SymbolTable"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_symbol_table.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_symbol_table.hpp
# End Source File
# End Group
# Begin Group "Compiler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_compiler.hpp
# End Source File
# End Group
# Begin Group "Errors"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_errors.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_errors.hpp
# End Source File
# End Group
# Begin Group "AST"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_ast.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_ast.hpp
# End Source File
# Begin Source File

SOURCE=.\xsc_ast_types.cpp
# End Source File
# End Group
# Begin Group "Utility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xsc_map.cpp
# End Source File
# Begin Source File

SOURCE=.\xsc_map.hpp
# End Source File
# End Group
# End Target
# End Project
