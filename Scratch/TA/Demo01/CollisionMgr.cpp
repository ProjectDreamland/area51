//==============================================================================
//
//  CollisionMgr.cpp
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================

#include "CollisionMgr.hpp"
#include "x_stdio.hpp"
#include "CollisionPrimatives.hpp"
#include "ObjectMgr\ObjectMgr.hpp"
#include "GameLib\RenderMgr.hpp"
#include "PlaySurface\PlaySurface.hpp"
#include "RigidInstance\RigidInstance.hpp"
//==============================================================================
//  TYPES
//==============================================================================


//==============================================================================
//  DEFINES
//==============================================================================
#define BBOX_EARLY_OUT 0
#define CELL_GRID_SIZE 400.0f

//==============================================================================
//  STORAGE
//==============================================================================
collision_mgr   g_CollisionMgr;
s32             CollideOnlyThis = -1;
xbool           g_UsingSpatialDBFunc = TRUE;
vector3         g_CylinderSanityTri[100][3];
s32             g_nCylinderSanityTris;
//==============================================================================
//  EXTERNS
//==============================================================================
#ifdef EDITOR
extern u32 g_RenderSelectedCollision;
extern xbool g_GenDebugFlag;
#endif
//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
// collision_mgr
//==============================================================================
collision_mgr::collision_mgr( void )
{
    Initialize();
    m_CheckCollisionTime.Reset();
    m_StaticVolumeSet.m_nStaticVolumes = 0;
    m_Sequence = 0;
}

//==============================================================================
// Initialize
//==============================================================================
void collision_mgr::Initialize( void )
{
    m_nCollisions = 0;
    m_DynamicPrimitive = PRIMITIVE_INVALID;
    m_nMaxCollsions = 1;
    m_pDynamicVolumeSet = NULL;
    m_ApplyStarted = FALSE;
    m_ContextInfo.Context = 0;
    m_ContextInfo.Guid = NULL;
}

//==============================================================================
// CompareCollisions
//==============================================================================
s32 CompareCollisions(
    const void* C1,
    const void* C2 )
{
    ASSERT( C1 != NULL );
    ASSERT( C2 != NULL );

    f32 T1 = ((collision_mgr::collision*)(C1))->T;
    f32 T2 = ((collision_mgr::collision*)(C2))->T;

    return (T1 > T2) ? 1 : ((T1 < T2) ? -1 : 0);
}


//==============================================================================
// CheckCollisions
//==============================================================================
struct coll_func_info
{
    u32 AttrFlags;
    guid MoveObjGuid;
    xhandle Set;
    u32 OccFlags;
};

xbool CheckCollisionsFunc( spatial_cell* pCell, void* pPrivateData )
{
//    (void)pPrivateData;
    coll_func_info* pInfo = (coll_func_info*)pPrivateData;
    u32 StaticObjAttrs = pInfo->AttrFlags;//*((u32*)pPrivateData);
    const bbox& DynamicBBox = g_CollisionMgr.GetDynamicBBox();

    // Loop through them
    u32 Channel = 0;
    for (Channel = 0; Channel < NUM_SPATIAL_CHANNELS; Channel++)
    {
        if (pInfo->OccFlags & (1 << Channel))
        {
	        s32 LI = pCell->FirstObjectLink[ Channel ];
            s32 C2=0;
            while( LI != -1 )
            {
                ASSERT( (C2++)<1000 );

                // Get object ptr
                object* pObject = (object*)g_ObjMgr.LockObjectFromSlot( LI/8 );
                ASSERT(pObject);

        #ifdef EDITOR
                if (g_CollisionMgr.GetDynamicPrimitive() == PRIMITIVE_DYNAMIC_RAY_SELECT)
                {
                    // early out of unselectable objects
                    if (pObject->GetUnselectable())
                    {
                        g_ObjMgr.UnlockObject( pObject->GetGuid() );
                        // Move to next link for this cell
                        LI = g_ObjMgr.GetCellLinks()[LI].Next;
                        continue;
                    }
                }
        #endif
                if( pObject->GetAttrBits() & StaticObjAttrs )
                {
                    if( pObject->GetGuid() != pInfo->MoveObjGuid )
                    {
                        if( pObject->GetBBox().Intersect( DynamicBBox ) )
                        {
                            selset_Add( pInfo->Set, pObject->GetGuid() );
                            //pObject->OnCheckCollision();
                        }
                    }
                }

                g_ObjMgr.UnlockObject( pObject->GetGuid() );

                // Move to next link for this cell
                LI = g_ObjMgr.GetCellLinks()[LI].Next;
	        }
        }
    }
    return TRUE;
}

//==============================================================================
xtimer CollTimer[3];

