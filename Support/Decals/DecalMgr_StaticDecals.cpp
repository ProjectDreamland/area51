//==============================================================================
//  DecalMgr_StaticDecals.cpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This file handles all parts of the decal_mgr class pertaining to static
//  decals.
//==============================================================================

//==============================================================================
// Includes
//==============================================================================

#include "DecalMgr.hpp"
#include "Decals\DecalPackage.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "e_draw.hpp"
#include "Render\Render.hpp"

//==============================================================================
// Constants
//==============================================================================

static const s32    kRegInfoGrowAmount  = 256;

//==============================================================================
// Static data used in the export process
//==============================================================================

#ifdef TARGET_PC
struct export_decal
{
    char                    PackageName[256];
    s32                     iGroup;
    s32                     iDecalDef;
    s32                     nVerts;
    decal_mgr::decal_vert   Verts[decal_mgr::MAX_VERTS_PER_DECAL];
    u32                     ZoneInfo;
    xcolor                  Color;
};

static char                 s_ExportFileName[X_MAX_PATH];
static xbool                s_bExportingStaticDecals = FALSE;
static xarray<export_decal> s_lDecalExportList;
#endif

//==============================================================================
// The static data file code
//==============================================================================

decal_mgr::static_data::static_data( void ) :
    Version         ( STATIC_DECAL_VERSION ),
    nPackages       ( 0 ),
    pPackage        ( NULL ),
    nDefinitions    ( 0 ),
    pDefinition     ( NULL ),
    nZones          ( 0 ),
    pZone           ( NULL ),
    nVerts          ( 0 ),
    pPos            ( NULL ),
    pUV             ( NULL ),
    pColor          ( NULL )
{
}

//==============================================================================

decal_mgr::static_data::static_data( fileio& File )
{
    (void)File;

    if ( Version != STATIC_DECAL_VERSION )
        x_throw( xfs( "Static decal version problem. App wants %d, data is %d", STATIC_DECAL_VERSION, Version ) );
}

//==============================================================================

void decal_mgr::position_data::FileIO( fileio& File )
{
    File.Static( Pos );
    File.Static( Flags );
}

//==============================================================================

void decal_mgr::uv_data::FileIO( fileio& File )
{
    File.Static( U );
    File.Static( V );
}

//==============================================================================

void decal_mgr::static_data::package::FileIO( fileio& File )
{
    File.Static( PackageName, 256 );
    File.Static( iDefinition );
    File.Static( nDefinitions );
}

//==============================================================================

void decal_mgr::static_data::definition::FileIO( fileio& File )
{
    File.Static( iGroup );
    File.Static( iDecalDef );
    File.Static( iZoneInfo );
    File.Static( nZones );
}

//==============================================================================

void decal_mgr::static_data::zone_info::FileIO( fileio& File )
{
    File.Static( iVert );
    File.Static( nVerts );
    File.Static( Zone );
}

//==============================================================================

void decal_mgr::static_data::FileIO( fileio& File )
{
    File.Static( Version );
    File.Static( nPackages );
    File.Static( pPackage, nPackages );
    File.Static( nDefinitions );
    File.Static( pDefinition, nDefinitions );
    File.Static( nZones );
    File.Static( pZone, nZones );
    File.Static( nVerts );
    File.Static( pPos, nVerts );
    File.Static( pUV, nVerts );
    File.Static( pColor, nVerts );
}

//==============================================================================
// Implementation
//==============================================================================

