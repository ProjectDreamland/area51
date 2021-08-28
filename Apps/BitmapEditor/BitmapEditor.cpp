#include "stdafx.h"
#include "BitmapEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

//=========================================================================
// LOCAL
//=========================================================================
DEFINE_RSC_TYPE( s_Desc,    bitmap_desc, "xbmp",   "Bitmap Files (*.xbmp)|*.xbmp",             "A package containing a bitmap " );
DEFINE_RSC_TYPE( s_DescEnv, envmap_desc, "envmap", "Enviroment Map Files (*.envmap)|*.envmap", "A package containing an enviroment bitmap set" );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================
// ENV MAP DESC
//=========================================================================

//=========================================================================

envmap_desc::envmap_desc( void ) : rsc_desc( s_DescEnv ) 
{
    m_FileName[0][0]=0;
    m_FileName[1][0]=0;
    m_FileName[2][0]=0;
    m_FileName[3][0]=0;
    m_FileName[4][0]=0;
    m_FileName[5][0]=0;
    x_strcpy( m_PreferFormat, "Palettice 8" );
    x_strcpy( m_OptLevel    , "Agressive" );
}

//=========================================================================

void envmap_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\Top", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumFileName( "ResDesc\\Bottom", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumFileName( "ResDesc\\Front", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumFileName( "ResDesc\\Back", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumFileName( "ResDesc\\Left", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumFileName( "ResDesc\\Right", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumEnum    ( "ResDesc\\Prefer Format", "Palettice 8\0Palettice 4\0Compress\0True Color(32bits)\0",
                            "Choose the format which you would like your bitmap to be compressed at.", 0 );

    List.PropEnumEnum    ( "ResDesc\\Optimisation", "Agressive\0None",
                            "Choose the optimisation level.", 0 );
}

//=========================================================================

xbool envmap_desc::OnProperty( prop_query& I )
{
    if( rsc_desc::OnProperty( I ) )
    {
        // Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Top", m_FileName[0], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Bottom", m_FileName[1], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Front", m_FileName[2], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Back", m_FileName[3], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Left", m_FileName[4], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\Right", m_FileName[5], 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarEnum( "ResDesc\\Prefer Format", m_PreferFormat ) )
    {
        // .. Nothing to do
    }
    else if( I.VarEnum( "ResDesc\\Optimisation", m_OptLevel ) )
    {
        // .. Nothing to do
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void envmap_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    for( s32 i=0;i<6;i++)
        if( m_FileName[i] == 0 )
        {
            x_throw( "You must enter all 6 file names for the env map." );
        }
}

//=========================================================================

void envmap_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();

    for( s32 i=0;i<6;i++)
        List.Append() = m_FileName[i];
}

//=========================================================================

void envmap_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "BitmapCompiler.exe ";

    for( s32 i=0;i<6;i++)
    {
        CompilerRules += "-APPEND \"";
        CompilerRules += m_FileName[i];
        CompilerRules += "\" ";

        // Optimisation rules
        if( !x_stricmp( m_OptLevel,"Agressive" ))
        {
            CompilerRules += "-O3 ";
        }

        // Preferred format rules
        if( x_stricmp( m_PreferFormat, "Palettice 8" ) == 0)
        {
            CompilerRules += "-P8 ";
        }
        else if( x_stricmp( m_PreferFormat, "Palettice 4" ) == 0)
        {
            CompilerRules += "-P4 ";
        }
        else if( x_stricmp( m_PreferFormat, "Compress" ) == 0)
        {
            CompilerRules += "-COM ";
        }
        else if( x_stricmp( m_PreferFormat, "True Color(32bits)" ) == 0)
        {
            CompilerRules += "-TC ";
        }
    }
}

//=========================================================================
// BITMAP DESC
//=========================================================================

//=========================================================================

bitmap_desc::bitmap_desc( void ) : rsc_desc( s_Desc ) 
{
    m_FileName[0]=0;
    x_strcpy( m_PreferFormat, "Palettice 8" );
    x_strcpy( m_OptLevel    , "Agressive" );
}

//=========================================================================

void bitmap_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\BitmapName", 
                            "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                            "This is the source picture to be compile", 0 );

    List.PropEnumEnum    ( "ResDesc\\Prefer Format", "Palettice 8\0Palettice 4\0Compress\0True Color(32bits)\0Shadow\0Palette Only 8\0Palette Only 4\0",
                            "Choose the format which you would like your bitmap to be compress at.", 0 );

    List.PropEnumEnum    ( "ResDesc\\Optimisation", "Agressive\0None",
                            "Choose the optimisation level.", 0 );
}

//=========================================================================

xbool bitmap_desc::OnProperty( prop_query& I )
{
    if( rsc_desc::OnProperty( I ) )
    {
        // Nothing to do
    }
    else if( I.VarFileName( "ResDesc\\BitmapName", m_FileName, 256 ) )
    {
        // .. Nothing to do
    }
    else if( I.VarEnum( "ResDesc\\Prefer Format", m_PreferFormat ) )
    {
        // .. Nothing to do
    }
    else if( I.VarEnum( "ResDesc\\Optimisation", m_OptLevel ) )
    {
        // .. Nothing to do
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void bitmap_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    if( m_FileName[0] == 0 )
    {
        x_throw( "You must enter a file name for the bitmap." );
    }
}

//=========================================================================

void bitmap_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();

    List.Append() = m_FileName;
}