xbool collision_mgr::CheckCollisions( void )
{
    m_Sequence++;

    CollTimer[0].Start();

    m_CheckCollisionTime.Start();

    // see if we're checking static volumes
    if ( m_StaticVolumeSet.m_nStaticVolumes > 0 )
    {
        // Get shortcut to structure
        const static_volume_set& SS = m_StaticVolumeSet;

        //
        // Set up the xforms for testing
        //
        matrix4 L2W;
        matrix4 W2L;
        L2W.Identity();
        W2L.Identity();
#if ROTATE_TEST
        L2W.Rotate( TEST_ROTATION );
        W2L = L2W;
        W2L.InvertSRT();
#endif
        // apply it...
        StartApply( NULL, L2W, W2L );
            
        // Are we in the ballpark?
        if( m_DynamicBBoxes[m_ContextInfo.Context]
            .Intersect( SS.m_BBox ) )
        {
            // Get Cell range that dynamic bbox overlaps
            f32 W = 1.0f / SS.m_CellSize;
            const bbox& BBox = m_DynamicBBoxes[m_ContextInfo.Context];
            s32 MinX = (s32)x_floor( (BBox.Min.X-SS.m_BBox.Min.X) * W );
            s32 MinY = (s32)x_floor( (BBox.Min.Y-SS.m_BBox.Min.Y) * W );
            s32 MinZ = (s32)x_floor( (BBox.Min.Z-SS.m_BBox.Min.Z) * W );
            s32 MaxX = (s32)x_floor( (BBox.Max.X-SS.m_BBox.Min.X) * W );
            s32 MaxY = (s32)x_floor( (BBox.Max.Y-SS.m_BBox.Min.Y) * W );
            s32 MaxZ = (s32)x_floor( (BBox.Max.Z-SS.m_BBox.Min.Z) * W );

            // Peg to cells we have
            MinX = MAX(0,MinX);
            MinY = MAX(0,MinY);
            MinZ = MAX(0,MinZ);
            MaxX = MIN(SS.m_nCells[0]-1,MaxX);
            MaxY = MIN(SS.m_nCells[1]-1,MaxY);
            MaxZ = MIN(SS.m_nCells[2]-1,MaxZ);

            ASSERT( MinX>=0 );
            ASSERT( MinX<SS.m_nCells[0] );
            ASSERT( MaxX>=MinX );
            ASSERT( MaxX>=0 );
            ASSERT( MaxX<SS.m_nCells[0] );
            ASSERT( MinY>=0 );
            ASSERT( MinY<SS.m_nCells[1] );
            ASSERT( MaxY>=MinY );
            ASSERT( MaxY>=0 );
            ASSERT( MaxY<SS.m_nCells[1] );
            ASSERT( MinZ>=0 );
            ASSERT( MinZ<SS.m_nCells[2] );
            ASSERT( MaxZ>=MinZ );
            ASSERT( MaxZ>=0);
            ASSERT( MaxZ<SS.m_nCells[2] );

            // Loop through those cells
            for( s32 CX=MinX; CX<=MaxX; CX++ )
            for( s32 CY=MinY; CY<=MaxY; CY++ )
            for( s32 CZ=MinZ; CZ<=MaxZ; CZ++ )
            {
                // Get Cell index
                s32 CI = CZ*SS.m_nCells[0]*SS.m_nCells[1] + CY*SS.m_nCells[0] + CX;

                // Get offset and count
                s32 Offset = SS.m_CellVolumeOffset[CI];
                s32 Count  = SS.m_CellVolumeCount[CI];

                // Loop through primitives
                for( s32 i=0; i<Count; i++ )
                {
                    // get primitive index
                    s32 PIndex = SS.m_VolumeIndex[ Offset+i ];
                    ASSERT( (PIndex>=0) && (PIndex<SS.m_nStaticVolumes) );

                    // If doing single collide skip
                    if( (CollideOnlyThis > -1) && (PIndex!=CollideOnlyThis) )
                        continue;

                    // get reference to primitive
                    collision_volume& Prim = SS.m_pStaticVolume[PIndex];

                    //ASSERT( Prim.Type >= PRIMITIVE_STATIC_START );
                    //ASSERT( Prim.Type <= PRIMITIVE_STATIC_END );

                    if( Prim.SearchSeq != m_Sequence )
                    {
                        Prim.SearchSeq = m_Sequence;

                        // Check if bboxes intersect
                        if( m_DynamicBBoxes[m_ContextInfo.Context].Intersect( Prim.AABBox ) )
                        {
/*
                            switch ( Prim.Type )
                            {
                                case PRIMITIVE_STATIC_SPHERE:
                                    ApplySphere( Prim.SpherePos, Prim.Radius );
                                    break;

                                case PRIMITIVE_STATIC_TRIANGLE:
                                    // pass i along so we can store which triangle in case of
                                    // a collision
                                    ApplyTriangle( Prim.P0, Prim.P1, Prim.P2, 0, i ); 
                                    break;

                                case PRIMITIVE_STATIC_AA_BBOX:
                                    ApplyAABBox( Prim.AABBox );
                                    break;

                                default:
                                    ASSERT( 0 ); // invalid primitive type
                            }
*/
                            ApplyTriangle( Prim.P0, Prim.P1, Prim.P2, 0, i );

                            if (   (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS)
                                && (m_nCollisions > 0 ) )
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
        EndApply();
    }
    else
    {

        if (!g_UsingSpatialDBFunc)
        {
        xhandle H;
	    H.Handle = -1;

CollTimer[1].Start();
        xhandle Set = g_ObjMgr.SelectBBox( H, m_StaticObjAttrs, m_DynamicBBoxes[m_ContextInfo.Context] );
CollTimer[1].Stop();

CollTimer[2].Start();
        s32 ObjectCount = 0;
//        s32 TriangleCount = 0;
        if(Set.IsNonNull())
        {
    	    selset_BeginLoop( Set );
            guid Guid;
            while((selset_GetNext( Set, Guid )))
            {
                if (m_MovingObjGuid == Guid)
                    continue;

                ObjectCount++;
                object *pObject = g_ObjMgr.LockObject(Guid);
                ASSERT(pObject);

                // This call should set up the static collision set
#ifdef EDITOR
                if (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT)
                {
                    // early out of unselectable objects
                    if (pObject->GetUnselectable())
                    {
                        g_ObjMgr.UnlockObject(Guid);
                        continue;
                    }
                }
                else if (pObject->DoesNotCollide())
                {
                    g_ObjMgr.UnlockObject(Guid);
                    continue;
                }
#else
                if (pObject->DoesNotCollide())
                {
                    g_ObjMgr.UnlockObject(Guid);
                    continue;
                }
#endif
                pObject->OnCheckCollision();

                g_ObjMgr.UnlockObject(Guid);
            }
		    selset_DelSet(Set);
        }

CollTimer[2].Stop();
        }

        else
        {
CollTimer[1].Start();

            xhandle H;
	        H.Handle = -1;
            xhandle Set = selset_NewSet();

            coll_func_info Info;
            Info.AttrFlags = m_StaticObjAttrs;
            Info.MoveObjGuid = m_MovingObjGuid;
            Info.Set = Set;
            Info.OccFlags = (1<<SPATIAL_CHANNEL_GENERIC);
#ifdef EDITOR
            // Allow selection of lights
            if (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT)
            {
                Info.OccFlags |= (1<<SPATIAL_CHANNEL_LIGHTS);
            }
#endif
            g_SpatialDBase.TraverseCells( m_DynamicBBoxes[m_ContextInfo.Context], CheckCollisionsFunc, &Info, Info.OccFlags );
CollTimer[1].Stop();


CollTimer[2].Start();
    	    selset_BeginLoop( Set );
            guid Guid;
            while((selset_GetNext( Set, Guid )))
            {
                if (m_MovingObjGuid == Guid)
                    continue;

                //ObjectCount++;
                object *pObject = g_ObjMgr.LockObject(Guid);
                ASSERT(pObject);

                // This call should set up the static collision set
#ifdef EDITOR
                if (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT)
                {
                    // early out of unselectable objects
                    if (pObject->GetUnselectable())
                    {
                        g_ObjMgr.UnlockObject(Guid);
                        continue;
                    }
                }
                else if (pObject->DoesNotCollide())
                {
                    g_ObjMgr.UnlockObject(Guid);
                    continue;
                }
#else
                if (pObject->DoesNotCollide())
                {
                    g_ObjMgr.UnlockObject(Guid);
                    continue;
                }
#endif

                // transform the sanity check tris into local space
                if ( m_ContextInfo.Context )
                {
                    s32 i;
                    for ( i = 0; i < g_nCylinderSanityTris; ++i )
                    {
                        g_CylinderSanityTri[i][0] = m_ContextInfo.W2L.Transform( g_CylinderSanityTri[i][0]);
                        g_CylinderSanityTri[i][1] = m_ContextInfo.W2L.Transform( g_CylinderSanityTri[i][1]);
                        g_CylinderSanityTri[i][2] = m_ContextInfo.W2L.Transform( g_CylinderSanityTri[i][2]);
                    }
                }
                
                pObject->OnCheckCollision();

                // transform the sanity check tris back into world space
                if ( m_ContextInfo.Context )
                {
                    s32 i;
                    for ( i = 0; i < g_nCylinderSanityTris; ++i )
                    {
                        g_CylinderSanityTri[i][0] = m_ContextInfo.L2W.Transform( g_CylinderSanityTri[i][0]);
                        g_CylinderSanityTri[i][1] = m_ContextInfo.L2W.Transform( g_CylinderSanityTri[i][1]);
                        g_CylinderSanityTri[i][2] = m_ContextInfo.L2W.Transform( g_CylinderSanityTri[i][2]);
                    }
                }
                
                g_ObjMgr.UnlockObject(Guid);
                m_Sequence++;
            }
		    selset_DelSet(Set);
CollTimer[2].Stop();
        }
    }

    // sort the collisions by T
    x_qsort(
        m_Collisions,
        m_nCollisions,
        sizeof( collision_mgr::collision ),
        CompareCollisions );

    s32 i;
    for ( i = 1; i < m_nCollisions; ++i )
    {
        ASSERT( m_Collisions[i].T >= m_Collisions[i-1].T );
    }

    m_CheckCollisionTime.Stop();
    g_RenderMgr.Stats.CollisionMS = m_CheckCollisionTime.ReadMs();

    CollTimer[0].Stop();

#if 0 
    if (g_UsingSpatialDBFunc)
    {
        if (g_UsingSpatialDBFunc)
        {
            x_printf("Using SpatialDB.TraverseCells, Time = %f\n", g_RenderMgr.Stats.CollisionMS);
        }
        else
        {
            x_printf("Using ObjMgr.SelectBBox, Time = %f\n", g_RenderMgr.Stats.CollisionMS);
        }
    }
#endif
    return m_nCollisions > 0;
}

//==============================================================================
// StartApply
//==============================================================================
void collision_mgr::StartApply(       guid      Guid, 
                                const matrix4&  L2W,
                                const matrix4&  W2L )
{
    ASSERT( !m_ApplyStarted ); // EndApply() wasn't called


    //
    // Object is colliding against self
    //
    ASSERT( (Guid==0) || (Guid != m_MovingObjGuid) );

    //
    // Set up context info
    //
    m_ApplyStarted = TRUE;
    m_ContextInfo.Context = 1;
    m_ContextInfo.Guid = Guid;
    m_ContextInfo.L2W = L2W;
    m_ContextInfo.W2L = W2L;

    //
    // Transform the dynamic primitive from world to local space
    //
    switch( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
    {
        // transform start/end bottom/top
        m_CylinderInfo[1].BotStart = W2L.Transform(m_CylinderInfo[0].BotStart);
        m_CylinderInfo[1].BotEnd = W2L.Transform(m_CylinderInfo[0].BotEnd);
        m_CylinderInfo[1].TopStart = W2L.Transform(m_CylinderInfo[0].TopStart);
        m_CylinderInfo[1].TopEnd = W2L.Transform(m_CylinderInfo[0].TopEnd);

        // transform radius
        const vector3 Scale = W2L.GetScale();
        ASSERT( (x_abs(Scale.X-Scale.Y) < 0.001f) &&
                (x_abs(Scale.Y-Scale.Z) < 0.001f) );

        m_CylinderInfo[1].Radius
            = m_CylinderInfo[0].Radius * Scale.X;

        // transform spheres
        s32 i;
        m_CylinderInfo[1].nStartSpheres = m_CylinderInfo[0].nStartSpheres;
        m_CylinderInfo[1].nEndSpheres = m_CylinderInfo[0].nEndSpheres;
        for ( i = 0; i < m_CylinderInfo[1].nStartSpheres; ++i )
        {
            m_CylinderInfo[1].StartSpherePositions[i]
                = W2L.Transform(m_CylinderInfo[0].StartSpherePositions[i]);
        }

        for ( i = 0; i < m_CylinderInfo[1].nEndSpheres; ++i )
        {
            m_CylinderInfo[1].EndSpherePositions[i]
                = W2L.Transform(m_CylinderInfo[0].EndSpherePositions[i]);
        }
        
        break;
    }

    case PRIMITIVE_DYNAMIC_SPHERE:
    {
        m_SphereInfo[1].Start = W2L.Transform(m_SphereInfo[0].Start);
        m_SphereInfo[1].End = W2L.Transform(m_SphereInfo[0].End);

        const vector3 Scale = W2L.GetScale();
        ASSERT( (x_abs(Scale.X-Scale.Y) < 0.001f) &&
                (x_abs(Scale.Y-Scale.Z) < 0.001f) );

        m_SphereInfo[1].Radius
            = m_SphereInfo[0].Radius * Scale.X;
        break;
    }
    
    case PRIMITIVE_DYNAMIC_RAY_SELECT:
    case PRIMITIVE_DYNAMIC_RAY:
    case PRIMITIVE_DYNAMIC_LOS:
        m_RayInfo[1].Start = W2L.Transform(m_RayInfo[0].Start);
        m_RayInfo[1].End = W2L.Transform(m_RayInfo[0].End);
        break;

    default:
        ASSERT( 0 ); // invalid dynamic primitive
    }

    //
    // Transform the dynamic bbox
    //
    m_DynamicBBoxes[1] = m_DynamicBBoxes[0];
    m_DynamicBBoxes[1].Transform( W2L );
}

//==============================================================================
// StartApply
//==============================================================================
void collision_mgr::StartApply( guid      Guid )
{
    //
    // Object is colliding against self
    //
    ASSERT( (Guid==0) || (Guid != m_MovingObjGuid) );


    ASSERT( !m_ApplyStarted ); // EndApply() wasn't called
    m_ApplyStarted = TRUE;
    m_ContextInfo.Context = 0;
    m_ContextInfo.Guid = Guid;

    // transforms do nothing
    m_ContextInfo.L2W.Identity();
    m_ContextInfo.W2L.Identity();
}

//==============================================================================
// EndApply
//==============================================================================
void collision_mgr::EndApply( void )
{
    ASSERT( m_ApplyStarted ); // StartApply() wasn't called
    m_ApplyStarted = FALSE;
}

//==============================================================================

void collision_mgr::GatherTriangle( const vector3& P0,
                                    const vector3& P1,
                                    const vector3& P2 )
{
    if( m_nTriangles == MAX_COLLISION_MGR_TRIANGLES )
        return;

    m_Triangle[ m_nTriangles ].P[0] = P0;
    m_Triangle[ m_nTriangles ].P[1] = P1;
    m_Triangle[ m_nTriangles ].P[2] = P2;
    m_nTriangles++;
}

//==============================================================================

xbool GatherTriangleFunc( spatial_cell* pCell, void* pPrivateData )
{
    (void)pPrivateData;
    // Loop through them
	s32 LI = pCell->FirstObjectLink[ SPATIAL_CHANNEL_GENERIC ];
    s32 C2=0;
    while( LI != -1 )
    {
        ASSERT( (C2++)<1000 );

        // Get object ptr
        object* pObject = (object*)g_ObjMgr.LockObjectFromSlot( LI/8 );
        ASSERT(pObject);

        pObject->OnGatherTriangles();

        g_ObjMgr.UnlockObject( pObject->GetGuid() );

        // Move to next link for this cell
        LI = g_ObjMgr.GetCellLinks()[LI].Next;
	}

    return TRUE;
}

//==============================================================================

void collision_mgr::GatherObjectTriangles( const bbox& BBox, guid ObjectGuid )
{
    m_nTriangles = 0;
    m_Sequence++;
    m_GatherBBox = BBox;

    object* pObject = g_ObjMgr.LockObject( ObjectGuid );
    if( !pObject )
        return;

    pObject->OnGatherTriangles();

    g_ObjMgr.UnlockObject( ObjectGuid );
}

//==============================================================================

void collision_mgr::GatherTriangles( const bbox& BBox, u32 Attributes )
{
    m_nTriangles = 0;
    m_Sequence++;
    m_GatherBBox = BBox;

    // see if we're checking static volumes
    if ( m_StaticVolumeSet.m_nStaticVolumes > 0 )
    {
        // Get shortcut to structure
        static_volume_set& SS = m_StaticVolumeSet;
        
        // Are we in the ballpark?
        if( BBox.Intersect( m_StaticVolumeSet.m_BBox ) )
        {
            // Get Cell range that dynamic bbox overlaps
            f32 W = 1.0f / m_StaticVolumeSet.m_CellSize;
            s32 MinX = (s32)x_floor( (BBox.Min.X-SS.m_BBox.Min.X) * W );
            s32 MinY = (s32)x_floor( (BBox.Min.Y-SS.m_BBox.Min.Y) * W );
            s32 MinZ = (s32)x_floor( (BBox.Min.Z-SS.m_BBox.Min.Z) * W );
            s32 MaxX = (s32)x_floor( (BBox.Max.X-SS.m_BBox.Min.X) * W );
            s32 MaxY = (s32)x_floor( (BBox.Max.Y-SS.m_BBox.Min.Y) * W );
            s32 MaxZ = (s32)x_floor( (BBox.Max.Z-SS.m_BBox.Min.Z) * W );

            // Peg to cells we have
            MinX = MAX(0,MinX);
            MinY = MAX(0,MinY);
            MinZ = MAX(0,MinZ);
            MaxX = MIN(SS.m_nCells[0]-1,MaxX);
            MaxY = MIN(SS.m_nCells[1]-1,MaxY);
            MaxZ = MIN(SS.m_nCells[2]-1,MaxZ);

            ASSERT( (MinX>=0) && (MinX<SS.m_nCells[0]) && (MaxX>=MinX) );
            ASSERT( (MaxX>=0) && (MaxX<SS.m_nCells[0]) );
            ASSERT( (MinY>=0) && (MinY<SS.m_nCells[1]) && (MaxY>=MinY) );
            ASSERT( (MaxY>=0) && (MaxY<SS.m_nCells[1]) );
            ASSERT( (MinZ>=0) && (MinZ<SS.m_nCells[2]) && (MaxZ>=MinZ) );
            ASSERT( (MaxZ>=0) && (MaxZ<SS.m_nCells[2]) );

            // Loop through those cells
            for( s32 CX=MinX; CX<=MaxX; CX++ )
            for( s32 CY=MinY; CY<=MaxY; CY++ )
            for( s32 CZ=MinZ; CZ<=MaxZ; CZ++ )
            {
                // Get Cell index
                s32 CI = CZ*SS.m_nCells[0]*SS.m_nCells[1] + CY*SS.m_nCells[0] + CX;

                // Get offset and count
                s32 Offset = SS.m_CellVolumeOffset[CI];
                s32 Count  = SS.m_CellVolumeCount[CI];

                // Loop through primitives
                for( s32 i=0; i<Count; i++ )
                {
                    // get primitive index
                    s32 PIndex = SS.m_VolumeIndex[ Offset+i ];
                    ASSERT( (PIndex>=0) && (PIndex<SS.m_nStaticVolumes) );

                    // If doing single collide skip
                    if( (CollideOnlyThis > -1) && (PIndex!=CollideOnlyThis) )
                        continue;

                    // get reference to primitive
                    collision_volume& Prim = SS.m_pStaticVolume[PIndex];

                    //ASSERT( Prim.Type >= PRIMITIVE_STATIC_START );
                    //ASSERT( Prim.Type <= PRIMITIVE_STATIC_END );

                    if( Prim.SearchSeq != m_Sequence )
                    {
                        Prim.SearchSeq = m_Sequence;

                        // Check if bboxes intersect
                        if( BBox.Intersect( Prim.AABBox ) )
                        {
                            GatherTriangle( Prim.P0, Prim.P1, Prim.P2 );
                        }
                    }
                }
            }
        }
    }
    else
    {

        if( Attributes == object::ATTR_INTERNAL )
        {
            g_ObjMgr.StartTypeLoop( object::TYPE_INTERNAL );
            guid Guid;
            while( (Guid = g_ObjMgr.GetNextInType()) )
            {
                object* pObject = g_ObjMgr.LockObject(Guid);
                ASSERT( pObject );
                if ( !(pObject->DoesNotCollide()) )
                    if( pObject->GetBBox().Intersect(BBox) )
                        pObject->OnGatherTriangles();

                g_ObjMgr.UnlockObject(Guid);
            }
            g_ObjMgr.EndTypeLoop();
        }
        else
        {
            xhandle H;
	        H.Handle = -1;
            xhandle Set = g_ObjMgr.SelectBBox( H, Attributes, BBox );
            s32 ObjectCount = 0;
            s32 TriangleCount = 0;
            if(Set.IsNonNull())
            {
    	        selset_BeginLoop( Set );
                guid Guid;
                while((selset_GetNext( Set, Guid )))
                {
                    ObjectCount++;
                    object *pObject = g_ObjMgr.LockObject(Guid);
                    ASSERT(pObject);

                    // This call should set up the static collision set
                    if ( !(pObject->DoesNotCollide()) )
                        pObject->OnGatherTriangles();


                    g_ObjMgr.UnlockObject(Guid);
                }
		        selset_DelSet(Set);
            }
            m_pDynamicVolumeSet = NULL;
            g_RenderMgr.Stats.CollisionObjects = ObjectCount;
            g_RenderMgr.Stats.CollisionTriangles = TriangleCount;
        }

        //g_SpatialDBase.TraverseCells( BBox, GatherTriangleFunc, NULL, (1<<SPATIAL_CHANNEL_GENERIC) );

    }
    

}

//==============================================================================
void collision_mgr::GatherTrianglesToFile( const bbox& BBox )
{
    GatherTriangles( BBox, object::ATTR_ALL );

    //
    // Move the triangles close to the origin
    //
    vector3 Delta = BBox.GetCenter();

    s32 i;
    for ( i = 0; i < m_nTriangles; ++i )
    {
        m_Triangle[i].P[0] -= Delta;
        m_Triangle[i].P[1] -= Delta;
        m_Triangle[i].P[2] -= Delta;
    }

    //
    // save the triangles off
    //
    X_FILE* pTriangleFile = x_fopen( "c:\\Triangles.tri", "wb" );
    ASSERT( pTriangleFile );
    
    // write the number of triangles
    x_fwrite( &m_nTriangles, sizeof( s32 ), 1, pTriangleFile );

    // write the triangles
    for ( i = 0; i < m_nTriangles; ++i )
    {
        x_fwrite( &(m_Triangle[i].P[0]), sizeof( vector3 ), 1, pTriangleFile );
        x_fwrite( &(m_Triangle[i].P[1]), sizeof( vector3 ), 1, pTriangleFile );
        x_fwrite( &(m_Triangle[i].P[2]), sizeof( vector3 ), 1, pTriangleFile );
    }

    x_fclose( pTriangleFile );

    x_fflush( pTriangleFile );
#if 0
    // check to see if we saved what we think we did
    triangle TestTriangles[MAX_COLLISION_MGR_TRIANGLES];
    s32 nTestTriangles;

    pTriangleFile = x_fopen( "c:\\Triangles.tri", "rb" );
    ASSERT( pTriangleFile );

    x_fread( &nTestTriangles, sizeof( s32 ), 1, pTriangleFile );
    ASSERT( nTestTriangles == m_nTriangles );

    for ( i = 0; i < nTestTriangles; ++i )
    {
        x_fread( &(TestTriangles[i].P[0]), sizeof( vector3 ) , 1, pTriangleFile );
        x_fread( &(TestTriangles[i].P[1]), sizeof( vector3 ) , 1, pTriangleFile );
        x_fread( &(TestTriangles[i].P[2]), sizeof( vector3 ) , 1, pTriangleFile );
    }
    
    x_fclose( pTriangleFile );

    for ( i = 0; i < nTestTriangles; ++i )
    {
        ASSERT( (TestTriangles[i].P[0] - m_Triangle[i].P[0]).LengthSquared() < 0.001f );
        ASSERT( (TestTriangles[i].P[1] - m_Triangle[i].P[1]).LengthSquared() < 0.001f );
        ASSERT( (TestTriangles[i].P[2] - m_Triangle[i].P[2]).LengthSquared() < 0.001f );
    }
    x_fflush( pTriangleFile );
#endif
}

//==============================================================================
void collision_mgr::LoadTriangleFile( void )
{
    // load the triangles
    X_FILE* pTriangleFile = x_fopen( "c:\\Triangles.tri", "rb" );
    ASSERT( pTriangleFile );
    x_fread( &m_nTriangles, sizeof( s32 ), 1, pTriangleFile );

    s32 i;
    for ( i = 0; i < m_nTriangles; ++i )
    {
        x_fread( &(m_Triangle[i].P[0]), sizeof( vector3 ) , 1, pTriangleFile );
        x_fread( &(m_Triangle[i].P[1]), sizeof( vector3 ) , 1, pTriangleFile );
        x_fread( &(m_Triangle[i].P[2]), sizeof( vector3 ) , 1, pTriangleFile );
    }
    
    x_fclose( pTriangleFile );
    x_fflush( pTriangleFile );

    // convert triangles to static volumes
    ConvertTrisToStaticVolumeSet();
}

//==============================================================================
void collision_mgr::ConvertTrisToStaticVolumeSet()
{
    // Setup new primitives
    collision_volume* StaticVolumes = new collision_volume[m_nTriangles];

    s32 i;
    
    for ( i = 0; i < m_nTriangles; ++i )
    {
        StaticVolumes[i].P0 = m_Triangle[i].P[0];
        StaticVolumes[i].P1 = m_Triangle[i].P[1];
        StaticVolumes[i].P2 = m_Triangle[i].P[2];
        StaticVolumes[i].AABBox.Clear();
        StaticVolumes[i].AABBox += StaticVolumes[i].P0;
        StaticVolumes[i].AABBox += StaticVolumes[i].P1;
        StaticVolumes[i].AABBox += StaticVolumes[i].P2;
    }

    CreateStaticVolumeSet( m_StaticVolumeSet, StaticVolumes, m_nTriangles );
}

//==============================================================================
// GetFirstCollision
//==============================================================================
s32 collision_mgr::GetFirstCollision( void )
{
    f32 NearestT = 2.0f;
    s32 i;
    s32 CollisionIndex = -1;

    for ( i = 0; i < m_nCollisions; ++i )
    {
        if ( m_Collisions[i].T < NearestT )
        {
            NearestT = m_Collisions[i].T;
            CollisionIndex = i;
        }
    }

    ASSERT( CollisionIndex == 0 );

    return CollisionIndex;
}




//==============================================================================
// CylinderSetup
//==============================================================================
void collision_mgr::CylinderSetup (
          guid      MovingObjGuid,
    const vector3&  WorldBotStart,
    const vector3&  WorldBotEnd,
    const vector3&  WorldTopStart,
    const vector3&  WorldTopEnd,
          f32       Radius,
          u32       StaticObjAttrs )
{
    Initialize();
    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_CYLINDER;
    m_MovingObjGuid         = MovingObjGuid;
    m_StaticObjAttrs        = StaticObjAttrs;

    m_CylinderInfo[0].BotStart  = WorldBotStart;
    m_CylinderInfo[0].BotEnd    = WorldBotEnd;
    m_CylinderInfo[0].TopStart  = WorldTopStart;
    m_CylinderInfo[0].TopEnd    = WorldTopEnd;
    m_CylinderInfo[0].Radius    = Radius;

    //--------------------------------------------------
    // Find the bbox around the motion
    //--------------------------------------------------

    // Set up two arrays of spheres, representing the
    // cylinder at the start, and the cylinder at the
    // end.
    m_CylinderInfo[0].nStartSpheres = GetCylinderSpherePositions(
        m_CylinderInfo[0].BotStart,
        m_CylinderInfo[0].TopStart,
        m_CylinderInfo[0].Radius,
        m_CylinderInfo[0].StartSpherePositions,
        MAX_NUM_SPHERES );

    m_CylinderInfo[0].nEndSpheres = GetCylinderSpherePositions(
        m_CylinderInfo[0].BotEnd,
        m_CylinderInfo[0].TopEnd,
        m_CylinderInfo[0].Radius,
        m_CylinderInfo[0].EndSpherePositions,
        MAX_NUM_SPHERES );
    
    ASSERT( m_CylinderInfo[0].nStartSpheres    // Something must be wrong with
         == m_CylinderInfo[0].nEndSpheres );   // GetCylinderSpherePositions() 
                                               // or the caller has changed the 
                                               // size of the cylinder during   
                                               // the move                      

    const s32 StartLast = m_CylinderInfo[0].nStartSpheres-1;
    const s32 EndLast   = m_CylinderInfo[0].nEndSpheres-1;
        
    m_DynamicBBoxes[0].Min.X = MIN(
        MIN(m_CylinderInfo[0].StartSpherePositions[0].X,
            m_CylinderInfo[0].StartSpherePositions[StartLast].X ),
        MIN(m_CylinderInfo[0].EndSpherePositions[0].X,
            m_CylinderInfo[0].EndSpherePositions[EndLast].X ) );
        
    m_DynamicBBoxes[0].Min.Y = MIN(
        MIN(m_CylinderInfo[0].StartSpherePositions[0].Y,
            m_CylinderInfo[0].StartSpherePositions[StartLast].Y ),
        MIN(m_CylinderInfo[0].EndSpherePositions[0].Y,
            m_CylinderInfo[0].EndSpherePositions[EndLast].Y ) );
        
    m_DynamicBBoxes[0].Min.Z = MIN(
        MIN(m_CylinderInfo[0].StartSpherePositions[0].Z,
            m_CylinderInfo[0].StartSpherePositions[StartLast].Z ),
        MIN(m_CylinderInfo[0].EndSpherePositions[0].Z,
            m_CylinderInfo[0].EndSpherePositions[EndLast].Z ) );
        
    m_DynamicBBoxes[0].Max.X = MAX(
        MAX(m_CylinderInfo[0].StartSpherePositions[0].X,
            m_CylinderInfo[0].StartSpherePositions[StartLast].X ),
        MAX(m_CylinderInfo[0].EndSpherePositions[0].X,
            m_CylinderInfo[0].EndSpherePositions[EndLast].X ) );
        
    m_DynamicBBoxes[0].Max.Y = MAX(
        MAX(m_CylinderInfo[0].StartSpherePositions[0].Y,
            m_CylinderInfo[0].StartSpherePositions[StartLast].Y ),
        MAX(m_CylinderInfo[0].EndSpherePositions[0].Y,
            m_CylinderInfo[0].EndSpherePositions[EndLast].Y ) );
        
    m_DynamicBBoxes[0].Max.Z = MAX(
        MAX(m_CylinderInfo[0].StartSpherePositions[0].Z,
            m_CylinderInfo[0].StartSpherePositions[StartLast].Z ),
        MAX(m_CylinderInfo[0].EndSpherePositions[0].Z,
            m_CylinderInfo[0].EndSpherePositions[EndLast].Z ) );

    m_DynamicBBoxes[0].Min.X -= Radius;
    m_DynamicBBoxes[0].Min.Y -= Radius;
    m_DynamicBBoxes[0].Min.Z -= Radius;

    m_DynamicBBoxes[0].Max.X += Radius;
    m_DynamicBBoxes[0].Max.Y += Radius;
    m_DynamicBBoxes[0].Max.Z += Radius;
}

//==============================================================================
// SphereSetup
//==============================================================================
void collision_mgr::SphereSetup (
          guid      MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          f32       Radius,
          u32       StaticObjAttrs )
{
    Initialize();
    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_SPHERE;
    m_MovingObjGuid         = MovingObjGuid;
    m_StaticObjAttrs        = StaticObjAttrs;
    m_SphereInfo[0].Start   = WorldStart;
    m_SphereInfo[0].End     = WorldEnd;
    m_SphereInfo[0].Radius  = Radius;

    m_DynamicBBoxes[0].Min.X = MIN( WorldStart.X, WorldEnd.X ) - Radius;
    m_DynamicBBoxes[0].Min.Y = MIN( WorldStart.Y, WorldEnd.Y ) - Radius;
    m_DynamicBBoxes[0].Min.Z = MIN( WorldStart.Z, WorldEnd.Z ) - Radius;
    m_DynamicBBoxes[0].Max.X = MAX( WorldStart.X, WorldEnd.X ) + Radius;
    m_DynamicBBoxes[0].Max.Y = MAX( WorldStart.Y, WorldEnd.Y ) + Radius;
    m_DynamicBBoxes[0].Max.Z = MAX( WorldStart.Z, WorldEnd.Z ) + Radius;
}

//==============================================================================
// RaySetup
//==============================================================================
void collision_mgr::RaySetup(
          guid      MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          u32       StaticObjAttrs )
{
    Initialize();
    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_RAY;
    m_MovingObjGuid         = MovingObjGuid;
    m_RayInfo[0].Start      = WorldStart;
    m_RayInfo[0].End        = WorldEnd;
    m_StaticObjAttrs        = StaticObjAttrs;

    m_DynamicBBoxes[0].Min.X = MIN( WorldStart.X, WorldEnd.X );
    m_DynamicBBoxes[0].Min.Y = MIN( WorldStart.Y, WorldEnd.Y );
    m_DynamicBBoxes[0].Min.Z = MIN( WorldStart.Z, WorldEnd.Z );
    m_DynamicBBoxes[0].Max.X = MAX( WorldStart.X, WorldEnd.X );
    m_DynamicBBoxes[0].Max.Y = MAX( WorldStart.Y, WorldEnd.Y );
    m_DynamicBBoxes[0].Max.Z = MAX( WorldStart.Z, WorldEnd.Z );
}
                                      
//==============================================================================
// RaySetupSelect
//==============================================================================
void collision_mgr::RaySetupSelect(
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          u32       StaticObjAttrs )
{
    Initialize();
    RaySetup( NULL, WorldStart, WorldEnd );
    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_RAY_SELECT;
    m_StaticObjAttrs        = StaticObjAttrs;
}
                                      
//==============================================================================
// LineOfSightSetup
//==============================================================================
void collision_mgr::LineOfSightSetup(
    guid            MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          u32       StaticObjAttrs )
{
    Initialize();
    RaySetup( MovingObjGuid, WorldStart, WorldEnd );
    m_DynamicPrimitive = PRIMITIVE_DYNAMIC_LOS;
    m_StaticObjAttrs        = StaticObjAttrs;
}

//==============================================================================
// CreateStaticVolumeSet
//==============================================================================

void    collision_mgr::CreateStaticVolumeSet(      static_volume_set&  StaticVolumeSet,
                                        collision_volume*   StaticVolumes,
                                        s32                 nStaticVolumes )
{
    s32 i,j;

    ASSERT( StaticVolumes != NULL );
    ASSERT( nStaticVolumes > 0 );

    // Clear sequence
    for( i=0; i<nStaticVolumes; i++ )
        StaticVolumes[i].SearchSeq = -1;

    
    // Free up previous set
    StaticVolumeSet.m_CellVolumeCount.Clear();
    StaticVolumeSet.m_CellVolumeOffset.Clear();
    StaticVolumeSet.m_VolumeIndex.Clear();

    // Setup new primitives
    StaticVolumeSet.m_pStaticVolume = StaticVolumes;
    StaticVolumeSet.m_nStaticVolumes = nStaticVolumes;

    // Get main bbox
    StaticVolumeSet.m_BBox.Clear();
    for( i=0; i<StaticVolumeSet.m_nStaticVolumes; i++ )
        StaticVolumeSet.m_BBox += StaticVolumeSet.m_pStaticVolume[i].AABBox;

    // Determine grid size
    StaticVolumeSet.m_CellSize = CELL_GRID_SIZE;
    StaticVolumeSet.m_nCells[0] = 1 + (s32)((StaticVolumeSet.m_BBox.Max.X - StaticVolumeSet.m_BBox.Min.X) / StaticVolumeSet.m_CellSize);
    StaticVolumeSet.m_nCells[1] = 1 + (s32)((StaticVolumeSet.m_BBox.Max.Y - StaticVolumeSet.m_BBox.Min.Y) / StaticVolumeSet.m_CellSize);
    StaticVolumeSet.m_nCells[2] = 1 + (s32)((StaticVolumeSet.m_BBox.Max.Z - StaticVolumeSet.m_BBox.Min.Z) / StaticVolumeSet.m_CellSize);
    StaticVolumeSet.m_nTotalCells = StaticVolumeSet.m_nCells[0] * StaticVolumeSet.m_nCells[1] * StaticVolumeSet.m_nCells[2];

    // Loop through each cell
    for( s32 CZ=0; CZ<StaticVolumeSet.m_nCells[2]; CZ++ )
    for( s32 CY=0; CY<StaticVolumeSet.m_nCells[1]; CY++ )
    for( s32 CX=0; CX<StaticVolumeSet.m_nCells[0]; CX++ )
    {
        // Compute cell index
        i = CZ*StaticVolumeSet.m_nCells[0]*StaticVolumeSet.m_nCells[1] + CY*StaticVolumeSet.m_nCells[0] + CX;

        // Build cell bbox
        bbox CBBox;
        CBBox.Min.X = StaticVolumeSet.m_BBox.Min.X + CX*StaticVolumeSet.m_CellSize;
        CBBox.Min.Y = StaticVolumeSet.m_BBox.Min.Y + CY*StaticVolumeSet.m_CellSize;
        CBBox.Min.Z = StaticVolumeSet.m_BBox.Min.Z + CZ*StaticVolumeSet.m_CellSize;
        CBBox.Max = CBBox.Min + vector3(StaticVolumeSet.m_CellSize,StaticVolumeSet.m_CellSize,StaticVolumeSet.m_CellSize);

        // Expand cell bbox by 1 meter
        CBBox.Min -= vector3(100,100,100);
        CBBox.Max += vector3(100,100,100);

        // Clear counts and offsets
        StaticVolumeSet.m_CellVolumeCount.Append( 0 );
        StaticVolumeSet.m_CellVolumeOffset.Append( StaticVolumeSet.m_VolumeIndex.GetCount() );

        // Search for primitives that overlaps with bbox
        for( j=0; j<StaticVolumeSet.m_nStaticVolumes; j++ )
        if( CBBox.Intersect( StaticVolumeSet.m_pStaticVolume[j].AABBox ) )
        {
            // Add index to primitive
            StaticVolumeSet.m_VolumeIndex.Append( j );

            // Increment count for this cell
            StaticVolumeSet.m_CellVolumeCount[i]++;
        }
    }

    StaticVolumeSet.m_CellVolumeCount.FreeExtra();
    StaticVolumeSet.m_CellVolumeOffset.FreeExtra();
    StaticVolumeSet.m_VolumeIndex.FreeExtra();
}

void    collision_mgr::SetStaticVolumeSet  (       static_volume_set*  pStaticVolumeSet )
{
    m_pDynamicVolumeSet = pStaticVolumeSet;
}

void    collision_mgr::ClearStaticVolumeSet(       void )
{
    SetStaticVolumeSet(NULL);
}

//==============================================================================
// SetStaticVolumes
//==============================================================================
void collision_mgr::SetStaticVolumes (
          collision_volume*   StaticVolumes,
          s32                 nStaticVolumes )
{
    s32 i,j;

    ASSERT( StaticVolumes != NULL );
    ASSERT( nStaticVolumes > 0 );

    // Clear sequence
    for( i=0; i<nStaticVolumes; i++ )
        StaticVolumes[i].SearchSeq = -1;

    // Get shortcut to structure
    static_volume_set& SS = m_StaticVolumeSet;

    // Free up previous set
    SS.m_CellVolumeCount.Clear();
    SS.m_CellVolumeOffset.Clear();
    SS.m_VolumeIndex.Clear();

    // Setup new primitives
    SS.m_pStaticVolume = StaticVolumes;
    SS.m_nStaticVolumes = nStaticVolumes;

    // Get main bbox
    SS.m_BBox.Clear();
    for( i=0; i<SS.m_nStaticVolumes; i++ )
        SS.m_BBox += SS.m_pStaticVolume[i].AABBox;

    // Determine grid size
    SS.m_CellSize = CELL_GRID_SIZE;
    SS.m_nCells[0] = 1 + (s32)((SS.m_BBox.Max.X - SS.m_BBox.Min.X) / SS.m_CellSize);
    SS.m_nCells[1] = 1 + (s32)((SS.m_BBox.Max.Y - SS.m_BBox.Min.Y) / SS.m_CellSize);
    SS.m_nCells[2] = 1 + (s32)((SS.m_BBox.Max.Z - SS.m_BBox.Min.Z) / SS.m_CellSize);
    SS.m_nTotalCells = SS.m_nCells[0] * SS.m_nCells[1] * SS.m_nCells[2];

    // Loop through each cell
    for( s32 CZ=0; CZ<SS.m_nCells[2]; CZ++ )
    for( s32 CY=0; CY<SS.m_nCells[1]; CY++ )
    for( s32 CX=0; CX<SS.m_nCells[0]; CX++ )
    {
        // Compute cell index
        i = CZ*SS.m_nCells[0]*SS.m_nCells[1] + CY*SS.m_nCells[0] + CX;

        // Build cell bbox
        bbox CBBox;
        CBBox.Min.X = SS.m_BBox.Min.X + CX*SS.m_CellSize;
        CBBox.Min.Y = SS.m_BBox.Min.Y + CY*SS.m_CellSize;
        CBBox.Min.Z = SS.m_BBox.Min.Z + CZ*SS.m_CellSize;
        CBBox.Max = CBBox.Min + vector3(SS.m_CellSize,SS.m_CellSize,SS.m_CellSize);

        // Expand cell bbox by 1 meter
        CBBox.Min -= vector3(100,100,100);
        CBBox.Max += vector3(100,100,100);

        // Clear counts and offsets
        SS.m_CellVolumeCount.Append( 0 );
        SS.m_CellVolumeOffset.Append( SS.m_VolumeIndex.GetCount() );

        // Search for primitives that overlaps with bbox
        for( j=0; j<SS.m_nStaticVolumes; j++ )
        if( CBBox.Intersect( SS.m_pStaticVolume[j].AABBox ) )
        {
            // Add index to primitive
            SS.m_VolumeIndex.Append( j );

            // Increment count for this cell
            SS.m_CellVolumeCount[i]++;
        }
    }

    SS.m_CellVolumeCount.FreeExtra();
    SS.m_CellVolumeOffset.FreeExtra();
    SS.m_VolumeIndex.FreeExtra();
}

//==============================================================================
// SetMaxCollisions
//==============================================================================
void collision_mgr::SetMaxCollisions( s32 nMaxCollisions )
{
    ASSERT( nMaxCollisions > 0 );
    if ( m_nMaxCollsions > MAX_COLLISION_MGR_COLLISIONS )
    {
        ASSERT( 0 ); // expecting too many collisions
        nMaxCollisions = MAX_COLLISION_MGR_COLLISIONS;
    }
    
    m_nMaxCollsions = nMaxCollisions;
}

//==============================================================================
// ApplySphere
//==============================================================================
void collision_mgr::ApplySphere(
    const vector3&  WorldPos,
          f32       Radius,
          guid      ObjectHitGuid, /* = 0 */
          s32       PrimitiveKey /* = 0 */ )
{
    ASSERT( m_ApplyStarted );
    
    ObjectHitGuid = m_ContextInfo.Guid;

#if BBOX_EARLY_OUT
    // check to see if the bboxes intersect
    bbox BBox;
    BBox.Min.X = WorldPos.X - Radius;
    BBox.Min.Y = WorldPos.Y - Radius;
    BBox.Min.Z = WorldPos.Z - Radius;
    BBox.Max.X = WorldPos.X + Radius;
    BBox.Max.Y = WorldPos.Y + Radius;
    BBox.Max.Z = WorldPos.Z + Radius;

    if ( !BBox.Intersect( m_DynamicBBoxes[m_ContextInfo.Context] ) )
    {
        return;
    }
#endif
    
    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplySphereToCylinder( WorldPos, Radius, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplySphereToSphere( WorldPos, Radius, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:
    case PRIMITIVE_DYNAMIC_RAY_SELECT:
        ApplySphereToRay( WorldPos, Radius, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_LOS:
        ApplySphereToRay( WorldPos, Radius, ObjectHitGuid, PrimitiveKey );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}
    
//==============================================================================
// ApplyTri
//==============================================================================
void collision_mgr::ApplyTriangle(
    const vector3&  P0,
    const vector3&  P1,
    const vector3&  P2,
          guid      ObjectHitGuid, /* = 0 */
          s32       PrimitiveKey /* = 0 */ )

{
    ASSERT( m_ApplyStarted );

    ObjectHitGuid = m_ContextInfo.Guid;

#if BBOX_EARLY_OUT
    // check to see if the bboxes intersect
    bbox BBox;
    BBox.Min.X = MIN( P0.X, MIN( P1.X, P2.X ) );
    BBox.Min.Y = MIN( P0.Y, MIN( P1.Y, P2.Y ) );
    BBox.Min.Z = MIN( P0.Z, MIN( P1.Z, P2.Z ) );
    BBox.Max.X = MAX( P0.X, MAX( P1.X, P2.X ) );
    BBox.Max.Y = MAX( P0.Y, MAX( P1.Y, P2.Y ) );
    BBox.Max.Z = MAX( P0.Z, MAX( P1.Z, P2.Z ) );

    if ( !BBox.Intersect( m_DynamicBBoxes[m_ContextInfo.Context] ) )
    {
        return;
    }
#endif
    
    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplyTriangleToCylinder( P0, P1, P2, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplyTriangleToSphere( P0, P1, P2, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:        // intentional fallthrough
    case PRIMITIVE_DYNAMIC_RAY_SELECT: // intentional fallthrough
    case PRIMITIVE_DYNAMIC_LOS:
        ApplyTriangleToRay( P0, P1, P2, ObjectHitGuid, PrimitiveKey );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}

//==============================================================================
// ApplyAABBox
//==============================================================================
void collision_mgr::ApplyAABBox(
    const bbox&     BBox,
          guid      ObjectHitGuid, /* = 0 */
          s32       PrimitiveKey /* = 0 */ )
    
{
    ASSERT( m_ApplyStarted );

    ObjectHitGuid = m_ContextInfo.Guid;

#if BBOX_EARLY_OUT
    // check to see if the bboxes intersect
    if ( !BBox.Intersect( m_DynamicBBoxes[m_ContextInfo.Context] ) )
    {
        return;
    }
#endif
    
    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplyAABBoxToCylinder( BBox, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplyAABBoxToSphere( BBox, ObjectHitGuid, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY_SELECT:
    case PRIMITIVE_DYNAMIC_RAY:
        ApplyAABBoxToRay( BBox );
        break;
    case PRIMITIVE_DYNAMIC_LOS:
        ApplyAABBoxToRay( BBox );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}

//==============================================================================
// ApplySphereToCylinder
//
// Apply a static sphere to a dynamic cylinder.
//==============================================================================
void collision_mgr::ApplySphereToCylinder(
    const vector3&  WorldPos,
          f32       Radius,
          guid      ObjectHitGuid,  /* = 0 */
          s32       PrimitiveKey    /* = 0 */ )
{
    // required because of data dependencies
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

    ObjectHitGuid = m_ContextInfo.Guid;

    //--------------------------------------------------
    // Check collisions between matching spheres in
    // the start cylinder and the end cylinder
    //--------------------------------------------------
    s32         i;
    f32         FinalT;
    vector3     FinalHitPoint;
        
    for ( i = 0; i < m_CylinderInfo[m_ContextInfo.Context].nStartSpheres; ++i )
    {
        if ( ComputeSphereSphereCollision(
              WorldPos,
              Radius,
              m_CylinderInfo[m_ContextInfo.Context].Radius,
              m_CylinderInfo[m_ContextInfo.Context].StartSpherePositions[i],
              m_CylinderInfo[m_ContextInfo.Context].EndSpherePositions[i],
              FinalT,
              FinalHitPoint ) )
        {
            // record the collision
            const vector3 Normal = (FinalHitPoint - WorldPos) * (1 / Radius);
            plane HitPlane;
            HitPlane.Setup( FinalHitPoint, Normal );

            plane SlipPlane = HitPlane;
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );

            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                ObjectHitGuid,
                PrimitiveKey,
                PRIMITIVE_STATIC_SPHERE,
                FALSE,
                WorldPos.Y + Radius );

            RecordCollision( TempCollision );
        }
    }
}

//==============================================================================
// ApplySphereToSphere
//
// Apply a static sphere to a dynamic sphere.
//==============================================================================
void collision_mgr::ApplySphereToSphere(
    const vector3&  WorldPos,
          f32       Radius,
          guid      ObjectHitGuid,  /* = 0 */
          s32       PrimitiveKey    /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );

    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;

    // is there a collison?
    if ( ComputeSphereSphereCollision(
        WorldPos
        , Radius
        , m_SphereInfo[m_ContextInfo.Context].Radius
        , m_SphereInfo[m_ContextInfo.Context].Start
        , m_SphereInfo[m_ContextInfo.Context].End
        , FinalT
        , FinalHitPoint ) )
    {
        // record the collision
        const vector3 Normal = (FinalHitPoint - WorldPos) * (1 / Radius);
        plane HitPlane;
        HitPlane.Setup( FinalHitPoint, Normal );

        plane SlipPlane = HitPlane;
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
        
        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_SPHERE,
            FALSE,
            WorldPos.Y + Radius );

        RecordCollision( TempCollision );
    }
}
 
//==============================================================================
// ApplySphereToRay
//
// Apply a static sphere to a dynamic ray.
//==============================================================================
void collision_mgr::ApplySphereToRay(
    const vector3&  WorldPos,
          f32       Radius,
          guid      ObjectHitGuid,  /* = 0 */
          s32       PrimitiveKey    /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY
         || m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT);

    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;

    // is there a collison?
    if ( ComputeRaySphereCollision(
        WorldPos
        , Radius
        , m_RayInfo[m_ContextInfo.Context].Start
        , m_RayInfo[m_ContextInfo.Context].End
        , FinalT
        , FinalHitPoint ) )
    {
        // record the collision
        const vector3 Normal = (FinalHitPoint - WorldPos) * (1 / Radius);
        plane HitPlane;
        HitPlane.Setup( FinalHitPoint, Normal );

        plane SlipPlane = HitPlane;
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_SPHERE,
            FALSE,
            WorldPos.Y + Radius );

        RecordCollision( TempCollision );
    }
    
}

//==============================================================================
// ApplyTriangleToCylinder
//
// Apply a static triangle to a dynamic cylinder
//==============================================================================
void collision_mgr::ApplyTriangleToCylinder(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

    ObjectHitGuid = m_ContextInfo.Guid;

    //--------------------------------------------------
    // Check collisions between matching spheres in
    // the start cylinder and the end cylinder
    //--------------------------------------------------
    s32         i;
    f32         FinalT;
    vector3     FinalHitPoint;
    vector3     Triangle[3];
    xbool       HitTriangleEdge;
    Triangle[0] = P0;
    Triangle[1] = P1;
    Triangle[2] = P2;
        
    for ( i = 0; i < m_CylinderInfo[m_ContextInfo.Context].nStartSpheres; ++i )
    {
        if ( ComputeSphereTriCollision(
              Triangle
            , m_CylinderInfo[m_ContextInfo.Context].StartSpherePositions[i]
            , m_CylinderInfo[m_ContextInfo.Context].EndSpherePositions[i]
            , m_CylinderInfo[m_ContextInfo.Context].Radius
            , FinalT
            , FinalHitPoint
            , HitTriangleEdge ) )
        {
            // record the collision
            plane HitPlane;
            plane SlipPlane;

            if ( HitTriangleEdge )
            {
                // Our slide plane is defined by the impact point, and a
                // normal from that point towards the sphere's center,
                // when the sphere is at the collision T
                const vector3 SphereImpactPosition
                    = m_CylinderInfo[m_ContextInfo.Context]
                        .StartSpherePositions[i]
                    + ((m_CylinderInfo[m_ContextInfo.Context]
                            .EndSpherePositions[i]
                        - m_CylinderInfo[m_ContextInfo.Context]
                            .StartSpherePositions[i])
                        * FinalT);
                vector3 Normal = SphereImpactPosition - FinalHitPoint;
                Normal.Normalize();
                SlipPlane.Setup( FinalHitPoint, Normal );
                ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
            }
            else
            {
                // The triangle plane is our slip plane
                SlipPlane.Setup( P0, P1, P2 );
                ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
            }

            HitPlane.Setup( P0, P1, P2 );
            
            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                ObjectHitGuid,
                PrimitiveKey,
                PRIMITIVE_STATIC_TRIANGLE,
                HitTriangleEdge,
                MAX( P0.Y, MAX( P1.Y, P2.Y ) ) );

            RecordCollision( TempCollision );
        }
    }
}


//==============================================================================
// ApplyTriangleToSphere
//==============================================================================
void collision_mgr::ApplyTriangleToSphere(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );

    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;
    vector3     Triangle[3];
    xbool       HitTriangleEdge;
    Triangle[0] = P0;
    Triangle[1] = P1;
    Triangle[2] = P2;
        
    if ( ComputeSphereTriCollision(
        Triangle,
        m_SphereInfo[m_ContextInfo.Context].Start,
        m_SphereInfo[m_ContextInfo.Context].End,
        m_SphereInfo[m_ContextInfo.Context].Radius,
        FinalT,
        FinalHitPoint,
        HitTriangleEdge ) )
    {
        // record the collision
        plane HitPlane;
        plane SlipPlane;

        if ( HitTriangleEdge )
        {
            // Our slide plane is defined by the impact point, and a
            // normal from that point towards the sphere's center,
            // when the sphere is at the collision T
            const vector3 SphereImpactPosition
                = m_SphereInfo[m_ContextInfo.Context].Start
                + ((m_SphereInfo[m_ContextInfo.Context].End
                    - m_SphereInfo[m_ContextInfo.Context].Start)
                    * FinalT);
            vector3 Normal = SphereImpactPosition - FinalHitPoint;
            Normal.Normalize();
            SlipPlane.Setup( FinalHitPoint, Normal );
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
        }
        else
        {
            SlipPlane.Setup( P0, P1, P2 );
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
        }

        // The triangle plane is our slide plane
        HitPlane.Setup( P0, P1, P2 );
            
        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_TRIANGLE,
            HitTriangleEdge,
            MAX( P0.Y, MAX( P1.Y, P2.Y ) ) );

        RecordCollision( TempCollision );
    }
}


//==============================================================================
// ApplyTriangleToRay
//==============================================================================
void collision_mgr::ApplyTriangleToRay(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT((m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY) 
        || (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT)
        || (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS) );
    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;
    vector3     Triangle[3];
    Triangle[0] = P0;
    Triangle[1] = P1;
    Triangle[2] = P2;
        
    if ( ComputeRayTriCollision(
        Triangle
        , m_RayInfo[m_ContextInfo.Context].Start
        , m_RayInfo[m_ContextInfo.Context].End
        , FinalT
        , FinalHitPoint ) )
    {
        // record the collision
        plane HitPlane( P0, P1, P2 );

        plane SlipPlane = HitPlane;
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_TRIANGLE,
            FALSE,
            MAX( P0.Y, MAX( P1.Y, P2.Y ) ) );

        RecordCollision( TempCollision );
    }
}


//==============================================================================
// ApplyAABBoxToCylinder
//==============================================================================
void collision_mgr::ApplyAABBoxToCylinder(
    const   bbox&       AABBox,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );
    ObjectHitGuid = m_ContextInfo.Guid;

    //--------------------------------------------------
    // Check collisions between matching spheres in
    // the start cylinder and the end cylinder
    //--------------------------------------------------
    s32         i;
    f32         FinalT;
    vector3     FinalHitPoint;
//    vector3     Triangle[3];
    plane       HitPlane;
    plane       SlipPlane;
        
    for ( i = 0; i < m_CylinderInfo[m_ContextInfo.Context].nStartSpheres; ++i )
    {
        if ( ComputeSphereAABBoxCollision(
              AABBox,
              m_CylinderInfo[m_ContextInfo.Context].StartSpherePositions[i],
              m_CylinderInfo[m_ContextInfo.Context].EndSpherePositions[i],
              m_CylinderInfo[m_ContextInfo.Context].Radius,
              FinalT,
              FinalHitPoint,
              HitPlane,
              SlipPlane) )
        {
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
            // record the collision

            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                ObjectHitGuid,
                PrimitiveKey,
                PRIMITIVE_STATIC_AA_BBOX,
                FALSE,
                AABBox.Max.Y );

            RecordCollision( TempCollision );
        }
    }
}