#ifdef TARGET_PC
void decal_mgr::registration_info::GrowStaticVertListBy( s32 nVerts )
{
    s32 AllocSize = GetAllocSize( m_nStaticVertsAlloced + nVerts );

    if ( m_nStaticVertsAlloced == 0 )
    {
        // easy case, just alloc the new verts
        m_nStaticVertsAlloced  = nVerts;
        byte* pAllocAddress    = (byte*)x_malloc( AllocSize );
        m_pStaticPositions     = (position_data*)pAllocAddress;
        pAllocAddress         += ALIGN_16( m_nStaticVertsAlloced * sizeof(position_data) );
        m_pStaticUVs           = (uv_data*)pAllocAddress;
        pAllocAddress         += ALIGN_16( m_nStaticVertsAlloced * sizeof(uv_data) );
        m_pStaticColors        = (u32*)pAllocAddress;
        pAllocAddress         += ALIGN_16( m_nStaticVertsAlloced * sizeof(u32) );
        if( m_Flags & decal_definition::DECAL_FLAG_FADE_OUT )
            pAllocAddress += ALIGN_16( m_nStaticVertsAlloced * sizeof(f32) );
        ASSERT( pAllocAddress == ((byte*)m_pStaticPositions)+AllocSize );
    }
    else
    {
        // reallocate the space
        s32 nVertsAllocedBefore = m_nStaticVertsAlloced;
        m_nStaticVertsAlloced = m_nStaticVertsAlloced + nVerts;
        byte* pAllocAddress   = (byte*)x_realloc( m_pStaticPositions, AllocSize );
        byte* pTemp           = pAllocAddress;
 
        // what will the new data pointers be?
        pTemp                = pAllocAddress;
        byte* pNewPositions  = pTemp;
        pTemp               += ALIGN_16( m_nStaticVertsAlloced * sizeof(position_data) );
        byte* pNewUVs        = pTemp;
        pTemp               += ALIGN_16( m_nStaticVertsAlloced * sizeof(uv_data) );
        byte* pNewColors     = pTemp;
        pTemp               += ALIGN_16( m_nStaticVertsAlloced * sizeof(u32) );
        if( m_Flags & decal_definition::DECAL_FLAG_FADE_OUT )
            pTemp += ALIGN_16( m_nStaticVertsAlloced * sizeof(f32) );
        ASSERT( pTemp == pAllocAddress+AllocSize );
 
        // copy the old data into the new (copy fromt the colors backwards so
        // we don't overwrite anything in case we grew in place)
        pTemp                = pAllocAddress;
        byte* pOldPositions  = pTemp;
        pTemp               += ALIGN_16( nVertsAllocedBefore * sizeof(position_data) );
        byte* pOldUVs        = pTemp;
        pTemp               += ALIGN_16( nVertsAllocedBefore * sizeof(uv_data) );
        byte* pOldColors     = pTemp;
        pTemp               += ALIGN_16( nVertsAllocedBefore * sizeof(u32) );
        x_memmove( pNewColors,    pOldColors,    m_nStaticVerts*sizeof(u32) );
        x_memmove( pNewUVs,       pOldUVs,       m_nStaticVerts*sizeof(uv_data) );
        x_memmove( pNewPositions, pOldPositions, m_nStaticVerts*sizeof(position_data) );

 
        // and set the new pointers
        m_pStaticPositions = (position_data*)pNewPositions;
        m_pStaticUVs       = (uv_data*)pNewUVs;
        m_pStaticColors    = (u32*)pNewColors;
    }
}



//==============================================================================

void decal_mgr::RenderStaticDecal( const decal_definition& Def,
                                   const decal_vert*       pVerts,
                                   s32                     nVerts,
                                   const matrix4&          L2W,
                                   xbool                   Wireframe )
{
    ASSERT( Def.m_Handle.IsNonNull() );

    // make sure we have room to add the verts
    registration_info& RegInfo = m_RegisteredDefs( Def.m_Handle );
    if ( (RegInfo.m_nStaticVerts + nVerts) > (RegInfo.m_nStaticVertsAlloced) )
        RegInfo.GrowStaticVertListBy( kRegInfoGrowAmount );

    // grab pointers to where we'll be filling in the vertex data
    position_data* pPos   = &RegInfo.m_pStaticPositions[RegInfo.m_nStaticVerts];
    uv_data*       pUV    = &RegInfo.m_pStaticUVs[RegInfo.m_nStaticVerts];
    u32*           pColor = &RegInfo.m_pStaticColors[RegInfo.m_nStaticVerts];

    // fill in the vertex data
    for ( s32 i = 0; i < nVerts; i++ )
    {
        pPos[i].Flags = pVerts[i].Flags;
        pPos[i].Pos   = L2W * pVerts[i].Pos;
        pUV[i].U      = (s16)(pVerts[i].UV.X*4096.0f);
        pUV[i].V      = (s16)(pVerts[i].UV.Y*4096.0f);
        pColor[i]     = Def.m_Color;

        if ( Wireframe )
            pPos[i].Flags |= decal_vert::FLAG_DRAW_WIREFRAME;
        else
            pPos[i].Flags &= ~decal_vert::FLAG_DRAW_WIREFRAME;
    }

    // success
    RegInfo.m_nStaticVerts += nVerts;
}

