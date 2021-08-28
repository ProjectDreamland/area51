//==============================================================================
//
//  CollisionMgr.cpp
//
//==============================================================================
// INSTALL PERMEABLES

//==============================================================================
//  INCLUDES
//==============================================================================

#include "CollisionMgr.hpp"
#include "x_stdio.hpp"
#include "CollisionPrimatives.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Entropy.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "GameLib\StatsMgr.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  STORAGE
//==============================================================================
collision_mgr   g_CollisionMgr;

xbool FORCE_LOW_POLY_LOS = TRUE;

//==============================================================================
// collision_mgr
//==============================================================================

collision_mgr::collision_mgr( void )
{
#ifdef ENABLE_COLLISION_STATS
    // Clear stats
    m_Stats.Reset();
    m_DisplayStats = FALSE;

    // Initialize logging
    m_pLogFileName      = NULL;
    m_LogLineNumber     = 0;
    m_bReportLog        = FALSE;
    m_LogTime           = 0;
    m_LogNLogicLoops    = 0;
    m_bDontLogCYL       = FALSE;
    m_bDontLogSPH       = FALSE;
    m_bDontLogRAY       = FALSE;
    m_bDontLogLOS       = FALSE;
#endif // ENABLE_COLLISION_STATS

    // Clear collision results
    InitializeCollisionCheckDefaults();
}

//==============================================================================
// InitializeCollisionCheckDefaults
//==============================================================================

void collision_mgr::InitializeCollisionCheckDefaults( void )
{
    m_nCollisions                   = 0;
    m_DynamicPrimitive              = PRIMITIVE_INVALID;
    m_nMaxCollisions                = 1;

    m_bUseIgnoreList                = FALSE;
	m_nIgnoredObjects               = 0;

    m_bCollectPermeable             = FALSE;
    m_nPermeables                   = 0;

    m_bUseLowPoly                   = FALSE;
    m_bIgnoreGlass                  = FALSE;
    m_bStopOnFirstCollisionFound    = FALSE;
    m_bIsRayCheck                   = FALSE;
    m_bIsEditorSelectRay            = FALSE;

    m_ContextInfo.Context           = 0;
    m_ContextInfo.Guid.Guid         = NULL_GUID;

    m_bApplyStarted                 = FALSE;
    m_bNotifyingPermeables          = FALSE;

    m_bRemoveDuplicateGuids         = TRUE;
    m_bUsePolyCache                 = FALSE;
}

//==============================================================================

#ifdef X_EDITOR

void collision_mgr::EditorSelectRay( const vector3& Start, const vector3& End, xbool bIncludeIcons )
{
    xhandle H;
	H.Handle = -1;

    //
    // Initialize the collision manager
    //
    vector3 Dir = End - Start;
    Dir.Normalize();
    vector3 NewStart( Start + Dir * 150 );

    g_CollisionMgr.EditorSelectRaySetup(NewStart, End);
    g_CollisionMgr.SetMaxCollisions(MAX_COLLISION_MGR_COLLISIONS);

    //
    // Select all the objects that the ray may hit
    //
    bbox BBox;
    BBox.Clear();
    BBox.AddVerts( &NewStart, 1 );
    BBox.AddVerts( &End,   1 );
    g_ObjMgr.SelectBBox( object::TYPE_ALL_TYPES, BBox );

    //
    // Go throw all the objects and collect all the collisions.
    //
    slot_id     ID;

    for( ID = g_ObjMgr.StartLoop(); ID != SLOT_NULL; ID = g_ObjMgr.GetNextResult(ID) )
    {
        // Get the candiate object
        object*             pObject = g_ObjMgr.GetObjectBySlot(ID);
        if( pObject == NULL ) 
            continue;

        // Check whether it is set to render or is hidden
        const object_desc&  ObjDesc = pObject->GetTypeDesc();        
        if( ObjDesc.CanRender() == FALSE )
        {
            continue;
        }

        // Check if selection is turned off
        if( (pObject->IsSelectable() == FALSE) || (pObject->IsHidden()) )
            continue;

        // Check whether it should select the the icon or its own collision
        if( ObjDesc.IsIconSelect() && pObject->IsIconSelectable() )
        {
            if (bIncludeIcons)
            {
                g_CollisionMgr.StartApply( pObject->GetGuid() );
                g_CollisionMgr.ApplySphere( pObject->GetPosition(), 50 );
                g_CollisionMgr.EndApply();
            }
        }
        else
        {
            pObject->OnColCheck();
        }
    }

    g_ObjMgr.EndLoop();

    //
    // Now Sort the Collisions
    //
    SortCollisions();
}

#endif

//==============================================================================