//==============================================================================
// ApplyAABBoxToSphere
//==============================================================================
void collision_mgr::ApplyAABBoxToSphere  (
    const   bbox&       AABBox,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );
    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;
//    vector3     Triangle[3];
    plane       HitPlane;
    plane       SlipPlane;
        
    if ( ComputeSphereAABBoxCollision(
            AABBox,
            m_SphereInfo[m_ContextInfo.Context].Start,
            m_SphereInfo[m_ContextInfo.Context].End,
            m_SphereInfo[m_ContextInfo.Context].Radius,
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane ) )
    {
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
        // record the collision

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_AA_BBOX,
            FALSE,
            AABBox.Max.Y );

        RecordCollision( TempCollision );
    }
}


//==============================================================================
// ApplyAABBoxToRay
//==============================================================================
void collision_mgr::ApplyAABBoxToRay(
    const   bbox&       AABBox,
            guid        ObjectHitGuid, /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY
        ||  (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY_SELECT));
    ObjectHitGuid = m_ContextInfo.Guid;

    f32         FinalT;
    vector3     FinalHitPoint;
//    vector3     Triangle[3];
    plane       HitPlane;
    plane       SlipPlane;
        
    if ( ComputeRayAABBoxCollision(
            AABBox,
            m_RayInfo[m_ContextInfo.Context].Start,
            m_RayInfo[m_ContextInfo.Context].End,
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane ) )
    {
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
        // record the collision

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            ObjectHitGuid,
            PrimitiveKey,
            PRIMITIVE_STATIC_AA_BBOX,
            FALSE,
            AABBox.Max.Y );

        RecordCollision( TempCollision );
    }
}