//==============================================================================

s32 decal_mgr::CreateStaticDecalData( const decal_definition& Def,
                                      const vector3&          Start,
                                      const vector3&          End,
                                      const vector2&          Size,
                                      radian                  Roll,
                                      decal_vert              DecalVerts[MAX_VERTS_PER_DECAL],
                                      matrix4&                L2W )
{
    // cast a ray, and see if there were any collisions
    g_CollisionMgr.RaySetup( NULL, Start, End );    
    g_CollisionMgr.CheckCollisions( object::TYPE_PLAY_SURFACE,
                                    object::ATTR_COLLIDABLE,
                                    object::ATTR_COLLISION_PERMEABLE );
    if ( g_CollisionMgr.m_nCollisions == 0 )
        return 0;

    // get the collision information
    collision_mgr::collision& Collision = g_CollisionMgr.m_Collisions[0];

    // generate the decal verts
    vector3 NegIncomingRay = (Start-End);
    NegIncomingRay.Normalize();
    
    s32 nVerts = CalcDecalVerts( Def.m_Flags,
                                 Collision.Point,
                                 Collision.Plane.Normal,
                                 NegIncomingRay,
                                 Size,
                                 Roll,
                                 DecalVerts,
                                 L2W );

    if ( !(Def.m_Flags & decal_definition::DECAL_FLAG_NO_CLIP) &&
         (Def.m_Flags & decal_definition::DECAL_FLAG_USE_PROJECTION) )
    {
        if ( nVerts )
        {
            // Transform the verts into the new local space which requires translation only.
            // This is preferable to keep decals from getting crazy huge bboxes for editor
            // selection, movement, etc.
            s32     i;
            matrix4 L2WRotateOnly = L2W;
            L2WRotateOnly.ClearTranslation();
            for ( i = 0; i < nVerts; i++ )
            {
                DecalVerts[i].Pos = L2WRotateOnly * DecalVerts[i].Pos;
            }

            L2W.ClearRotation();
            L2W.ClearScale();
        }
    }

    return nVerts;


/*
    // figure out the normal (it will either come from the collision poly or from
    // the incoming ray)
    vector3 Normal = Collision.Plane.Normal;
    Normal.Normalize();

    if ( Def.m_Flags & decal_definition::DECAL_FLAG_NO_CLIP )
    {
        // The most efficient way to do a decal is a coplanar poly that is not clipped.
        // Because of that, we'll go through a separate pipeline to get the points.
        // This means we will not be doing any kind of projection, but that should
        // be fine since this is meant for small stuff like bullet holes.
        return CalcNoClipDecal( Def.m_Flags, Collision.Point, -Normal, Size, Roll, DecalVerts, L2W );
    }
    else
    {
        // Create a proper 3d decal. We'll set up an orthogonal projection matrix,
        // clip to that volume, calculate alpha based on the angle of approach,
        // and modify alpha based on distance from the initial strike point.
        if ( Def.m_Flags & decal_definition::DECAL_FLAG_USE_PROJECTION )
        {
            vector3 NegIncomingRay = (Start-End);
            NegIncomingRay.Normalize();

            s32 nVerts = CalcProjectedDecal( Collision.Point, Normal, NegIncomingRay, Size, Roll, DecalVerts, L2W );

            if ( nVerts )
            {
                // Transform the verts into the new local space which requires translation only.
                // This is preferable to keep decals from getting crazy huge bboxes for editor
                // selection, movement, etc.
                s32     i;
                matrix4 L2WRotateOnly = L2W;
                L2WRotateOnly.ClearTranslation();
                for ( i = 0; i < nVerts; i++ )
                {
                    DecalVerts[i].Pos = L2WRotateOnly * DecalVerts[i].Pos;
                }

                L2W.ClearRotation();
                L2W.ClearScale();
            }
            return nVerts;
        }
        else
        {
            return CalcProjectedDecal( Collision.Point, Normal, Normal, Size, Roll, DecalVerts, L2W );
        }
    }
    */
}

//==============================================================================

void decal_mgr::BeginStaticDecalExport( const char* FileName )
{
    ASSERT( !m_pStaticData );
    m_pStaticData = new static_data;
    if ( !m_pStaticData )
        return;

    x_strcpy( s_ExportFileName, FileName );
    s_bExportingStaticDecals = TRUE;
    s_lDecalExportList.Clear();
}