s32 RAYBBOX_TEST=0;
s32 RAYBBOX_CULL=0;
xbool COLL_DISPLAY_OBJECTS=FALSE;
xbool collision_mgr::fn_CheckCollisions( object::type        ThisType        /*= TYPE_ALL_TYPES*/, 
                                         u32                 TheseAttributes /*= ATTR_COLLIDABLE */, 
                                         u32                 NotTheseAttributes /*= 0x00000000*/ )
{
    LOG_STAT( k_stats_Collision );

#ifdef ENABLE_COLLISION_STATS
    xtimer StatsTimer;
    StatsTimer.Start();
#endif

    m_FilterThisType = ThisType;
    m_FilterTheseAttributes = TheseAttributes;
    m_FilterNotTheseAttributes = NotTheseAttributes;

    if( FORCE_LOW_POLY_LOS && (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS) )
        m_bUseLowPoly = TRUE;

    // Turn on polycache if lowpoly
    if( m_bUseLowPoly ) 
        m_bUsePolyCache = TRUE;

    // Shut off polycache if looking for permeables
    if( m_FilterTheseAttributes & object::ATTR_COLLISION_PERMEABLE )
        m_bUsePolyCache = FALSE;

    //if( COLL_DISPLAY_OBJECTS )
    //    x_DebugMsg("---------------------------------------\n");

    //
    // If using polycache let specialized functions handle it
    //
    if( m_bUsePolyCache )
    {
        // Confirm Low polys
        ASSERT( m_bUseLowPoly );
        
        if( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER )
            ApplyCylinderToPolyCache();

        if( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE )
            ApplySphereToPolyCache();

        if( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS )
            ApplyRayToPolyCache();

        if( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY )
            ApplyRayToPolyCache();
    }
    else
    {
        xbool bEarlyFinish = FALSE;

        //
        // Playsurfaces are special, handle them now
        //
        if( (ThisType == object::TYPE_ALL_TYPES) || (ThisType == object::TYPE_PLAY_SURFACE) )
        {
            if( m_bIsRayCheck )
                g_PlaySurfaceMgr.CollectSurfaces( m_RayInfo[0].Start, m_RayInfo[0].End, TheseAttributes, NotTheseAttributes );
            else
                g_PlaySurfaceMgr.CollectSurfaces( m_DynamicBBoxes[m_ContextInfo.Context], TheseAttributes, NotTheseAttributes );

            playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.GetNextSurface();
            while ( pSurface != NULL )
            {
                playsurface_mgr::surface* pS = pSurface;
                guid SurfaceGuid = g_PlaySurfaceMgr.GetPlaySurfaceGuid();
                pSurface = g_PlaySurfaceMgr.GetNextSurface();

                //
                // Do a ray check against bbox for early culling
                //
                if( m_bIsRayCheck )
                {
                    f32 T;
                    RAYBBOX_TEST++;
                    if( pS->WorldBBox.Intersect( T, m_RayInfo[0].Start, m_RayInfo[0].End ) == FALSE )
                    {
                        RAYBBOX_CULL++;
                        continue;
                    }
                }

                rigid_geom* pGeom = (rigid_geom*)render::GetGeom( pS->RenderInst );

                //xtimer Timer;
                //Timer.Start();
                RigidGeom_ApplyCollision(  SurfaceGuid, 
                                           pS->WorldBBox, 
                                           (u64)-1, 
                                           &pS->L2W, 
                                           pGeom ); 
                //Timer.Stop();

                //if( COLL_DISPLAY_OBJECTS )
                //    x_DebugMsg("%8.4f  PLAYSURFACE\n",Timer.ReadMs());

                // If we are doing LOS then we might be done!
                if( m_bStopOnFirstCollisionFound && (m_nCollisions>0) )
                {
                    bEarlyFinish = TRUE;
                    break;
                }
            }
        }

        //
        // Check other objects if not finished
        //
        if( !bEarlyFinish )
        {
            if( m_bIsRayCheck )
                g_ObjMgr.SelectRay(TheseAttributes, m_RayInfo[0].Start, m_RayInfo[0].End,ThisType,NotTheseAttributes );
            else
                g_ObjMgr.SelectBBox(TheseAttributes, m_DynamicBBoxes[0],ThisType,NotTheseAttributes );

            for( slot_id aID = g_ObjMgr.StartLoop(); aID != SLOT_NULL; aID = g_ObjMgr.GetNextResult(aID) )
            {
                if( m_MovingObjGuid != g_ObjMgr.GetObjectBySlot(aID)->GetGuid() &&  
                    !IsInIgnoreList( g_ObjMgr.GetObjectBySlot(aID)->GetGuid() )    )
                {

                    object& Object = *g_ObjMgr.GetObjectBySlot(aID);

                    /// here we make sure the object is collidable or else we just move on.
                    if( (Object.GetAttrBits() & object::ATTR_COLLIDABLE) != object::ATTR_COLLIDABLE )
                    {
                        continue;
                    }
                    //
                    // Do a ray check against bbox for early culling (Handled by SelectRay())
                    //

                    //xtimer Timer;
                    //Timer.Start();
                    Object.OnColCheck();
                    //Timer.Stop();

                    //if( COLL_DISPLAY_OBJECTS )
                    //    x_DebugMsg("%8.4f  %s\n",Timer.ReadMs(),Object.GetTypeDesc().GetTypeName());


                    // If we are doing LOS then we might be done!
                    if( m_bStopOnFirstCollisionFound && (m_nCollisions>0) )
                    {
                        bEarlyFinish = TRUE;
                        break;
                    }

                }    
            }

            g_ObjMgr.EndLoop();
        }
    }

    //
    // sort the collisions by T
    //
    SortCollisions();

    //
    // clean permeables
    //
    if( m_bCollectPermeable )
        CleanPermeables();

#ifdef ENABLE_COLLISION_STATS
    StatsTimer.Stop();
    {
        f32 MS = StatsTimer.ReadMs();

        switch( m_DynamicPrimitive )
        {
            case PRIMITIVE_DYNAMIC_CYLINDER:{m_Stats.Time[0] += MS; m_Stats.Count[0]++;} break;
            case PRIMITIVE_DYNAMIC_SPHERE:  {m_Stats.Time[1] += MS; m_Stats.Count[1]++;} break;
            case PRIMITIVE_DYNAMIC_RAY:     {m_Stats.Time[2] += MS; m_Stats.Count[2]++;} break;
            case PRIMITIVE_DYNAMIC_LOS:     {m_Stats.Time[3] += MS; m_Stats.Count[3]++;} break;
        }

        m_LogTime = MS;
    }

    //
    // Log collision requests 
    //
    if( m_bReportLog )
        fn_REPORT();

#endif // ENABLE_COLLISION_STATS

    return (m_nCollisions > 0);
}

//==============================================================================
extern s32 GetNLogicLoops( void );

#ifdef ENABLE_COLLISION_STATS
void collision_mgr::fn_REPORT( void )
{
    static f32 LastTime = 0;

    char* PrimName = "NONE";
    switch( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:{PrimName = "CYL"; if( m_bDontLogCYL ) return; } break;
    case PRIMITIVE_DYNAMIC_SPHERE:  {PrimName = "SPH"; if( m_bDontLogSPH ) return; } break;
    case PRIMITIVE_DYNAMIC_RAY:     {PrimName = "RAY"; if( m_bDontLogRAY ) return; } break;
    case PRIMITIVE_DYNAMIC_LOS:     {PrimName = "LOS"; if( m_bDontLogLOS ) return; } break;
    }

    if( GetNLogicLoops() != m_LogNLogicLoops )
    {
        m_LogNLogicLoops  = GetNLogicLoops();
        x_DebugMsg("              %5.2f\n",LastTime);
        x_DebugMsg("(%d) ------------------------------------------------------------\n",m_LogNLogicLoops);
        LastTime=0;
    }

    s32 StrLen = x_strlen(m_pLogFileName);
    if( StrLen > 24 )
        m_pLogFileName += StrLen - 24;

    x_DebugMsg("COLL: %c %c (%s) %5.2f %4d %s\n",
        (m_bUseLowPoly)?('L'):('H'),
        (m_bUsePolyCache)?('Y'):('N'),
        PrimName,
        m_LogTime,
        m_LogLineNumber,
        m_pLogFileName);

    LastTime += m_LogTime;
}
#endif // ENABLE_COLLISION_STATS

//==============================================================================

