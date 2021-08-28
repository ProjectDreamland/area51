#include "stdafx.h"
#include "FontEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
//GLOBAL VARIABLE USED FOR ANONYMOUS REGISTERATION
//=========================================================================

s32 g_font_link = 0;

//=========================================================================

DEFINE_RSC_TYPE( s_Desc, font_desc, "font", "Font Object File (*.font)|*.font", "A font object file." );


//=========================================================================
// FUNCTIONS
//=========================================================================

font_desc::font_desc( void ) : rsc_desc( s_Desc )
{
    m_SourceFileName[0] = 0;
    m_MapFileName[0] = 0;
    x_strcpy( m_format, "Palette 8" );
}

//=========================================================================

void font_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\SourceBitmapFileName", 
                            "Source Bitmap File (*.tga)|*.tga", 
                            "This is the source bitmap file to be compiled", 0 );

    List.PropEnumFileName( "ResDesc\\FontMapFileName", 
                            "Font Map File (*.map)|*.map", 
                            "This is the source character mapping file to be compiled", 0 );

    List.PropEnumEnum    ( "ResDesc\\Format", "Palette 8\0Palette 4\0True Color(32bits)",
        "Choose the format which you would like your bitmap to be.", 0 );

}

//=========================================================================

xbool font_desc::OnProperty( prop_query& I )
{
    if ( rsc_desc::OnProperty( I ) )
    {
        // nothing to do...
    }
    else if( I.VarFileName( "ResDesc\\SourceBitmapFileName", m_SourceFileName, 256 ) )
    {
        // nothing to do...
    }
    else if( I.VarFileName( "ResDesc\\FontMapFileName", m_MapFileName, 256 ) )
    {
        // nothing to do...
    }
    else if( I.VarEnum( "ResDesc\\Format", m_format ) )
    {
        // nothing to do...
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void font_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    if( m_SourceFileName[0] == 0 )
    {
        x_throw( "You must enter a source file name." );
    }

    if( m_MapFileName[0] == 0 )
    {
        x_throw( "You must enter a map file name." );
    }
}

//=========================================================================

void font_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();
    List.Append() = m_SourceFileName;
    List.Append() = m_MapFileName;
}

//=========================================================================

void font_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "fontbuilder.exe ";
    
    CompilerRules += "-infile ";

    CompilerRules += m_SourceFileName;
    
    CompilerRules += " ";

    CompilerRules += "-mapfile ";
    
    CompilerRules += m_MapFileName;
    
    CompilerRules += " ";

    if( x_stricmp( m_format, "Palette 8") == 0 )
    {
        CompilerRules += "-P8 ";
    }
    else if( x_stricmp( m_format, "Palette 4") == 0 )
    {
        CompilerRules += "-P4 ";
    }
    else if( x_stricmp( m_format, "True Color(32bits)") == 0 )
    {
        CompilerRules += "-TC ";
    }

    CompilerRules += "-writexbmp";

}

//=========================================================================

void font_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    (void)Platform;
    (void)pDirectory;
}