//==============================================================================

void decal_mgr::AddStaticDecalToExport( const char*         PackageName,
                                        s32                 iGroup,
                                        s32                 iDecalDef,
                                        s32                 nVerts,
                                        const decal_vert    Verts[MAX_VERTS_PER_DECAL],
                                        const matrix4&      L2W,
                                        u16                 ZoneInfo )
{
    ASSERT( s_bExportingStaticDecals );
    if ( !s_bExportingStaticDecals )
        return;

    // make sure the package is loaded
    rhandle<decal_package> PackageRsc;
    PackageRsc.SetName( PackageName );
    decal_package* pPackage = PackageRsc.GetPointer();
    if ( !pPackage )
        return;

    // do some sanity checking before saving out an invalid decal
    if ( PackageName[0] == '\0' )
        return;

    if ( (iGroup<0) || (iGroup>=pPackage->GetNGroups()) )
        return;

    if ( (iDecalDef<0) || (iDecalDef>=pPackage->GetNDecalDefs(iGroup)) )
        return;

    if ( nVerts == 0 )
        return;

    // save out the decal into our temp list
    export_decal& Decal = s_lDecalExportList.Append();
    x_strsavecpy( Decal.PackageName, PackageName, 256 );
    Decal.iGroup    = iGroup;
    Decal.iDecalDef = iDecalDef;
    Decal.nVerts    = nVerts;
    Decal.ZoneInfo  = ZoneInfo;
    Decal.Color     = pPackage->GetDecalDef(iGroup,iDecalDef).m_Color;
    for ( s32 i = 0; i < nVerts; i++ )
    {
        Decal.Verts[i].Flags = Verts[i].Flags & decal_vert::FLAG_SKIP_TRIANGLE;
        Decal.Verts[i].Pos   = L2W * Verts[i].Pos;
        Decal.Verts[i].UV    = Verts[i].UV;
    }
}

//==============================================================================

static s32 ExportDecalCompareFn( const void* pA, const void* pB )
{
    const export_decal* pDecalA = (const export_decal*)pA;
    const export_decal* pDecalB = (const export_decal*)pB;

    s32 Result = x_stricmp( pDecalA->PackageName, pDecalB->PackageName );
    if ( Result > 0 )   return 1;
    if ( Result < 0 )   return -1;
    
    if ( pDecalA->iGroup    > pDecalB->iGroup    ) return 1;
    if ( pDecalA->iGroup    < pDecalB->iGroup    ) return -1;
    if ( pDecalA->iDecalDef > pDecalB->iDecalDef ) return 1;
    if ( pDecalA->iDecalDef < pDecalB->iDecalDef ) return -1;
    if ( pDecalA->ZoneInfo  > pDecalB->ZoneInfo  ) return 1;
    if ( pDecalA->ZoneInfo  < pDecalB->ZoneInfo  ) return -1;

    return 0;
}

//==============================================================================