void collision_mgr::StartApply(       guid      Guid, 
                                const matrix4&  L2W,
                                const matrix4&  W2L )
{
    ASSERT( !m_bApplyStarted ); // EndApply() wasn't called

    //
    // Object is colliding against self
    //
    ASSERT( (Guid==0) || (Guid != m_MovingObjGuid) );

    //
    // Set up context info
    //
    m_bApplyStarted          = TRUE;
    m_ContextInfo.Context   = 1;
    m_ContextInfo.Guid      = Guid;
    m_ContextInfo.L2W       = L2W;
    m_ContextInfo.W2L       = W2L;

    //
    // Transform the dynamic primitive from world to local space
    //
    switch( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
    {
        // transform start/end bottom/top
        m_CylinderInfo[1].BotStart = W2L.Transform(m_CylinderInfo[0].BotStart);
        m_CylinderInfo[1].BotEnd   = W2L.Transform(m_CylinderInfo[0].BotEnd);
        m_CylinderInfo[1].TopStart = W2L.Transform(m_CylinderInfo[0].TopStart);
        m_CylinderInfo[1].TopEnd   = W2L.Transform(m_CylinderInfo[0].TopEnd);

        // transform radius
        const vector3 Scale = W2L.GetScale();
        ASSERT( x_abs(Scale.GetX()-1.0f) < 0.001f );

        m_CylinderInfo[1].Radius = m_CylinderInfo[0].Radius * Scale.GetX();
        m_CylinderInfo[1].Height = m_CylinderInfo[0].Height * Scale.GetX();
        

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
        ASSERT( (x_abs(Scale.GetX()-Scale.GetY()) < 0.001f) &&
                (x_abs(Scale.GetY()-Scale.GetZ()) < 0.001f) );

        m_SphereInfo[1].Radius
            = m_SphereInfo[0].Radius * Scale.GetX();
        break;
    }
    
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

void collision_mgr::CylinderSetup (
          guid      MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          f32       Radius,
          f32       Height )
{
    LOG_STAT( k_stats_Collision );

    // Initialize
    InitializeCollisionCheckDefaults();
    UseLowPoly();

    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_CYLINDER;
    m_MovingObjGuid         = MovingObjGuid;


    m_CylinderInfo[0].BotStart  = WorldStart;
    m_CylinderInfo[0].BotEnd    = WorldEnd;
    m_CylinderInfo[0].TopStart  = WorldStart + vector3(0,Height,0);
    m_CylinderInfo[0].TopEnd    = WorldEnd + vector3(0,Height,0);
    m_CylinderInfo[0].Radius    = Radius;
    m_CylinderInfo[0].Height    = Height;

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

    // SB 2/21/05:
    // Compute end sphere positions from start positions + movement delta since
    // calling GetCylinderSpherePositions() with the BotEnd and TopEnd 
    // positions can return a different sphere count due to float precision.
    vector3 MoveDelta = WorldEnd - WorldStart;
    s32     nSpheres  = m_CylinderInfo[0].nStartSpheres;
    for( s32 i = 0; i < nSpheres; i++ )
    {
        m_CylinderInfo[0].EndSpherePositions[i] = m_CylinderInfo[0].StartSpherePositions[i] + MoveDelta;
    }
    m_CylinderInfo[0].nEndSpheres = nSpheres;

    // Check    
    ASSERT( m_CylinderInfo[0].nStartSpheres    // Something must be wrong with
         == m_CylinderInfo[0].nEndSpheres );   // GetCylinderSpherePositions() 
                                               // or the caller has changed the 
                                               // size of the cylinder during   
                                               // the move                      

    const s32 StartLast = m_CylinderInfo[0].nStartSpheres-1;
    const s32 EndLast   = m_CylinderInfo[0].nEndSpheres-1;

    m_DynamicBBoxes[0].Min = m_CylinderInfo[0].StartSpherePositions[0];
    m_DynamicBBoxes[0].Max = m_CylinderInfo[0].StartSpherePositions[0];        
    m_DynamicBBoxes[0].Min.Min( m_CylinderInfo[0].StartSpherePositions[StartLast] );
    m_DynamicBBoxes[0].Max.Max( m_CylinderInfo[0].StartSpherePositions[StartLast] );
    m_DynamicBBoxes[0].Min.Min( m_CylinderInfo[0].EndSpherePositions[0] );
    m_DynamicBBoxes[0].Max.Max( m_CylinderInfo[0].EndSpherePositions[0] );
    m_DynamicBBoxes[0].Min.Min( m_CylinderInfo[0].EndSpherePositions[EndLast] );
    m_DynamicBBoxes[0].Max.Max( m_CylinderInfo[0].EndSpherePositions[EndLast] );
        
    vector3 vRadius( Radius, Radius, Radius );
    m_DynamicBBoxes[0].Min -= vRadius;
    m_DynamicBBoxes[0].Max += vRadius;
}

//==============================================================================

void collision_mgr::SphereSetup (
          guid      MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd,
          f32       Radius )
{
    LOG_STAT( k_stats_Collision );

    // Initialize
    InitializeCollisionCheckDefaults();

    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_SPHERE;
    m_MovingObjGuid         = MovingObjGuid;
    m_SphereInfo[0].Start   = WorldStart;
    m_SphereInfo[0].End     = WorldEnd;
    m_SphereInfo[0].Radius  = Radius;

    vector3 vRadius( Radius, Radius, Radius );
    m_DynamicBBoxes[0].Min = WorldStart;
    m_DynamicBBoxes[0].Min.Min( WorldEnd );
    m_DynamicBBoxes[0].Min -= vRadius;

    m_DynamicBBoxes[0].Max = WorldStart;
    m_DynamicBBoxes[0].Max.Max( WorldEnd );
    m_DynamicBBoxes[0].Max += vRadius;
}

//==============================================================================

void collision_mgr::RaySetup(
          guid      MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd )
{
    LOG_STAT( k_stats_Collision );

    // Initialize
    InitializeCollisionCheckDefaults();

    m_DynamicPrimitive      = PRIMITIVE_DYNAMIC_RAY;
    m_MovingObjGuid         = MovingObjGuid;
    m_RayInfo[0].Start      = WorldStart;
    m_RayInfo[0].End        = WorldEnd;

    m_DynamicBBoxes[0].Min = WorldStart;
    m_DynamicBBoxes[0].Min.Min( WorldEnd );
    m_DynamicBBoxes[0].Max = WorldStart;
    m_DynamicBBoxes[0].Max.Max( WorldEnd );

    m_bIsRayCheck = TRUE;
}

//==============================================================================

void collision_mgr::EditorSelectRaySetup( 
    const vector3&  WorldStart,
    const vector3&  WorldEnd )
{
    LOG_STAT( k_stats_Collision );

    // Initialize
    RaySetup( 0, WorldStart, WorldEnd );

    // Record that it's an editor select ray
    m_bIsEditorSelectRay = TRUE ;
}
                                      
//==============================================================================

void collision_mgr::LineOfSightSetup(
    guid            MovingObjGuid,
    const vector3&  WorldStart,
    const vector3&  WorldEnd )
{
    LOG_STAT( k_stats_Collision );

    // Initialize
    RaySetup( MovingObjGuid, WorldStart, WorldEnd );

    m_DynamicPrimitive = PRIMITIVE_DYNAMIC_LOS;

    StopOnFirstCollisionFound();
    IgnoreGlass();
}

//==============================================================================

void collision_mgr::ApplySphere(
    const vector3&  WorldPos,
          f32       Radius,
          u32       Flags,
          s32       PrimitiveKey /* = 0 */ )
{

    LOG_STAT( k_stats_Collision );
    ASSERT( m_bApplyStarted );

    // check to see if the bboxes intersect
    vector3 vRadius( Radius, Radius, Radius );
    bbox BBox( WorldPos - vRadius, WorldPos + vRadius );
    if ( !BBox.Intersect( m_DynamicBBoxes[m_ContextInfo.Context] ) )
    {
        return;
    }
    
    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplySphereToCylinder( WorldPos, Radius, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplySphereToSphere( WorldPos, Radius, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:
    case PRIMITIVE_DYNAMIC_LOS:
        ApplySphereToRay( WorldPos, Radius, Flags, PrimitiveKey );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}
    
//==============================================================================

void collision_mgr::ApplyTriangle(
    const vector3&  P0,
    const vector3&  P1,
    const vector3&  P2,
          u32       Flags,          /* = 0 */
          s32       PrimitiveKey    /* = 0 */ )

{
    LOG_STAT( k_stats_Collision );
    CONTEXT( "collision_mgr::ApplyTriangle" );

    ASSERT( m_bApplyStarted );

    const bbox& DBox = m_DynamicBBoxes[m_ContextInfo.Context];

    if ( P0.GetX() < DBox.Min.GetX() && P1.GetX() < DBox.Min.GetX() && P2.GetX() < DBox.Min.GetX() ) return;
    if ( P0.GetY() < DBox.Min.GetY() && P1.GetY() < DBox.Min.GetY() && P2.GetY() < DBox.Min.GetY() ) return;
    if ( P0.GetZ() < DBox.Min.GetZ() && P1.GetZ() < DBox.Min.GetZ() && P2.GetZ() < DBox.Min.GetZ() ) return;
    if ( P0.GetX() > DBox.Max.GetX() && P1.GetX() > DBox.Max.GetX() && P2.GetX() > DBox.Max.GetX() ) return;
    if ( P0.GetY() > DBox.Max.GetY() && P1.GetY() > DBox.Max.GetY() && P2.GetY() > DBox.Max.GetY() ) return;
    if ( P0.GetZ() > DBox.Max.GetZ() && P1.GetZ() > DBox.Max.GetZ() && P2.GetZ() > DBox.Max.GetZ() ) return;

    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplyTriangleToCylinder( P0, P1, P2, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplyTriangleToSphere( P0, P1, P2, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:        
    case PRIMITIVE_DYNAMIC_LOS:
        ApplyTriangleToRay( P0, P1, P2, Flags, PrimitiveKey );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}

//==============================================================================

void collision_mgr::ApplyQuad( const vector3&    P0,
                               const vector3&    P1,
                               const vector3&    P2,
                               const vector3&    P3,
                                     u32         Flags,
                                     s32         PrimitiveKey )
{
    LOG_STAT( k_stats_Collision );
    CONTEXT( "collision_mgr::ApplyQuad" );

    ASSERT( m_bApplyStarted );

    const bbox& DBox = m_DynamicBBoxes[m_ContextInfo.Context];

    if ( (P0.GetX() < DBox.Min.GetX()) && (P1.GetX() < DBox.Min.GetX()) && (P2.GetX() < DBox.Min.GetX()) && (P3.GetX() < DBox.Min.GetX()) ) return;
    if ( (P0.GetY() < DBox.Min.GetY()) && (P1.GetY() < DBox.Min.GetY()) && (P2.GetY() < DBox.Min.GetY()) && (P3.GetY() < DBox.Min.GetY()) ) return;
    if ( (P0.GetZ() < DBox.Min.GetZ()) && (P1.GetZ() < DBox.Min.GetZ()) && (P2.GetZ() < DBox.Min.GetZ()) && (P3.GetZ() < DBox.Min.GetZ()) ) return;
    if ( (P0.GetX() > DBox.Max.GetX()) && (P1.GetX() > DBox.Max.GetX()) && (P2.GetX() > DBox.Max.GetX()) && (P3.GetX() > DBox.Max.GetX()) ) return;
    if ( (P0.GetY() > DBox.Max.GetY()) && (P1.GetY() > DBox.Max.GetY()) && (P2.GetY() > DBox.Max.GetY()) && (P3.GetY() > DBox.Max.GetY()) ) return;
    if ( (P0.GetZ() > DBox.Max.GetZ()) && (P1.GetZ() > DBox.Max.GetZ()) && (P2.GetZ() > DBox.Max.GetZ()) && (P3.GetZ() > DBox.Max.GetZ()) ) return;

    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplyTriangleToCylinder( P0, P1, P2, Flags, PrimitiveKey );
        ApplyTriangleToCylinder( P2, P3, P0, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplyTriangleToSphere( P0, P1, P2, Flags, PrimitiveKey );
        ApplyTriangleToSphere( P2, P3, P0, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:        
    case PRIMITIVE_DYNAMIC_LOS:
        ApplyTriangleToRay( P0, P1, P2, Flags, PrimitiveKey );
        ApplyTriangleToRay( P2, P3, P0, Flags, PrimitiveKey );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }

}

//==============================================================================

void collision_mgr::ApplyAABBox(
    const bbox&     BBox,
          u32       Flags,
          s32       PrimitiveKey /* = 0 */ )
{
    LOG_STAT( k_stats_Collision );
    ASSERT( m_bApplyStarted );

    // check to see if the bboxes intersect
    if ( !BBox.Intersect( m_DynamicBBoxes[m_ContextInfo.Context] ) )
    {
        return;
    }
    
    // what's our moving primitive?
    switch ( m_DynamicPrimitive )
    {
    case PRIMITIVE_DYNAMIC_CYLINDER:
        ApplyAABBoxToCylinder( BBox, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_SPHERE:
        ApplyAABBoxToSphere( BBox, Flags, PrimitiveKey );
        break;
    case PRIMITIVE_DYNAMIC_RAY:
    case PRIMITIVE_DYNAMIC_LOS:
        ApplyAABBoxToRay( BBox, Flags );
        break;
    default:
        ASSERT( 0 ); // invalid primitive
    }
}

//==============================================================================

void collision_mgr::ApplyOOBBox ( const bbox&       LocalBBox,
                                  const matrix4&    L2W,
                                             u32    Flags,
                                             s32    PrimitiveKey )
{
    // Indices used to convert min + max of bbox into 8 corners
    static s32 CornerIndices[8*3]   = { 0,1,2, 
                                        4,1,2,
                                        0,5,2,
                                        4,5,2,
                                        0,1,6, 
                                        4,1,6,
                                        0,5,6,
                                        4,5,6 };

    // Indices used to convert 8 corners into a 4 sided NGon
    static s32 SideIndices[6*4]     = { 0,2,3,1,
                                        1,3,7,5,
                                        5,7,6,4,
                                        4,6,2,0,
                                        2,6,7,3,
                                        4,0,1,5 };

    // Locals
    s32         i ;
    vector3     Local;
    vector3     Corners[8];
    const s32*  pIndices;
    const f32*  pBBoxF;

    // Transform all corners of the local AA bbox into world space
    pIndices = CornerIndices;
    pBBoxF   = (f32*)&LocalBBox;
    for (i = 0; i < 8; i++ )
    {
        // Setup corner in local space
        Local.Set( pBBoxF[pIndices[0]], pBBoxF[pIndices[1]], pBBoxF[pIndices[2]] );

        // Transform into world space
        Corners[i] = L2W * Local;

        // Next vert
        pIndices += 3;
    }

    // Apply 6 sides of world oobbox
    pIndices = SideIndices;
    for (i = 0; i < 6; i++)
    {
        // Indices of side plane are pIndices[0], pIndices[1], pIndices[2], pIndices[3]

        // Apply tri0
        ApplyQuad(Corners[pIndices[0]], // P0
                  Corners[pIndices[1]], // P1
                  Corners[pIndices[2]], // P2
                  Corners[pIndices[3]], // P3
                  Flags,                // Flags
                  PrimitiveKey);        // PrimitiveKey

        // Next side
        pIndices += 4;
    }
}

//==============================================================================

void collision_mgr::ApplySphereToCylinder(
    const vector3&  WorldPos,
          f32       Radius,
          u32       Flags,
          s32       PrimitiveKey    /* = 0 */ )
{
    // required because of data dependencies
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

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
            vector3 Normal = (FinalHitPoint - WorldPos) * (1 / Radius);
            plane HitPlane;

            // Deal with creating a plane when we are already inside the collision cylinder
            if( FinalT == 0 )
            {
                Normal = m_CylinderInfo[m_ContextInfo.Context].StartSpherePositions[i] - WorldPos;
            }

            //#### WE SHOULDN'T HAVE TO DO THIS IF STATEMENT. FIGURE OUT WHAT IS
            //     WRONG WITH THE ABOVE LOGIC!!!!
            if ( Normal.LengthSquared() < 0.01f )
            {
                Normal.Set( 0.0f, 1.0f, 0.0f );
            }
            HitPlane.Setup( FinalHitPoint, Normal );

            plane SlipPlane = HitPlane;

            //  This assert is to verify that the number is normalized BUT
            //  it was causing an assert failure due purely to rounding errors.
            //  The number was increased to avoid the assert failure but the 
            //  ASSERT was left to verify that it was still close to 1.0, just
            //  not as close.  If it fails again with a normalized value, just
            //  up it again.
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.0005f );

            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                m_ContextInfo.Guid,
                PrimitiveKey,
                PRIMITIVE_STATIC_SPHERE,
                FALSE,
                WorldPos.GetY() + Radius,
                Flags );

            RecordCollision( TempCollision );
        }
    }
}

//==============================================================================

void collision_mgr::ApplySphereToSphere(
    const vector3&  WorldPos,
          f32       Radius,
          u32       Flags,
          s32       PrimitiveKey    /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );

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
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.01f );
        
        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_SPHERE,
            FALSE,
            WorldPos.GetY() + Radius,
            Flags );

        RecordCollision( TempCollision );
    }
}
 
