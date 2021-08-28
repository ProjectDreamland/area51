@echo off

set STRINGTOOL_PATH=C:\GameData\A51\Apps\Compilers\
set SOURCE_PATH=C:\GameData\A51\Source\Themes\Strings\TextFiles\temp\
set DESTINATION_PATH=C:\GameData\A51\Release\PS2\

for %X in (%SOURCE_PATH%FRE*.txt) do %STRINGTOOL_PATH%stringtool -PS2 %DESTINATION_PATH%%@NAME[%X].stringbin %SOURCE_PATH%%@FILENAME[%X] 

set STRINGTOOL_PATH=
set SOURCE_PATH=
set DESTINATION_PATH=

