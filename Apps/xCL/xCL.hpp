//==============================================================================
//
//  xCL - Cross Platform Compile and Link
//
//==============================================================================

#ifndef XCL_HPP
#define XCL_HPP

//==============================================================================
//   INCLUDES
//==============================================================================

#include "CmdLineData.hpp"

//==============================================================================
//   VARIABLE DECLARATIONS
//==============================================================================

extern cmd_line_data    g_CmdLineData;

extern bool             g_ShowTimes;
extern bool             g_Verbose;
extern bool             g_EchoCmdLine;
extern bool             g_StripSymbols;
extern bool             g_DistributedBuild;

//==============================================================================
//  FUNCTION DECLARATIONS
//==============================================================================
                                        
void    OpenSection                     ( char* pTitle );
void    DivideSection                   ( void );
void    CloseSection                    ( void );
                                        
void    DumpTextFileContent             ( char* pFileName );
void    DumpCommandLineAndResponseFile  ( char* pTitle,
                                          char* pCmdLine,
                                          char* pResponseFileName );
                                         
char*   UseOnlyBackSlashes              ( char* pPath );
char*   UseOnlySlashes                  ( char* pPath );

int     ExecuteCMD                      ( const char* pCommand );

//==============================================================================
#endif // XCL_HPP
//==============================================================================
