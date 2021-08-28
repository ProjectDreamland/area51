//==============================================================================
//  WE_Decals.cpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
// This file contains the portion of the world_editor class that deals with
// creating and moving around decals.
//==============================================================================


#include "stdafx.h"
#include "WorldEditor.hpp"
#include "static_decal.hpp"
#include "..\Support\Decals\DecalMgr.hpp"
#include "..\Support\Decals\DecalDefinition.hpp"

//==============================================================================

static_decal* world_editor::GetTempDecal( void )
{
    guid    DecalGuid = m_guidLstTempObject[0];
    object* pObject   = g_ObjMgr.GetObjectByGuid(DecalGuid);

    // check if we have decal object to play with
    if( !pObject )
        return NULL;
    if( !pObject->IsKindOf( static_decal::GetRTTI() ) )
        return NULL;
    
    return &static_decal::GetSafeType( *pObject );
}

//==============================================================================

void world_editor::StartNewDecal( void )
{
    // set up the initial decal settings
    static_decal* pDecalObj = GetTempDecal();
    if ( !pDecalObj )
        return;

    const decal_definition* pDecalDef = pDecalObj->GetDecalDefinition();
    if ( pDecalDef )
    {
        pDecalObj->SetSize( pDecalDef->RandomSize() );
        pDecalObj->SetRoll( pDecalDef->RandomRoll() );
    }

    pDecalObj->SetValid( FALSE );
}

//==============================================================================

xbool   g_CaptureRay = TRUE;
vector3 g_LastRayStart;
vector3 g_LastRayEnd;

xbool world_editor::CalcDecalPlacement( vector3& RayStart, vector3& RayEnd )
{
    // get the decal object to be moved
    static_decal* pDecalObj = GetTempDecal();
    if ( !pDecalObj )
        return FALSE;

    // get the decal's definition
    const decal_definition* pDecalDef = pDecalObj->GetDecalDefinition();
    if ( !pDecalDef )
    {
        pDecalObj->SetValid( FALSE );
        return FALSE;
    }

    // place the decal using the decal manager
    matrix4 L2W;

    //####
    //RayStart.Set( 2915.42f, 998.093f, 709.382f );
    //RayEnd.Set( 2915.42f, -8940.60f, -709.382f );
    //RayStart.Set( 2000.0f, 1000.0f, 2000.0f );
    //RayEnd.Set( 6738.44f, -3714.28f, -5437.92f );
    //RayStart.Set( 2888.83f, 365.220f, 1026.69f );
    //RayEnd.Set( 5499.74f, -8619.75f, -2502.26f );
    //RayStart.Set( 2877.40f, 1078.38f, -159.555f );
    //RayEnd.Set( 2787.51f, -7908.15f, 4226.12f );
    //RayStart.Set( 4497.41f, 1485.10f, 750.027f );
    //RayEnd.Set( 2595.72f, -8286.97f, 1693.58f );
    if (!g_CaptureRay)
    {
        RayStart.Set( 3011.48f, 273.086f, -154.651f );
        RayEnd.Set( 6570.08f, -2291.77f, 8831.89f );
    }
    else
    {
        g_LastRayStart = RayStart;
        g_LastRayEnd = RayEnd;
    }
    
    s32     nVerts = g_DecalMgr.CreateStaticDecalData( *pDecalDef,
                                                       RayStart,
                                                       RayEnd,
                                                       pDecalObj->GetSize(),
                                                       pDecalObj->GetRoll(),
                                                       pDecalObj->GetVertPtr(),
                                                       L2W );

    // let the decal know about our success
    pDecalObj->SetNVerts( nVerts );
    pDecalObj->SetValid( nVerts ? TRUE : FALSE );
    if ( nVerts )
    {
        pDecalObj->OnTransform( L2W );
    }

    return (nVerts>0);
}

//==============================================================================

guid world_editor::PlaceDecalFromTemporary( void )
{
    guid Guid;

    static_decal* pDecal = g_WorldEditor.GetTempDecal();
    if ( pDecal && pDecal->IsValid() )
    {
        Guid = PlaceObjectsFromTemporary();

        // force the transform to get updated so the world bbox is calcuated
        // AFTER the valid flag is set
        object* pObj = g_ObjMgr.GetObjectByGuid(Guid);

        if ( pObj )
            pObj->OnTransform( pObj->GetL2W() );
    }

    return Guid;
}

//==============================================================================

void world_editor::ExportDecals( const char* FileName, xarray<guid>& lstGuids, platform PlatformType )
{
    g_DecalMgr.BeginStaticDecalExport( FileName );

    s32 i;
    for ( i = 0; i < lstGuids.GetCount(); i++ )
    {
        guid Guid = lstGuids[i];

        object* pObj = g_ObjMgr.GetObjectByGuid(Guid);
        if ( !pObj )
            continue;

        if( !pObj->IsKindOf( static_decal::GetRTTI() ) )
            continue;

        static_decal& Decal = static_decal::GetSafeType( *pObj );
        if ( !Decal.IsValid() )
            continue;

        g_DecalMgr.AddStaticDecalToExport( Decal.GetPackageName(),
                                           Decal.GetGroup(),
                                           Decal.GetDecalDef(),
                                           Decal.GetNVerts(),
                                           Decal.GetVertPtr(),
                                           Decal.GetL2W(),
                                           Decal.GetZone1()|(Decal.GetZone2()<<8) );
    }

    g_DecalMgr.EndStaticDecalExport( PlatformType );
}

//==============================================================================