//==============================================================================
// GetCylinderSpherePositions
//==============================================================================
s32 collision_mgr::GetCylinderSpherePositions(
    const vector3&  Bottom,
    const vector3&  Top,
    f32             Radius,
    vector3*        SpherePositions,
    s32             MaxnSpheres )
{
    ASSERT( (Top-Bottom).LengthSquared() > 0.0001 );

    vector3 Dir = Top - Bottom;
    Dir.Normalize();

    // Position the bottom and top spheres
    const vector3 RadiusDir = Dir * Radius;
    const vector3 First = Bottom + RadiusDir;
    const vector3 Last = Top - RadiusDir;

    // Determine the number of spheres
    const f32 Dist = (Last-First).Length();
    s32 nSpheres = (s32)(Dist / Radius);

    // Patch for cases where Dist ~=~ Radius
    f32 fSpheres = (Dist / Radius) - (f32)nSpheres;
    if( fSpheres > 0.9f ) nSpheres++;

    nSpheres+=2;
    while ( nSpheres > MaxnSpheres ) --nSpheres;

    // Determine the spacing between the spheres
    const f32 Interval = Dist / (nSpheres - 1);
    const vector3 IntervalDir = Dir * Interval;

    // Create and store the sphere positions
    s32 i;
    vector3 Cur = First;
    for ( i = 0; i < nSpheres; ++i )
    {
        SpherePositions[i] = Cur;
        Cur += IntervalDir;
    }

    return nSpheres;
}