void decal_mgr::SetupExportVertBuffers( platform PlatformType )
{
    static const s32 kVertsForAlign = 4;
    s32 HWBufferSize = render::GetHardwareBufferSize();

    // figure out how many verts we'll need to allocate
    s32 iZone, iDecal;
    s32 nVertsTotal  = 0;
    s32 nVertsInZone = 0;
    for ( iZone = 0; iZone < m_pStaticData->nZones; iZone++ )
    {
        // make sure this zone starts out aligned
        while ( nVertsTotal % kVertsForAlign )
            nVertsTotal++;

        // calculate how many verts we'll need for this zone
        nVertsInZone = 0;
        for ( iDecal = m_pStaticData->pZone[iZone].iVert;
              iDecal < m_pStaticData->pZone[iZone].iVert+m_pStaticData->pZone[iZone].nVerts;
              iDecal++ )
        {
            export_decal& Decal = s_lDecalExportList[iDecal];

            s32 StripStart = 0;
            s32 StripEnd   = 0;
            while ( StripEnd < Decal.nVerts )
            {
                StripStart = StripEnd;
                StripEnd   = StripStart+3;
                while ( StripEnd < Decal.nVerts )
                {
                    decal_vert& Vert = Decal.Verts[StripEnd];
                    if ( Vert.Flags & decal_vert::FLAG_SKIP_TRIANGLE )
                    {
                        break;
                    }
                    StripEnd++;
                }

                // we can't let a strip span a hardware buffer
                s32 nVertsInStrip = StripEnd-StripStart;
                if ( (nVertsInZone/HWBufferSize) !=
                     ((nVertsInZone+nVertsInStrip)/HWBufferSize) )
                {
                    nVertsInZone += HWBufferSize -
                                    (nVertsInZone%HWBufferSize);
                }

                // add the strip
                ASSERT( nVertsInStrip < HWBufferSize );
                nVertsInZone += nVertsInStrip;
            }
        }

        // add the zone verts to our total
        nVertsTotal += nVertsInZone;
    }

    // allocate some verts
    m_pStaticData->nVerts = nVertsTotal;
    m_pStaticData->pPos   = new decal_mgr::position_data[m_pStaticData->nVerts];
    m_pStaticData->pUV    = new decal_mgr::uv_data[m_pStaticData->nVerts];
    m_pStaticData->pColor = new u32[m_pStaticData->nVerts];

    // fill in some default verts to fill in the empty spaces at the end
    // of each vert buffer
    s32 iVert;
    for ( iVert = 0; iVert < m_pStaticData->nVerts; iVert++ )
    {
        m_pStaticData->pPos[iVert].Flags = decal_vert::FLAG_SKIP_TRIANGLE;
        m_pStaticData->pPos[iVert].Pos.Set( 0.0f, 0.0f, 0.0f );
        m_pStaticData->pUV[iVert].U  = 0;
        m_pStaticData->pUV[iVert].V  = 0;
        m_pStaticData->pColor[iVert] = 0;
    }

    // add the verts for each zone
    nVertsTotal = 0;
    for ( iZone = 0; iZone < m_pStaticData->nZones; iZone++ )
    {
        // make sure this zone starts out aligned
        while ( nVertsTotal % kVertsForAlign )
            nVertsTotal++;

        // add the verts for this zone
        nVertsInZone = 0;
        for ( iDecal = m_pStaticData->pZone[iZone].iVert;
              iDecal < m_pStaticData->pZone[iZone].iVert+m_pStaticData->pZone[iZone].nVerts;
              iDecal++ )
        {
            export_decal& Decal = s_lDecalExportList[iDecal];

            s32 StripStart = 0;
            s32 StripEnd   = 0;
            while ( StripEnd < Decal.nVerts )
            {
                StripStart = StripEnd;
                StripEnd   = StripStart+3;
                while ( StripEnd < Decal.nVerts )
                {
                    decal_vert& Vert = Decal.Verts[StripEnd];
                    if ( Vert.Flags & decal_vert::FLAG_SKIP_TRIANGLE )
                    {
                        break;
                    }
                    StripEnd++;
                }

                // we can't let a strip span a hardware buffer
                s32 nVertsInStrip = StripEnd-StripStart;
                if ( (nVertsInZone/HWBufferSize) !=
                     ((nVertsInZone+nVertsInStrip)/HWBufferSize) )
                {
                    nVertsInZone += HWBufferSize -
                                    (nVertsInZone%HWBufferSize);
                }

                // add the strip
                for ( iVert = StripStart; iVert < StripEnd; iVert++ )
                {
                    decal_vert& Vert = Decal.Verts[iVert];
                    m_pStaticData->pPos[nVertsTotal+nVertsInZone].Pos   = Vert.Pos;
                    m_pStaticData->pPos[nVertsTotal+nVertsInZone].Flags = Vert.Flags & decal_vert::FLAG_SKIP_TRIANGLE;
                    m_pStaticData->pUV[nVertsTotal+nVertsInZone].U      = (s16)(Vert.UV.X*4096.0f);
                    m_pStaticData->pUV[nVertsTotal+nVertsInZone].V      = (s16)(Vert.UV.Y*4096.0f);

                    u32& Color = m_pStaticData->pColor[nVertsTotal+nVertsInZone];
                    if( PlatformType == PLATFORM_PS2 )
                    {
                        Color = ((u8)(128.0f*(f32)(Decal.Color.R/255.0f))<<0) +
                                ((u8)(128.0f*(f32)(Decal.Color.G/255.0f))<<8) +
                                ((u8)(128.0f*(f32)(Decal.Color.B/255.0f))<<16) +
                                (0x80<<24);
                    }
                    else
                    {
                        Color = ( u8(Decal.Color.R) << 0 ) +
                                ( u8(Decal.Color.G) << 8 ) +
                                ( u8(Decal.Color.B) << 16 ) +
                                ( 0xff << 24 );
                    }
                    nVertsInZone++;
                }
            }
        }

        // now we can fill in the proper vert offset and count for the zone
        m_pStaticData->pZone[iZone].iVert  = nVertsTotal;
        m_pStaticData->pZone[iZone].nVerts = nVertsInZone;

        nVertsTotal += nVertsInZone;
    }

    // sanity check
    ASSERT( nVertsTotal == m_pStaticData->nVerts );
}