//==============================================================================

void collision_mgr::ApplySphereToRay(
    const vector3&  WorldPos,
          f32       Radius,
          u32       Flags,
          s32       PrimitiveKey    /* = 0 */ )
{
    ASSERT( m_bIsRayCheck );

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
        // Setup normal
        vector3 Normal ;
        if (FinalT == 0) // SB - Fix the normal for when ray start + end are inside the sphere!
            Normal = m_RayInfo[m_ContextInfo.Context].End - m_RayInfo[m_ContextInfo.Context].Start ;
        else
            Normal = (FinalHitPoint - WorldPos) * (1 / Radius);

        // record the collision
        plane HitPlane;
        HitPlane.Setup( FinalHitPoint, Normal );

        plane SlipPlane = HitPlane;
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.01f );

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_SPHERE,
            FALSE,
            WorldPos.GetY() + Radius,
            Flags );

        RecordCollision( TempCollision );
    }
    
}

//==============================================================================

void collision_mgr::ApplyTriangleToCylinder(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            u32         Flags,
            s32         PrimitiveKey    )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

    //--------------------------------------------------
    // Check collisions between matching spheres in
    // the start cylinder and the end cylinder
    //--------------------------------------------------
    s32         i;
    f32         FinalT;
    vector3     FinalHitPoint;
    vector3     Triangle[3];
    //xbool       HitTriangleEdge;
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
            //, HitTriangleEdge 
            ) )
        {
            // record the collision
            plane HitPlane;
            plane SlipPlane;

            //if ( HitTriangleEdge )
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

            HitPlane.Setup( P0, P1, P2 );
            
            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                m_ContextInfo.Guid,
                PrimitiveKey,
                PRIMITIVE_STATIC_TRIANGLE,
                FALSE,//HitTriangleEdge,
                MAX( P0.GetY(), MAX( P1.GetY(), P2.GetY() ) ),
                Flags );

            RecordCollision( TempCollision );
        }
    }
}