//==============================================================================

xbool IsTriangleInSphere( const vector3&    Center, 
                                f32         Radius,
                          const vector3*    pP )
{
    s32 i;

    plane Plane(pP[0],pP[1],pP[2]);

    // Get distance from sphere center to plane
    f32 D = Plane.Distance(Center);

    // Check if sphere is too far away from plane
    if( x_abs(D) > Radius )
        return FALSE;

    // Get closest pt on plane to center
    vector3 PPoint = Center - Plane.Normal*D;

    // Check if pt is inside tri
    for( i=0; i<3; i++ )
    {
        plane EdgePlane( pP[(i+1)%3], pP[i], pP[i]+Plane.Normal );
        if( EdgePlane.Distance( PPoint ) < 0 )
            break;
    }
    if( i==3 )
        return TRUE;

    // Plane point is outside triangle so see if edges touch sphere
    for( i=0; i<3; i++ )
    {
        vector3 CP = Center.GetClosestPToLSeg( pP[i], pP[(i+1)%3] );
        f32 Dist = (CP-Center).Length();
        if( Dist < Radius )
            return TRUE;
    }

    // No edge was closer than radius
    return FALSE;
}

//==============================================================================

xbool collision_mgr::CylinderSanityCheck( const vector3&  Bottom,
                                          const vector3&  Top,
                                          f32       Radius )
{
    s32 i,j;
    g_nCylinderSanityTris = 0;

    // Get Spheres
    vector3 SpherePos[16];
    s32     nSpheres;
    nSpheres = GetCylinderSpherePositions( Bottom, Top, Radius, SpherePos, 16 );

    // Get BBox
    bbox BBox;
    for( i=0; i<nSpheres; i++ )
        BBox += bbox( SpherePos[i], Radius );

    //bbox BBox = bbox((Bottom+Top)*0.5f, 2*MAX( ((Bottom-Top).Length()*0.5f), Radius ));

    // Gather Triangles
    GatherTriangles( BBox, object::ATTR_SOLID );

    // Loop through spheres
    for( i=0; i<nSpheres; i++ )
    {
        vector3 SPos = SpherePos[i];
        
        // Loop through triangles
        for( j=0; j<m_nTriangles; j++ )
        {
            if( IsTriangleInSphere( SPos, Radius, m_Triangle[j].P) )
            {
                ASSERT( g_nCylinderSanityTris < 100 );
                g_CylinderSanityTri[g_nCylinderSanityTris][0] = m_Triangle[j].P[0];
                g_CylinderSanityTri[g_nCylinderSanityTris][1] = m_Triangle[j].P[1];
                g_CylinderSanityTri[g_nCylinderSanityTris][2] = m_Triangle[j].P[2];
                ++g_nCylinderSanityTris;
            }
        }
    }

    return (g_nCylinderSanityTris == 0);
}

