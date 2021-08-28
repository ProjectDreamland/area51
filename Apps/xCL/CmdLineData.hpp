//==============================================================================
//
//  CmdLineData.hpp
//
//==============================================================================

#ifndef CMDLINEDATA_HPP
#define CMDLINEDATA_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include <stdlib.h>
#include "TokenList.hpp"

//==============================================================================
//  TYPES
//==============================================================================

enum {
    TOOL_UNKNOWN = 0,
    TOOL_COMPILER,
    TOOL_LINKER,
    TOOL_LIBRARIAN,
};

enum {
    BUILD_TARGET_UNKNOWN = 0,
    BUILD_TARGET_PC,
    BUILD_TARGET_PS2,
    BUILD_TARGET_PS2_IOP,
    BUILD_TARGET_PS2_DLL,
    BUILD_TARGET_XBOX,
    BUILD_TARGET_GCN,
    BUILD_TARGET_END_OF_LIST,
};

enum {
    VENDOR_UNKNOWN = 0,
    VENDOR_MS,              // Microsoft
    VENDOR_SN,              // SN Systems    
    VENDOR_MW,              // Metrowerks
    VENDOR_END_OF_LIST,
};

enum {
    SUBSYS_DEFAULT = 0,
    SUBSYS_WINDOWS,
    SUBSYS_WINCON,
    SUBSYS_END_OF_LIST,
};

//==============================================================================

class cmd_line_data
{
public:
                cmd_line_data( void );
               ~cmd_line_data();

    void        Activate( int argc, char** argv );

    token_list  m_Define;
    token_list  m_IncludeDir;
    token_list  m_LibraryDir;
    token_list  m_SourceCode;
    token_list  m_SourceOther;
    token_list  m_ObjectFile;
    token_list  m_LibraryFile;
    token_list  m_UnknownOption;

    int         m_argc;
    char**      m_argv;

    int         m_Tool;
    int         m_Target;
    int         m_Vendor;
    int         m_SubSystem;
    bool        m_DebugMode;
    bool        m_Preprocess;
    bool        m_ForceC;
    bool        m_ForceCPP;
    char        m_Optimization;
    char        m_TargetString[ 128 ];
    char        m_VendorString[ 128 ];
    char        m_LinkOptions [ 128 ];
    char        m_PCHFile     [ _MAX_PATH ];
    char        m_OutputFile  [ _MAX_PATH ];
    char        m_OutputPath  [ _MAX_PATH ];
    char        m_SBRPath     [ _MAX_PATH ];
    char        m_PDBPath     [ _MAX_PATH ];

private:
    void        ProcessOption( char* pToken );
    void        ProcessFile  ( char* pToken );
    void        Dump         ( void );
};

//==============================================================================
#endif // CMDLINEDATA_HPP
//==============================================================================
