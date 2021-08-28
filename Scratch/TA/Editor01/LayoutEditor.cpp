
#include "LayoutEditor.hpp"
#include <stdio.h>
#include <io.h>
#include "x_files.hpp"
#include "Geometry\GeomMgr.hpp"
#include "Parsing\TextOut.hpp"
#include "Parsing\TextIn.hpp"
#include "Geometry\TextureMgr.hpp"

//=========================================================================

layout_editor::layout_editor( void )
{
    m_hUndoCursor.Handle = HNULL;
    m_WorkingDir[0]      = 0;
    m_bUpdateUndo        = TRUE;
    m_AmbientLight.Set( 16, 16, 16, 255 );

    m_lGuid2Handle.SetCapacity( 10000, TRUE );

    // Initialize the guid system
    guid_Init();
}

//=========================================================================

void layout_editor::SetWorkingDir( const char* pWorkingDir )
{
    x_strcpy( m_WorkingDir, pWorkingDir );
}

//=========================================================================

s32 layout_editor::GetDescIndexByName( const char* pName )
{
    // Not name then no description
    if( pName == NULL ) 
        return -1;

    //
    // Do we have that file?
    //
    for( s32 i=0; i<m_lPieceDesc.GetCount(); i++ )
    {
        if( x_stricmp( m_lPieceDesc[i].FileName, pName ) == 0 )
            return i;
    }

    return -1;
}

//=========================================================================

xhandle layout_editor::GetDescHandleByName( const char* pName )
{
    s32 Index = GetDescIndexByName( pName );

    if( Index < 0 ) 
        return xhandle( HNULL );

    return m_lPieceDesc.GetHandleByIndex( Index );
}

//=========================================================================

s32 layout_editor::GetNDescs( void )
{
    return m_lPieceDesc.GetCount();
}

//=========================================================================

layout_editor::piece_desc& layout_editor::GetDesc( s32 Index )
{
    if( Index < 0 || Index >= m_lPieceDesc.GetCount() ) 
        e_throw( "Unable to get the requested piece" );

    return m_lPieceDesc[Index];
}

//=========================================================================

layout_editor::piece_desc& layout_editor::GetDesc( xhandle hDesc )
{
    if( hDesc.IsNull() )
        e_throw( "Invalid handle for a description" );

    return m_lPieceDesc( hDesc );
}

//=========================================================================

layout_editor::piece_desc& layout_editor::GetDesc( const char* pName )
{
    return GetDesc( GetDescIndexByName( pName ) );
}

//=========================================================================

s32 layout_editor::GetPfxDescIndexByName  ( const char* pName )
{
    // Not name then no description
    if( pName == NULL ) 
        return -1;

    //
    // Do we have that file?
    //
    for( s32 i=0; i<m_lPfxDesc.GetCount(); i++ )
    {
        if( x_stricmp( m_lPfxDesc[i].Name, pName ) == 0 )
            return i;
    }

    return -1;
}

//=========================================================================

xhandle layout_editor::GetPfxDescHandleByName ( const char* pName )
{
    s32 Index = GetPfxDescIndexByName( pName );

    if( Index < 0 ) 
        return xhandle( HNULL );

    return m_lPfxDesc.GetHandleByIndex( Index );
}

//=========================================================================

s32 layout_editor::GetSoundDescIndexByName( const char* pName )
{
    // Not name then no description
    if( pName == NULL ) 
        return -1;

    //
    // Do we have that file?
    //
    for( s32 i=0; i<m_lSoundDesc.GetCount(); i++ )
    {
        if( x_stricmp( m_lSoundDesc[i].Name, pName ) == 0 )
            return i;
    }

    return -1;
}

//=========================================================================

xhandle layout_editor::GetSoundDescHandleByName( const char* pName )
{
    s32 Index = GetSoundDescIndexByName( pName );

    if( Index < 0 ) 
        return xhandle( HNULL );

    return m_lSoundDesc.GetHandleByIndex( Index );
}

//=========================================================================

s32 layout_editor::GetSoundNDescs( void )
{
    return m_lSoundDesc.GetCount();
}

//=========================================================================

layout_editor::sound_desc& layout_editor::GetSoundDesc( s32 Index )
{
    if( Index < 0 || Index >= m_lSoundDesc.GetCount() ) 
        e_throw( "Unable to get the requested piece" );

    return m_lSoundDesc[Index];
}

//=========================================================================

layout_editor::sound_desc& layout_editor::GetSoundDesc( xhandle hDesc )
{
    if( hDesc.IsNull() )
        e_throw( "Invalid handle for a description" );

    return m_lSoundDesc( hDesc );
}

//=========================================================================

layout_editor::sound_desc& layout_editor::GetSoundDesc( const char* pName )
{
    return GetSoundDesc( GetSoundDescIndexByName( pName ) );
}

//=========================================================================
// TODO: Check for time stamps
void layout_editor::RefreshAllFiles( void )
{

    //
    // Deal with the wall descriptions
    //
    {
        _finddata_t     c_file;
        long            hFile;

        if( (hFile = _findfirst( xfs( "%s\\*.wallgeom", m_WorkingDir ), &c_file )) == -1L )
           return;

        // Get the first file in there
        AddPieceDescFile( c_file.name );

        // Find the rest of the .c files 
        while( _findnext( hFile, &c_file ) == 0 )
        {
            AddPieceDescFile( c_file.name );
        }

        _findclose( hFile );
    }

    //
    // Deal with the sound types
    //
    {
        _finddata_t     c_file;
        long            hFile;

        if( (hFile = _findfirst( xfs( "%s\\*.hpp", m_WorkingDir ), &c_file )) == -1L )
           return;

        // Get the first file in there
        AddSoundDescFile( c_file.name );

        // Find the rest of the .c files 
        while( _findnext( hFile, &c_file ) == 0 )
        {
            AddSoundDescFile( c_file.name );
        }

        _findclose( hFile );

        // read the magical file
//        fopend( "c\gamedata\release\pc\sound.c" );        
    }

    InitParticleFXList();
    InitPickupList();

}

//=========================================================================

void layout_editor::AddSoundDescFile( const char* pFileName )
{
    char        Buffer[512];

    FILE* Fp = fopen(  xfs( "%s\\%s", m_WorkingDir, pFileName ), "rt" );
    if( Fp == 0 )
        e_throw( xfs("Unalbe to load the sound descriptor file [%s]", pFileName) );

    //
    // Do the actual loading of the descriptor
    //

    // First search for the beging of the lavel
    char* pTitle=NULL;
    do
    {
        if( fscanf( Fp, "%s", Buffer ) != 1 )
            e_throw( xfs("Error while searching for the 'BEGIN_LABEL_SET' in [%s] ", pFileName) );

         pTitle = x_strstr( Buffer, "BEGIN_LABEL_SET(" );

    } while( pTitle == NULL );

    fgets( Buffer, 512, Fp );

    //
    // Read all the descritor into the system
    //
    do
    {
        
        // Read in the lavel
        if( fgets( Buffer, 512, Fp ) == 0 )
            e_throw ( xfs("Error reading file[%s] ", pFileName) );

        if( x_stristr( Buffer, "END_LABEL_SET(" ) ) 
            break;

        sound_desc& Desc = m_lSoundDesc.Add();
        char* pParse;
        s32   i;

        // Read name
        pParse = x_stristr( Buffer, "(" )+1;
        for( i=0; pParse[i] !=' ' && pParse[i] !=','; i++ )
            Desc.Name[i] = pParse[i];
        Desc.Name[i] = 0;
        
        // Read number        
        pParse = x_stristr( Buffer, "," )+1;
        Desc.ID = x_atoi( pParse );

    } while(1);

    fclose( Fp );
}

//=========================================================================

void layout_editor::InitParticleFXList  ( void )
{
    // add the effects to the list
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[0].Name, "Steam Small" );        m_lPfxDesc[0].ID = 0;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[1].Name, "Steam Medium" );       m_lPfxDesc[1].ID = 1;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[2].Name, "Steam Large" );        m_lPfxDesc[2].ID = 2;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[3].Name, "Blood Steam" );        m_lPfxDesc[3].ID = 3;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[4].Name, "Spark Const" );        m_lPfxDesc[4].ID = 4;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[5].Name, "Smoke Const" );        m_lPfxDesc[5].ID = 5;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[6].Name, "Fire S Const" );       m_lPfxDesc[6].ID = 6;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[7].Name, "Fire M Const" );       m_lPfxDesc[7].ID = 7;
    m_lPfxDesc.Add();   x_strcpy( m_lPfxDesc[8].Name, "Spore Const" );       m_lPfxDesc[8].ID = 8;

}

//=========================================================================

void layout_editor::InitPickupList  ( void )
{
    // add the effects to the list
    m_lPickupDesc.Add();   x_strcpy( m_lPickupDesc[0].Name, "Health" );     m_lPickupDesc[0].Type = 0;
    m_lPickupDesc.Add();   x_strcpy( m_lPickupDesc[1].Name, "Ammo" );       m_lPickupDesc[1].Type = 1;
    m_lPickupDesc.Add();   x_strcpy( m_lPickupDesc[2].Name, "Item" );       m_lPickupDesc[2].Type = 2;
}

//=========================================================================

s32 layout_editor::GetNumFXTypes       ( void )
{
    return m_lPfxDesc.GetCount();
}

//=========================================================================

void layout_editor::GetFXType           ( int Idx, char* pName, s32& ID )
{
    x_strcpy( pName, m_lPfxDesc[Idx].Name );
    ID = m_lPfxDesc[Idx].ID;
}

//=========================================================================

void layout_editor::AddPieceDescFile( const char* pFileName )
{
    piece_desc  Desc;

    //
    // Do we have this piece already?
    //
    if( GetDescIndexByName( pFileName ) != - 1 )
        return;

    // We could not find it so load the new piece
    {
        s32    i;
        fileio File;
        x_strcpy( Desc.FileName, pFileName );
        File.Load( xfs( "%s\\%s", m_WorkingDir, pFileName ), Desc.pWallGeom );

        Desc.hGeom      = gman_Register( *Desc.pWallGeom );
        wall_geom& Geom = gman_GetGeom( Desc.hGeom );

        //
        // Compute the surface area for this piece
        //
        wall_geom::pc& PCWall = *Desc.pWallGeom->m_System.pPC;
        Desc.SurfaceArea      = 0;
        for( i=0; i<PCWall.nIndices; i+=3 )
        {
            vector3 V1      = PCWall.pVertex[ PCWall.pIndex[i+0] ].Pos;
            vector3 V2      = PCWall.pVertex[ PCWall.pIndex[i+1] ].Pos;
            vector3 V3      = PCWall.pVertex[ PCWall.pIndex[i+2] ].Pos;
            vector3 C       = (( V3 - V2 ).Cross( V1 - V2 ));
            f32     SufA    = C.Length() / 2;
            Desc.SurfaceArea += SufA;
        }

        //
        // Load Textures if we have to
        //
        for( i=0; i<Geom.m_nTextures; i++ )
        {
            xhandle hTex = texmgr_Register( Geom.m_pTexture[i].FileName );
            Geom.m_pMaterial[i].iTexture = hTex.Handle;
        }

        /*
        X_FILE* Fp = x_fopen( "test.txt", "wt" );

        ASSERT( Desc.pGeom->m_nDLists == 1 );
        x_fprintf( Fp, "\n\n%d\n", Desc.pGeom->m_nIndices );
        for( s32 i=0; i<Desc.pGeom->m_nIndices;i++ )
        {
            x_fprintf( Fp, "%d\n", (s32)Desc.pGeom->m_pIndex[i] );
        }

        x_fprintf( Fp, "\n\n%d\n", Desc.pGeom->m_nVerts );
        for( i=0; i<Desc.pGeom->m_nVerts;i++ )
        {
            ASSERT( x_abs( Desc.pGeom->m_pVertex[ i ].Pos.X ) < 500000.0f );
            ASSERT( x_abs( Desc.pGeom->m_pVertex[ i ].Pos.Y ) < 500000.0f );
            ASSERT( x_abs( Desc.pGeom->m_pVertex[ i ].Pos.Z ) < 500000.0f );

            x_fprintf( Fp, "%f %f %f\n", 
                Desc.pGeom->m_pVertex[i].Pos.X, 
                Desc.pGeom->m_pVertex[i].Pos.Y, 
                Desc.pGeom->m_pVertex[i].Pos.Z );
        }
        x_fclose( Fp );
        */
    }

    //
    // Add the new piece into our list
    //
    m_lPieceDesc.Add() = Desc;
}

