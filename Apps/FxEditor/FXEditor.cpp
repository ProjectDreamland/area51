#include "stdafx.h"
#include "FxEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
//GLOBAL VARIABLE USED FOR ANONYMOUS REGISTERATION
//=========================================================================

s32 g_fx_link = 0;

//=========================================================================

DEFINE_RSC_TYPE( s_Desc, fx_desc, "fxo", "FX Files (*.fxo)|*.fxo", "A FX file." );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

fx_desc::fx_desc( void ) : rsc_desc( s_Desc ) 
{
    
}

//=========================================================================

void fx_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );
  
    List.PropEnumFileName("ResDesc\\FX File", "FX Source Files (*.fxs)|*.fxs", "Source file of an FX." , PROP_TYPE_MUST_ENUM);
}

//=========================================================================

xbool fx_desc::OnProperty( prop_query& I )
{
    if ( rsc_desc::OnProperty( I ) )
        return TRUE;

    if (I.VarFileName( "ResDesc\\FX File", m_FxsFile.Get(), m_FxsFile.MaxLen() ))
        return TRUE;
    
    return FALSE;
}

//=========================================================================

void fx_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();
}

//=========================================================================

void fx_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();

    // Append the fxs file
    if (!m_FxsFile.IsEmpty())
        List.Append( m_FxsFile.Get() );

    // Read the fxs file and parse for textures to export
    xstring File;
    if( File.LoadFile( m_FxsFile.Get() ) )
    {
        s32 Index = -1;
        Index = File.Find( "STRING \"Bitmap\"" );
        while( Index != -1 )
        {
            // Advance to the name
            Index += 15;

            // Read the bitmap name
            while( isspace( File[Index] ) )
                Index++;
            if( File[Index] == '"' )
            {
                Index++;
                s32 Start = Index;
                while( File[Index] && (File[Index] != '"') && File[Index] != '\n' )
                    Index++;
                xstring FileName = File.Mid( Start, Index-Start );
                List.Append( FileName );
            }

            // Find the next one
            Index = File.Find( "STRING \"Bitmap\"", Index );
        }
    }
}

//=========================================================================

struct info_bin_file
{
    u32 MagicNumber;
    s32 TotalSize;
    u32 Flags;
    s32 NStagingAreaValues;
    s32 NControllers;
    s32 NElements;
    s32 NBitmaps;
};

void fx_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    //
    // Read in the file
    //
    X_FILE* Fp = x_fopen( xfs( "%s\\%s", pDirectory, GetName() ), "rb" );
    if( Fp == NULL )
    {
        x_throw( xfs("Unable to open [%s] So I can't check for dependencies", xfs( "%s\\%s", pDirectory, GetName())) );
    }
    
    s32 Count = x_flength( Fp );
    char* pFxBuff = new char[Count];
    if( pFxBuff == NULL )
        x_throw( "Out of memory" );

    x_fread( pFxBuff, Count, 1, Fp );
    x_fclose( Fp );

    //
    // Add all the dependencies 
    //
    info_bin_file* pInfo = (info_bin_file*)pFxBuff;
    const char*    pBuff = &pFxBuff[Count-1];

    for(s32 i=0; i<pInfo->NBitmaps; i++ )
    {
        // Skip the zeros.
        while( *pBuff == 0 ) pBuff--;

        // Search backwards to find the start of the string.
        while( *pBuff != 0 ) pBuff--;

        // Add string to the dependency list
        List.Append() = (pBuff+1);
    }

    //
    // Free temp memory
    //
    delete []pFxBuff;
}

//=========================================================================

void fx_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "fx_Export.exe ";
    CompilerRules += m_FxsFile.Get();
}