//==============================================================================

void decal_mgr::EndStaticDecalExport( platform PlatformType )
{
    ASSERT( s_bExportingStaticDecals );
    if ( !s_bExportingStaticDecals )
        return;

    // make sure we have decals to export
    if ( !s_lDecalExportList.GetCount() )
    {
        // if there are no static decals to export, save out the empty
        // structure and do an early out
        fileio File;
        File.Save( s_ExportFileName, *m_pStaticData, FALSE );

        delete m_pStaticData;
        m_pStaticData            = NULL;
        s_bExportingStaticDecals = FALSE;
        return;
    }

    // sort the decals to be exported
    x_qsort( s_lDecalExportList.GetPtr(),
             s_lDecalExportList.GetCount(),
             sizeof(export_decal),
             ExportDecalCompareFn );

    // count up the number of unique packages, definitions, and zones
    m_pStaticData->nPackages    = 0;
    m_pStaticData->nDefinitions = 0;
    m_pStaticData->nZones       = 0;
    m_pStaticData->nVerts       = 0;
    
    s32  i;
    s32  CurrGroup    = -1;
    s32  CurrDecalDef = -1;
    u32  CurrZone     = 0xffffffff;
    char CurrPackageName[256];
    CurrPackageName[0] = '\0';

    for ( i = 0; i < s_lDecalExportList.GetCount(); i++ )
    {
        const export_decal& Decal = s_lDecalExportList[i];

        // new package?
        if ( x_stricmp( CurrPackageName, Decal.PackageName ) )
        {
            m_pStaticData->nPackages++;
            x_strcpy( CurrPackageName, Decal.PackageName );
            CurrGroup    = -1;
            CurrDecalDef = -1;
            CurrZone     = 0xffffffff;
        }

        // new definition?
        if ( (Decal.iGroup != CurrGroup) || (Decal.iDecalDef != CurrDecalDef) )
        {
            m_pStaticData->nDefinitions++;
            CurrGroup    = Decal.iGroup;
            CurrDecalDef = Decal.iDecalDef;
            CurrZone     = 0xffffffff;
        }

        // new zone?
        if ( Decal.ZoneInfo != CurrZone )
        {
            m_pStaticData->nZones++;
            CurrZone     = Decal.ZoneInfo;
        }
    }

    // now we can allocate space for everything except verts
    m_pStaticData->pPackage    = new decal_mgr::static_data::package[m_pStaticData->nPackages];
    m_pStaticData->pDefinition = new decal_mgr::static_data::definition[m_pStaticData->nDefinitions];
    m_pStaticData->pZone       = new decal_mgr::static_data::zone_info[m_pStaticData->nZones];

    // and now we can fill in all of the data
    s32 iGroup    = 0;
    s32 iDecalDef = 0;
    s32 iZoneInfo = 0;
    s32 iPackage  = 0;
    
    CurrGroup          = -1;
    CurrDecalDef       = -1;
    CurrZone           = 0xffffffff;
    CurrPackageName[0] = '\0';

    const decal_package*    pPackage = NULL;
    const decal_definition* pDef     = NULL;

    for ( i = 0; i < s_lDecalExportList.GetCount(); i++ )
    {
        const export_decal& Decal = s_lDecalExportList[i];

        // new package?
        if ( x_stricmp( CurrPackageName, Decal.PackageName ) )
        {
            x_strcpy( CurrPackageName, Decal.PackageName );
            x_strtoupper( CurrPackageName );
            CurrGroup    = -1;
            CurrDecalDef = -1;
            CurrZone     = 0xffffffff;

            x_strcpy( m_pStaticData->pPackage[iPackage].PackageName, CurrPackageName );
            m_pStaticData->pPackage[iPackage].iDefinition  = iDecalDef;
            m_pStaticData->pPackage[iPackage].nDefinitions = 0;
            iPackage++;

            rhandle<decal_package> PackageRsc;
            PackageRsc.SetName( CurrPackageName );
            pPackage = PackageRsc.GetPointer();
            ASSERT( pPackage );
        }

        // new definition?
        if ( (Decal.iGroup != CurrGroup) || (Decal.iDecalDef != CurrDecalDef) )
        {
            CurrGroup    = Decal.iGroup;
            CurrDecalDef = Decal.iDecalDef;
            CurrZone     = 0xffffffff;

            m_pStaticData->pPackage[iPackage-1].nDefinitions++;
            m_pStaticData->pDefinition[iDecalDef].iGroup    = CurrGroup;
            m_pStaticData->pDefinition[iDecalDef].iDecalDef = CurrDecalDef;
            m_pStaticData->pDefinition[iDecalDef].iZoneInfo = iZoneInfo;
            m_pStaticData->pDefinition[iDecalDef].nZones    = 0;
            iDecalDef++;

            pDef = &pPackage->GetDecalDef( CurrGroup, CurrDecalDef );
        }

        // new zone?
        if ( Decal.ZoneInfo != CurrZone )
        {
            CurrZone     = Decal.ZoneInfo;
            
            m_pStaticData->pDefinition[iDecalDef-1].nZones++;
            m_pStaticData->pZone[iZoneInfo].Zone   = CurrZone;
            m_pStaticData->pZone[iZoneInfo].iVert  = i; // this will change once we calc the vert offsets
            m_pStaticData->pZone[iZoneInfo].nVerts = 0; // this will change once we calc the vert offsets
            iZoneInfo++;
        }

        // hijack the vert count as a decal count--this will all get recalculated
        // when we set up the export vert buffers
        m_pStaticData->pZone[iZoneInfo-1].nVerts++;
    }

    // calculate the vert offsets and buffers
    SetupExportVertBuffers( PlatformType );

    // sanity check
    ASSERT( iPackage  == m_pStaticData->nPackages    );
    ASSERT( iDecalDef == m_pStaticData->nDefinitions );
    ASSERT( iZoneInfo == m_pStaticData->nZones       );

    // save out the file
    fileio File;
    File.Save( s_ExportFileName, *m_pStaticData, FALSE );

    // delete the static data (the reason we're doing this here and
    // not in the destructor is because typically the data is not allocated
    // manually but comes from fileio
    delete []m_pStaticData->pPackage;
    delete []m_pStaticData->pDefinition;
    delete []m_pStaticData->pZone;
    delete []m_pStaticData->pPos;
    delete []m_pStaticData->pUV;
    delete []m_pStaticData->pColor;
    delete m_pStaticData;

    m_pStaticData            = NULL;
    s_bExportingStaticDecals = FALSE;
    s_lDecalExportList.Clear();
}