//==============================================================================
// RecordCollision
//==============================================================================
void collision_mgr::RecordCollision( collision_mgr::collision Collision )
{
    if ( m_ContextInfo.Context )
    {
        // Transform the collision back to world space
        Collision.Point = m_ContextInfo.L2W.Transform( Collision.Point );
        Collision.Plane.Transform( m_ContextInfo.L2W );
        Collision.SlipPlane.Transform( m_ContextInfo.L2W );
        ASSERT( x_abs(Collision.SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0001f );
    }
    
    ASSERT( (m_nCollisions <= m_nMaxCollsions)
        || (m_nMaxCollsions == -1));
    
    if ( (m_nCollisions == m_nMaxCollsions)
        && (m_nMaxCollsions > 0) )
    {
        // we may need to displace the collision with the largest T
        s32 i;
        f32 MaxT = -1.0f;
        s32 MaxIndex = -1;
        
        for ( i = 0; i < m_nCollisions; ++i )
        {
            if ( m_Collisions[i].T > MaxT )
            {
                MaxT = m_Collisions[i].T;
                MaxIndex = i;
            }
        }

        ASSERT( MaxIndex >=0 );

        if ( MaxT > Collision.T )
        {
            // Replace the largest
            m_Collisions[MaxIndex] = Collision;
        }
    }
    else
    {
        // just append
        if( m_nCollisions == MAX_COLLISION_MGR_COLLISIONS )
            return;
        m_Collisions[m_nCollisions] = Collision;
        ++m_nCollisions;
    }
}

//==============================================================================

const bbox& collision_mgr::GetDynamicBBox( void ) const
{
    return m_DynamicBBoxes[m_ContextInfo.Context];
}

//==============================================================================

s32 collision_mgr::GetSequence( void ) const
{
    return m_Sequence;
}

//==============================================================================

const bbox& collision_mgr::GetGatherBBox    ( void )
{
    return m_GatherBBox;
}

//==============================================================================

byte collision_mgr::GetSurfaceFlags ( const guid &Guid, s32 TriId )
{
    object* pObj = g_ObjMgr.LockObject(Guid);
    ASSERT(pObj);

    byte Result = 0;
    if (pObj->HasSurfaceFlags())    
        Result = pObj->GetTriFlags(TriId);

    g_ObjMgr.UnlockObject(Guid);
    return Result;
}

//==============================================================================
    
#ifdef EDITOR

void collision_mgr::EditorDisableCollisionRender ( const guid& Guid )
{
    object* pObject = g_ObjMgr.LockObject(Guid);
    ASSERT(pObject);
    pObject->DisableCollisionRender();
    g_ObjMgr.UnlockObject(Guid);
}
//==============================================================================

void collision_mgr::EditorEnableCollisionRender ( const guid& Guid, xbool DisableOthers )
{
    xarray<guid> Guids;
    Guids.Clear();
    Guids.Append(Guid);
    EditorEnableCollisionRender(Guids, DisableOthers);
}

//==============================================================================

void collision_mgr::EditorEnableCollisionRender ( const xarray<guid>& Guids, xbool DisableOthers )
{
    if (DisableOthers)
        g_RenderSelectedCollision++;

    s32 Count = Guids.GetCount();
    guid Guid;
    for (s32 i = 0; i < Count; i++)
    {
        Guid = Guids.GetAt(i);
        object* pObject = g_ObjMgr.LockObject(Guid);
        ASSERT(pObject);

        pObject->EnableCollisionRender();
        g_ObjMgr.UnlockObject(Guid);
    }
}
#endif

//==============================================================================
// collision
//==============================================================================
collision_mgr::collision::collision(
          f32           FinalT,
    const vector3&      HitPoint,
    const plane&        HitPlane,
    const plane&        SlipPlane,
          guid          HitGuid,
          s32           HitKey,
          primitive     HitStatic,
          xbool         HitEdge,
          f32           HitObjectHeight )
    :
    T(                  FinalT ),
    Point(              HitPoint ),
    Plane(              HitPlane ),
    SlipPlane(          SlipPlane ),
    ObjectHitGuid(      HitGuid ),
    PrimitiveKey(       HitKey ),
    StaticPrimitive(    HitStatic ),
    HitTriangleEdge(    HitEdge )
{
    (void)HitObjectHeight;
}