//==============================================================================

void collision_mgr::ApplyTriangleToStretchedSphere(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            u32         Flags,
            s32         PrimitiveKey    )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

    f32     Radius              = m_CylinderInfo[0].Radius;
    f32     HalfHeight          = m_CylinderInfo[0].Height * 0.5f;
    f32     WorldToSphereScale  = Radius/HalfHeight;
    f32     SphereToWorldScale  = 1.0f / WorldToSphereScale;
    vector3 WorldSpaceCenter    = m_CylinderInfo[0].BotStart + vector3(0,HalfHeight,0);
    WorldSpaceCenter.GetX() = 0;
    WorldSpaceCenter.GetZ() = 0;

    //
    // Scale all important info from world into sphere space
    //
    vector3 SphereSpaceTriangle[3];
    SphereSpaceTriangle[0] = (P0-WorldSpaceCenter);
    SphereSpaceTriangle[1] = (P1-WorldSpaceCenter);
    SphereSpaceTriangle[2] = (P2-WorldSpaceCenter);
    SphereSpaceTriangle[0].GetY() *= WorldToSphereScale;
    SphereSpaceTriangle[1].GetY() *= WorldToSphereScale;
    SphereSpaceTriangle[2].GetY() *= WorldToSphereScale;

    vector3 SphereSpaceStart;
    vector3 SphereSpaceStop;
    
    SphereSpaceStart    = (m_CylinderInfo[0].BotStart + vector3(0,HalfHeight,0)) - WorldSpaceCenter;
    SphereSpaceStop     = (m_CylinderInfo[0].BotEnd   + vector3(0,HalfHeight,0)) - WorldSpaceCenter;
    SphereSpaceStart.GetY() *= WorldToSphereScale;
    SphereSpaceStop.GetY()  *= WorldToSphereScale;

    //
    // Now compute sphere triangle collision
    //
    f32         FinalT;
    vector3     FinalHitPoint;

    if ( ComputeSphereTriCollision(
          SphereSpaceTriangle
        , SphereSpaceStart
        , SphereSpaceStop
        , Radius
        , FinalT
        , FinalHitPoint
        //, HitTriangleEdge 
        ) )
    {
        // record the collision
        plane HitPlane;
        plane SlipPlane;

        //
        // Keep sphere space hit point
        //
        vector3 SphereSpaceHitPoint = FinalHitPoint;

        //
        // Transform the final hit point back into world space
        //
        FinalHitPoint.GetY() *= SphereToWorldScale;
        FinalHitPoint   += WorldSpaceCenter;

        // Compute ellipsoid space hit point
        vector3 SphereSpaceCenterAtImpact = SphereSpaceStart + FinalT*(SphereSpaceStop-SphereSpaceStart);
        vector3 EllipsoidSpaceHitPoint    = SphereSpaceHitPoint - SphereSpaceCenterAtImpact;
        EllipsoidSpaceHitPoint.GetY() *= SphereToWorldScale;

        // The normal(Nx,Ny,Nz) of a point(x,y,z) on an ellipsoid of radii (a,b,c) is:
        //      ( Nx, Ny, Nz ) = ( 2x/a^2, 2y/b^2, 2z/c^2 )
        // See this link for details on how it is derived:
        // http://www.peroxide.dk/download/tutorials/tut10/pxdtut10.html

        // Compute slide world normal using ( Nx, Ny, Nz ) = ( x/a^2, y/b^2, z/c^2 )
        //   where:
        //      ( x, y, z ) = EllipsoidSpaceHitPoint
        //      ( a, b, c ) = ( Radius, HalfHeight, Radius )
        // NOTE: The *2 is skipped since the end result is normalized
        vector3 WorldSpaceNormal = -EllipsoidSpaceHitPoint;
        f32     OneOverRadiusSqr = 1.0f / ( Radius * Radius );
        WorldSpaceNormal.GetX() *= OneOverRadiusSqr;
        WorldSpaceNormal.GetY() /= HalfHeight * HalfHeight;
        WorldSpaceNormal.GetZ() *= OneOverRadiusSqr;
        WorldSpaceNormal.Normalize();

/*
        // Compute sphere space slip normal
        //vector3 SphereSpaceCenterAtImpact = SphereSpaceStart + FinalT*(SphereSpaceStop-SphereSpaceStart);
        //vector3 SphereSpaceNormal = SphereSpaceCenterAtImpact - SphereSpaceHitPoint;

        vector3 OldWorldSpaceNormal;
        {
            f32     AbsA, AbsB, AbsC;
            vector3 Dir;
            vector3 AxisA;
            vector3 AxisB;

            // Get a non-parallel axis to normal.
            AbsA = x_abs( SphereSpaceNormal.GetX() );
            AbsB = x_abs( SphereSpaceNormal.GetY() );
            AbsC = x_abs( SphereSpaceNormal.GetZ() );
            if( (AbsA<=AbsB) && (AbsA<=AbsC) ) Dir = vector3(1,0,0);
            else
            if( (AbsB<=AbsA) && (AbsB<=AbsC) ) Dir = vector3(0,1,0);
            else                               Dir = vector3(0,0,1);

            AxisA = SphereSpaceNormal.Cross(Dir);
            AxisB = SphereSpaceNormal.Cross(AxisA);
            AxisA.GetY() *= SphereToWorldScale;
            AxisB.GetY() *= SphereToWorldScale;
            OldWorldSpaceNormal = AxisA.Cross(AxisB);
            OldWorldSpaceNormal.Normalize();
        }
*/

        SlipPlane.Setup( FinalHitPoint, WorldSpaceNormal );

        //
        // Setup the hit plane from the original triangle
        //
        HitPlane.Setup( P0, P1, P2 );

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_TRIANGLE,
            FALSE,
            MAX( P0.GetY(), MAX( P1.GetY(), P2.GetY() ) ),
            Flags );

        RecordCollision( TempCollision );
    }
}

//==============================================================================

void collision_mgr::ApplyTriangleToSphere(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            u32         Flags,         /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );

    f32         FinalT;
    vector3     FinalHitPoint;
    vector3     Triangle[3];
    Triangle[0] = P0;
    Triangle[1] = P1;
    Triangle[2] = P2;
        
    if ( ComputeSphereTriCollision(
        Triangle,
        m_SphereInfo[m_ContextInfo.Context].Start,
        m_SphereInfo[m_ContextInfo.Context].End,
        m_SphereInfo[m_ContextInfo.Context].Radius,
        FinalT,
        FinalHitPoint
        ) )
    {
        // record the collision
        plane HitPlane;
        plane SlipPlane;

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

        // The triangle plane is our slide plane
        HitPlane.Setup( P0, P1, P2 );
            
        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_TRIANGLE,
            FALSE,
            MAX( P0.GetY(), MAX( P1.GetY(), P2.GetY() ) ),
            Flags );

        RecordCollision( TempCollision );
    }
}

