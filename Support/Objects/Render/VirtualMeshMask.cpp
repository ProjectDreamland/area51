//==============================================================================
//  VirtualMeshMask.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//==============================================================================

#include "VirtualMeshMask.hpp"
#include "MiscUtils\Property.hpp"
#include "Render\Geom.hpp"

//==============================================================================
// STATICS
//==============================================================================

#if defined(X_EDITOR)
dictionary virtual_mesh_mask::VMeshDictionary;
extern xbool g_game_running;
#endif

//==============================================================================
// IMPLEMENTATION
//==============================================================================

virtual_mesh_mask::virtual_mesh_mask( void ) :
#if defined(X_EDITOR)
    nVMeshes    ( 0 ),
#endif
    VMeshMask   ( 0xffffffff )
{
}

//==============================================================================

void virtual_mesh_mask::OnEnumProp( prop_enum& List, geom* pGeom )
{
    (void)pGeom;
    
    List.PropEnumHeader( "VMeshList", "Properties related to a virtual mesh mask", 0 );
    s32 HeaderId = List.PushPath( "VMeshList\\" );

#if defined(X_EDITOR)
    s32 i;

    // if we have geometry present we should match up the vmeshes we think we should
    // have with the vmeshes the geometry has available
    SyncVMeshes( pGeom );

    // enumerate the list of virtual meshes that are marked visible, but don't display them
    // SPECIAL NOTE: SavedData needs to stick around because of a flaw in the initial logic.
    // I thought we'd just be okay saving the visible meshes, but that doesn't work when
    // you start overriding blueprint data. Now the SavedData is used just to get things
    // started and to also keep a list around in case we save the layer file with
    // geometry loaded. If this were to be re-written, I'd probably just store a bool
    // value for every mesh and call it a day! Unfortunately, we need to be backwards
    // compatible or everything explodes, so we're stuck with this for a while.
    s32 SaveId = List.PushPath( "SavedData\\" );
    List.PropEnumInt( "NumVMeshes",
                      "Number of visible vmeshes",
                      PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT | PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE );
    for( i = 0; i < nVMeshes; i++ )
    {
        List.PropEnumString( xfs( "VisibleMesh[%d]", i ),
                             "The name of a visible vmesh",
                             PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SAVE );
    }
    List.PopPath( SaveId );

    // now enumerate the list the geometry says we have
    if( pGeom )
    {
        for( i = 0; i < pGeom->m_nVirtualMeshes; i++ )
        {
            List.PropEnumBool( xfs("VMesh[%d] - %s", i, pGeom->GetVMeshName(i) ),
                                "Is this mesh visible or not?",
                                PROP_TYPE_DONT_EXPORT );
        }
    }
    else
    {
        for( i = 0; i < nVMeshes; i++ )
        {
            List.PropEnumBool( xfs("VMesh[%d] - %s", i, VMeshDictionary.GetString(VMeshNameIds[i]) ),
                               "Is this mesh visible or not?",
                               PROP_TYPE_DONT_EXPORT );
        }
    }
#endif

    // add the mask property
    List.PropEnumInt( "Mask", "The final mask that the game uses", PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE );

    List.PopPath( HeaderId );
}

//==============================================================================

