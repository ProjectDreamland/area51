****************************************************************************
*  Miles Sound System                          RAD Game Tools 425-893-4300 *
*                                                                          *
*  Copyright (C) 1991-2001  RAD Game Tools, Inc.                           *
****************************************************************************

                    *******************************
                    * Important Licensing Details *
                    *******************************

To obtain the MSS MPEG Layer-3 decompression code, you must sign a new
license addendum first (contact Mitch for details).

To obtain the Voxware voice chat codecs, you must also sign a new license
addendum first (contact Mitch for details).

The QSound M3D provider is not redistributable (that is why it is only
found in the \examples\win directory and *not* in the \redist\win32
directory (again, contact Mitch for details).


                        ***********************
                        * Directory Structure *
                        ***********************

\Redist   - The redistributable portions of MSS
              \Redist\Win32 - Win32 files: MSS32.DLL, *.ASI, *.M3D, *.FLT
              \Redist\Mac   - MacOS files: Miles Shared Library, *.ASI, *.M3D, *.FLT
              \Redist\Win16 - Win16 files: MSS16.DLL, *.AS6, *.TSK
              \Redist\DOS   - DOS drivers: *.DIG, *.MDI, SetSound.EXE

\Include  - The MSS include files - MSS.H, *.PAS (for Delphi)

\Lib      - The MSS libraries for linking
              \Lib\Win - Windows import libs (MSS32.lib and MSS16.lib)
              \Lib\DOS - DOS static libs (MSSWAT*.lib and MSSBOR*.lib)

\Examples - The MSS examples tree
              \Examples\Console - Text applications for DOS, Win32, or MacOS
              \Examples\Win     - Windows specific examples
              \Examples\Media   - Example multimedia files

\Tools    - The MSS Tools
              \Tools\Win - Windows tools (Miles Sound Studio, Miles Sound
                           Player and MIDIEcho).
              \Tools\Mac - MacOS tools (Miles Sound Studio, Miles Sound
                           Player and MIDIEcho).
              \Tools\DOS - DOS tools (Wavelib, MIDIEcho, MIDIRec, MIDILog,
                           CLAD, GLIB)

\Src      - The MSS source tree
              \SRC\Win    - Windows specific source code and make files
              \SRC\DOS    - DOS specific source code and make files
              \SRC\Mac    - MacOS specific source code and make files
              \SRC\Shared - Shared source code
              \SRC\ASI    - ASI codecs source code and make files
              \SRC\M3D    - M3D providers source code and make files
              \SRC\FLT    - Windows specific source code and make files

\Docs     - The MSS online documentation
              \DOCS\WinHelp\Miles.chm - The main Windows HTML help file.
              \DOCS\HTMLHelp          - The non-compiled version of the HTML 
                                          help (start page is "index.html").


                         *********************
                         * Contacting RAD... *
                         *********************

Please contact RAD Game Tools at 425-893-4300 (or sales@radgametools.com)
  with any licensing inquiries, comments, technical-support questions, or
  feature requests!

RAD Game Tools
335 Park Place - G109
Kirkland, WA  98033
98033

425-893-4300
FAX: 425-893-9111

E-mail:  msssdk@radgametools.com   or   sales@radgametools.com