#endif // TARGET_PC

//==============================================================================

void decal_mgr::LoadStaticDecals( const char* FileName )
{
    // load the static data
    X_FILE* fh = x_fopen( FileName, "rb" );
    if ( fh )
    {
        fileio File;
        File.Load( fh, m_pStaticData );
        x_fclose(fh);
    }
    else
    {
        m_pStaticData = NULL;
    }

    // make sure all of the packages are loaded and let the registered
    // definitions know if they have any static decals attached
    if ( m_pStaticData )
    {
        s32 iPackage;
        
        for ( iPackage = 0; iPackage < m_pStaticData->nPackages; iPackage++ )
        {
            rhandle<decal_package> PackageRsc;
            PackageRsc.SetName( m_pStaticData->pPackage[iPackage].PackageName );
            
            const decal_package* pPackage = PackageRsc.GetPointer();
            ASSERT( pPackage );
            if ( pPackage )
            {
                // let the decal definitions know they have decals attached
                static_data::package& Package = m_pStaticData->pPackage[iPackage];
                s32 iDecalDef;
                for ( iDecalDef = Package.iDefinition;
                      iDecalDef < (Package.iDefinition+Package.nDefinitions);
                      iDecalDef++ )
                {
                    static_data::definition& Def = m_pStaticData->pDefinition[iDecalDef];

                    decal_definition& DecalDef = pPackage->GetDecalDef( Def.iGroup, Def.iDecalDef );
                    ASSERT( DecalDef.m_Handle.IsNonNull() );
                    if ( DecalDef.m_Handle.IsNonNull() )
                    {
                        registration_info& RegInfo = m_RegisteredDefs( DecalDef.m_Handle );
                        RegInfo.m_StaticDataOffset = iDecalDef;
                    }
                }
            }
        }
    }
}