//=========================================================================

void layout_editor::AddInst( xhandle hDesc, matrix4& L2W, u32 Flag, char* pObjectName)
{
    if( hDesc.IsNull() )
        e_throw( "Unable to add that piece into the world" );

    //
    // Add the actual piece into the world
    //
    xhandle     hInst;
    piece_inst& T = m_lPieceInst.Add( hInst );
    T.Guid        = guid_New();
    T.hDesc       = hDesc;
    T.L2W         = L2W;
    T.W2L         = m4_InvertSRT( L2W );
    T.Flags       = Flag;
    
    if( pObjectName == NULL )
        T.ObjectName[0] = 0;
    else
        x_strcpy( T.ObjectName, pObjectName );

    T.hGeom       = gman_Register( *m_lPieceDesc( hDesc ).pWallGeom );
    InstUpdateLight( hInst );

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        piece_inst&     Data    = Undo.GetInstace();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }
}

//=========================================================================

void layout_editor::AddLight( sphere& Light, xcolor Color, f32 Intensity )
{
    //
    // Add the actual light into the world
    //
    xhandle     hInst;
    light&      T = m_lLight.Add( hInst );
    T.Guid        = guid_New();
    T.Sphere      = Light;
    T.Color       = Color;
    T.Intensity   = Intensity;
    T.Flags       = 0;

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        light&          Data    = Undo.GetLight();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }
}

//=========================================================================

void layout_editor::AddEffect( vector3& Point, vector3& Normal, s32 Type )
{
    //
    // Add a particle effect into the world
    //
    xhandle     hInst;
    pfx_inst&      T = m_lPfxInst.Add( hInst );
    T.Guid        = guid_New();
    T.Sphere.Pos  = Point;
    T.Sphere.R    = PFX_SPHERE_RADIUS;
    T.Normal      = Normal;
    T.hDesc       = m_lPfxDesc.GetHandleByIndex( Type );
    T.Flags       = 0;

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        pfx_inst&       Data    = Undo.GetPfx();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }
}

//=========================================================================
s32 layout_editor::GetLightCount( void ) const
{
    return m_lLight.GetCount();
}

//=========================================================================

layout_editor::light& layout_editor::GetLight( s32 I )
{
    return m_lLight[I];
}


//=========================================================================

void layout_editor::AddInst( const char* pDescName, matrix4& Matrix, u32 Flag, char* pObjectName)
{
    // Add the instance
    xhandle Handle = GetDescHandleByName( pDescName );
    AddInst( Handle, Matrix, Flag, pObjectName );
}

//=========================================================================

xbool layout_editor::IsModified( void )
{
    return m_hUndoCursor.IsNull() == FALSE;
}

//=========================================================================

void layout_editor::DeleteInst( s32 Index )
{
    if( Index <= 0 || Index >= m_lPieceInst.GetCount() )
        e_throw( "Unable to delete the instance" );

    // Delete the piece from the world
    piece_inst& T = m_lPieceInst[ Index ];

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        piece_inst&     Data    = Undo.GetInstace();

        Data = T;

        Command.RedoCommand     = COMMAND_DEL_INST;    
        Command.UndoCommand     = COMMAND_ADD_INST;    
    }
    
    //
    // Do the actual deletion
    //

    // update the guid lookup
    s32 I = m_lGuid2Handle.GetIndex( T.Guid );
    ASSERT( I != -1 );
    m_lGuid2Handle.GetS32( I ) = -1;

    // Now remove the node from the list
    m_lPieceInst.DeleteByIndex( Index );
}

//=========================================================================

void layout_editor::DeleteInst( xhandle hInst )
{
    if( hInst.IsNull() )
        e_throw( "Unable to delete instance" );

    DeleteInst( m_lPieceInst.GetIndexByHandle( hInst ) );
}

//=========================================================================

void layout_editor::UnselectAll( void )
{
    s32 i;

    for( i=0; i<m_lPieceInst.GetCount();i++ )
    {
        m_lPieceInst[i].Flags &= ~INST_FLAGS_SELECTED;
    }

    for( i=0; i<m_lSoundInst.GetCount();i++ )
    {
        m_lSoundInst[i].Flags &= ~INST_FLAGS_SELECTED;
    }

    for( i=0; i<m_lLight.GetCount();i++ )
    {
        m_lLight[i].Flags &= ~INST_FLAGS_SELECTED;
    }

    for( i=0; i<m_lPfxInst.GetCount();i++ )
    {
        m_lPfxInst[i].Flags &= ~INST_FLAGS_SELECTED;
    }

    for( i=0; i<m_lPickupInst.GetCount();i++ )
    {
        m_lPickupInst[i].Flags &= ~INST_FLAGS_SELECTED;
    }
    for( i=0; i<m_lGlow.GetCount();i++ )
    {
        m_lGlow[i].Flags &= ~INST_FLAGS_SELECTED;
    }

}

//=========================================================================