//==============================================================================

void collision_mgr::ApplyTriangleToRay(
    const   vector3&    P0,
    const   vector3&    P1,
    const   vector3&    P2,
            u32         Flags,         /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT((m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY) 
        || (m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS) );
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

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_TRIANGLE,
            FALSE,
            MAX( P0.GetY(), MAX( P1.GetY(), P2.GetY() ) ),
            Flags );

        RecordCollision( TempCollision );
    }
}


//==============================================================================

void collision_mgr::ApplyAABBoxToCylinder(
    const   bbox&       AABBox,
            u32         Flags,         /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_CYLINDER );

    //--------------------------------------------------
    // Check collisions between matching spheres in
    // the start cylinder and the end cylinder
    //--------------------------------------------------
    s32         i;
    f32         FinalT;
    vector3     FinalHitPoint;
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
            ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.01f );
            // record the collision

            collision_mgr::collision TempCollision(
                FinalT,
                FinalHitPoint,
                HitPlane,
                SlipPlane,
                m_ContextInfo.Guid,
                PrimitiveKey,
                PRIMITIVE_STATIC_AA_BBOX,
                FALSE,
                AABBox.Max.GetY(),
                Flags );

            RecordCollision( TempCollision );
        }
    }
}

//==============================================================================

void collision_mgr::ApplyAABBoxToSphere  (
    const   bbox&       AABBox,
            u32         Flags,         /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_SPHERE );

    f32         FinalT;
    vector3     FinalHitPoint;
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
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.01f );
        // record the collision

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_AA_BBOX,
            FALSE,
            AABBox.Max.GetY(),
            Flags );

        RecordCollision( TempCollision );
    }
}


//==============================================================================

void collision_mgr::ApplyAABBoxToRay(
    const   bbox&       AABBox,
            u32         Flags,         /* = 0 */
            s32         PrimitiveKey   /* = 0 */ )
{
    ASSERT( m_DynamicPrimitive == PRIMITIVE_DYNAMIC_RAY
        ||  m_DynamicPrimitive == PRIMITIVE_DYNAMIC_LOS
        );

    f32         FinalT;
    vector3     FinalHitPoint;
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
        ASSERT( x_abs(SlipPlane.Normal.LengthSquared() - 1.0f) < 0.01f );
        // record the collision

        collision_mgr::collision TempCollision(
            FinalT,
            FinalHitPoint,
            HitPlane,
            SlipPlane,
            m_ContextInfo.Guid,
            PrimitiveKey,
            PRIMITIVE_STATIC_AA_BBOX,
            FALSE,
            AABBox.Max.GetY(),
            Flags );

        RecordCollision( TempCollision );
    }
}


//==============================================================================

s32 collision_mgr::GetCylinderSpherePositions(
    const vector3&  Bottom,
    const vector3&  Top,
    f32             Radius,
    vector3*        SpherePositions,
    s32             MaxnSpheres )
{
    ASSERT( (Top-Bottom).LengthSquared() > 0.0001f );

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

void collision_mgr::RecordCollision( const collision_mgr::collision& Collision )
{
    //
    // Check data is decent
    //
    ASSERT( x_abs(Collision.Plane.Normal.LengthSquared()-1.0f) < 0.001f ); 
    ASSERT( x_abs(Collision.SlipPlane.Normal.LengthSquared()-1.0f) < 0.001f ); 
    ASSERT( (Collision.T >= 0.0f) && (Collision.T <= 1.0f) );

    //
    // Check if we need to watch for permeable collisions
    //
    if( m_bCollectPermeable )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( Collision.ObjectHitGuid );
        if( pObject && (pObject->GetAttrBits() & object::ATTR_COLLISION_PERMEABLE) )
        {
            s32 i;

            // Look and see if already in the list
            for( i=0; i<m_nPermeables; i++ )
            if( m_Permeable[i] == Collision.ObjectHitGuid )
            {
                // Remember closest collision with permeable
                m_PermeableT[i] = MIN( m_PermeableT[i], Collision.T ); 
                break;
            }

            // Add object to permeable list
            if( (i==m_nPermeables) && (i<MAX_PERMEABLE_OBJECTS) )
            {
                m_Permeable[ m_nPermeables ]  = Collision.ObjectHitGuid;
                m_PermeableT[ m_nPermeables ] = Collision.T;
                m_nPermeables++;
            }

            // Don't list as a normal collision
            return;
        }
    }

    //
    // This will point to the collision slot we will keep
    //
    xbool bFoundDuplicate = FALSE;
    collision_mgr::collision* pDestC = NULL;

    //
    // Decide where the destination collision will be
    //

    // If there is a duplicate guid possibly replace it.
    if( m_bRemoveDuplicateGuids )
    {
        for( s32 i=0; i<m_nCollisions; i++ )
        if( m_Collisions[i].ObjectHitGuid == Collision.ObjectHitGuid )
        {
            bFoundDuplicate = TRUE;

            if( m_Collisions[i].T > Collision.T )
            {
                // Replace with closer collision
                pDestC = &m_Collisions[i];
                break;
            }
        }
    }

    // If we didn't find a duplicate then we need to find a fresh
    // collision entry
    if( !bFoundDuplicate )
    {
        if( m_nCollisions < m_nMaxCollisions )
        {
            ASSERT( m_nCollisions < MAX_COLLISION_MGR_COLLISIONS );
            pDestC = &m_Collisions[m_nCollisions];
            m_nCollisions++;
        }
        else
        {
            // We may need to displace the collision with the largest T
            s32 i;
            f32 MaxT = -1.0f;
            s32 MaxIndex = -1;
        
            for( i = 0; i < m_nCollisions; ++i )
            {
                if ( m_Collisions[i].T > MaxT )
                {
                    MaxT = m_Collisions[i].T;
                    MaxIndex = i;
                }
            }

            // Did we find one with a great T than the new collision?
            if( MaxT > Collision.T )
            {
                pDestC = &m_Collisions[MaxIndex];
            }
        }
    }

    // If we could never find a good DestC then skip the final steps
    if( pDestC != NULL )
    {
        *pDestC = Collision;

        if( m_ContextInfo.Context==1 )
        {
            // Transform the collision back to world space
            pDestC->Point = m_ContextInfo.L2W.Transform( pDestC->Point );
            pDestC->Plane.Transform( m_ContextInfo.L2W );
            pDestC->SlipPlane.Transform( m_ContextInfo.L2W );
        }
    }
}

//==============================================================================

void collision_mgr::CleanPermeables( void )
{
    f32 MaxT = 1.0f;
    if( m_nCollisions > 0 )
        MaxT = m_Collisions[0].T;

    // Only keep permeables with T <= MaxT
    s32 i=0;
    for( s32 j=0; j<m_nPermeables; j++ )
    {
        if( m_PermeableT[j] <= MaxT )
        {
            m_Permeable[i] = m_Permeable[j];
            m_PermeableT[i]= m_PermeableT[j];
            i++;
        }
    }

    m_nPermeables = i;
}

//==============================================================================

void collision_mgr::NotifyPermeables( void )
{
    m_bNotifyingPermeables = TRUE;

    object* pMovingObj = g_ObjMgr.GetObjectByGuid( m_MovingObjGuid );
    if( pMovingObj )
    {
        //if( m_nPermeables ) x_DebugMsg("*********************************\n");

        for( s32 i=0; i<m_nPermeables; i++ )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_Permeable[i] );
            if( pObj )
            {
                //x_DebugMsg("Notify %2d %s\n",i,pObj->m_DebugInfo.m_pDesc->GetTypeName());
                pObj->OnColNotify( *pMovingObj );
            }
        }
    }

    m_bNotifyingPermeables = FALSE;
}

