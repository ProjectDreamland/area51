//==============================================================================
//  VirtualTextureMask.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//==============================================================================

#include "VirtualTextureMask.hpp"
#include "MiscUtils\Property.hpp"
#include "Render\Geom.hpp"

//==============================================================================
// STATICS
//==============================================================================

#if defined(X_EDITOR)
dictionary virtual_texture_mask::VTextureDictionary;
#endif

//==============================================================================
// IMPLEMENTATION
//==============================================================================

virtual_texture_mask::virtual_texture_mask( void ) :
#if defined(X_EDITOR)
nVTextures    ( 0 ),
#endif
VTextureMask  ( 0x0 )
{
}

//==============================================================================

void virtual_texture_mask::OnEnumProp( prop_enum& List, geom* pGeom )
{
    (void)pGeom;

    List.PropEnumHeader( "VTextureList", "Properties related to a virtual texture mask", 0 );
    s32 HeaderId = List.PushPath( "VTextureList\\" );

#if defined(X_EDITOR)
    s32 i;

    // if we have geometry present we should match up the vtextures we think we should
    // have with the vtextures the geometry has available
    SyncVTextures( pGeom );

    // enumerate the list of virtual textures, but don't display them
    s32 SaveId = List.PushPath( "SavedData\\" );
    List.PropEnumInt( "NumVTextures",
                        "Number of virtual textures",
                        PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT | PROP_TYPE_MUST_ENUM );
    for( i = 0; i < nVTextures; i++ )
    {
        List.PropEnumString( xfs( "VTexture[%d]", i ),
                             "The name of the virtual texture to be set",
                             PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT );
        List.PropEnumString( xfs( "VTextureChoice[%d]", i ),
                             "The current setting of the virtual texture",
                             PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT );
    }
    List.PopPath( SaveId );

    // now enumerate the list the geometry says we have
    if( pGeom )
    {
        for( i = 0; i < pGeom->m_nVirtualTextures; i++ )
        {
            List.PropEnumEnum( xfs("VTexture[%d] - %s", i, pGeom->GetVTextureName(i) ),
                                BuildEnumList( pGeom, i ),
                                "Which texture set should this mesh use?",
                                PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
        }
    }
#endif

    // add the mask property
    List.PropEnumInt( "Mask", "The final mask that the game uses", PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE );

    List.PopPath( HeaderId );
}

//==============================================================================

xbool virtual_texture_mask::OnProperty( prop_query& I, geom* pGeom )
{
    (void)pGeom;

    // early out
    if( !I.IsBasePath( "VTextureList" ) )
    {
        return FALSE;
    }

    s32 HeaderId = I.PushPath( "VTextureList\\" );

#if defined(X_EDITOR)
    // handle the saved data
    if( I.IsVar( "SavedData\\NumVTextures" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( nVTextures );
        }
        else
        {
            nVTextures = I.GetVarInt();
            for( s32 i = 0; i < nVTextures; i++ )
            {
                VTextureNameIds[i]   = VTextureDictionary.Add( "" );
                VTextureChoiceIds[i] = VTextureDictionary.Add( "" );
            }
        }
    }
    else if( I.IsVar( "SavedData\\VTexture[]" ) )
    {
        s32 Index = I.GetIndex( I.GetIndexCount() - 1 );
        if( I.IsRead() )
        {
            I.SetVarString( VTextureDictionary.GetString( VTextureNameIds[Index] ), 256 );
        }
        else
        {
            VTextureNameIds[Index] = VTextureDictionary.Add( I.GetVarString() );
        }
    }
    else if( I.IsVar( "SavedData\\VTextureChoice[]" ) )
    {
        s32 Index = I.GetIndex( I.GetIndexCount() - 1 );
        if( I.IsRead() )
        {
            I.SetVarString( VTextureDictionary.GetString( VTextureChoiceIds[Index] ), 256 );
        }
        else
        {
            VTextureChoiceIds[Index] = VTextureDictionary.Add( I.GetVarString() );
        }
    }
    else if( I.IsBasePath( "VTexture[] - " ) )
    {
        const char* pVTextureName = x_strstr( I.GetName(), "VTexture[] - " );
        if( pVTextureName )
        {
            pVTextureName += x_strlen( "VTexture[] - " );
            if( I.IsRead() )
            {
                // is this vtexture in our list of vtextures?
                s32 iVTexture;
                for( iVTexture = 0; iVTexture < nVTextures; iVTexture++ )
                {
                    if( !x_strcmp( VTextureDictionary.GetString(VTextureNameIds[iVTexture]), pVTextureName ) )
                    {
                        I.SetVarEnum( VTextureDictionary.GetString( VTextureChoiceIds[iVTexture] ) );
                        break;
                    }
                }
                if( iVTexture == nVTextures )
                {
                    I.SetVarEnum( "" );
                }
            }
            else
            {
                // is this vtexture in our list of vtextures?
                s32 iVTexture;
                for( iVTexture = 0; iVTexture < nVTextures; iVTexture++ )
                {
                    if( !x_strcmp( VTextureDictionary.GetString(VTextureNameIds[iVTexture]), pVTextureName ) )
                    {
                        VTextureChoiceIds[iVTexture] = VTextureDictionary.Add( I.GetVarEnum() );
                        SyncVTextures( pGeom );
                        break;
                    }
                }
            }
        }
    }
    else
#endif
    if( I.VarInt( "Mask", (s32&)VTextureMask ) )
    {
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

s32 virtual_texture_mask::FindFirstMat( geom* pGeom, s32 iVTexture )
{
    ASSERT( (iVTexture >= 0) && (iVTexture < pGeom->m_nVirtualTextures) );

    // find any material this vtexture operates on
    s32 iMaterial;
    for( iMaterial = 0; iMaterial < pGeom->m_nMaterials; iMaterial++ )
    {
        if( pGeom->m_pVirtualTextures[iVTexture].MaterialMask & (1<<iMaterial) )
        {
            return iMaterial;
        }
    }

    return -1;
}

//==============================================================================

const char* virtual_texture_mask::BuildEnumList( geom* pGeom, s32 iVTexture )
{
    static xstring EnumList;
    ASSERT( (iVTexture >= 0) && (iVTexture < pGeom->m_nVirtualTextures) );

    EnumList.Clear();

    s32 iMaterial = FindFirstMat( pGeom, iVTexture );

    // now add the textures this material uses, separating them with a '~'
    if( iMaterial < 0 )
    {
        EnumList += "~~";
    }
    else
    {
        geom::material& GeomMat = pGeom->m_pMaterial[iMaterial];

        s32 iTexture;
        for( iTexture = GeomMat.iTexture; iTexture < GeomMat.iTexture + GeomMat.nTextures; iTexture++ )
        {
            EnumList += pGeom->GetTextureDesc( iTexture );
            EnumList += "~";
        }
    }

    // replace the '~' characters with terminators
    s32 i;
    for( i = 0; EnumList[i]; i++ )
    {
        if( EnumList[i] == '~' )
            EnumList[i] = 0;
    }

    return EnumList;
}

//==============================================================================

void virtual_texture_mask::SyncVTextures( geom* pGeom )
{
    (void)pGeom;
#if defined(X_EDITOR)
    if( pGeom )
    {
        s32 i, j;

        // match up the vtextures to what is actually available in the geometry
        s32 NewVTextureNameIds[MAX_VTEXTURES];
        s32 NewVTextureChoiceIds[MAX_VTEXTURES];
        for( i = 0; i < pGeom->m_nVirtualTextures; i++ )
        {
            // the vtexture name id comes directly from the geometry
            NewVTextureNameIds[i] = VTextureDictionary.Add( pGeom->GetVTextureName( i ) );

            // default the vtexture choice to the first available choice to start with
            s32 iMaterial = FindFirstMat( pGeom, i );
            if( iMaterial < 0 )
            {
                NewVTextureChoiceIds[i] = VTextureDictionary.Add( "" );
            }
            else
            {
                s32 iTexture = pGeom->m_pMaterial[iMaterial].iTexture;
                NewVTextureChoiceIds[i] = VTextureDictionary.Add( pGeom->GetTextureDesc(iTexture) );
            }

            // if this vtexture already had a choice specified, use that one instead
            for( j = 0; j < nVTextures; j++ )
            {
                if( !x_strcmp( pGeom->GetVTextureName(i), VTextureDictionary.GetString( VTextureNameIds[j] ) ) )
                {
                    NewVTextureChoiceIds[i] = VTextureChoiceIds[j];
                    break;
                }
            }
        }

        x_memcpy( VTextureNameIds, NewVTextureNameIds, MAX_VTEXTURES*sizeof(s32) );
        x_memcpy( VTextureChoiceIds, NewVTextureChoiceIds, MAX_VTEXTURES*sizeof(s32) );
        nVTextures = pGeom->m_nVirtualTextures;

        // now we should reset our vtexture mask
        VTextureMask = 0;
        for( i = 0; i < nVTextures; i++ )
        {
            s32 VTextureIndex = pGeom->GetVTextureIndex( VTextureDictionary.GetString(VTextureNameIds[i]) );
            if( VTextureIndex != -1 )
            {
                s32 iMaterial = FindFirstMat( pGeom, VTextureIndex );
                if( iMaterial != -1 )
                {
                    s32 Choice = 0;
                    s32 iTexture;
                    geom::material& GeomMat = pGeom->m_pMaterial[iMaterial];
                    for( iTexture = GeomMat.iTexture;
                         iTexture < GeomMat.iTexture + GeomMat.nVirtualMats;
                         iTexture++ )
                    {
                        if( !x_strcmp( pGeom->GetTextureDesc(iTexture),
                                       VTextureDictionary.GetString(VTextureChoiceIds[i]) ) )
                        {
                            VTextureMask |= (iTexture-GeomMat.iTexture) << (i*4);
                            break;
                        }
                    }
                }
            }
        }
    }
#endif
}

//==============================================================================
