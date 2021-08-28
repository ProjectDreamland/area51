#include "stdafx.h"
#include "BinaryStringEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
//GLOBAL VARIABLE USED FOR ANONYMOUS REGISTERATION
//=========================================================================

s32 g_stringbin_link = 0;

//=========================================================================

DEFINE_RSC_TYPE( s_Desc, binstring_desc, "stringbin", "Binary String Object File (*.stringbin)|*.stringbin", "A binary string object file." );


//=========================================================================
// FUNCTIONS
//=========================================================================

binstring_desc::binstring_desc( void ) : rsc_desc( s_Desc )
{
    m_FileName[0] = 0;
}

//=========================================================================

void binstring_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\TextFileName", 
                            "Tab Delimited File (*.txt)|*.txt", 
                            "This is the source Text file to be compile", 0 );
}

//=========================================================================

xbool binstring_desc::OnProperty( prop_query& I )
{
    if ( rsc_desc::OnProperty( I ) )
        return TRUE;

    if( I.VarFileName( "ResDesc\\TextFileName", m_FileName, 256 ) )
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void binstring_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    if( m_FileName[0] == 0 )
    {
        x_throw( "You must enter a source file name." );
    }
}

//=========================================================================

void binstring_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();
    List.Append() = m_FileName;
}

//=========================================================================

void binstring_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "stringTool.exe ";
    
    CompilerRules += m_FileName;
    
    CompilerRules += " ";
}

//=========================================================================

void binstring_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    (void)Platform;
    (void)pDirectory;
}