//==============================================================================

inline
s32 FindFirstIntersect( const bbox& BBox, const bbox* pBBox, s32 nBBoxes, s32 iStart )
{
    while( iStart < nBBoxes )
    {
        if( BBox.Intersect( pBBox[iStart] ) ) break;
        iStart++;
    }
    return iStart;
}

//==============================================================================

void collision_mgr::ApplySphereToPolyCache( void )
{
    bbox DynamicBBox = m_DynamicBBoxes[0];
    DynamicBBox.Inflate(1,1,1);

    //
    // Gather factored out list of clusters in dynamic area
    //
    g_PolyCache.BuildClusterList( DynamicBBox, 
                                  m_FilterThisType, 
                                  m_FilterTheseAttributes, 
                                  m_FilterNotTheseAttributes, 
                                  m_IgnoreList, 
                                  m_nIgnoredObjects );

    //
    // Were there no clusters?
    //
    if( g_PolyCache.m_nClusters==0 )
        return;

    vector3 SphereStart = m_SphereInfo[0].Start;
    vector3 SphereEnd   = m_SphereInfo[0].End;
    f32     Radius      = m_SphereInfo[0].Radius;

    //
    // Build culling flags
    //
    u32 CullFlags=0;
    vector3 Dir = SphereEnd - SphereStart;
    Dir.Normalize();
    if( Dir.GetX() > +0.001f ) CullFlags |= BOUNDS_X_POS;
    if( Dir.GetX() < -0.001f ) CullFlags |= BOUNDS_X_NEG;
    if( Dir.GetY() > +0.001f ) CullFlags |= BOUNDS_Y_POS;
    if( Dir.GetY() < -0.001f ) CullFlags |= BOUNDS_Y_NEG;
    if( Dir.GetZ() > +0.001f ) CullFlags |= BOUNDS_Z_POS;
    if( Dir.GetZ() < -0.001f ) CullFlags |= BOUNDS_Z_NEG;

    //
    // Loop through the clusters and process the triangles
    //
    for( s32 iCL=0; iCL<g_PolyCache.m_nClusters; iCL++ )
    {
        poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

        // Setup context
        StartApply( CL.Guid );

        s32 iQ = -1;
        while( 1 )
        {
            // Do tight loop on bbox checks and cull flags
            {
                iQ++;
                while( iQ < CL.nQuads )
                {
                    // Do flag culling
                    if( (CL.pBounds[iQ].Flags & CullFlags) == 0 )
                    {
                        //#### This could be moved outside the loop
                        // and do a pointer incremenet after we
                        // optimized the vector3/bbox stuff
                        bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                        if( DynamicBBox.Intersect( *pBBox ) ) 
                        {
                            break;
                        }
                    }
                    iQ++;
                }
                if( iQ==CL.nQuads )
                    break;
            }

            // Get access to this quad
            poly_cache::cluster::quad& QD = CL.pQuad[iQ];

            // Skip if moving away from quad
            vector3& N = CL.pNormal[ QD.iN ];
            if( N.Dot( Dir ) > 0 )
                continue;

            // Check if starting sphere is behind plane
            if( (N.Dot(SphereStart) + CL.pBounds[iQ].PlaneD) < -Radius )
                continue;

            // Check if ending sphere is in front of plane
            if( (N.Dot(SphereEnd) + CL.pBounds[iQ].PlaneD) > Radius )
                continue;

            // Get the verts and call the actual collision routines
            {
                vector3* P4[4];
                P4[0] = &CL.pPoint[ QD.iP[0] ];
                P4[1] = &CL.pPoint[ QD.iP[1] ];
                P4[2] = &CL.pPoint[ QD.iP[2] ];
                P4[3] = &CL.pPoint[ QD.iP[3] ];

                ApplyTriangleToSphere( *P4[0], *P4[1], *P4[2], 0, CL.PrimKey );

                if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                {
                    ApplyTriangleToSphere( *P4[0], *P4[2], *P4[3], 0, CL.PrimKey );
                }
            }
        }

        EndApply();

        if( (m_nCollisions>0) && (m_bStopOnFirstCollisionFound) )
            return;
    }
}

//==============================================================================

void collision_mgr::ApplyCylinderToPolyCache( void )
{
    bbox DynamicBBox = m_DynamicBBoxes[0];
    DynamicBBox.Inflate(1,1,1);

    //
    // Gather factored out list of clusters in dynamic area
    //
    g_PolyCache.BuildClusterList( DynamicBBox, 
                                  m_FilterThisType, 
                                  m_FilterTheseAttributes, 
                                  m_FilterNotTheseAttributes, 
                                  m_IgnoreList, 
                                  m_nIgnoredObjects );

    //
    // Were there no clusters?
    //
    if( g_PolyCache.m_nClusters==0 )
        return;

    //
    // Build culling flags
    //
    u32 CullFlags=0;
    vector3 Dir = m_CylinderInfo[0].BotEnd - m_CylinderInfo[0].BotStart;
    Dir.Normalize();
    if( Dir.GetX() > +0.001f ) CullFlags |= BOUNDS_X_POS;
    if( Dir.GetX() < -0.001f ) CullFlags |= BOUNDS_X_NEG;
    if( Dir.GetY() > +0.001f ) CullFlags |= BOUNDS_Y_POS;
    if( Dir.GetY() < -0.001f ) CullFlags |= BOUNDS_Y_NEG;
    if( Dir.GetZ() > +0.001f ) CullFlags |= BOUNDS_Z_POS;
    if( Dir.GetZ() < -0.001f ) CullFlags |= BOUNDS_Z_NEG;

    //
    // Loop through the clusters and process the triangles
    //
    for( s32 iCL=0; iCL<g_PolyCache.m_nClusters; iCL++ )
    {
        poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

        // Setup context
        StartApply( CL.Guid );

        s32 iQ = -1;
        while( 1 )
        {
            // Do tight loop on bbox checks and cull flags
            {
                iQ++;
                while( iQ < CL.nQuads )
                {
                    // Do flag culling
                    if( (CL.pBounds[iQ].Flags & CullFlags) == 0 )
                    {
                        // Do bbox culling
                        //#### This could be moved outside the loop
                        // and do a pointer incremenet after we
                        // optimized the vector3/bbox stuff
                        bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                        if( DynamicBBox.Intersect( *pBBox ) ) 
                        {
                            break;
                        }
                    }
                    iQ++;
                }
                if( iQ==CL.nQuads )
                    break;
            }

            // Process this quad
            poly_cache::cluster::quad& QD = CL.pQuad[iQ];

            // Skip if moving away from quad
            if( CL.pNormal[QD.iN].Dot(Dir) > -0.0001f )
                continue;

            {
                vector3* P4[4];
                P4[0] = &CL.pPoint[ QD.iP[0] ];
                P4[1] = &CL.pPoint[ QD.iP[1] ];
                P4[2] = &CL.pPoint[ QD.iP[2] ];
                P4[3] = &CL.pPoint[ QD.iP[3] ];

                ApplyTriangleToStretchedSphere( *P4[0], *P4[1], *P4[2], 0, CL.PrimKey );
                if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                {
                    ApplyTriangleToStretchedSphere( *P4[0], *P4[2], *P4[3], 0, CL.PrimKey );
                }
            }
        }

        EndApply();

        if( (m_nCollisions>0) && (m_bStopOnFirstCollisionFound) )
            return;
    }
}