void layout_editor::DeleteSelection( void )
{
    //
    // First lest make sure that we have something to delete
    //
    {
        s32     i;
        xbool   bDelete = FALSE;

        for( i=0; i<m_lPieceInst.GetCount();i++ )
        {
            if( m_lPieceInst[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }

        for( i=0; i<m_lLight.GetCount();i++ )
        {
            if( m_lLight[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }
        
        for( i=0; i < m_lSoundInst.GetCount(); i++ )
        {
            if( m_lSoundInst[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }

        for( i=0; i < m_lPfxInst.GetCount(); i++ )
        {
            if( m_lPfxInst[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }

        for( i=0; i < m_lPickupInst.GetCount(); i++ )
        {
            if( m_lPickupInst[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }
        for( i=0; i<m_lGlow.GetCount();i++ )
        {
            if( m_lGlow[i].Flags & INST_FLAGS_SELECTED )
            {
                bDelete = TRUE;
                break;
            }
        }

        if( bDelete == FALSE ) 
            e_throw( "There is nothing selected to delete" );
    }

    //
    // Lets set get it ready for the undo
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );

        Command.RedoCommand     = COMMAND_DEL_INST;    
        Command.UndoCommand     = COMMAND_ADD_INST;    

        for( s32 i=0; i<m_lPieceInst.GetCount(); i++ )
        {
            if( m_lPieceInst[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                piece_inst&     Data    = Undo.GetInstace();
                Data = m_lPieceInst[i];
            }
        }

        for( i=0; i<m_lLight.GetCount(); i++ )
        {
            if( m_lLight[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                light&          Data    = Undo.GetLight();
                Data = m_lLight[i];
            }
        }

        for( i=0; i < m_lSoundInst.GetCount(); i++ )
        {
            if( m_lSoundInst[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                sound_inst&     Data    = Undo.GetSound();
                Data = m_lSoundInst[i];
            }
        }

        for( i=0; i < m_lPfxInst.GetCount(); i++ )
        {
            if( m_lPfxInst[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                pfx_inst&       Data    = Undo.GetPfx();
                Data = m_lPfxInst[i];
            }
        }

        for( i=0; i < m_lPickupInst.GetCount(); i++ )
        {
            if( m_lPickupInst[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                pickup_inst&    Data    = Undo.GetPickup();
                Data = m_lPickupInst[i];
            }
        }

        for( i=0; i<m_lGlow.GetCount(); i++ )
        {
            if( m_lGlow[i].Flags & INST_FLAGS_SELECTED )
            {
                undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
                glow&          Data     = Undo.GetGlow();
                Data = m_lGlow[i];
            }
        }

    }

    // Do the actual deletion
    HandleCommand( COMMAND_DEL_INST );
}

//=========================================================================

s32 layout_editor::GetNInsts( void )
{
    return m_lPieceInst.GetCount();
}

//=========================================================================

layout_editor::piece_inst& layout_editor::GetInst( s32 Index )
{
    if( Index < 0 || Index >= m_lPieceInst.GetCount() ) 
        e_throw( "Unable to get instance" );

    return m_lPieceInst[ Index ];
}

//=========================================================================

xbool layout_editor::UpdateUndo( void )
{
    if( m_bUpdateUndo == FALSE )
        return FALSE;

    //
    // Okay now it is time to update the undo
    //
    xhandle hNext = m_lUndo.GetFirstList();
    while( hNext != m_hUndoCursor )
    {
        m_lUndo.DelList( hNext );
        hNext = m_lUndo.GetFirstList();
    }

    // Make sure that the cursor points always to the last node
    m_hUndoCursor = m_lUndo.GetFirstList();

    return TRUE;
}

//=========================================================================

void layout_editor::HandleCommand( command_type Command )
{
    s32 bReLightLevel = FALSE;
    //
    // Process command
    //
    switch( Command )
    {
    case COMMAND_ADD_INST: 
        {
            // 
            // We have to delete all the instance 
            // 
            slist_hnode hNode = m_lUndo.GetFirstNode( m_hUndoCursor );
            while( hNode.IsNull() == FALSE )
            {
                undo_data& Undo = m_lUndo.GetNode( hNode );

                if( Undo.Type == undo_data::TYPE_INSTANCE )
                {
                    piece_inst& Inst    = Undo.GetInstace();

                    s32 Index = m_lGuid2Handle.GetIndex( Inst.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an instance didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lPieceInst.Add( hInst ) = Inst;
                    Handle     = hInst;

                    //
                    // Add the geometry 
                    // 
                    m_lPieceInst(hInst).hGeom       = gman_Register( *m_lPieceDesc( Inst.hDesc ).pWallGeom );

                    // Make sure to light it.
                    InstUpdateLight(hInst);
                }

                if( Undo.Type == undo_data::TYPE_LIGHT )
                {
                    light& Light    = Undo.GetLight();

                    bReLightLevel = TRUE;
                    s32 Index = m_lGuid2Handle.GetIndex( Light.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lLight.Add( hInst ) = Light;
                    Handle     = hInst;
                }


                if( Undo.Type == undo_data::TYPE_SOUND )
                {
                    sound_inst& Sound    = Undo.GetSound();

                    s32 Index = m_lGuid2Handle.GetIndex( Sound.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lSoundInst.Add( hInst ) = Sound;
                    Handle     = hInst;
                }
    
                if( Undo.Type == undo_data::TYPE_PFX )
                {
                    pfx_inst& Pfx    = Undo.GetPfx();

                    s32 Index = m_lGuid2Handle.GetIndex( Pfx.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lPfxInst.Add( hInst ) = Pfx;
                    Handle     = hInst;
                }

                if( Undo.Type == undo_data::TYPE_PICKUP )
                {
                    pickup_inst& Pickup    = Undo.GetPickup();

                    s32 Index = m_lGuid2Handle.GetIndex( Pickup.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lPickupInst.Add( hInst ) = Pickup;
                    Handle     = hInst;
                }

                if( Undo.Type == undo_data::TYPE_GLOW )
                {
                    glow& Glow    = Undo.GetGlow();

                    s32 Index = m_lGuid2Handle.GetIndex( Glow.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an Glow didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle != -1 )
                        e_throw( "Internal Error. GUID contain a valid handle" );

                    // Set the new handle into the guid lookup
                    xhandle     hInst;
                    m_lGlow.Add( hInst ) = Glow;
                    Handle     = hInst;
                }

                // Get the next instance to add
                hNode = m_lUndo.GetNextNode( m_hUndoCursor, hNode );                
            }
            break;
        }

    case COMMAND_DEL_INST: 
        {
            // 
            // We have to delete all the instance 
            // 
            slist_hnode hNode = m_lUndo.GetFirstNode( m_hUndoCursor );
            while( hNode.IsNull() == FALSE )
            {
                undo_data& Undo = m_lUndo.GetNode( hNode );

                if( Undo.Type == undo_data::TYPE_INSTANCE )
                {
                    piece_inst& Inst    = Undo.GetInstace();

                    s32 Index = m_lGuid2Handle.GetIndex( Inst.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an instance didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    //
                    // Del the geometry 
                    // 
                    gman_Unregister( Inst.hGeom );

                    // Delete the piece
                    m_lPieceInst.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                if( Undo.Type == undo_data::TYPE_LIGHT )
                {
                    light& Light    = Undo.GetLight();

                    bReLightLevel = TRUE;

                    s32 Index = m_lGuid2Handle.GetIndex( Light.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    // Delete the piece
                    m_lLight.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                if( Undo.Type == undo_data::TYPE_SOUND )
                {
                    sound_inst& Sound    = Undo.GetSound();

                    s32 Index = m_lGuid2Handle.GetIndex( Sound.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    // Delete the sound
                    m_lSoundInst.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                if( Undo.Type == undo_data::TYPE_PFX )
                {
                    pfx_inst& Pfx    = Undo.GetPfx();

                    s32 Index = m_lGuid2Handle.GetIndex( Pfx.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    // Delete the effect
                    m_lPfxInst.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                if( Undo.Type == undo_data::TYPE_PICKUP )
                {
                    pickup_inst& Pickup    = Undo.GetPickup();

                    s32 Index = m_lGuid2Handle.GetIndex( Pickup.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an light didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    // Delete the Pickup
                    m_lPickupInst.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                if( Undo.Type == undo_data::TYPE_GLOW )
                {
                    glow& Glow    = Undo.GetGlow();

                    s32 Index = m_lGuid2Handle.GetIndex( Glow.Guid );
                    if( Index == -1 )
                        e_throw( "Internal Error. GUID for an Glow didn't exits" );

                    s32& Handle = m_lGuid2Handle.GetS32( Index );
                    if( Handle == -1 )
                        e_throw( "Internal Error. GUID contain an invalid instance handle" );

                    // Delete the glow
                    m_lGlow.DeleteByHandle( xhandle(Handle) );

                    // Set the Handle to be NULL
                    Handle = -1;
                }

                // Move on to the next piece
                hNode = m_lUndo.GetNextNode( m_hUndoCursor, hNode );                
            }
            break;
        }
    }

    if( bReLightLevel )
        UpdateLighting();
}

//=========================================================================

void layout_editor::Undo( void )
{
    // Get the first command in the list if we are not
    // already in the middle of the list
    if( m_hUndoCursor.IsNull() ) 
        e_throw( "There is not more commands to undo" );

    //
    // Undo the command
    //
    command& Command = m_lUndo.GetList( m_hUndoCursor );    
    HandleCommand( Command.UndoCommand );

    //
    // Move the cursor
    //
    m_hUndoCursor = m_lUndo.GetNextList( m_hUndoCursor );
}

//=========================================================================

void layout_editor::Redo( void )
{
    xhandle hList = m_lUndo.GetFirstList();

    // Check wether we have something to redo
    if( m_hUndoCursor == hList )
        e_throw( "Nothing to redo" );

    //
    // Find the command before the last command.
    // This will be faster if the template class will have
    // a previous. But this is okay as well.
    //
    do 
    {
        xhandle hNext = m_lUndo.GetNextList( hList );
        if( hNext == m_hUndoCursor )
            break;
        hList = hNext;

    } while( hList.IsNonNull() );

    // Set the cursor for the command to redo
    m_hUndoCursor = hList;    

    //
    // Redo the command
    //
    command& Command = m_lUndo.GetList( m_hUndoCursor );    
    HandleCommand( Command.RedoCommand );
}

//=============================================================================
static
xbool ComputeRayTriCollision   ( const vector3* Tri,
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32&     T,
                                       vector3& Normal )
{
    s32 i;
    plane Plane;
    Plane.Setup( Tri[0], Tri[1], Tri[2] );

    // Set the normal
    Normal = Plane.Normal;

    // Are we completely in front or starting from behind?
    if ( !(Plane.InFront( Start ) && Plane.InBack( End )) )
    {
        return FALSE;
    }
        
    // Find where we hit the plane
    Plane.Intersect( T, Start, End );

    if ( (T < 0.0f) || (T > 1.0f) )
    {
        return FALSE;
    }

    vector3 HitPoint = Start + ((End - Start) * T);

    // See if hit point is inside tri
    vector3 EdgeNormal;
    
    for ( i = 0; i < 3; ++i )
    {
        EdgeNormal = Plane.Normal.Cross( Tri[(i+1)%3] - Tri[i] );
        if( EdgeNormal.Dot( HitPoint - Tri[i] ) < -0.001f )
        {
            return FALSE;
        }
    }

    // Collision
    return TRUE;
}


//=========================================================================

f32 layout_editor::GetHitPos( const vector3& P0, const vector3& P1, vector3& Normal )
{
    s32 i;
    s32 iInstance = -1;
    f32 BestT     = 999999999.9f;
    vector3 BestNormal;

    for( i=0; i<m_lPieceInst.GetCount(); i++ )
    {
        piece_inst&                 Inst   = m_lPieceInst[i];
        piece_desc&                 Desc   = GetDesc( Inst.hDesc );
        wall_geom&                  WGeom  = *Desc.pWallGeom;
        collision_volume&           Volume = WGeom.m_Volumen;
        collision_volume::volume&   Vol    = Volume.m_Prim.pVol[0];
        f32                         t;
        vector3                     p0, p1;

        p0 = Inst.W2L * P0;
        p1 = Inst.W2L * P1;
        
        if( Vol.BBox.Intersect( t, p0, p1 ) )
        {
            // Check for collisions
            for( s32 j=0; j<Vol.nTris; j++ )
            {
                if( ComputeRayTriCollision( Vol.pTri[j].P, p0, p1, t, BestNormal ) )
                {
                    if( t < BestT ) 
                    {
                        BestT = t;
                        iInstance = i;
                        Normal = BestNormal;
                    }
                }
            }
        }
    }

    //
    // Did we selected a piece
    //
    if( iInstance == -1 )
        return -1;

    return BestT;
}

//=========================================================================

f32 layout_editor::GetHitPos( const vector3& P0, const vector3& P1 )
{
    vector3 Normal;
    return GetHitPos( P0, P1, Normal );
}

//=========================================================================

void layout_editor::Select( vector3& P0, vector3& P1, xbool bSelect )
{
    undo_data::object_type  Type = undo_data::TYPE_NONE;
    s32                     i;
    s32                     iInstance = -1;
    f32                     BestT     = 999999999.9f;


    // Loop throw all the wall instances
    for( i=0; i<m_lPieceInst.GetCount(); i++ )
    {
        piece_inst&                 Inst   = m_lPieceInst[i];
        piece_desc&                 Desc   = GetDesc( Inst.hDesc );
        wall_geom&                  WGeom  = *Desc.pWallGeom;
        collision_volume&           Volume = WGeom.m_Volumen;
        collision_volume::volume&   Vol    = Volume.m_Prim.pVol[0];
        f32                         t;
        vector3                     p0, p1;

        p0 = Inst.W2L * P0;
        p1 = Inst.W2L * P1;
        
        if( Vol.BBox.Intersect( t, p0, p1 ) )
        {
            // Check for collisions
            for( s32 j=0; j<Vol.nTris; j++ )
            {
                vector3 Normal;
                if( ComputeRayTriCollision( Vol.pTri[j].P, p0, p1, t, Normal) )
                {
                    if( t < BestT ) 
                    {
                        BestT     = t;
                        iInstance = i;
                        Type      = undo_data::TYPE_INSTANCE;
                    }
                }
            }
        }
    }

    // Lopp throw all the lights
    for( i=0; i<m_lLight.GetCount(); i++ )
    {
        f32    t;
        light& Light = m_lLight[i];
        sphere Sphere;

        // Light are one meter in size (in the editor)
        Sphere = Light.Sphere;
        Sphere.R = 50;

        if( Sphere.Intersect( t, P0, P1 ) )
        {
            if( t < BestT ) 
            {
                BestT     = t;
                iInstance = i;
                Type      = undo_data::TYPE_LIGHT;
            }
        }
    }

    // Loop throw all the sounds.
    for( i=0; i<m_lSoundInst.GetCount(); i++ )
    {
        f32    t;
        sound_inst& Sound = m_lSoundInst[i];
        sphere Sphere;

        Sphere = Sound.Sphere;
        Sphere.R = SOUND_SPHERE_RADIUS;

        if( Sphere.Intersect( t, P0, P1 ) )
        {
            if( t < BestT ) 
            {
                BestT     = t;
                iInstance = i;
                Type      = undo_data::TYPE_SOUND;
            }
        }
    }

    // Loop throw all the Pfxs.
    for( i=0; i<m_lPfxInst.GetCount(); i++ )
    {
        f32    t;
        pfx_inst& Pfx = m_lPfxInst[i];
        sphere Sphere;

        Sphere = Pfx.Sphere;
        Sphere.R = PFX_SPHERE_RADIUS;

        if( Sphere.Intersect( t, P0, P1 ) )
        {
            if( t < BestT ) 
            {
                BestT     = t;
                iInstance = i;
                Type      = undo_data::TYPE_PFX;
            }
        }
    }

    // Loop throw all the Pickups.
    for( i=0; i<m_lPickupInst.GetCount(); i++ )
    {
        f32    t;
        pickup_inst& Pickup = m_lPickupInst[i];
        sphere Sphere;

        Sphere = Pickup.Sphere;
        Sphere.R = PICKUP_SPHERE_RADIUS;

        if( Sphere.Intersect( t, P0, P1 ) )
        {
            if( t < BestT ) 
            {
                BestT     = t;
                iInstance = i;
                Type      = undo_data::TYPE_PICKUP;
            }
        }
    }

    // Lopp throw all the glows
    for( i=0; i<m_lGlow.GetCount(); i++ )
    {
        f32    t;
        glow& Glow = m_lGlow[i];
        sphere Sphere;

        // Glow are one meter in size (in the editor)
        Sphere = Glow.Sphere;
        Sphere.R = 50;

        if( Sphere.Intersect( t, P0, P1 ) )
        {
            if( t < BestT ) 
            {
                BestT     = t;
                iInstance = i;
                Type      = undo_data::TYPE_GLOW;
            }
        }
    }

    //
    // Did we selected a piece
    //
    if( iInstance != -1 )
    {
        if( Type == undo_data::TYPE_INSTANCE )  
        {
            if( bSelect )
                m_lPieceInst[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lPieceInst[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        
        }
        else if( Type == undo_data::TYPE_LIGHT )
        {
            if( bSelect )
                m_lLight[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lLight[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        
        }
        else if( Type == undo_data::TYPE_SOUND )
        {
            if( bSelect )
                m_lSoundInst[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lSoundInst[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        

        }
        else if( Type == undo_data::TYPE_PFX )
        {
            if( bSelect )
                m_lPfxInst[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lPfxInst[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        

        }
        else if( Type == undo_data::TYPE_PICKUP )
        {
            if( bSelect )
                m_lPickupInst[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lPickupInst[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        

        }
        else if( Type == undo_data::TYPE_GLOW )
        {
            if( bSelect )
                m_lGlow[ iInstance ].Flags |= INST_FLAGS_SELECTED;
            else
                m_lGlow[ iInstance ].Flags &= ~INST_FLAGS_SELECTED;        
        }
    }
}

//=========================================================================
void layout_editor::NewProject( void )
{
    //
    // Make sure that the undo stuff is reset
    //
    m_lUndo.Clear();
    m_hUndoCursor.Handle = HNULL;
    m_lGuid2Handle.Clear();

    //
    // Clear all the instances
    //
    m_lPieceInst.Clear();

    //
    // Clear all the lights
    //
    m_lLight.Clear();

    //
    // Clear all the sounds instances
    //
    m_lSoundInst.Clear();

    //
    // Clear all the effects instances
    //
    m_lPfxInst.Clear();

    //
    // Clear all the pickup instances
    //
    m_lPickupInst.Clear();
    
    //
    // Clear all the glow instances
    //
    m_lGlow.Clear();
}

//=========================================================================
#include <windows.h>    // OutputDebugString()

void layout_editor::UpdateLighting( void )
{
    for( s32 i=0; i<GetNInsts(); i++ )
    {
        InstUpdateLight( i );
        OutputDebugString( xfs( "%d of %d\n", i,GetNInsts() ) );
    }
}

//=========================================================================

void layout_editor::GetAttenConstants( f32 Intensity, f32 AttenEnd, f32& k0, f32& k1, f32& k2 ) const
{
    f32 Brightness = Intensity - 1.0f;

    //
    //  Brightness must be clamped to [0,2)
    //
    if (Brightness < 0)
        Brightness = 0;
    if (Brightness > 1.99f)
        Brightness = 1.99f;

    //
    //  Currently, a brightness of 0 requires a radius divisor approx = 2.8
    //             a brightness of 2 requires a radius divisor approx = 3.3
    //
    f32 Divisor = 3.3f; 
    f32 Radius  = AttenEnd / Divisor;

    k0 = 1;
    k2 = 1.0f / (Radius*Radius);
    k1 = -Brightness * (f32)x_sqrt(k2*0.99f);
}

//=========================================================================

void layout_editor::InstUpdateLight( xhandle hInst )
{
    //InstUpdateFinalLight( hInst );
    //return;

    s32                   i;
    piece_inst&           Inst   = m_lPieceInst( hInst );
    piece_desc&           Desc   = GetDesc( Inst.hDesc );
    bbox&                 BBox   = Desc.pWallGeom->m_Volumen.m_Prim.pVol->BBox;
    s32                   nVerts = Desc.pWallGeom->m_System.pPC->nVerts;
    wall_geom::vertex_pc* pVert  = (wall_geom::vertex_pc*)gman_LockVertexData( Inst.hGeom );

    if( pVert == NULL )
        return;

    for( i=0; i<nVerts; i++ )
    {
        pVert[i].Color = m_AmbientLight;
    }
    
    for( i=0; i<m_lLight.GetCount(); i++ )
    {
        light& Light = m_lLight[i];
        f32                   K0,K1,K2;
        GetAttenConstants( Light.Intensity, Light.Sphere.R, K0, K1, K2 );

        
        vector3 Pos    = Inst.W2L * Light.Sphere.Pos;
        sphere  Sphere( Pos, Light.Sphere.R );
        bbox    BBoxL = Sphere.GetBBox();

        if( BBoxL.Intersect( BBox ) == FALSE )
            continue;

        for( s32 j=0; j<nVerts; j++ )
        {
            vector3 LDir = Pos - pVert[j].Pos;

            f32 d = LDir.Length();
            f32 I;
            if( x_abs( d ) < 0.0001f )
            {
                I = 1;
            }
            else
            {
                LDir *= (1/d);
                I = LDir.Dot( pVert[j].Normal );
            }

            f32 Att = 1/(K0 + (K1*d) + K2*d*d);
            pVert[j].Color += (Light.Color * (I*Att));
        }
    }

    gman_UnlockVertexData( Inst.hGeom );
}

//=========================================================================

void layout_editor::InstUpdateLight( s32 Index )
{   
    InstUpdateLight( m_lPieceInst.GetHandleByIndex( Index  ) );
}

//=========================================================================

void layout_editor::Save( const char* pFileName )
{
    text_out    TextOut;
    s32         i;

    TextOut.OpenFile( pFileName );

    TextOut.AddHeader( "DescInfo", m_lPieceDesc.GetCount() );
    for( i=0; i<m_lPieceDesc.GetCount(); i++ )
    {
        piece_desc& Desc = GetDesc( i );

        TextOut.AddField( "Desc:s",              Desc.FileName );
        switch( Desc.MatType )
        {
            case MAT_TYPE_NULL  : TextOut.AddField( "Mat:s", "NULL" );  break;
            case MAT_TYPE_WOOD  : TextOut.AddField( "Mat:s", "WOOD" );  break;
            case MAT_TYPE_METAL : TextOut.AddField( "Mat:s", "METAL" ); break;
            case MAT_TYPE_FLESH : TextOut.AddField( "Mat:s", "FLESH" ); break;
            case MAT_TYPE_STONE : TextOut.AddField( "Mat:s", "STONE" ); break;
        }

        TextOut.AddEndLine();
    }

    //
    // Save all the wall pieces
    //
    TextOut.AddHeader( "WallInstance", m_lPieceInst.GetCount() );

    for(  i=0; i<m_lPieceInst.GetCount(); i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];
        piece_desc& Desc = GetDesc( Inst.hDesc );
        vector3     Pos  = Inst.L2W.GetTranslation();

        TextOut.AddField( "Guid:g",              Inst.Guid );
        TextOut.AddField( "Desc:s",              Desc.FileName );
        TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
        TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                 Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                 Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );
        TextOut.AddField( "Flags:d",             Inst.Flags );
        TextOut.AddField( "ObjectName:s",        Inst.ObjectName );

        TextOut.AddEndLine();
    }

    //
    // Save all the ligts
    //
    TextOut.AddHeader( "Lights", m_lLight.GetCount() );

    for( i=0; i<m_lLight.GetCount(); i++ )
    {
        light& Light = m_lLight[i];
        f32 R, G, B, A;

        Light.Color.GetfRGBA(R,G,B,A);

        TextOut.AddField( "Guid:g",      Light.Guid );
        TextOut.AddField( "Sphere:ffff", Light.Sphere.Pos.X, Light.Sphere.Pos.Y, Light.Sphere.Pos.Z, Light.Sphere.R );
        TextOut.AddField( "Color:ffff",  R, G, B, A );
        TextOut.AddField( "Intensity:f", Light.Intensity );
        TextOut.AddField( "Flags:d",     Light.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the sounds.
    //
    TextOut.AddHeader( "Sounds", m_lSoundInst.GetCount() );

    for( i=0; i<m_lSoundInst.GetCount(); i++ )
    {
        sound_inst& Sound = m_lSoundInst[i];
        sound_desc& Desc = GetSoundDesc( Sound.hDesc );

        TextOut.AddField( "Guid:g",      Sound.Guid );
        TextOut.AddField( "Sphere:ffff", Sound.Sphere.Pos.X, Sound.Sphere.Pos.Y, Sound.Sphere.Pos.Z, Sound.Sphere.R );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescID:d",    Desc.ID );
        TextOut.AddField( "DescLoop:d",  Desc.Looped );
        TextOut.AddField( "DescClip:d",  Desc.Clipped );
        TextOut.AddField( "Flags:d",     Sound.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the effects.
    //
    TextOut.AddHeader( "Effects", m_lPfxInst.GetCount() );

    for( i=0; i<m_lPfxInst.GetCount(); i++ )
    {
        pfx_inst& Pfx = m_lPfxInst[i];
        pfx_desc& Desc = GetPfxDesc( Pfx.hDesc );

        TextOut.AddField( "Guid:g",      Pfx.Guid );
        TextOut.AddField( "Position:fff",Pfx.Sphere.Pos.X, Pfx.Sphere.Pos.Y, Pfx.Sphere.Pos.Z );
        TextOut.AddField( "Normal:fff",  Pfx.Normal.X, Pfx.Normal.Y, Pfx.Normal.Z );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescID:d",    Desc.ID );
        TextOut.AddField( "Flags:d",     Pfx.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the pickups.
    //
    TextOut.AddHeader( "Pickups", m_lPickupInst.GetCount() );

    for( i=0; i<m_lPickupInst.GetCount(); i++ )
    {
        pickup_inst& Pickup = m_lPickupInst[i];
        pickup_desc& Desc = GetPickupDesc( Pickup.hDesc );

        TextOut.AddField( "Guid:g",      Pickup.Guid );
        TextOut.AddField( "Position:fff",Pickup.Sphere.Pos.X, Pickup.Sphere.Pos.Y, Pickup.Sphere.Pos.Z );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescType:d",  Desc.Type );
        TextOut.AddField( "Flags:d",     Pickup.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the glows
    //
    TextOut.AddHeader( "Glows", m_lGlow.GetCount() );

    for( i=0; i<m_lGlow.GetCount(); i++ )
    {
        glow& Glow = m_lGlow[i];
        f32 R, G, B, A;

        Glow.Color.GetfRGBA(R,G,B,A);

        TextOut.AddField( "Guid:g",      Glow.Guid );
        TextOut.AddField( "Sphere:ffff", Glow.Sphere.Pos.X, Glow.Sphere.Pos.Y, Glow.Sphere.Pos.Z, Glow.Sphere.R );
        TextOut.AddField( "Color:ffff",  R, G, B, A );
        TextOut.AddField( "Intensity:f", Glow.Intensity );
        TextOut.AddField( "Flags:d",     Glow.Flags );

        TextOut.AddEndLine();
    }

    // Done
    TextOut.CloseFile();
}

//=========================================================================

void layout_editor::Load( const char* pFileName )
{
    text_in    TextIn;

    //
    // Reset the editor
    //
    NewProject();

    //
    // Save all the wall pieces
    //
    TextIn.OpenFile( pFileName );

    while( TextIn.ReadHeader() )
    {
        if( x_stricmp( TextIn.GetHeaderName(), "WallInstance" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                char        FileDesc[256];
                xhandle     hInst;
                piece_inst& Inst = m_lPieceInst.Add( hInst );

                TextIn.ReadFields();

                Inst.L2W.Identity();
                TextIn.GetField( "Guid:g",              &Inst.Guid );
                TextIn.GetField( "Desc:s",              FileDesc );
                TextIn.GetField( "Position:fff",        &Inst.L2W(3,0), &Inst.L2W(3,1), &Inst.L2W(3,2) );
                TextIn.GetField( "Matrix3x3:fffffffff", &Inst.L2W(0,0), &Inst.L2W(0,1), &Inst.L2W(0,2),
                                                        &Inst.L2W(1,0), &Inst.L2W(1,1), &Inst.L2W(1,2),
                                                        &Inst.L2W(2,0), &Inst.L2W(2,1), &Inst.L2W(2,2) );
                TextIn.GetField( "Flags:d",             &Inst.Flags );

                if( TextIn.GetField( "ObjectName:s",        Inst.ObjectName ) == FALSE )
                {
                    Inst.ObjectName[0]=0;
                }

                if( x_strlen( Inst.ObjectName ) >= 64 )
                    e_throw ("Fatal error loading the data file" );

                if( Inst.Flags < 2 )
                    Inst.Flags |= INST_FLAGS_WALL;

                Inst.W2L = m4_InvertSRT( Inst.L2W );

                // Get the description
                e_begin;
                    // Get the description
                    Inst.hDesc = GetDescHandleByName( FileDesc );
                    if( Inst.hDesc.IsNull() )
                    {
                        m_lPieceInst.DeleteByHandle( hInst );
                        e_throw( xfs("Unable to find the [%s] description",FileDesc )); 
                    }
                e_block;
                    xExceptionHandler(  __FILE__, __LINE__, "We will ignore this instance entry", TRUE );
                    xExceptionDisplay();
                    continue;
                e_block_end;

                // update the guid lookup
                if( m_lGuid2Handle.Find( Inst.Guid ) )
                    e_throw( "There was an error loading the level. I Found a instance guid several times.");
                m_lGuid2Handle.Add( Inst.Guid, hInst );

                // Add the new geom
                Inst.hGeom       = gman_Register( *m_lPieceDesc( Inst.hDesc ).pWallGeom );

            }
        }
        else if( x_stricmp( TextIn.GetHeaderName(), "Lights" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                xhandle  hLight;
                light&   Light = m_lLight.Add( hLight );
                f32 R, G, B, A;

                TextIn.ReadFields();

                TextIn.GetField( "Guid:g",      &Light.Guid );
                TextIn.GetField( "Sphere:ffff", &Light.Sphere.Pos.X, &Light.Sphere.Pos.Y, &Light.Sphere.Pos.Z, &Light.Sphere.R );
                TextIn.GetField( "Color:ffff",  &R, &G, &B, &A );
                TextIn.GetField( "Intensity:f", &Light.Intensity );
                TextIn.GetField( "Flags:d",     &Light.Flags );                

                Light.Color.SetfRGBA( R,G,B,A );

                // update the guid lookup
                if( m_lGuid2Handle.Find( Light.Guid ) )
                    e_throw( "There was an error loading the level. I Found a light guid several times.");
                m_lGuid2Handle.Add( Light.Guid, hLight );
            }
        }
        else if( x_stricmp( TextIn.GetHeaderName(), "Sounds" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                char FileDesc[256];
                s32 ID;
                xbool Looped;
                xbool Clip;
                xhandle hSound;
                sound_inst&   Sound = m_lSoundInst.Add( hSound );

                TextIn.ReadFields();

                TextIn.GetField( "Guid:g",      &Sound.Guid );
                TextIn.GetField( "Sphere:ffff", &Sound.Sphere.Pos.X, &Sound.Sphere.Pos.Y, &Sound.Sphere.Pos.Z, &Sound.Sphere.R );
                TextIn.GetField( "DescName:s", FileDesc );
                TextIn.GetField( "DescID:d",   &ID );
                
                if( !(TextIn.GetField( "DescLoop:d", &Looped )) )
                    Looped = FALSE;
                
                if( !(TextIn.GetField( "DescClip:d", &Clip )) )
                    Clip = FALSE;

                TextIn.GetField( "Flags:d",     &Sound.Flags );                

                // Get the description
                e_begin;
                    Sound.hDesc = GetSoundDescHandleByName( FileDesc );
                    if( Sound.hDesc.IsNull() )
                    {
                        m_lSoundInst.DeleteByHandle( hSound );
                        e_throw( xfs("Unable to find the [%s] description",FileDesc )); 
                    }
                e_block;

                    xExceptionHandler(  __FILE__, __LINE__, "We will ignore the current sound info for that entry", TRUE );
                    xExceptionDisplay();
                    continue;
                e_block_end;

                // update the guid lookup
                if( m_lGuid2Handle.Find( Sound.Guid ) )
                    e_throw( "There was an error loading the level. I Found a instance guid several times.");

                // update the guid lookup
                m_lGuid2Handle.Add( Sound.Guid, hSound );
            }

            FILE* Fp = fopen(  xfs( "%s\\%s", m_WorkingDir, "soundtypes.hpp" ), "rt" );
            if( Fp == 0 )
                e_throw( xfs("Unalbe to load the sound descriptor file [%s]", pFileName) );

            {

                char        Buffer[512];


                //
                // Do the actual loading of the descriptor
                //

                // First search for the beging of the lavel
                char* pTitle=NULL;
                do
                {
                    if( fscanf( Fp, "%s", Buffer ) != 1 )
                        e_throw( xfs("Error while searching for the 'BEGIN_LABEL_SET' in [%s] ", pFileName) );

                     pTitle = x_strstr( Buffer, "BEGIN_LABEL_SET(" );

                } while( pTitle == NULL );

                fgets( Buffer, 512, Fp );

                //
                // Read all the descritor into the system
                //
                do
                {
    
                    // Read in the lavel
                    if( fgets( Buffer, 512, Fp ) == 0 )
                        e_throw ( xfs("Error reading file[%s] ", pFileName) );

                    if( x_stristr( Buffer, "END_LABEL_SET(" ) ) 
                        break;

                    char* pParse;
                    s32   i;
                    char pName[256];
                    // Read name
                    pParse = x_stristr( Buffer, "(" )+1;
                    for( i=0; pParse[i] !=' ' && pParse[i] !=','; i++ )
                        pName[i] = pParse[i];
                    pName[i] = 0;
    
                    for( s32 j = 0; j < GetSoundNDescs(); j++ )
                    {
                        sound_desc Desc = GetSoundDesc( j );
                        if( x_stricmp( pName, Desc.Name ) == 0 )
                        {
                            // Read number        
                            pParse = x_stristr( Buffer, "," )+1;
                            Desc.ID = x_atoi( pParse );
                        }
                    }

                } while(1);

            }


            fclose( Fp );

        }
        else if( x_stricmp( TextIn.GetHeaderName(), "Effects" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                char FileDesc[256];
                s32 ID;
                xhandle hPfx;
                pfx_inst&   Pfx = m_lPfxInst.Add( hPfx );

                TextIn.ReadFields();

                TextIn.GetField( "Guid:g",       &Pfx.Guid );
                TextIn.GetField( "Position:fff",&Pfx.Sphere.Pos.X, &Pfx.Sphere.Pos.Y, &Pfx.Sphere.Pos.Z );
                TextIn.GetField( "Normal:fff",  &Pfx.Normal.X, &Pfx.Normal.Y, &Pfx.Normal.Z );
                TextIn.GetField( "DescName:s",   FileDesc );
                TextIn.GetField( "DescID:d",     &ID );
                TextIn.GetField( "Flags:d",      &Pfx.Flags );                
                
                Pfx.Sphere.R = PFX_SPHERE_RADIUS;

                // Get the description
                e_begin;
                    Pfx.hDesc = GetPfxDescHandleByName( FileDesc );
                    if( Pfx.hDesc.IsNull() )
                    {
                        m_lPfxInst.DeleteByHandle( hPfx );
                        e_throw( xfs("Unable to find the [%s] description",FileDesc )); 
                    }
                e_block;

                    xExceptionHandler(  __FILE__, __LINE__, "We will ignore the current Pfx info for that entry", TRUE );
                    xExceptionDisplay();
                    continue;
                e_block_end;

                // update the guid lookup
                if( m_lGuid2Handle.Find( Pfx.Guid ) )
                    e_throw( "There was an error loading the level. I Found a instance guid several times.");

                // update the guid lookup
                m_lGuid2Handle.Add( Pfx.Guid, hPfx );
            }
        }
        else if( x_stricmp( TextIn.GetHeaderName(), "Pickups" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                char FileDesc[256];
                s32 Type;
                xhandle hPickup;
                pickup_inst&   Pickup = m_lPickupInst.Add( hPickup );

                TextIn.ReadFields();

                TextIn.GetField( "Guid:g",       &Pickup.Guid );
                TextIn.GetField( "Position:fff", &Pickup.Sphere.Pos.X, &Pickup.Sphere.Pos.Y, &Pickup.Sphere.Pos.Z );
                TextIn.GetField( "DescName:s",   FileDesc );
                TextIn.GetField( "DescType:d",   &Type );
                TextIn.GetField( "Flags:d",      &Pickup.Flags );                
                
                Pickup.Sphere.R = PICKUP_SPHERE_RADIUS;

                // Get the description
                e_begin;
                    Pickup.hDesc = GetPickupDescHandleByName( FileDesc );
                    if( Pickup.hDesc.IsNull() )
                    {
                        m_lPickupInst.DeleteByHandle( hPickup );
                        e_throw( xfs("Unable to find the [%s] description",FileDesc )); 
                    }
                e_block;

                    xExceptionHandler(  __FILE__, __LINE__, "We will ignore the current Pickup info for that entry", TRUE );
                    xExceptionDisplay();
                    continue;
                e_block_end;

                // update the guid lookup
                if( m_lGuid2Handle.Find( Pickup.Guid ) )
                    e_throw( "There was an error loading the level. I Found a instance guid several times.");

                // update the guid lookup
                m_lGuid2Handle.Add( Pickup.Guid, hPickup );
            }
        }
        else if( x_stricmp( TextIn.GetHeaderName(), "Glows" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();

            for( s32 i=0; i<Count; i++ )
            {
                xhandle  hGlow;
                glow&   Glow = m_lGlow.Add( hGlow );
                f32 R, G, B, A;

                TextIn.ReadFields();

                TextIn.GetField( "Guid:g",      &Glow.Guid );
                TextIn.GetField( "Sphere:ffff", &Glow.Sphere.Pos.X, &Glow.Sphere.Pos.Y, &Glow.Sphere.Pos.Z, &Glow.Sphere.R );
                TextIn.GetField( "Color:ffff",  &R, &G, &B, &A );
                TextIn.GetField( "Intensity:f", &Glow.Intensity );
                TextIn.GetField( "Flags:d",     &Glow.Flags );                

                Glow.Color.SetfRGBA( R,G,B,A );

                // update the guid lookup
                if( m_lGuid2Handle.Find( Glow.Guid ) )
                    e_throw( "There was an error loading the level. I Found a Glow guid several times.");
                m_lGuid2Handle.Add( Glow.Guid, hGlow );
            }
        }
        else if( x_stricmp( TextIn.GetHeaderName(), "DescInfo" ) == 0 )
        {
            s32 Count = TextIn.GetHeaderCount();
            for( s32 i=0; i<Count; i++ )
            {
                char        FileDesc[256];
                xhandle     hDesc;

                TextIn.ReadFields();

                //
                // Get the description
                //
                e_begin;
                    TextIn.GetField( "Desc:s", FileDesc );
                    hDesc = GetDescHandleByName( FileDesc );
                    if( hDesc.IsNull() )
                    {
                        e_throw( xfs("Unable to find the [%s] description",FileDesc )); 
                    }
                e_block;

                    xExceptionHandler(  __FILE__, __LINE__, "We will ignore the current material entry", TRUE );
                    xExceptionDisplay();
                    continue;
                e_block_end;

                piece_desc& Desc = GetDesc( hDesc );

                //
                // Read the material
                //
                char MatType[256];
                if( TextIn.GetField( "Mat:s",              MatType ) )
                {
                    if( x_stricmp( MatType, "NULL" ) == 0 )
                    {
                        Desc.MatType = MAT_TYPE_NULL;
                    }
                    else if( x_stricmp( MatType, "WOOD" ) == 0 )
                    {
                        Desc.MatType = MAT_TYPE_WOOD;
                    }
                    else if( x_stricmp( MatType, "METAL" ) == 0 )
                    {
                        Desc.MatType = MAT_TYPE_METAL;
                    }
                    else if( x_stricmp( MatType, "FLESH" ) == 0 )
                    {
                        Desc.MatType = MAT_TYPE_FLESH;
                    }
                    else if( x_stricmp( MatType, "STONE" ) == 0 )
                    {
                        Desc.MatType = MAT_TYPE_STONE;
                    }
                }
                else
                {
                    Desc.MatType = MAT_TYPE_NULL;
                }
            }
        }
    }

    // Done
    TextIn.CloseFile();

    // Make sure that all the instances are lighted
    UpdateLighting();
}

//=========================================================================

void layout_editor::Export( const char* pFileName )
{
    xarray<vector2>    InsColInfo;
    text_out           TextOut;
    s32                nVertColors=0;
    s32                i;

    //
    // Save all the wall colors for the PC
    //
    if( x_stristr( pFileName, "C:\\GameData\\A51\\Release\\PC\\" ) )
    {
        // Find how many colors we are going to have total
        nVertColors = 0;
        for( i=0; i<m_lPieceInst.GetCount(); i++ )
        {
            piece_inst& Inst = m_lPieceInst[i];
            piece_desc& Desc = GetDesc( Inst.hDesc );
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;
        }

        //
        // Now create the export data
        //
        wall_color_info WallColor;
        WallColor.nColors   = nVertColors;
        WallColor.pColor    = new u16[ WallColor.nColors ];
        if( WallColor.pColor == NULL )
            e_throw( "Out of memory" );

        nVertColors = 0;
        for( i=0; i<m_lPieceInst.GetCount(); i++ )
        {
            piece_inst& Inst = m_lPieceInst[i];
            piece_desc& Desc = GetDesc( Inst.hDesc );

            wall_geom::vertex_pc* pVert = (wall_geom::vertex_pc*)gman_LockVertexData( Inst.hGeom );
            for( s32 j=0; j<Desc.pWallGeom->m_System.pPC->nVerts; j++ )
            {
                WallColor.pColor[nVertColors+j] = ((u32)((pVert[j].Color.R>>4)&XBIN(11111))<<0) | 
                                                  ((u32)((pVert[j].Color.G>>4)&XBIN(11111))<<5) |
                                                  ((u32)((pVert[j].Color.B>>4)&XBIN(11111))<<10);
            }

            gman_UnlockVertexData( Inst.hGeom );

            vector2& Info = InsColInfo.Append();
            Info.X  = (f32)nVertColors;
            Info.Y  = (f32)Desc.pWallGeom->m_System.pPC->nVerts;

            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;
        }

        // Output to a file for the PC
        {
            fileio File;
            File.Save( "C:\\GameData\\A51\\Release\\PC\\WallColorData.wallcolor", WallColor, FALSE );
        }

        delete []WallColor.pColor;
    }
    else
    //
    // Save of the colors for the PS2
    //
    if( x_stristr( pFileName, "C:\\GameData\\A51\\Release\\PS2\\" ) ) 
    {
        //xarray<s32*>       m_ReIndex;
        xarray<wall_geom*> m_lPS2Wall;
        //xarray<s32>        m_Check;

        // Load all the wall geoms for the PS2
        for( i=0; i<m_lPieceDesc.GetCount(); i++ )
        {
            fileio File;
            wall_geom*& pWallGeom = m_lPS2Wall.Append();

            e_begin;
                File.Load( xfs("C:\\GameData\\A51\\Release\\PS2\\%s",m_lPieceDesc[i].FileName), pWallGeom );
            e_append( xfs("The PS2 Wall[%s] could't not be loaded", m_lPieceDesc[i].FileName) );
        }
/*
        for( i=0; i<m_lPieceDesc.GetCount(); i++ )
        {
            piece_desc& Desc     = m_lPieceDesc[i];
            X_FILE* Fp = NULL;

            Fp = x_fopen( xfs("C:\\GameData\\A51\\Release\\PS2\\%s.obj",m_lPieceDesc[i].FileName ), "rb" );
            ASSERT( Fp );
            s32 c;
            x_fread( &c, 4, 1, Fp );
            m_Check.Append() = c;

            s32* pBuf = m_ReIndex.Append() = new s32[ c ];
            ASSERT(pBuf);
            x_fread( pBuf, 4, c, Fp );
            x_fclose( Fp );
        }
*/
        // Find the total count for vertex colors
        nVertColors = 0;
        for( i=0; i<m_lPieceInst.GetCount(); i++ )
        {
            piece_inst& Inst     = m_lPieceInst[i];
            piece_desc& Desc     = GetDesc( Inst.hDesc );
            s32         Index    = m_lPieceDesc.GetIndexByHandle( Inst.hDesc );
            wall_geom&  WallGeom = *m_lPS2Wall[Index];

//            ASSERT( m_Check[Index] == WallGeom.m_System.pPS2->pDList[0].nVerts );

            nVertColors += WallGeom.m_System.pPS2->pDList[0].nVerts;
            nVertColors  = ALIGN_16( nVertColors );
        }

        // Allocate the memory for the colors
        wall_color_info WallColor;
        WallColor.nColors   = nVertColors;
        WallColor.pColor    = new u16[ WallColor.nColors ];
        if( WallColor.pColor == NULL )
            e_throw( "Out of memory" );

        nVertColors = 0;
        for( i=0; i<m_lPieceInst.GetCount(); i++ )
        {
            piece_inst& Inst     = m_lPieceInst[i];
            piece_desc& Desc     = GetDesc( Inst.hDesc );
            s32         Index    = m_lPieceDesc.GetIndexByHandle( Inst.hDesc );
            wall_geom&  WallGeom = *m_lPS2Wall[Index];
//            s32*       pReIndex  = m_ReIndex[ Index ];

            ASSERT( Desc.pWallGeom->m_System.pPC->nDLists == 1 );
            ASSERT( WallGeom.m_System.pPS2->nDLists == 1 );
            wall_geom::vertex_pc* pVert = (wall_geom::vertex_pc*)gman_LockVertexData( Inst.hGeom );

            /*
            for( s32 k=0; k<WallGeom.m_System.pPS2->pDList[0].nVerts; k++ )
            {
                u16 Color = ((u32)((pVert[ pReIndex[k] ].Color.R>>3)&0x1F)<<10) | 
                            ((u32)((pVert[ pReIndex[k] ].Color.G>>3)&0x1F)<<5)  |
                            ((u32)((pVert[ pReIndex[k] ].Color.B>>3)&0x1F)<<0);

                WallColor.pColor[nVertColors+k] = Color;

            }
            */
            
            for( s32 j=0; j<Desc.pWallGeom->m_System.pPC->nVerts; j++ )
            {
                xbool       GotWrittern = FALSE;
                u16 Color = ((u32)((pVert[j].Color.B>>(3+1))&0x1F)<<10) | 
                            ((u32)((pVert[j].Color.G>>(3+1))&0x1F)<<5)  |
                            ((u32)((pVert[j].Color.R>>(3+1))&0x1F)<<0)  |
                            ((u32)(1<<15));

                // Find the right vert for the PS2
                for( s32 k=0; k<WallGeom.m_System.pPS2->pDList[0].nVerts; k++ )
                {
                    vector4& Pos   = WallGeom.m_System.pPS2->pDList[0].pPosition[k];
                    s8*      pNorm = &WallGeom.m_System.pPS2->pDList[0].pNormal[k*3];
                    s16*     pUV   = &WallGeom.m_System.pPS2->pDList[0].pUV[k*2];

                    // Make sure that we are dealing with the same vert before writtin the color
                    if( Pos.X != pVert[j].Pos.X ) continue;
                    if( Pos.Y != pVert[j].Pos.Y ) continue;
                    if( Pos.Z != pVert[j].Pos.Z ) continue;

                    if( pNorm[0] != (s8)(pVert[j].Normal.X * (0xff>>1)) ) continue;                    
                    if( pNorm[1] != (s8)(pVert[j].Normal.Y * (0xff>>1)) ) continue;
                    if( pNorm[2] != (s8)(pVert[j].Normal.Z * (0xff>>1)) ) continue;

                    if( pUV[0] != (s16)(pVert[j].UV.X * (1<<12)) ) continue;
                    if( pUV[1] != (s16)(pVert[j].UV.Y * (1<<12)) ) continue;

                    WallColor.pColor[nVertColors+k] = Color;
                    GotWrittern = TRUE;
                }

//                ASSERT( GotWrittern );
            }
                       
            gman_UnlockVertexData( Inst.hGeom );

            vector2& Info = InsColInfo.Append();
            Info.X  = (f32)nVertColors;
            Info.Y  = (f32)WallGeom.m_System.pPS2->pDList[0].nVerts;

            nVertColors += WallGeom.m_System.pPS2->pDList[0].nVerts;
            nVertColors  = ALIGN_16( nVertColors );
        }

        // Save the file
        fileio File;
        File.Save( "C:\\GameData\\A51\\Release\\PS2\\WallColorData.wallcolor", WallColor, FALSE );

        // Free memory
        for( i=0; i<m_lPS2Wall.GetCount(); i++ )
        {
            delete m_lPS2Wall[i];
        }

        // delete all the colors
        delete []WallColor.pColor;
    }
    else
    {
        e_throw( "We don't know what platform your are saving your data for\nPlease export in the correct directory" );
    }

    s32 PieceCount = m_lPieceInst.GetCount();
    s32 WallCount, AmmoCount, HealthCount, ItemCount, ObjectCount;
    WallCount = AmmoCount = HealthCount = ItemCount = ObjectCount = 0;

    
    for( i = 0; i < PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];
        switch ( Inst.Flags&(INST_FLAGS_WALL   | 
                             INST_FLAGS_AMMO   |  
                             INST_FLAGS_HEALTH |  
                             INST_FLAGS_ITEM   | 
                             INST_FLAGS_OBJECT ))
        {
            case INST_FLAGS_WALL:
                WallCount++;
            break;
            case INST_FLAGS_AMMO:
                AmmoCount++;
            break;
            case INST_FLAGS_HEALTH:
                HealthCount++;
            break;
            case INST_FLAGS_ITEM:
                ItemCount++;
            break;
            case INST_FLAGS_OBJECT:
                ObjectCount++;
            break;
        };

    }

    //
    // Save all the wall pieces
    //
    TextOut.OpenFile( pFileName );
    TextOut.AddHeader( "WallInstance", WallCount );

    for( i=0; i<PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];

        if( Inst.Flags == INST_FLAGS_WALL )
        {

            piece_desc& Desc = GetDesc( Inst.hDesc );
            vector3     Pos  = Inst.L2W.GetTranslation();

            //
            // Save the base object portion
            //
            TextOut.AddField( "Guid:g",              Inst.Guid );
            TextOut.AddField( "Mat:d",               Desc.MatType );
            TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
            TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                     Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                     Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );

            TextOut.AddField( "Attributes:d",        0  );
            TextOut.AddField( "CustomName:s",        Inst.ObjectName );

            //
            // Save wall specific stuff
            //
            TextOut.AddField( "Desc:s",              Desc.FileName );
        
            TextOut.AddField( "ColorInfo:s",         "WallColorData.wallcolor" );
            TextOut.AddField( "iColor:d",            (s32)InsColInfo[i].X  );
            TextOut.AddField( "nColor:d",            (s32)InsColInfo[i].Y  );
            TextOut.AddField( "ObjectName:s",        Inst.ObjectName );
            TextOut.AddField( "Flags:d",             Inst.Flags );
    
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;

            // Move on to the next line
            TextOut.AddEndLine();
        }
    }


    //
    // Save all the ammo pieces
    //
    TextOut.AddHeader( "AmmoInstance", AmmoCount );

    for( i=0; i<PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];

        if( Inst.Flags == INST_FLAGS_AMMO )
        {

            piece_desc& Desc = GetDesc( Inst.hDesc );
            vector3     Pos  = Inst.L2W.GetTranslation();

            //
            // Save the base object portion
            //
            TextOut.AddField( "Guid:g",              Inst.Guid );
            TextOut.AddField( "Mat:d",               Desc.MatType );
            TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
            TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                     Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                     Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );

            TextOut.AddField( "Attributes:d",        0  );
            TextOut.AddField( "CustomName:s",        Inst.ObjectName );

            //
            // Save wall specific stuff
            //
            TextOut.AddField( "Desc:s",              Desc.FileName );
        
            TextOut.AddField( "ColorInfo:s",         "WallColorData.wallcolor" );
            TextOut.AddField( "iColor:d",            (s32)InsColInfo[i].X  );
            TextOut.AddField( "nColor:d",            (s32)InsColInfo[i].Y  );
            TextOut.AddField( "ObjectName:s",        Inst.ObjectName );
            TextOut.AddField( "Flags:d",             Inst.Flags );
    
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;

            // Move on to the next line
            TextOut.AddEndLine();
        }
    }

    //
    // Save all the health pieces
    //
    TextOut.AddHeader( "HealthInstance", HealthCount );

    for( i=0; i<PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];

        if( Inst.Flags == INST_FLAGS_HEALTH )
        {

            piece_desc& Desc = GetDesc( Inst.hDesc );
            vector3     Pos  = Inst.L2W.GetTranslation();

            //
            // Save the base object portion
            //
            TextOut.AddField( "Guid:g",              Inst.Guid );
            TextOut.AddField( "Mat:d",               Desc.MatType );
            TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
            TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                     Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                     Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );

            TextOut.AddField( "Attributes:d",        0  );
            TextOut.AddField( "CustomName:s",        Inst.ObjectName );

            //
            // Save wall specific stuff
            //
            TextOut.AddField( "Desc:s",              Desc.FileName );
        
            TextOut.AddField( "ColorInfo:s",         "WallColorData.wallcolor" );
            TextOut.AddField( "iColor:d",            (s32)InsColInfo[i].X  );
            TextOut.AddField( "nColor:d",            (s32)InsColInfo[i].Y  );
            TextOut.AddField( "ObjectName:s",        Inst.ObjectName );
            TextOut.AddField( "Flags:d",             Inst.Flags );
    
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;

            // Move on to the next line
            TextOut.AddEndLine();
        }
    }

    //
    // Save all the Item pieces
    //
    TextOut.AddHeader( "ItemInstance", ItemCount );

    for( i=0; i<PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];

        if( Inst.Flags == INST_FLAGS_ITEM )
        {

            piece_desc& Desc = GetDesc( Inst.hDesc );
            vector3     Pos  = Inst.L2W.GetTranslation();

            //
            // Save the base object portion
            //
            TextOut.AddField( "Guid:g",              Inst.Guid );
            TextOut.AddField( "Mat:d",               Desc.MatType );
            TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
            TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                     Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                     Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );

            TextOut.AddField( "Attributes:d",        0  );
            TextOut.AddField( "CustomName:s",        Inst.ObjectName );

            //
            // Save wall specific stuff
            //
            TextOut.AddField( "Desc:s",              Desc.FileName );
        
            TextOut.AddField( "ColorInfo:s",         "WallColorData.wallcolor" );
            TextOut.AddField( "iColor:d",            (s32)InsColInfo[i].X  );
            TextOut.AddField( "nColor:d",            (s32)InsColInfo[i].Y  );
            TextOut.AddField( "ObjectName:s",        Inst.ObjectName );
            TextOut.AddField( "Flags:d",             Inst.Flags );
    
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;

            // Move on to the next line
            TextOut.AddEndLine();
        }
    }

    //
    // Save all the object pieces
    //
    TextOut.AddHeader( "ObjectInstance", ObjectCount );

    for( i=0; i<PieceCount; i++ )
    {
        piece_inst& Inst = m_lPieceInst[i];

        if( Inst.Flags == INST_FLAGS_OBJECT )
        {

            piece_desc& Desc = GetDesc( Inst.hDesc );
            vector3     Pos  = Inst.L2W.GetTranslation();

            //
            // Save the base object portion
            //
            TextOut.AddField( "Guid:g",              Inst.Guid );
            TextOut.AddField( "Mat:d",               Desc.MatType );
            TextOut.AddField( "Position:fff",        Pos.X, Pos.Y, Pos.Z );
            TextOut.AddField( "Matrix3x3:fffffffff", Inst.L2W(0,0), Inst.L2W(0,1), Inst.L2W(0,2),
                                                     Inst.L2W(1,0), Inst.L2W(1,1), Inst.L2W(1,2),
                                                     Inst.L2W(2,0), Inst.L2W(2,1), Inst.L2W(2,2) );

            TextOut.AddField( "Attributes:d",        0  );
            TextOut.AddField( "CustomName:s",        Inst.ObjectName );

            //
            // Save wall specific stuff
            //
            TextOut.AddField( "Desc:s",              Desc.FileName );
        
            TextOut.AddField( "ColorInfo:s",         "WallColorData.wallcolor" );
            TextOut.AddField( "iColor:d",            (s32)InsColInfo[i].X  );
            TextOut.AddField( "nColor:d",            (s32)InsColInfo[i].Y  );
            TextOut.AddField( "Flags:d",             Inst.Flags );
    
            nVertColors += Desc.pWallGeom->m_System.pPC->nVerts;

            // Move on to the next line
            TextOut.AddEndLine();
        }
    }

    //
    // Save all the sounds.
    //
    TextOut.AddHeader( "Sounds", m_lSoundInst.GetCount() );
    
    for( i = 0; i < m_lSoundInst.GetCount(); i++ )
    {
        sound_inst& Sound = m_lSoundInst[i];
        sound_desc& Desc  = GetSoundDesc( Sound.hDesc );
        
        //
        // Save the base object portion
        //
        TextOut.AddField( "Guid:g",              Sound.Guid );
        TextOut.AddField( "Position:fff",        Sound.Sphere.Pos.X, Sound.Sphere.Pos.Y, Sound.Sphere.Pos.Z );
        TextOut.AddField( "Matrix3x3:fffffffff", 0,0,0, 0,0,0, 0,0,0);

        TextOut.AddField( "Attributes:d",        0  );
        TextOut.AddField( "CustomName:s",        "" );
        
        if( Desc.Looped != 0 && Desc.Looped != 1 )
            Desc.Looped = FALSE;

        if( Desc.Clipped != 0 && Desc.Looped != 1 )
            Desc.Clipped = FALSE;

        //
        // Sound stuff
        //                 
        TextOut.AddField( "Sphere:ffff", Sound.Sphere.Pos.X, Sound.Sphere.Pos.Y, Sound.Sphere.Pos.Z, Sound.Sphere.R );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescID:d",    Desc.ID );
        TextOut.AddField( "DescLoop:d",  Desc.Looped );
        TextOut.AddField( "DescClip:d",  Desc.Clipped );
        TextOut.AddField( "Flags:d",     Sound.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the pfx.
    //
    TextOut.AddHeader( "Effects", m_lPfxInst.GetCount() );
    
    for( i = 0; i < m_lPfxInst.GetCount(); i++ )
    {
        pfx_inst& Pfx = m_lPfxInst[i];
        pfx_desc& Desc  = GetPfxDesc( Pfx.hDesc );

        //
        // Save the base object portion
        //
        TextOut.AddField( "Guid:g",              Pfx.Guid );
        TextOut.AddField( "Position:fff",        Pfx.Sphere.Pos.X, Pfx.Sphere.Pos.Y, Pfx.Sphere.Pos.Z );
        TextOut.AddField( "Matrix3x3:fffffffff", 0,0,0, 0,0,0, 0,0,0);
        TextOut.AddField( "Attributes:d",        0  );
        TextOut.AddField( "CustomName:s",        "" );

        //
        // Pfx stuff
        //                 
        TextOut.AddField( "Sphere:fff",  Pfx.Sphere.Pos.X, Pfx.Sphere.Pos.Y, Pfx.Sphere.Pos.Z );
        TextOut.AddField( "Normal:fff",  Pfx.Normal.X, Pfx.Normal.Y, Pfx.Normal.Z );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescID:d",    Desc.ID );
        TextOut.AddField( "Flags:d",     Pfx.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the pickups.
    //
    TextOut.AddHeader( "Pickups", m_lPickupInst.GetCount() );
    
    for( i = 0; i < m_lPickupInst.GetCount(); i++ )
    {
        pickup_inst& Pickup = m_lPickupInst[i];
        pickup_desc& Desc  = GetPickupDesc( Pickup.hDesc );

        //
        // Save the base object portion
        //
        TextOut.AddField( "Guid:g",              Pickup.Guid );
        TextOut.AddField( "Position:fff",        Pickup.Sphere.Pos.X, Pickup.Sphere.Pos.Y, Pickup.Sphere.Pos.Z );
        TextOut.AddField( "Matrix3x3:fffffffff", 0,0,0, 0,0,0, 0,0,0);
        TextOut.AddField( "Attributes:d",        0  );
        TextOut.AddField( "CustomName:s",        "" );

        //
        // Pickup stuff
        //                 
        TextOut.AddField( "Sphere:fff",  Pickup.Sphere.Pos.X, Pickup.Sphere.Pos.Y, Pickup.Sphere.Pos.Z );
        TextOut.AddField( "DescName:s",  Desc.Name );
        TextOut.AddField( "DescType:d",  Desc.Type );
        TextOut.AddField( "Flags:d",     Pickup.Flags );

        TextOut.AddEndLine();
    }

    //
    // Save all the pfx.
    //
    TextOut.AddHeader( "Glows", m_lGlow.GetCount() );
    
    for( i = 0; i < m_lGlow.GetCount(); i++ )
    {
        glow& Glow = m_lGlow[i];

        //
        // Save the base object portion
        //
        TextOut.AddField( "Guid:g",              Glow.Guid );
        TextOut.AddField( "Position:fff",        Glow.Sphere.Pos.X, Glow.Sphere.Pos.Y, Glow.Sphere.Pos.Z );
        TextOut.AddField( "Matrix3x3:fffffffff", 0,0,0, 0,0,0, 0,0,0);
        TextOut.AddField( "Attributes:d",        0  );
        TextOut.AddField( "CustomName:s",        "" );

        //
        // Glow stuff
        //                 
        TextOut.AddField( "Sphere:ffff",  Glow.Sphere.Pos.X, Glow.Sphere.Pos.Y, Glow.Sphere.Pos.Z, Glow.Sphere.R );
        TextOut.AddField( "RGBA:dddd",   Glow.Color.R, Glow.Color.G, Glow.Color.B, Glow.Color.A );
        TextOut.AddField( "Intensity:f", Glow.Intensity );
        TextOut.AddField( "Flags:d",     Glow.Flags );

        TextOut.AddEndLine();
    }


    // Done
    TextOut.CloseFile();

}

//=========================================================================

f32 layout_editor::ComputeAttenuation( const light& Light, const vector3& Pos ) const 
{
    f32  K0,K1,K2;
    f32  d = (Pos - Light.Sphere.Pos).Length();

    GetAttenConstants( Light.Intensity, Light.Sphere.R, K0, K1, K2 );

    f32 Atten = (K0 + (K1*d) + K2*d*d);
    if( Atten < 0.0001f ) return 0;
    return 1/Atten;
}

//=========================================================================

xbool layout_editor::CanSeeLight( const light& Light, const vector3& P1 ) const
{
    s32 i;

    for( i=0; i<m_lPieceInst.GetCount(); i++ )
    {
        const piece_inst&                 Inst   = m_lPieceInst[i];
        const piece_desc&                 Desc   = m_lPieceDesc(Inst.hDesc);
        const wall_geom&                  WGeom  = *Desc.pWallGeom;
        const collision_volume&           Volume = WGeom.m_Volumen;
        const collision_volume::volume&   Vol    = Volume.m_Prim.pVol[0];
        f32                         t;
        vector3                     p0, p1;

        p0 = Inst.W2L * Light.Sphere.Pos;
        p1 = Inst.W2L * P1;
        
        if( Vol.BBox.Intersect( t, p0, p1 ) )
        {
            // Check for collisions
            for( s32 j=0; j<Vol.nTris; j++ )
            {
                vector3 Normal;
                if( ComputeRayTriCollision( Vol.pTri[j].P, p0, p1, t, Normal ) )
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


//=========================================================================
#define LIGHTING_STEPS 4
void layout_editor::InstUpdateFinalLight( xhandle hInst )
{
    struct col_vert
    {
        vector3 C1;
        vector3 C2;
        f32     W;
    };

    s32                   i;
    piece_inst&           Inst     = m_lPieceInst( hInst );
    piece_desc&           Desc     = GetDesc( Inst.hDesc );
    bbox&                 BBox     = Desc.pWallGeom->m_Volumen.m_Prim.pVol->BBox;
    s32                   nVerts   = Desc.pWallGeom->m_System.pPC->nVerts;
    wall_geom::vertex_pc* pVert    = (wall_geom::vertex_pc*)gman_LockVertexData( Inst.hGeom );
    s32                   nTris    = Desc.pWallGeom->m_System.pPC->nIndices/3;
    u16*                  pIndex   = Desc.pWallGeom->m_System.pPC->pIndex;
    col_vert*             pColVert = new col_vert[ nVerts ];

    // Protect with error checking
    e_begin;

    if( pVert == NULL )
        e_throw("Unalbe to lock the vert in D3D");

    if( pColVert == NULL ) 
        e_throw( "Out of memory");

    // Initialize the color verts
    x_memset( pColVert, 0, sizeof( col_vert ) * nVerts );

    // Loop through all triangles
    for( i=0; i<nTris; i++ )
    {
        f32 Step = 1.0f/((f32)LIGHTING_STEPS);
        f32 u,v,w;

        for( u=0; u<=1.0f; u+=Step )
        for( v=0; v<=(1-u); v+=Step )
        {
            w = 1 - u - v;

            vector3 P = u*pVert[ pIndex[i*3+0] ].Pos + 
                        v*pVert[ pIndex[i*3+1] ].Pos + 
                        w*pVert[ pIndex[i*3+2] ].Pos;

            vector3 N = u*pVert[ pIndex[i*3+0] ].Normal + 
                        v*pVert[ pIndex[i*3+1] ].Normal + 
                        w*pVert[ pIndex[i*3+2] ].Normal;

            N.Normalize();

            vector3 L1(0,0,0);
            vector3 L2(0,0,0);

            // Accumulate lighting
            for( s32 j=0; j<m_lLight.GetCount(); j++ )
            {
                light&  Light    = m_lLight[j];
                vector3 LightPos = Inst.W2L * Light.Sphere.Pos;

                if( Light.Sphere.GetBBox().Intersect( Inst.L2W * P ) == FALSE )
                    continue;

                vector3 Diff = LightPos - P;
                f32 D = Diff.Length();

                // Compute lighting
                xbool InShadow = CanSeeLight( Light, Inst.L2W * (P+(N*0.5f)) );

                f32 NI = N.Dot( Diff ) / D;
                if( NI < 0 ) NI = 0;

                f32 I = (Light.Sphere.R - D) / (Light.Sphere.R);
                if( I < 0 ) I = 0;
                if( I > 1 ) I = 1;

                I *= NI;
                if( I < 0 ) I = 0;
                if( I > 2 ) I = 2;


                if( !InShadow )
                {
                    L2.X += I * Light.Color.R;
                    L2.Y += I * Light.Color.G;
                    L2.Z += I * Light.Color.B;
                }

                L1.X += I * Light.Color.R;
                L1.Y += I * Light.Color.G;
                L1.Z += I * Light.Color.B;
            }

            if( L1.X <   0 ) L1.X = 0;
            if( L1.X > 255 ) L1.X = 255;
            if( L1.Y <   0 ) L1.Y = 0;
            if( L1.Y > 255 ) L1.Y = 255;
            if( L1.Z <   0 ) L1.Z = 0;
            if( L1.Z > 255 ) L1.Z = 255;

            if( L2.X <   0 ) L2.X = 0;
            if( L2.X > 255 ) L2.X = 255;
            if( L2.Y <   0 ) L2.Y = 0;
            if( L2.Y > 255 ) L2.Y = 255;
            if( L2.Z <   0 ) L2.Z = 0;
            if( L2.Z > 255 ) L2.Z = 255;

            pColVert[ pIndex[i*3+0] ].C1 += L1 * u;
            pColVert[ pIndex[i*3+1] ].C1 += L1 * v;
            pColVert[ pIndex[i*3+2] ].C1 += L1 * w;
            pColVert[ pIndex[i*3+0] ].C2 += L2 * u;
            pColVert[ pIndex[i*3+1] ].C2 += L2 * v;
            pColVert[ pIndex[i*3+2] ].C2 += L2 * w;
            pColVert[ pIndex[i*3+0] ].W += u;
            pColVert[ pIndex[i*3+1] ].W += v;
            pColVert[ pIndex[i*3+2] ].W += w;
        }
    }

    for( i=0; i<nVerts; i++ )
    {
        vector3 L1 = pColVert[i].C1;
        if( pColVert[i].W == 0 )
            L1.Set( 0,0,0 );
        else
            L1 /= pColVert[i].W;

        L1.X += m_AmbientLight.R;
        L1.Y += m_AmbientLight.G;
        L1.Z += m_AmbientLight.B;
        if( L1.X <   0 ) L1.X = 0;
        if( L1.X > 255 ) L1.X = 255;
        if( L1.Y <   0 ) L1.Y = 0;
        if( L1.Y > 255 ) L1.Y = 255;
        if( L1.Z <   0 ) L1.Z = 0;
        if( L1.Z > 255 ) L1.Z = 255;


        vector3 L2 = pColVert[i].C2;
        if( pColVert[i].W == 0 )
            L2.Set( 0,0,0 );
        else
            L2 /= pColVert[i].W;

        L2.X += m_AmbientLight.R;
        L2.Y += m_AmbientLight.G;
        L2.Z += m_AmbientLight.B;
        if( L2.X <   0 ) L2.X = 0;
        if( L2.X > 255 ) L2.X = 255;
        if( L2.Y <   0 ) L2.Y = 0;
        if( L2.Y > 255 ) L2.Y = 255;
        if( L2.Z <   0 ) L2.Z = 0;
        if( L2.Z > 255 ) L2.Z = 255;

        f32 t = 0;
        vector3 Final = L2;//L1 + t*( L2 - L1 );

        pVert[i].Color = xcolor( (byte)Final.X, (byte)Final.Y, (byte)Final.Z );
    }

    e_block;

    if( pVert    ) gman_UnlockVertexData( Inst.hGeom );
    if( pColVert ) delete []pColVert;

    e_block_end;

    if( pVert    ) gman_UnlockVertexData( Inst.hGeom );
    if( pColVert ) delete []pColVert;
}

//=========================================================================

void layout_editor::AddSound ( sphere& Sound, s32 Type )
{
    //
    // Add a sound into the world
    //
    xhandle     hInst;
    sound_inst&      T = m_lSoundInst.Add( hInst );
    T.Guid        = guid_New();
    T.Sphere      = Sound;
    T.hDesc       = m_lSoundDesc.GetHandleByIndex( Type );
    T.Flags       = 0;

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        sound_inst&     Data    = Undo.GetSound();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }
}

//=========================================================================

s32 layout_editor::GetSoundCount ( void ) const
{
    return m_lSoundInst.GetCount();

}

//=========================================================================

layout_editor::sound_inst& layout_editor::GetSound ( s32 I )
{
    return m_lSoundInst[I];
}

//=========================================================================

s32 layout_editor::GetPfxNDescs ( void )
{
    return m_lPfxDesc.GetCount();
}

//=========================================================================

layout_editor::pfx_desc& layout_editor::GetPfxDesc ( xhandle hDesc )
{
    if( hDesc.IsNull() )
        e_throw( "Invalid handle for a description" );

    return m_lPfxDesc( hDesc );
}

//=========================================================================

layout_editor::pfx_desc& layout_editor::GetPfxDesc ( s32 Index )
{
    if( Index < 0 || Index >= m_lPfxDesc.GetCount() ) 
        e_throw( "Unable to get the requested piece" );

    return m_lPfxDesc[Index];
}

//=========================================================================

layout_editor::pfx_desc& layout_editor::GetPfxDesc ( const char* pFileName )
{
    return GetPfxDesc( GetPfxDescIndexByName( pFileName ) );
}

//=========================================================================

s32 layout_editor::GetParticleCount ( void ) const
{   
    return m_lPfxInst.GetCount();
}

//=========================================================================

layout_editor::pfx_inst& layout_editor::GetParticle ( s32 I )
{
    return m_lPfxInst[I];
}

//=========================================================================

void layout_editor::AddPickup ( sphere& Sphere, s32 Type )
{
    //
    // Add a pickup object into the world
    //
    xhandle     hInst;
    pickup_inst&  T = m_lPickupInst.Add( hInst );
    T.Guid        = guid_New();
    T.Sphere      = Sphere;
    T.hDesc       = m_lPickupDesc.GetHandleByIndex( Type );
    T.Flags       = 0;

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        pickup_inst&    Data    = Undo.GetPickup();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }

}

//=========================================================================

s32 layout_editor::GetPickupCount ( void ) const
{
    return m_lPickupInst.GetCount();
}

//=========================================================================

layout_editor::pickup_inst& layout_editor::GetPickup ( s32 I )
{
    return m_lPickupInst[I];
}

//=========================================================================

s32 layout_editor::GetNumPickupTypes ( void )
{
    return m_lPickupDesc.GetCount();
}

//=========================================================================

void layout_editor::GetPickupType ( int Idx, char* pName, s32& ID )
{
    x_strcpy( pName, m_lPickupDesc[Idx].Name );
    ID = m_lPickupDesc[Idx].Type;
}

//=========================================================================

s32 layout_editor::GetPickupDescIndexByName ( const char* pName )
{
    // Not name then no description
    if( pName == NULL ) 
        return -1;

    //
    // Do we have that file?
    //
    for( s32 i=0; i<m_lPickupDesc.GetCount(); i++ )
    {
        if( x_stricmp( m_lPickupDesc[i].Name, pName ) == 0 )
            return i;
    }

    return -1;
}

//=========================================================================

xhandle layout_editor::GetPickupDescHandleByName ( const char* pName )
{
    s32 Index = GetPickupDescIndexByName( pName );

    if( Index < 0 ) 
        return xhandle( HNULL );

    return m_lPickupDesc.GetHandleByIndex( Index );
}

//=========================================================================

s32 layout_editor::GetPickupNDescs ( void )
{
    return m_lPickupDesc.GetCount();
}

//=========================================================================

layout_editor::pickup_desc& layout_editor::GetPickupDesc ( xhandle hDesc )
{
    if( hDesc.IsNull() )
        e_throw( "Invalid handle for a description" );

    return m_lPickupDesc( hDesc );
}

//=========================================================================

layout_editor::pickup_desc& layout_editor::GetPickupDesc ( s32 Index )
{
    if( Index < 0 || Index >= m_lPickupDesc.GetCount() ) 
        e_throw( "Unable to get the requested piece" );

    return m_lPickupDesc[Index];
}

//========================================================================= 

layout_editor::pickup_desc& layout_editor::GetPickupDesc ( const char* pFileName )
{
    return GetPickupDesc( GetPickupDescIndexByName( pFileName ) );
}

//=========================================================================

void layout_editor::AddGlow ( sphere& Glow, xcolor Color, f32 Intensity )
{
    //
    // Add the glow into the world
    //
    xhandle     hInst;
    glow&       T = m_lGlow.Add( hInst );
    T.Guid        = guid_New();
    T.Sphere      = Glow;
    T.Color       = Color;
    T.Intensity   = Intensity;
    T.Flags       = 0;

    // update the guid lookup
    m_lGuid2Handle.Add( T.Guid, hInst );

    //
    // Create the undo for it
    //
    if( UpdateUndo() )
    {
        command&        Command = m_lUndo.AddList( m_hUndoCursor );
        undo_data&      Undo    = m_lUndo.AddNode( m_hUndoCursor );
        glow&          Data     = Undo.GetGlow();

        Data = T;

        Command.RedoCommand     = COMMAND_ADD_INST;    
        Command.UndoCommand     = COMMAND_DEL_INST;    
    }

}

//=========================================================================

s32 layout_editor::GetGlowCount ( void ) const
{
    return m_lGlow.GetCount();
}

//=========================================================================

layout_editor::glow& layout_editor::GetGlow ( s32 I )
{
    return m_lGlow[I];
}

//=========================================================================