xbool virtual_mesh_mask::OnProperty( prop_query& I, geom* pGeom )
{
    (void)pGeom;

    // early out
    if( !I.IsBasePath( "VMeshList" ) )
    {
        return FALSE;
    }

    s32 HeaderId = I.PushPath( "VMeshList\\" );

#if defined(X_EDITOR)
    // handle the saved data - SEE THE NOTES IN OnEnumProp - this is here
    // for backwards compatibility only!!!
    if( I.IsVar( "SavedData\\NumVMeshes" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( nVMeshes );
        }
        else
        {
            nVMeshes = I.GetVarInt();
            for( s32 i = 0; i < nVMeshes; i++ )
            {
                VMeshNameIds[i] = VMeshDictionary.Add( "" );
            }

            VMeshMask = 0;
        }
    }
    else if( I.IsVar( "SavedData\\VisibleMesh[]" ) )
    {
        s32 Index = I.GetIndex( I.GetIndexCount()-1 );
        if( I.IsRead() )
        {
            I.SetVarString( VMeshDictionary.GetString( VMeshNameIds[Index] ), 256 );
        }
        else
        {
            VMeshNameIds[Index] = VMeshDictionary.Add( I.GetVarString() );
            if( Index == nVMeshes-1 )
            {
                // if it's the last mesh to be read in, re-sync with the geometry
                SyncVMeshes( pGeom );
            }
        }
    }
    else if( I.IsBasePath( "VMesh[] - " ) )
    {
        const char* pVMeshName = x_strstr( I.GetName(), "VMesh[] - " );
        if( pVMeshName )
        {
            pVMeshName += x_strlen( "VMesh[] - " );
            if( I.IsRead() )
            {
                // is this mesh in our list of vmeshes?
                s32 iVMesh;
                for( iVMesh = 0; iVMesh < nVMeshes; iVMesh++ )
                {
                    if( !x_strcmp( VMeshDictionary.GetString(VMeshNameIds[iVMesh]), pVMeshName ) )
                    {
                        I.SetVarBool( TRUE );
                        break;
                    }
                }
                if( iVMesh == nVMeshes )
                {
                    I.SetVarBool( FALSE );
                }
            }
            else
            {
                // should we add or remove this vmesh our list of vmeshes?
                if( I.GetVarBool() )
                {
                    // is this vmesh already in the list of visible vmeshes?
                    s32 iVMesh;
                    for( iVMesh = 0; iVMesh < nVMeshes; iVMesh++ )
                    {
                        if( !x_strcmp( VMeshDictionary.GetString(VMeshNameIds[iVMesh]), pVMeshName ) )
                        {
                            break;
                        }
                    }
                    // no? then add it
                    if( iVMesh == nVMeshes )
                    {
                        VMeshNameIds[nVMeshes] = VMeshDictionary.Add( pVMeshName );
                        nVMeshes++;

                        // if we've changed something, sync everything
                        SyncVMeshes( pGeom );
                    }
                }
                else
                {
                    if( nVMeshes == 0 )
                    {
                        SyncVMeshes (pGeom);
                    }

                    // is this vmesh in the list of visible vmeshes?
                    s32 iVMesh;
                    for( iVMesh = 0; iVMesh < nVMeshes; iVMesh++ )
                    {
                        if( !x_strcmp( VMeshDictionary.GetString(VMeshNameIds[iVMesh]), pVMeshName ) )
                        {
                            break;
                        }
                    }

                    // if so, then remove it
                    if( iVMesh < nVMeshes )
                    {
                        nVMeshes--;
                        x_memmove( &VMeshNameIds[iVMesh],
                                   &VMeshNameIds[iVMesh+1],
                                   sizeof(s32)*(nVMeshes-iVMesh) );

                        // and since we've changed something, re-sync everything
                        SyncVMeshes( pGeom );
                    }
                }
            }
        }
    }
    else
#endif
    if( I.VarInt( "Mask", (s32&)VMeshMask ) )
    {
        // Blech...nasty hack to keep guys rendering when clicking on them
        // while the game is running in the editor. This is because some
        // of the properties are editor side only and don't get enumerated
        // from the blueprint, because it's mimicking what the game would
        // do. And the game uses the mask only, and doesn't give a fig
        // about the rest of the properties.
#ifdef X_EDITOR
        if( !I.IsRead() && g_game_running && pGeom )
        {
            s32 i;
            nVMeshes = 0;
            for( i = 0; i < pGeom->m_nVirtualMeshes; i++ )
            {
                if( VMeshMask & (1<<i) )
                {
                    VMeshNameIds[nVMeshes] = VMeshDictionary.Add( pGeom->GetVMeshName(i) );
                    nVMeshes++;
                }
            }
        }
#endif
    }
    else
    {
        I.PopPath( HeaderId );
        return FALSE;
    }

    I.PopPath( HeaderId );
    return TRUE;
}

//==============================================================================

void virtual_mesh_mask::SyncVMeshes( geom* pGeom )
{
    (void)pGeom;
#if defined(X_EDITOR)
    if( pGeom )
    {
        s32 i;

        // if we have no vmeshes in our list, yet the mask says everything is
        // visible, then we've never had geometry before...now we have geometry,
        // so we can set it up properly
        if( (VMeshMask == 0xffffffff) && (nVMeshes == 0) )
        {
            nVMeshes = pGeom->m_nVirtualMeshes;
            for( i = 0; i < pGeom->m_nVirtualMeshes; i++ )
            {
                VMeshNameIds[i] = VMeshDictionary.Add( pGeom->GetVMeshName( i ) );
            }
        }

        // match up the vmeshes we have to what is actually available in the geometry
        for( i = 0; i < nVMeshes; i++ )
        {
            s32 VMeshIndex = pGeom->GetVMeshIndex( VMeshDictionary.GetString(VMeshNameIds[i]) );
            if( VMeshIndex == -1 )
            {
                // this vmesh isn't present in the geometry, so remove it from our list as well
                nVMeshes--;
                x_memmove( &VMeshNameIds[i], &VMeshNameIds[i+1], sizeof(s32)*(nVMeshes-i) );
            }
        }

        // now we should reset our mesh mask
        VMeshMask = 0;
        for( i = 0; i < nVMeshes; i++ )
        {
            s32 VMeshIndex = pGeom->GetVMeshIndex( VMeshDictionary.GetString(VMeshNameIds[i]) );
            if( VMeshIndex != -1 )
            {
                VMeshMask |= (1<<VMeshIndex);
            }
        }
    }
#endif
}

//==============================================================================