//==============================================================================
static xbool ClipRay( const bbox&    BBox,
                      const vector3& P0,
                      const vector3& P1,
                      f32&           T0,
                      f32&           T1 )
{
    vector3 Dir = P1 - P0;
    f32 tx_min, tx_max;
    f32 ty_min, ty_max;
    f32 tz_min, tz_max;
    f32 t_min, t_max;

    vector3 MinMinusP0 = BBox.Min - P0;
    vector3 MaxMinusP0 = BBox.Max - P0;

    if( Dir.GetX() >= 0.0f )
    {
        t_min = tx_min = MinMinusP0.GetX() / Dir.GetX();
        t_max = tx_max = MaxMinusP0.GetX() / Dir.GetX();
    }
    else
    {
        t_min = tx_min = MaxMinusP0.GetX() / Dir.GetX();
        t_max = tx_max = MinMinusP0.GetX() / Dir.GetX();
    }

    if( Dir.GetY() >= 0.0f )
    {
        ty_min = MinMinusP0.GetY() / Dir.GetY();
        ty_max = MaxMinusP0.GetY() / Dir.GetY();
    }
    else
    {
        ty_min = MaxMinusP0.GetY() / Dir.GetY();
        ty_max = MinMinusP0.GetY() / Dir.GetY();
    }

    if( t_min > ty_max || ty_min > t_max )
        return FALSE;

    if( t_min < ty_min )
        t_min = ty_min;
    if( t_max > ty_max )
        t_max = ty_max;

    if( Dir.GetZ() >= 0.0f )
    {
        tz_min = MinMinusP0.GetZ() / Dir.GetZ();
        tz_max = MaxMinusP0.GetZ() / Dir.GetZ();
    }
    else
    {
        tz_min = MaxMinusP0.GetZ() / Dir.GetZ();
        tz_max = MinMinusP0.GetZ() / Dir.GetZ();
    }

    if( t_min > tz_max || tz_min > t_max )
        return FALSE;

    if( t_min < tz_min )
        t_min = tz_min;
    if( t_max > tz_max )
        t_max = tz_max;

    if( !((t_min <= 1.0f) && (t_max >= 0.0f)) ) 
        return FALSE;

    // Set the initial entry and exit points of the ray
    T0 = t_min;
    T1 = t_max;

    return TRUE;
}

//==============================================================================
void collision_mgr::ApplyRayToPolyCache( void )
{
    vector3 RayStart = m_RayInfo[0].Start;
    vector3 RayEnd   = m_RayInfo[0].End;
    vector3 Dir      = RayEnd - RayStart;
    f32     RayLen   = Dir.Length();
    if( RayLen < 0.00001f ) return;
    vector3 NDir     = Dir;
            NDir    /= RayLen;
    f32     T1CM     = 1.0f / RayLen;

    //
    // Build culling flags
    //
    u32 CullFlags=0;
    if( NDir.GetX() > +0.001f ) CullFlags |= BOUNDS_X_POS;
    if( NDir.GetX() < -0.001f ) CullFlags |= BOUNDS_X_NEG;
    if( NDir.GetY() > +0.001f ) CullFlags |= BOUNDS_Y_POS;
    if( NDir.GetY() < -0.001f ) CullFlags |= BOUNDS_Y_NEG;
    if( NDir.GetZ() > +0.001f ) CullFlags |= BOUNDS_Z_POS;
    if( NDir.GetZ() < -0.001f ) CullFlags |= BOUNDS_Z_NEG;


    // Begin the polycache ray walk
    g_PolyCache.BeginRayClusterWalk(RayStart,
                                    RayEnd,
                                    m_FilterThisType, 
                                    m_FilterTheseAttributes, 
                                    m_FilterNotTheseAttributes, 
                                    m_IgnoreList, 
                                    m_nIgnoredObjects );
    
    // Walk through polycache clusters
    while( 1 )
    {
        poly_cache::cluster* pCluster = g_PolyCache.GetNextClusterFromRayWalk();
        if( !pCluster )
            break;

        poly_cache::cluster& CL = *pCluster;

        //
        // Try ray to cluster bbox
        //
        f32 T0,T1;
        if( ClipRay(CL.BBox,RayStart,RayEnd,T0,T1) )
        {
            // Back of parametric just slightly
            T0 -= T1CM; if( T0<0 ) T0 = 0;
            T1 += T1CM; if( T1>1 ) T1 = 1;

            // Compute smaller bbox points
            vector3 HitBBoxStart = RayStart + Dir*T0;
            vector3 HitBBoxEnd   = RayStart + Dir*T1;
                bbox    RayBBox( HitBBoxStart, HitBBoxEnd );
            
            // Setup context
            StartApply( CL.Guid );

            s32 iQ = -1;
            while( 1 )
            {
                // Do tight loop on bbox checks and cull flags
                {
                    iQ++;
                    while( iQ < CL.nQuads )
                    {
                        // Do flag culling
                        if( (CL.pBounds[iQ].Flags & CullFlags) == 0 )
                        {
                            // Do bbox culling
                            //#### This could be moved outside the loop
                            // and do a pointer incremenet after we
                            // optimized the vector3/bbox stuff
                                bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                            if( RayBBox.Intersect( *pBBox ) ) 
                            {
                                break;
                            }
                        }
                        iQ++;
                    }
                    if( iQ==CL.nQuads )
                        break;
                }

                // Process this quad
                poly_cache::cluster::quad& QD = CL.pQuad[iQ];

                // Skip if moving away from quad
                vector3& N = CL.pNormal[ QD.iN ];
                f32 Dot = N.Dot( Dir );
                if( (Dot) > -0.0001f )
                    continue;

                {
                    vector3* P4[4];
                    P4[0] = &CL.pPoint[ QD.iP[0] ];
                    P4[1] = &CL.pPoint[ QD.iP[1] ];
                    P4[2] = &CL.pPoint[ QD.iP[2] ];
                    P4[3] = &CL.pPoint[ QD.iP[3] ];

                    ApplyTriangleToRay( *P4[0], *P4[1], *P4[2], 0, CL.PrimKey );
                    if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                    {
                        ApplyTriangleToRay( *P4[0], *P4[2], *P4[3], 0, CL.PrimKey );
                    }

                }
            }

            EndApply();

            if( (m_nCollisions>0) && (m_bStopOnFirstCollisionFound) )
                return;
        }
    }
}

//==============================================================================

#ifdef ENABLE_COLLISION_STATS
void collision_mgr::Render( void )
{
    if( m_DisplayStats )
    {
        collision_mgr::stats& S = g_CollisionMgr.m_Stats;

        f32 TotalTime  = S.Time[0]  + S.Time[1]  + S.Time[2]  + S.Time[3];
        s32 TotalCount = S.Count[0] + S.Count[1] + S.Count[2] + S.Count[3];

        s32 X=0;
        s32 Y=6;
        x_printfxy(X,Y++,"CYL %05.2f %02d %05.2f",S.Time[0],S.Count[0],(S.Count[0])?(S.Time[0]/S.Count[0]):(0));
        x_printfxy(X,Y++,"SPH %05.2f %02d %05.2f",S.Time[1],S.Count[1],(S.Count[1])?(S.Time[1]/S.Count[1]):(0));
        x_printfxy(X,Y++,"RAY %05.2f %02d %05.2f",S.Time[2],S.Count[2],(S.Count[2])?(S.Time[2]/S.Count[2]):(0));
        x_printfxy(X,Y++,"LOS %05.2f %02d %05.2f",S.Time[3],S.Count[3],(S.Count[3])?(S.Time[3]/S.Count[3]):(0));
        x_printfxy(X,Y++,"TOT %05.2f %02d",TotalTime,TotalCount);
    }
}
#endif // ENABLE_COLLISION_STATS

//==============================================================================

void	collision_mgr::AddToIgnoreList( guid* Guid, s32 nGuids )
{

	ASSERT( (m_nIgnoredObjects+nGuids) <= MAX_IGNORED_OBJECTS );

    for( s32 i=0; i<nGuids; i++ )
    {
	    m_IgnoreList[ m_nIgnoredObjects ] = Guid[i];
	    m_nIgnoredObjects++;
    }
}

//==============================================================================

