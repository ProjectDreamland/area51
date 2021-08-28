//==============================================================================
//
//  CommandLine.hpp
//
//==============================================================================
//
//  Command Line Parser
//
//==============================================================================

#ifndef COMMAND_LINE_HPP
#define COMMAND_LINE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  command_line
//==============================================================================

class command_line
{
public:
    enum option_type
    {
        SWITCH,
        STRING,
        NUMBER
    };

protected:
    struct option_def
    {
        xstring     Name;
        option_type Type;
    };

    struct option_entry
    {
        s32         OptionDefIndex;
        xstring     String;
    };

protected:
    xstring                 m_ExecutableName;
    xarray<option_def>      m_OptionDefs;
    xarray<option_entry>    m_Options;
    xarray<xstring>         m_Arguments;

    void            StoreOption         ( s32 iOption, xstring& String );
    void            StoreArgument       ( xstring &String );
    xbool           ReadResponseFile    ( xstring& PathName, xarray<xstring>& Args );

public:
                    command_line        ( );
                   ~command_line        ( );

    void            AddOptionDef        ( xstring Option, option_type OptionType = SWITCH );

    xbool           Parse               ( int argc, char** argv );

    s32             GetNumOptions       ( void ) const;
    s32             GetNumArguments     ( void ) const;
    s32             FindOption          ( xstring& Option ) const;

    const xstring&  GetExecutableName   ( void ) const;
    const xstring&  GetOptionName       ( s32 Index ) const;
    const xstring&  GetOptionString     ( s32 Index ) const;
    const xstring&  GetArgument         ( s32 Index ) const;

	// Command Line assist functions
	static void		SplitPath			( const xstring& PathName, xstring& Path, xstring& File );
    static xstring  JoinPath            ( const xstring& Path, const xstring& File );
    static xstring  ChangeExtension     ( const xstring& PathName, const xstring& Extension );
	static s32		Glob				( const xstring& Pattern, xarray<xstring>& Results, xbool Recursive = FALSE );
    static xbool    FileExists          ( const char* pPathName );
};

//==============================================================================

#endif //COMMAND_LINE_HPP