//==============================================================================

void decal_mgr::UnloadStaticDecals( void )
{
    // unattach any static decals
    s32 i;
    for ( i = 0; i < m_RegisteredDefs.GetCount(); i++ )
    {
        m_RegisteredDefs[i].m_StaticDataOffset = -1;
    }

    delete m_pStaticData;
    m_pStaticData = NULL;
}

//==============================================================================

#ifdef TARGET_PC
void decal_mgr::RenderEditorStatics( registration_info& RegInfo, u32 DrawFlags )
{
    //======================================================================
    // Draw the textured decals
    //======================================================================
    RenderVerts( RegInfo.m_nStaticVerts,
                 RegInfo.m_pStaticPositions,
                 RegInfo.m_pStaticUVs,
                 RegInfo.m_pStaticColors );
    draw_End();

    //======================================================================
    // Draw the decals in wireframe for selected decals
    //======================================================================
    draw_Begin( DRAW_TRIANGLES, DRAW_WIRE_FRAME );

    // Note: this is editor-side only. The game-side data is precompiled,
    // and will use microcode.
    draw_ClearL2W();
    draw_Color(xcolor(255,50,50,255));
    
    s32   iVert;
    xbool WindingCW = TRUE;        
    for ( iVert = 0; iVert < RegInfo.m_nStaticVerts; iVert++ )
    {
        // start of a new strip?
        if ( RegInfo.m_pStaticPositions[iVert].Flags & decal_vert::FLAG_SKIP_TRIANGLE )
        {
            WindingCW = TRUE;
            continue;
        }

        if ( RegInfo.m_pStaticPositions[iVert].Flags & decal_vert::FLAG_DRAW_WIREFRAME )
        {
            // fill in the verts
            ASSERT( iVert >= 2 );
            if ( WindingCW )
            {
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-2].Pos );
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-1].Pos );
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-0].Pos );
            }
            else
            {
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-0].Pos );
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-1].Pos );
                draw_Vertex     ( RegInfo.m_pStaticPositions[iVert-2].Pos );
            }
        }

        WindingCW = !WindingCW;
    }

    // clear out the verts we have rendered
    RegInfo.m_nStaticVerts = 0;
}
#endif

//==============================================================================

void decal_mgr::RenderStaticDecals( registration_info& RegInfo )
{
#ifdef TARGET_PC
    // figure out the blend mode
    u32 DrawFlags = DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_TEXTURED | DRAW_UV_CLAMP;
    switch ( RegInfo.m_BlendMode )
    {
    default:
    case decal_definition::DECAL_BLEND_NORMAL:
    case decal_definition::DECAL_BLEND_INTENSITY:
        break;

    case decal_definition::DECAL_BLEND_ADD:
        DrawFlags |= DRAW_BLEND_ADD;
        break;

    case decal_definition::DECAL_BLEND_SUBTRACT:
        DrawFlags |= DRAW_BLEND_SUB;
        break;
    }

    // draw the editor-side decals only
    RenderEditorStatics( RegInfo, DrawFlags );
    return;
#else
    // make sure this decal type has statics
    if ( !m_pStaticData || RegInfo.m_StaticDataOffset == -1 )
        return;

    // render statics only if their zone is visible
    static_data::definition& StaticDef = m_pStaticData->pDefinition[RegInfo.m_StaticDataOffset];
    for ( s32 iZone = StaticDef.iZoneInfo;
          iZone < (StaticDef.iZoneInfo+StaticDef.nZones);
          iZone++ )
    {
        static_data::zone_info& ZoneInfo = m_pStaticData->pZone[iZone];
        if ( g_ZoneMgr.IsZoneVisible( (u8)ZoneInfo.Zone&0xff ) ||
             g_ZoneMgr.IsZoneVisible( (u8)((ZoneInfo.Zone>>8)&0xff) ) )
        {
            RenderVerts( ZoneInfo.nVerts,
                         &m_pStaticData->pPos[ZoneInfo.iVert],
                         &m_pStaticData->pUV[ZoneInfo.iVert],
                         &m_pStaticData->pColor[ZoneInfo.iVert] );
        }
    }
#endif
}

//==============================================================================