//=========================================================================

void bitmap_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "BitmapCompiler.exe ";
    CompilerRules += "-APPEND \"";
    CompilerRules += m_FileName;
    CompilerRules += "\" ";

    // Optimisation rules
    if( !x_stricmp( m_OptLevel,"Agressive" ))
    {
        CompilerRules += "-O3 ";
    }

    // Preferred format rules
    if( x_stricmp( m_PreferFormat, "Palettice 8" ) == 0)
    {
        CompilerRules += "-P8 ";
    }
    else if( x_stricmp( m_PreferFormat, "Palettice 4" ) == 0)
    {
        CompilerRules += "-P4 ";
    }
    else if( x_stricmp( m_PreferFormat, "Compress" ) == 0)
    {
        CompilerRules += "-COM ";
    }
    else if( x_stricmp( m_PreferFormat, "True Color(32bits)" ) == 0)
    {
        CompilerRules += "-TC ";
    }
    else if( x_stricmp( m_PreferFormat, "Shadow" ) == 0)
    {
        CompilerRules += "-SHAD ";
    }
    else if( x_stricmp( m_PreferFormat, "Palette Only 8" ) == 0)
    {
        CompilerRules += "-PAL8 ";
    }
    else if( x_stricmp( m_PreferFormat, "Palette Only 4" ) == 0)
    {
        CompilerRules += "-PAL4 ";
    }
}

//=========================================================================

Bitmapmotion_ed::Bitmapmotion_ed( void )
{
    m_pBitmapDesc = NULL;
    m_pEnvmapDesc = NULL;
}

//=========================================================================

Bitmapmotion_ed::~Bitmapmotion_ed( void )
{
    EndEdit();
}
    
//=========================================================================

void Bitmapmotion_ed::OnEnumProp( prop_enum& List )
{
    if( m_pBitmapDesc )
        m_pBitmapDesc->OnEnumProp( List );

    if( m_pEnvmapDesc )
        m_pEnvmapDesc->OnEnumProp( List );
}

//=========================================================================

xbool Bitmapmotion_ed::OnProperty( prop_query& I )
{
    if( m_pBitmapDesc )
        return m_pBitmapDesc->OnProperty( I );    

    if( m_pEnvmapDesc )
        return m_pEnvmapDesc->OnProperty( I );    


    return FALSE;
}

//=========================================================================

void Bitmapmotion_ed::Save( void )
{
    if( m_pBitmapDesc == NULL && m_pEnvmapDesc == NULL )
        x_throw ("There is not package open or created" );

    if( m_pBitmapDesc )
        g_RescDescMGR.Save( *m_pBitmapDesc );

    if( m_pEnvmapDesc )
        g_RescDescMGR.Save( *m_pEnvmapDesc );
}

//=========================================================================
xbool Bitmapmotion_ed::NeedSave( void )
{
    return (m_pBitmapDesc != NULL && m_pBitmapDesc->IsChanged()) ||
           (m_pEnvmapDesc != NULL && m_pEnvmapDesc->IsChanged());
}

//=========================================================================

void Bitmapmotion_ed::EndEdit( void )
{
    if( m_pBitmapDesc )
    {
        m_pBitmapDesc->SetBeingEdited( FALSE );
        m_pBitmapDesc = NULL;
    }

    if( m_pEnvmapDesc )
    {
        m_pEnvmapDesc->SetBeingEdited( FALSE );
        m_pEnvmapDesc = NULL;
    }
}

//=========================================================================

void Bitmapmotion_ed::NewBitmap( void )
{
    EndEdit();
    BeginEdit( bitmap_desc::GetSafeType( g_RescDescMGR.CreateRscDesc( ".xbmp" ) ) );
}

//=========================================================================

void Bitmapmotion_ed::NewEnvmap( void )
{
    EndEdit();
    BeginEdit( envmap_desc::GetSafeType( g_RescDescMGR.CreateRscDesc( ".envmap" ) ) );
}


//=========================================================================

void Bitmapmotion_ed::Load( const char* pFileName )
{
    EndEdit();

    char Ext[256];
    x_splitpath( pFileName, NULL, NULL, NULL, Ext );

    if( x_stricmp( "xbmp", Ext ) == 0 )
    {
        BeginEdit( bitmap_desc::GetSafeType( g_RescDescMGR.Load( pFileName ) ) );
    }
    else if( x_stricmp( "envmap", Ext ) == 0 )
    {
        BeginEdit( envmap_desc::GetSafeType( g_RescDescMGR.Load( pFileName ) ) );
    }
}

//=========================================================================

void Bitmapmotion_ed::BeginEdit( bitmap_desc& BitmapDesc )
{
    BitmapDesc.SetBeingEdited( TRUE );
    m_pBitmapDesc = &BitmapDesc;
}

//=========================================================================

void Bitmapmotion_ed::BeginEdit( envmap_desc& EnvmapDesc )
{
    EnvmapDesc.SetBeingEdited( TRUE );
    m_pEnvmapDesc = &EnvmapDesc;
}


