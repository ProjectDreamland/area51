
#include "CharacterPhysics.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Objects\AnimSurface.hpp"
#include "Objects\Player.hpp"
#include "PainMgr\Pain.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Characters\Character.hpp"
#include "Objects\Door.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Objects\CokeCan.hpp"

#ifndef X_EDITOR
#include "NetworkMgr\NetworkMgr.hpp"
#endif

static const f32 REALLY_SMALL                =  0.00001f;
static const f32 MIN_DELTA_TO_CLIMB_STEEPS   = 10.0f;
static const f32 MIN_DELTA_TO_CLIMB_STEEPS_2 = x_sqr( MIN_DELTA_TO_CLIMB_STEEPS );
static const f32 SLIDE_PLANE_BACKOFF         = 0.01f;


#ifdef mreed
struct breadcrumb
{
    vector3 Pos;
    xcolor  Color;
};
    static const s32    N_BREADCRUMBS       = 100;
    static const xbool  g_UseBreadcrumbs    = FALSE;
    s32                 g_FirstBreadcrumb   = -1;
    s32                 g_LastBreadcrumb    = -1;
    breadcrumb          g_Breadcrumbs[N_BREADCRUMBS];

void AddBreadcrumb( const vector3& Pos, xcolor Color )
{
    if ( g_UseBreadcrumbs )
    {
        g_LastBreadcrumb++;
    
        if ( g_LastBreadcrumb == N_BREADCRUMBS )
        {
            g_LastBreadcrumb = 0;
        }

        if ( g_LastBreadcrumb == g_FirstBreadcrumb )
        {
            g_FirstBreadcrumb++;

            if ( g_FirstBreadcrumb == N_BREADCRUMBS )
            {
                g_FirstBreadcrumb = 0;
            }
        }

        if ( g_FirstBreadcrumb < 0 )
        {
            // First time only
            g_FirstBreadcrumb = 0;
        }

        g_Breadcrumbs[g_LastBreadcrumb].Pos     = Pos;
        g_Breadcrumbs[g_LastBreadcrumb].Color   = Color;
    }
}
#endif

xbool  g_UpdateRidingPlatform = TRUE;
xbool  g_ApplyPlatformCollision = TRUE;


// TODO: Debug Code
#if 0
//#ifdef cgalley
#define LOG_CHARACTER_PHYSICS
xbool  g_LogCharacterPhysics = FALSE;
#endif

//=========================================================================
// FUNCTIONS
//=========================================================================

#ifdef X_ASSERT

xbool IsValidPosition( const vector3& P )
{
    // Get world bbox
    const bbox& BBox = g_ObjMgr.GetSafeBBox();

    // Check min
    if( P.GetX() < BBox.Min.GetX() )
        return FALSE;
    if( P.GetY() < BBox.Min.GetY() )
        return FALSE;
    if( P.GetZ() < BBox.Min.GetZ() )
        return FALSE;

    // Check max
    if( P.GetX() > BBox.Max.GetX() )
        return FALSE;
    if( P.GetY() > BBox.Max.GetY() )
        return FALSE;
    if( P.GetZ() > BBox.Max.GetZ() )
        return FALSE;

    return TRUE;        
}

#endif

//=========================================================================

character_physics::character_physics( void )
{
    m_NavCollisionHeight        =   180.0f;
    m_NavCollisionCurentHeight  =  m_NavCollisionHeight;
    m_NavCollisionRadius        =    30.0f;
    m_NavCollisionCrouchOffset  =    40.0f;
    m_AirControl                =     0.04f; 
    m_FlingAC                   =     0.0f;
    m_MaxCollisions             =     8;
    m_bHandlePermeable          =     FALSE;
    m_bNavCollided              =     FALSE;
    m_bFallMode                 =     FALSE;
    m_bJumpMode                 =     FALSE;
    m_bFlingMode                =     FALSE;
    m_bTrackingGround           =     TRUE;
    m_bUseGravity               =     TRUE;
    m_bLocoGravityOn            =     TRUE;
    m_bLocoCollisionOn          =     TRUE;
    m_CollisionSnuggleDistance  =     1.0f;
    m_GroundTolerance           =     5.0f;
    m_FallTolerance             =    50.0f;
    m_VelocityForFallMode       =   980.0f;
    m_GravityAcceleration       = -1000.0f;
    m_MaxDistanceToGround       =    20.0f;
    m_SteepestSlide             =     0.5f;
    m_GroundPlane.Setup(1,0,0,0);
    m_GroundGuid                =        0;
    m_GroundPos.Zero();
    m_GroundBoneIndex           =        0;
    m_MovingPlatformGuid        =        0;
    m_MovingPlatformBone        =       -1;
    m_DeltaPosIndex             =        0;
    m_ActorCollisionRadius      =       50.0f;
    m_SolveActorCollisions      =    FALSE;
    m_IgnoreGuid                =        0;
    m_LastFling                 =        0;
    m_OldMovingPlatformVelocity.Zero();
    m_OldMovingPlatformL2W.Identity();

    m_nPlatformsToIgnore = 0;

    m_Velocity.Set(0,0,0);
    m_Position.Set(0,0,0);

    m_LastMove.Set( 0.0f, 0.0f, 0.0f );
    m_LastSteepSurface = NULL_GUID;
    
    ResetDeltaPos() ;

    x_memset(m_TriggerGuid,0,sizeof(m_TriggerGuid));
    m_nTriggerGuids = 0;
}

//=========================================================================

void character_physics::Init( guid Guid )
{
    m_Guid = Guid;
}

//=========================================================================
static const f32 k_KeepAwayDistance = 150.0f;

void character_physics::SolveActorAndPlatformCollisions( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::SolveActorAndPlatformCollisions", "" );
#endif

    if (m_bLocoCollisionOn == FALSE)
    {
        return ;
    }

    //
    // Build bbox around character
    //
    bbox WorldBBox;
    WorldBBox.Max.GetZ() = WorldBBox.Max.GetX() = m_ActorCollisionRadius;
    WorldBBox.Min.GetZ() = WorldBBox.Min.GetX() = -WorldBBox.Max.GetX();
    WorldBBox.Max.GetY() = GetColHeight();
    WorldBBox.Min.GetY() = 0;
    WorldBBox.Translate(m_Position);

    if ( m_SolveActorCollisions )
    {
        //
        // Loop through list and process characters
        //
        SolveActorCollisions( WorldBBox );
    }

    //
    // Loop through list and process animated surfaces
    //
    SolvePlatformCollisions( );
}

//=========================================================================

void character_physics::SolveActorCollisions ( const bbox& ActorBBox )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::SolveActorCollisions", "" );
#endif

    object* pOurCharacter = g_ObjMgr.GetObjectByGuid( m_Guid );
    if( !pOurCharacter ) return;
    ASSERT( pOurCharacter->IsKindOf(actor::GetRTTI()) );

    // Build actor's bbox
    s32 nActorsChecked=0;
    vector3 FinalMoveDelta(0.0f, 0.0f, 0.0f);

    actor* pActor = actor::m_pFirstActive;
    while( pActor )
    {
        if( (pActor != pOurCharacter) && 
            (pActor->GetGuid() != m_IgnoreGuid) &&
            (!pActor->IsDead()) )  // We don't want to collide with dead players!
        {
            f32 ActorCollisonRadius = pActor->GetActorCollisionRadius();

            // calc cylindrical distance
            vector3 CToUs = m_Position - pActor->GetPosition();
            CToUs.GetY() = 0;
            f32 DistBetweenSq = CToUs.LengthSquared();
            f32 RadiusSum = m_ActorCollisionRadius + ActorCollisonRadius;

            if( DistBetweenSq < RadiusSum*RadiusSum )
            {
                // Do vertical overlap test
                bbox BBox = pActor->GetBBox();
                if( (BBox.Max.GetY() >= ActorBBox.Min.GetY()) && (ActorBBox.Max.GetY() >= BBox.Min.GetY()) )
                {
                    f32 PenetrationDist = RadiusSum - x_sqrt(DistBetweenSq);
                    ASSERT( PenetrationDist >= 0 );

                    vector3 MoveDelta = CToUs;
                    MoveDelta.NormalizeAndScale(PenetrationDist);

                    // adding a player check so that NPCs can push the player but the player can't push NPCs. 
                    // Otherwise the player can squeeze between things and push NPCs away from consoles and the like 
                    // creating nasty bugs.
                    if( !pActor->IsPlayer() || pOurCharacter->IsKindOf( player::GetRTTI() ) )
                    {
                        FinalMoveDelta += MoveDelta;
                    }

                    nActorsChecked++;

                    ((actor*)pOurCharacter)->SetCollidedActor(pActor->GetGuid());
                }
            }
        }

        // move to next actor
        pActor = pActor->m_pNextActive;
    }

    if( nActorsChecked )
        Push( FinalMoveDelta );
}

//=========================================================================

void character_physics::AdvanceWithoutCollision( const vector3& MoveTo, f32 DeltaTime, xbool bIsDead )
{
    (void)bIsDead;
    vector3 Delta    = MoveTo - m_Position;

    m_Velocity = Delta/DeltaTime;
    m_LastMove = MoveTo - m_Position;
    m_Position = MoveTo;

    m_bFallMode  = FALSE;
    m_bFlingMode = FALSE;

    m_GroundGuid = 0;
    m_GroundPlane.Setup(m_Position,vector3(0,1,0));
    m_GroundPos = m_Position;
    m_GroundBoneIndex = 0;
}

//=========================================================================

void character_physics::Advance( const vector3& MoveTo, f32 DeltaTime, xbool bIsDead )
{
    static s32 FallCounter = 0;
    vector3 PositionOnEntry = m_Position;
    vector3 VelocityOnEntry = m_Velocity;

#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::Advance", "" );
    xstring s;
    if( m_bFallMode ) s += "Fall ";
    if( m_bFlingMode ) s += "Fling ";
    if( m_bTrackingGround ) s += "Tracking ";
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::Advance", s );
#endif

    vector3 OldPos   = m_Position;
    vector3 NewPos   = m_Position;
    vector3 Delta    = MoveTo - m_Position;
    vector3 Momentum = m_Velocity;

    m_Velocity = Delta/DeltaTime;

    if (m_bLocoGravityOn == FALSE)
    {
        m_bFallMode         = FALSE;
        m_bFlingMode        = FALSE;
        m_bTrackingGround   = FALSE;
    }

    //
    // Update information about ground!
    //
    f32 GroundCheckDist = 50.0f;//-fMin( DeltaTime * m_Velocity.Y - m_GroundTolerance*2.0f, -m_GroundTolerance*2.0f);
//    UpdateGround( GroundCheckDist );

    //
    // Update physics and apply gravity etc.
    //
    UpdatePhysics( DeltaTime );

    if( m_bFallMode == TRUE )
    {
        f32 AirControl = m_AirControl;

        if( g_MPTweaks.Active )
        {
            if( m_bFlingMode )   AirControl = m_FlingAC;
            else                 AirControl = g_MPTweaks.AirControl;
        }

        m_Velocity.GetX()  = Momentum.GetX() + AirControl * ( m_Velocity.GetX() - Momentum.GetX() );
        m_Velocity.GetZ()  = Momentum.GetZ() + AirControl * ( m_Velocity.GetZ() - Momentum.GetZ() );
        m_Velocity.GetY() += Momentum.GetY();
    }

    HandleMove( NewPos, 
                m_Velocity,
                DeltaTime, 
                0.0f,
                m_SteepestSlide );

    // Resolve any penetrations
    m_Position = NewPos;
    ResolvePenetrations();
    NewPos = m_Position;

    // Track ground following the move
    if( UpdateGround( GroundCheckDist ) )
    {
        if( m_bTrackingGround )
        {
            if( NewPos.GetY() > m_GroundPos.GetY() )
                NewPos.GetY() = m_GroundPos.GetY();
        }
    }
    else
    {
        m_bTrackingGround = FALSE;
    }

    m_LastMove = NewPos - OldPos;
    m_Position = NewPos;

// TODO - Revisit this logic.
// The player being dead in an online game is the only case we want to keep from doing this
// logic.  If this is not here, other players will push the ghost around.
#ifndef X_EDITOR
    if( !g_NetworkMgr.IsOnline() || !bIsDead )
#endif
        SolveActorAndPlatformCollisions();  


    // TODO: E3: CJ: Hack to get the player out of stuck situations
    if( m_bFallMode && ( VelocityOnEntry.GetY() < -50.0f ) )
    {
        if( x_abs( PositionOnEntry.GetY() - m_Position.GetY() ) < 0.1f )
        {
            FallCounter++;
            if( FallCounter > 2 )
            {
                LOG_WARNING( "character_physics::Advance", "Stuck!" );
                m_Velocity.GetY() = 0.0f;
                m_bFallMode  = FALSE;
                m_bFlingMode = FALSE;
            }
        }
        else
        {
            FallCounter = 0;
        }
    }
    else
    {
        FallCounter = 0;
    }
}

//=========================================================================

#define HACK_MAX_PERMEABLE_CP (MAX_COLLISION_MGR_COLLISIONS)
void character_physics::CollectPermeable( object* pActor, const vector3& StartPos, const vector3& EndPos )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::CollectPermeable", "" );
#endif

    ASSERT( pActor );
    ASSERT( pActor->IsKindOf(actor::GetRTTI()));

    if( pActor->IsKindOf(character::GetRTTI()) )
    {
        // We are assuming this actor is a character!

        s32 i,j;

        guid NewGuid[CHARACTER_PHYSICS_MAX_TRIGGERS]={0};
        s32 nNewGuids=0;

        // Do select bbox around actor using an inflated bbox (necessary for doors)
        bbox BBox = GetBBox();
        BBox.Translate( EndPos );

        bbox InflatedBBox = BBox;
        InflatedBBox.Inflate( 350.0f, 0.0f, 350.0f );

        // Handle special-case permeable object interaction
        g_ObjMgr.SelectBBox( object::ATTR_COLLIDABLE|object::ATTR_COLLISION_PERMEABLE, InflatedBBox, object::TYPE_ALL_TYPES );
        s32 ObjectSlot;
        for( ObjectSlot = g_ObjMgr.StartLoop(); 
             ObjectSlot != SLOT_NULL; 
             ObjectSlot = g_ObjMgr.GetNextResult( ObjectSlot ) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( ObjectSlot );
            ASSERT(pObject);

            switch( pObject->GetType() )
            {
            case object::TYPE_TRIGGER_EX:
                // triggers just get added to a guid list if they have intersected with
                // our non-inflated bbox...we'll deal with the actual collision in a moment
                if( BBox.Intersect( pObject->GetBBox() ) )
                {
                    if( nNewGuids < CHARACTER_PHYSICS_MAX_TRIGGERS )
                        NewGuid[nNewGuids++] = pObject->GetGuid();
                }
                break;
            case object::TYPE_DOOR:
                {
                    // wake up the door so it knows to start looking for us
                    door& DoorObj = door::GetSafeType( *pObject );
                    DoorObj.WakeUp();
                }
                break;
            case object::TYPE_DAMAGE_FIELD:
                // handle the damage field collision
                pObject->OnColNotify( *pActor );
                break;
            case object::TYPE_COKE_CAN:
                {
                    actor& Actor = actor::GetSafeType( *pActor );
                    coke_can& Can = coke_can::GetSafeType( *pObject );
                    Can.ApplyActorConstraints( Actor );
                }
                break;
            default:
                // nothing to do
                break;
            }
        }
        g_ObjMgr.EndLoop();

        // Look for any NEW triggers
        for( i=0; i<nNewGuids; i++ )
        {
            for( j=0; j<m_nTriggerGuids; j++ )
            if( m_TriggerGuid[j] == NewGuid[i] )
                break;

            // if we did not find the new guid in the triggerguid list
            // then we have just entered it and should notify the object
            if( j==m_nTriggerGuids )
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( NewGuid[i] );
                if( pObject )
                    pObject->OnColNotify( *pActor );
            }
        }

        // Copy new triggers into old triggers
        for( i=0; i<nNewGuids; i++ )
            m_TriggerGuid[i] = NewGuid[i];
        m_nTriggerGuids = nNewGuids;
    }
    else
    {
        // We are assuming this actor is a player!

        //
        // Walk from where we were to where we are.
        //
        vector3 Start = StartPos; 
        vector3 Stop  = EndPos;

        SetupPlayerCollisionCheck( Start, Stop );
        g_CollisionMgr.SetMaxCollisions( HACK_MAX_PERMEABLE_CP);
    

        //
        // Check the pickups here.
        //
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLISION_PERMEABLE) ;

        for( s32 i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( g_CollisionMgr.m_Collisions[i].ObjectHitGuid );
            pObj->OnColNotify( *pActor );
        }

        // We need the player to check for can objects.
        // To speed it up a minute amount, let's check to see if there even are
        // cans present before asking for a bbox check
        if (g_ObjMgr.GetFirst( object::TYPE_COKE_CAN ) != SLOT_NULL)
        {
            actor& Player = actor::GetSafeType( *pActor );
            bbox BBox = GetBBox();
            BBox.Translate( EndPos );
            bbox InflatedBBox = BBox;
            InflatedBBox.Inflate( 100.0f, 0.0f, 100.0f );

            g_ObjMgr.SelectBBox( object::ATTR_COLLIDABLE, InflatedBBox, object::TYPE_COKE_CAN );
            s32 ObjectSlot;
            for( ObjectSlot = g_ObjMgr.StartLoop(); 
                ObjectSlot != SLOT_NULL; 
                ObjectSlot = g_ObjMgr.GetNextResult( ObjectSlot ) )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( ObjectSlot );
                ASSERT(pObject);
                coke_can& Can = coke_can::GetSafeType( *pObject );
                Can.ApplyActorConstraints( Player );
            }
            g_ObjMgr.EndLoop();
        }
    }


}
#ifdef X_ASSERT
struct handle_move_data
{
    vector3 Start;
    vector3 Stop;
    f32     MoveLen;
    f32     CollT;
    vector3 OldDelta;
    xbool   MovingBetweenAcutePlanes;
    vector3 SlideDeltaAfterSlip;
    xbool   TooSteep;
    vector3 Flat;
    xbool   SlidingAlong;
    vector3 Into1;
    vector3 Into2;
    f32     Dot;
    xbool   MoveAlongPlane;
    vector3 SlideDeltaAfterMoveAlongPlane;
    vector3 PerpAfterMoveAlongPlane;
    vector3 SlideDeltaFinal;
    vector3 DeltaFinal;
} ;

#define MOVE_DATA_HISTORY 32

handle_move_data MoveData[MOVE_DATA_HISTORY];
s32 nMoveData = 0;
#endif

//=========================================================================

void character_physics::HandleMove( 
    vector3&        NewPos, 
    vector3&        OriginalVelocity,
    f32             DeltaTime,
    f32             SlideFriction,
    f32             SteepestSlide )
{
#if defined(X_ASSERT) && !defined(X_EDITOR)

    // Store info    
    vector3 InNewPos = NewPos;
    vector3 InVel    = OriginalVelocity;
    
    // Rewind to here
    NewPos           = InNewPos;
    OriginalVelocity = InVel;

    ASSERT( IsValidPosition( NewPos ) );

#endif


#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::HandleMove", "" );
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::HandleMove", "NewPos   = %.3f, %.3f, %.3f", NewPos.GetX(), NewPos.GetY(), NewPos.GetZ() );
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::HandleMove", "Velocity = %.3f, %.3f, %.3f", OriginalVelocity.GetX(), OriginalVelocity.GetY(), OriginalVelocity.GetZ() );
#endif

    vector3 OldPos   = NewPos;
    vector3 Velocity = OriginalVelocity * DeltaTime;

#ifdef X_ASSERT
    nMoveData = 0; // Start at 0
#endif

    (void)SteepestSlide;
    ASSERT(SlideFriction >= 0.0f);
    ASSERT(SlideFriction <= 1.0f);
//  ASSERT( Velocity.LengthSquared() < (300.0f * 300.0f) ); // We're trying to move too far in a single frame
                                                            // See mreed...

    vector3     StartPosForPermeables = NewPos;
    vector3     Delta       = Velocity;
    s32         MaxLoops    = 3;
    object*     pObject     = NULL;
    plane       LastPlane;
    m_bNavCollided          = FALSE;

    // Skip if collision turned off
    if (m_bLocoCollisionOn == FALSE)
    {
        NewPos += Velocity ;
        return ;
    }

    // Quick check to see if we should even bother
    if( Delta.LengthSquared() < REALLY_SMALL )
        return;

    pObject = g_ObjMgr.GetObjectByGuid( m_Guid );

    while( 1 )
    {
#ifdef X_ASSERT
        handle_move_data& CurMoveData = MoveData[nMoveData];
        nMoveData = (nMoveData + 1) % MOVE_DATA_HISTORY;
#endif
        //
        // Check if we have iterated too many times
        //
        if( (MaxLoops--) == 0 )
        {
            Delta.Zero();
            break;
        }
        
        //
        // Check if the current delta is too small to bother with
        //
        if( Delta.LengthSquared() < REALLY_SMALL )
        {
            Delta.Zero();
            break;
        }

        //
        // Decide on Start and Stop positions
        //
        vector3 Start = NewPos; 
        vector3 Stop  = NewPos + Delta;
        f32     MoveLen = Delta.Length();

#ifdef X_ASSERT
        CurMoveData.Start = Start;
        CurMoveData.Stop = Stop;
        CurMoveData.MoveLen = MoveLen;
#endif

        //
        // Fire up cylinder collision 
        //
        SetupPlayerCollisionCheck( Start, Stop );

        g_CollisionMgr.SetMaxCollisions( m_MaxCollisions );        
        g_CollisionMgr.AddToIgnoreList( m_PlatformCollisionIgnoreList, m_nPlatformsToIgnore );
        if( m_IgnoreGuid )
        {        
            g_CollisionMgr.AddToIgnoreList( &m_IgnoreGuid, 1 );
        }

        //
        // Collect all possible collisions
        //
        //g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );
        object *ourObject = g_ObjMgr.GetObjectByGuid(m_Guid);
        if( ourObject && ourObject->IsKindOf(player::GetRTTI()) )
        {
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_BLOCKS_PLAYER,
                object::ATTR_COLLISION_PERMEABLE|
                object::ATTR_LIVING );
        }
        else 
        {
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_BLOCKS_CHARACTER,
                object::ATTR_COLLISION_PERMEABLE|
                object::ATTR_LIVING );
        }
        //
        // Process any permeables we might have hit
        //
        //g_CollisionMgr.NotifyPermeables();

        //
        // If we didn't hit anything then move and return
        //
        if( g_CollisionMgr.m_nCollisions == 0 )
        {
            // The move and return occurs following this loop
            break;
        }

        //
        // We have a collision, deal with it
        //
        m_bNavCollided = TRUE;

        const collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];

        #ifdef athyssen
        if( m_nPlatformsToIgnore )
        {
            x_DebugMsg("collision!!! %08X:%08X\n",((u32)(Coll.ObjectHitGuid>>32)),(u32)Coll.ObjectHitGuid);
        }
        #endif


        //
        // let's notify the object that we just hit
        //
        if( pObject )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( Coll.ObjectHitGuid );
            if( pObj )
                pObj->OnColNotify( *pObject );
        }

        //
        // We must have hit something so compute the collision T
        // but pull back a snuggle distance for numerical safety.
        // Keep from backing up into something else
        //
        f32     SnuggleT        = m_CollisionSnuggleDistance / MoveLen;
        f32     CollT           = Coll.T - SnuggleT;
        ASSERT( (CollT<=1.0f) );
        if( CollT < 0 ) CollT = 0;
        if( CollT > 1 ) CollT = 1;

        //
        // Update NewPos with part of Delta
        //
        NewPos += Delta * CollT;
#ifdef X_ASSERT
        CurMoveData.CollT = CollT;
#endif

        //
        // Compute the slide delta and normal
        //
        f32 RemainingT = 1-CollT;
        ASSERT( RemainingT >= 0 );
        ASSERT( RemainingT <= 1 );
        Delta *= RemainingT;
        vector3 SlideDelta;
        vector3 SlidePlaneNormal = Coll.Plane.Normal;
        vector3 Perp;
        vector3 OldDelta = Delta;

#ifdef X_ASSERT
        CurMoveData.OldDelta = OldDelta;
#endif

        //
        // Is this our 2nd collision on this move between acute planes?
        //
        if ( (MaxLoops == 1)
            && (LastPlane.Normal.Dot( Coll.SlipPlane.Normal )
                <= 0) )
        {
#ifdef X_ASSERT
            CurMoveData.MovingBetweenAcutePlanes = TRUE;
#endif
            // we should move parallel to the crease between the planes
            SlideDelta = LastPlane.Normal.Cross( Coll.SlipPlane.Normal );
            SlideDelta.Normalize();
            const f32 Dist = Delta.Dot( SlideDelta );
            SlideDelta *= Dist;

            // Use average of normals for slide plane normal
            SlidePlaneNormal = LastPlane.Normal + Coll.SlipPlane.Normal;
            SlidePlaneNormal.Normalize();

            if( !( SlideDelta.LengthSquared() <= Delta.LengthSquared() + 0.01f ) )
            {
                SlideDelta = Delta;
                SlidePlaneNormal = Coll.Plane.Normal;
            }

            if( !( SlideDelta.LengthSquared() <= OldDelta.LengthSquared() + 0.01f ) )
            {
                SlideDelta       = OldDelta;
                SlidePlaneNormal = Coll.Plane.Normal;
            }

            #ifdef X_ASSERT
            {
                f32 SDLen = SlideDelta.LengthSquared();
                f32 ODLen = OldDelta.LengthSquared();
                ASSERT( SDLen <= (ODLen + 1.0f) );
            }
            #endif
        }
        else
        {
#ifdef X_ASSERT
            CurMoveData.MovingBetweenAcutePlanes = FALSE;
#endif
            LastPlane = Coll.SlipPlane;
            Coll.SlipPlane.GetComponents( Delta, SlideDelta, Perp );

            #ifdef X_ASSERT
            {
                f32 SDLen = SlideDelta.LengthSquared();
                f32 ODLen = OldDelta.LengthSquared();
                f32 DLen  = Delta.LengthSquared();
                ASSERT( SDLen <= ( DLen + 1.0f) );
                ASSERT( SDLen <= (ODLen + 1.0f) );
            }
            #endif
        }
        
#ifdef X_ASSERT
        CurMoveData.SlideDeltaAfterSlip = SlideDelta;
#endif

        //
        // See if this is too steep for us.
        //
        if ( (Coll.SlipPlane.Normal.GetY() >= 0.0f) &&
             (SlideDelta           .GetY() >  0.0f) && 
             (Coll.SlipPlane.Normal.GetY() < SteepestSlide) )
        {
#ifdef X_ASSERT
            CurMoveData.TooSteep = TRUE;
#endif
            // Vertical version of our plane
            plane Plane( Coll.SlipPlane );
            Plane.Normal.GetY() = MIN( 0.0f, Plane.Normal.GetY() );
            Plane.Normal.Normalize();

            // Flattened Delta
            vector3 Flat( SlideDelta );
            Flat.GetY() = MAX( 0.0f, SlideDelta.GetY());

#ifdef X_ASSERT
            CurMoveData.Flat = Flat;
#endif

            // If we're not moving significantly into the plane, then we
            // just slide horizontally along it.
            if ( Plane.Normal.Dot( Delta ) < 0.0f )
            {
#ifdef X_ASSERT
                CurMoveData.SlidingAlong = TRUE;
#endif
                // We're moving into it, see if our last move was into it
                // slow enough to prevent coasting up.
                vector3 Into( m_LastMove );
                Into.GetY() = 0.0f;
#ifdef X_ASSERT
                CurMoveData.Into1 = Into;
#endif

                f32 Dot = Plane.Normal.Dot( Into );
                Into = Plane.Normal * Dot;

#ifdef X_ASSERT
                CurMoveData.Into2 = Into;
                CurMoveData.Dot = Dot;
                
#endif
                if (   ((Dot < 0) && (Into.LengthSquared() < x_sqr( MIN_DELTA_TO_CLIMB_STEEPS )) 
                    || (Coll.ObjectHitGuid == m_LastSteepSurface)) )
                {
                    // Ok, let's move along the plane
                    vector3 Perp;// dummy
                    Plane.GetComponents( Flat, SlideDelta, Perp );
#ifdef X_ASSERT
                    CurMoveData.MoveAlongPlane = TRUE;
                    CurMoveData.SlideDeltaAfterMoveAlongPlane = SlideDelta;
                    CurMoveData.PerpAfterMoveAlongPlane = Perp;
#endif
                }

                #ifdef X_ASSERT
                {
                    f32 SDLen = SlideDelta.LengthSquared();
                    f32 ODLen = OldDelta.LengthSquared();
                    ASSERT( SDLen <= (ODLen + 1.0f) );
                }
                #endif
            }
#ifdef X_ASSERT
            else
            {
                CurMoveData.SlidingAlong = FALSE;
            }
#endif
        }
#ifdef X_ASSERT
        else
        {
            CurMoveData.TooSteep = FALSE;
        }
#endif

        //
        // Use the slide delta
        //
        Delta = SlideDelta*(1-SlideFriction);
#ifdef X_ASSERT
        CurMoveData.SlideDeltaFinal = SlideDelta;
        CurMoveData.DeltaFinal = Delta;
#endif

        #ifdef X_ASSERT
        {
            f32 SDLen = SlideDelta.LengthSquared();
            f32 ODLen = OldDelta.LengthSquared();
            ASSERT( SDLen <= (ODLen + 1.0f) );
        }
        #endif

        // SB: 3/5/05
        // Make next delta be slightly away from the collision plane to 
        // avoid further collisions with the same plane (due to float precision)
        Delta += SlidePlaneNormal * SLIDE_PLANE_BACKOFF;

        //
        // Repeat process
        //
    }

    ASSERT( IsValidPosition( NewPos ) );

    //
    // Move whatever delta is left after the collisions
    //
    if( Delta.LengthSquared() > REALLY_SMALL )
    {
        NewPos += Delta;
        ASSERT( IsValidPosition( NewPos ) );
    }
    OriginalVelocity = (NewPos - OldPos) * (1.0f / DeltaTime);

    //
    // Collect all the pickups and such
    //
    if( m_bHandlePermeable && pObject )
        CollectPermeable( pObject, StartPosForPermeables, NewPos );

    ASSERT( IsValidPosition( NewPos ) );
}

//=========================================================================

xbool character_physics::UpdateGround( f32 BelowDist )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdateGround", "" );
#endif

    xbool   FoundGround = FALSE;
    vector3 FromPos     = m_Position;
    vector3 ToPos       = m_Position - vector3(0,BelowDist,0);

    FromPos.GetY() += 2.0f;

    SetupPlayerCollisionCheck( FromPos, ToPos );

    object *ourObject = g_ObjMgr.GetObjectByGuid(m_Guid);
    if( ourObject && ourObject->IsKindOf(player::GetRTTI()) )
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
            object::ATTR_BLOCKS_PLAYER,
            object::ATTR_COLLISION_PERMEABLE|
            object::ATTR_LIVING );
    }
    else 
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
            object::ATTR_BLOCKS_CHARACTER,
            object::ATTR_COLLISION_PERMEABLE|
            object::ATTR_LIVING );
    }

    if( g_CollisionMgr.m_nCollisions > 0 )
    {
        m_GroundGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
        m_GroundPos  = g_CollisionMgr.m_Collisions[0].Point;
        m_GroundPos.GetY() += 1.0f;
        m_GroundPlane = g_CollisionMgr.m_Collisions[0].SlipPlane;
        m_GroundBoneIndex = GetBoneIndexFromPolyCachePrimKey( g_CollisionMgr.m_Collisions[0].PrimitiveKey ); 
        FoundGround = TRUE;

#ifdef LOG_CHARACTER_PHYSICS
        CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdateGround", "Ground - From (%.3f,%.3f,%.3f) To (%.3f,%.3f,%.3f) Guid (%08X:%08X) Pos (%.3f,%.3f,%.3f) Bone (%d)",
                      FromPos.GetX(), FromPos.GetY(), FromPos.GetZ(),
                      ToPos.GetX(), ToPos.GetY(), ToPos.GetZ(),
                      m_GroundGuid.GetHigh(), m_GroundGuid.GetLow(),
                      m_GroundPos.GetX(), m_GroundPos.GetY(), m_GroundPos.GetZ(),
                      m_GroundBoneIndex );
#endif
    }
    else
    {
#ifdef LOG_CHARACTER_PHYSICS
        CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdateGround", "No Ground - From (%.3f,%.3f,%.3f) To (%.3f,%.3f,%.3f)",
                      FromPos.GetX(), FromPos.GetY(), FromPos.GetZ(),
                      ToPos.GetX(), ToPos.GetY(), ToPos.GetZ() );
#endif

        m_GroundGuid = 0;
        m_GroundPlane.Setup(ToPos,vector3(0,1,0));
        m_GroundPos = ToPos;
        // TODO: Maybe force ground position lower down here
        m_GroundBoneIndex = 0;
    }

    return FoundGround;
}

//=========================================================================

void character_physics::UpdatePhysics( f32 DeltaTime )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdatePhysics", "" );
#endif

    if( m_bJumpMode )
    {
        m_bJumpMode = FALSE;
        m_bFallMode = TRUE;
#ifdef LOG_CHARACTER_PHYSICS
        CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdatePhysics", "Was Jump, set to Fall" );
#endif
        return;
    }

    //FindGround( GroundVector, m_Position, DeltaTime );
    //plane       GroundPlane( g_CollisionMgr.m_Collisions[0].Plane );

    if( m_bUseGravity && m_bLocoGravityOn )
    {
        //  if this object should use gravity and the ground is greater than
        //  ground tolerance away from us then we modify the downward velocity
        //  Or, if the ground is too steep...
        f32 Tolerance = m_GroundTolerance;
        if( m_bTrackingGround )
        {
            f32 dx = m_Velocity.GetX();
            f32 dz = m_Velocity.GetZ();
            f32 d2 = dx*dx + dz*dz;
            if( d2 > 0.01f )
            {
                Tolerance = x_sqrt( d2 ) * 1.5f * DeltaTime;
            }
            Tolerance = MAX( m_GroundTolerance, Tolerance );
        }

        if( (m_Position.GetY() - m_GroundPos.GetY()) > Tolerance )
        {   
#ifdef LOG_CHARACTER_PHYSICS
            CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdatePhysics", "Ground too far or steep - Dist (%.3f) Normal (%.3f), set to Fall, Normal",
                          m_Position.GetY() - m_GroundPos.GetY(),
                          m_GroundPlane.Normal.GetY() );
#endif
            m_bFallMode = TRUE;                   
            m_bTrackingGround = FALSE;

            //  Simple physics.
            if( g_MPTweaks.Active )
            {
                m_Velocity.GetY() += DeltaTime * m_GravityAcceleration * g_MPTweaks.Gravity;
            }
            else
            {
                m_Velocity.GetY() += DeltaTime * m_GravityAcceleration;
            }
        }
        // On a really steep surface?
        else if( m_GroundPlane.Normal.GetY() < m_SteepestSlide )
        {
            m_bFallMode = TRUE;
            m_bTrackingGround = FALSE;

            //  Simple physics, 
            m_Velocity.GetY() += DeltaTime * m_GravityAcceleration;

            // too steep, record the surface
            m_LastSteepSurface = m_GroundGuid;
        }
        // Ok, we're on the ground, make sure we don't think we're falling
        else
        {
            if ( m_bFallMode ) // mreed: this if () makes it possible to set a breakpoint on landing
            {
#ifdef LOG_CHARACTER_PHYSICS
                CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::UpdatePhysics", "Landed" );
#endif
                m_bFallMode         = FALSE;
                m_bFlingMode        = FALSE;
                m_bTrackingGround   = TRUE;
                m_Position          = m_GroundPos;
            }
        }
    }
    else
    {
        m_bFallMode  = FALSE;
        m_bFlingMode = FALSE;
    }

    // Here we modify the velocity vector to make it follow downslopes <= 50 degrees
    if ( !m_bFallMode && m_bUseGravity && m_bLocoGravityOn )
    {
        vector3 Velocity = m_Velocity;
        Velocity.Normalize();
        f32 Dot = m_GroundPlane.Dot( Velocity );
        if(    (Dot >= 0)       // Angle < 90 degrees
            && (Dot <  0.68f) ) // Angle > 48 degrees
        {
            // stick to the ground
            f32 Speed = m_Velocity.Length();
            vector3 Perpendicular; // dummy placeholder
            m_GroundPlane.GetComponents( m_Velocity, m_Velocity, Perpendicular );
            m_Velocity.NormalizeAndScale( Speed );
        }
    }
}

//=========================================================================

xbool character_physics::SetCrouchParametric( f32 NormalizePercent )
{
    // TO DO: Traub - can you put the appropriate test here please - thanks!
    
    // Playing a multi-player/split screen game?
    if( 0 )
    {
        //-----------------------------------------------------------------
        // Multi-player/split screen crouch
        //-----------------------------------------------------------------
    
        // Must be valid range
        ASSERT( NormalizePercent >= 0 );
        ASSERT( NormalizePercent <= 1 );

        // Compute new height
        f32 NewHeight = m_NavCollisionHeight - ( NormalizePercent * m_NavCollisionCrouchOffset );

        // If crouching down or not moving, just set since there can be no collision
        if( NewHeight <= m_NavCollisionCurentHeight )
        {
            m_NavCollisionCurentHeight = NewHeight;
            return TRUE;
        }

        // Standing back up, so check collision above head to make sure there is room
        g_CollisionMgr.SphereSetup( m_Guid, 
                                    m_Position, 
                                    m_Position + 
                                    vector3( 0, ( NewHeight - m_NavCollisionRadius ) + 1.0f, 0 ),
                                    m_NavCollisionRadius );
        g_CollisionMgr.UseLowPoly();

        // Check collision
        object* pActor = g_ObjMgr.GetObjectByGuid( m_Guid );
        if( pActor && pActor->GetType() == object::TYPE_PLAYER )
        {
            // Player
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_PLAYER,
                                            object::ATTR_COLLISION_PERMEABLE|
                                            object::ATTR_LIVING );
        }
        else 
        {
            // NPC
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_CHARACTER,
                                            object::ATTR_COLLISION_PERMEABLE|
                                            object::ATTR_LIVING );
        }

        // If there was a collision, then we cannot stand up yet
        if( g_CollisionMgr.m_nCollisions )
            return FALSE;

        // There's room above the head, so record new height
        m_NavCollisionCurentHeight = NewHeight;

        return TRUE;
    }
    else
    {
        //-----------------------------------------------------------------
        // Single player campaign crouch
        //-----------------------------------------------------------------
    
        ASSERT( NormalizePercent >= 0 );
        ASSERT( NormalizePercent <= 1 );

        vector3 NewPosition;
        f32     NewHeight;

        if( m_bFallMode == TRUE )
        {
            f32 HeadPosition = m_Position.GetY() + m_NavCollisionCurentHeight;
            NewHeight        = m_NavCollisionHeight - NormalizePercent*m_NavCollisionCrouchOffset;

            NewPosition        = m_Position;
            NewPosition.GetY() = HeadPosition - NewHeight;

            // The head doesn't move so check from the head to the floor
            g_CollisionMgr.SphereSetup( m_Guid, NewPosition+vector3(0,(NewHeight-m_NavCollisionRadius),0), NewPosition, m_NavCollisionRadius );
            g_CollisionMgr.UseLowPoly();
        }
        else
        {
            NewPosition   = m_Position;
            NewHeight     = m_NavCollisionHeight - NormalizePercent*m_NavCollisionCrouchOffset;

            // The head moves down so must check from the floor the the head
            g_CollisionMgr.SphereSetup( m_Guid, NewPosition, NewPosition+vector3(0,(NewHeight-m_NavCollisionRadius),0), m_NavCollisionRadius );
            g_CollisionMgr.UseLowPoly();
        }

        object *ourObject = g_ObjMgr.GetObjectByGuid(m_Guid);
        if( ourObject && ourObject->IsKindOf(player::GetRTTI()) )
        {
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_BLOCKS_PLAYER,
                object::ATTR_COLLISION_PERMEABLE|
                object::ATTR_LIVING );
        }
        else 
        {
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_BLOCKS_CHARACTER,
                object::ATTR_COLLISION_PERMEABLE|
                object::ATTR_LIVING );
        }

        if( g_CollisionMgr.m_nCollisions != 0 )
            return FALSE;

        m_NavCollisionCurentHeight = NewHeight;
        m_Position                 = NewPosition;

        return TRUE;
    }
}

//=========================================================================
void character_physics::Jump( f32 Vel )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::Jump", "" );
#endif

    if( m_bFallMode )
        return;

    m_Velocity += vector3( 0, Vel, 0 );
    m_bJumpMode = TRUE;
    m_bTrackingGround = FALSE;

#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::Jump", "Set Jump" );
#endif
}

//=========================================================================

void character_physics::Fling( const vector3& Velocity,
                                     f32      DeltaTime,
                                     f32      AirControl, 
                                     xbool    FlingOnly,
                                     xbool    ReflingOnly,
                                     xbool    Instantaneous,
                                     guid     FlingGuid )
{
    if( FlingOnly && m_bFlingMode )
        return;

    if( ReflingOnly && !m_bFlingMode )
        return;

    if( m_bFlingMode && (m_LastFling == FlingGuid) )
        return;

    if( Instantaneous )
    {
        m_Velocity  = Velocity;
        m_LastFling = FlingGuid;
    }
    else
    {
        m_Velocity  += Velocity * DeltaTime;
        m_LastFling  = 0;
    }

    m_FlingAC           = AirControl;
    m_bJumpMode         = TRUE;
    m_bFlingMode        = TRUE;
    m_bTrackingGround   = FALSE;
}

//=========================================================================

void character_physics::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader ( "CharacterPhysics",                    "This is were the physics of the character get descrive", 0 );
    List.PropEnumBool   ( "CharacterPhysics\\IsCollided",        "Tells the system that the object encounter a collision", PROP_TYPE_READ_ONLY );
    List.PropEnumBool   ( "CharacterPhysics\\IsFalling",         "Tells the system that the object is in a falling mode", PROP_TYPE_READ_ONLY );
    List.PropEnumVector3( "CharacterPhysics\\Position",          "The position of the object", PROP_TYPE_READ_ONLY );
    List.PropEnumVector3( "CharacterPhysics\\Velocity",          "The Velocity of the object", PROP_TYPE_READ_ONLY );
    List.PropEnumFloat  ( "CharacterPhysics\\ColHeight",         "This is the physical height of the object in cm. It descrives a cylinder.", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\ColRadius",         "This is the physical width of the object in cm. It descrives a cylinder.", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\ColCrouchOffset",   "This is when in couch how much the Y off the bbox nneds to be subtracted", 0 );
    List.PropEnumInt    ( "CharacterPhysics\\MaxCollisions",     "This descrives how many collisions can the object handle at ones", 0 );
    List.PropEnumBool   ( "CharacterPhysics\\DoPermeable",       "When true objects that are Permeable will be notify of collisions", 0 );
    List.PropEnumBool   ( "CharacterPhysics\\UseGravity",        "Tells the system whether it should use gravery or not", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\GravityAcc",        "This is the gravety acceleration for the physics", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\ColSnuggleDis",     "This is the minimun distance that the object maintains with its colliders", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\GroundTolerance",   "This variable tells the system which height the system will start chking for the ground", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\FallTolerance",     "Tells the system what height the player needs to be to the ground before he consider itself to be falling", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\AirControl",        "While in the jump or fall how much control does the character has in the air. From 0 to 1.", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\VelForFall",        "Tells the system at what Y velocity the object needs to be traveling before it consider itseld to be falling", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\MaxGroundDistance", "Tells the system what is the maximun distance that it will search for the ground", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\SlideFriction",     "Indicates the slide friction of the object", 0 );
    List.PropEnumFloat  ( "CharacterPhysics\\ActorCollisionRadius", "Our collision radius vs other actors", 0 );
}

//=========================================================================

xbool character_physics::OnProperty( prop_query& I )
{
    if( I.VarBool( "CharacterPhysics\\IsCollided", m_bNavCollided ) )
    {
    }
    else if( I.VarBool( "CharacterPhysics\\IsFalling", m_bFallMode ) )
    {
    }    
    else if( I.VarVector3( "CharacterPhysics\\Position", m_Position ) )
    {
    }
    else if( I.VarVector3( "CharacterPhysics\\Velocity", m_Velocity ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\ColHeight", m_NavCollisionHeight ) )
    {
        if( I.IsRead() == FALSE )
            m_NavCollisionCurentHeight = m_NavCollisionHeight;
    }
    else if( I.VarFloat( "CharacterPhysics\\ColRadius", m_NavCollisionRadius ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\ColCrouchOffset", m_NavCollisionCrouchOffset ) )
    {
    }
    else if( I.VarInt( "CharacterPhysics\\MaxCollisions", m_MaxCollisions ) )
    {
    }
    else if( I.VarBool( "CharacterPhysics\\DoPermeable", m_bHandlePermeable ) )
    {
    }
    else if( I.VarBool( "CharacterPhysics\\DoPermeable", m_bHandlePermeable ) )
    {
    }
    else if( I.VarBool( "CharacterPhysics\\UseGravity", m_bUseGravity ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\GravityAcc", m_GravityAcceleration ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\ColSnuggleDis", m_CollisionSnuggleDistance ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\GroundTolerance", m_GroundTolerance ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\FallTolerance", m_FallTolerance ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\AirControl", m_AirControl ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\VelForFall", m_VelocityForFallMode ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\MaxGroundDistance", m_MaxDistanceToGround ) )
    {
    }
    else if( I.VarFloat( "CharacterPhysics\\SlideFriction", m_SteepestSlide ) )
    {
        I.VarFloat( "CharacterPhysics\\SlideFriction", m_SteepestSlide );
    }
    else if( I.VarFloat( "CharacterPhysics\\ActorCollisionRadius", m_ActorCollisionRadius ) )
    {
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void character_physics::ApplyCollision( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::ApplyCollision", "" );
#endif

    vector3 SpherePos[16];
    s32     nSpheres;
    s32     i;

    g_CollisionMgr.StartApply( m_Guid );

    nSpheres = g_CollisionMgr.GetCylinderSpherePositions(  
                                            m_Position,
                                            m_Position + vector3(0,m_NavCollisionCurentHeight,0),
                                            m_NavCollisionRadius,
                                            SpherePos,
                                            object::MAT_TYPE_FLESH );
    for( i=0; i<nSpheres; i++ )
    {
        g_CollisionMgr.ApplySphere( SpherePos[i], m_NavCollisionRadius, object::MAT_TYPE_FLESH );
    }

    g_CollisionMgr.EndApply();
}

//=========================================================================

#ifndef X_RETAIL
void character_physics::RenderCollision( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::RenderCollision", "" );
#endif

    vector3 SpherePos[16];
    s32     nSpheres;
    s32     i;

    nSpheres = g_CollisionMgr.GetCylinderSpherePositions(  
                                            m_Position,
                                            m_Position + vector3(0,m_NavCollisionCurentHeight,0),
                                            m_NavCollisionRadius,
                                            SpherePos,
                                            object::MAT_TYPE_FLESH );
    for( i=0; i<nSpheres; i++ )
    {
        draw_Sphere( SpherePos[i], m_NavCollisionRadius );
    }
}
#endif

//===========================================================================

void character_physics::CopyValues( character_physics& rPhysics )
{
    m_bFallMode = rPhysics.GetFallMode();
    m_bJumpMode = rPhysics.GetJumpMode();
    m_Velocity = rPhysics.GetVelocity();
    m_Position = rPhysics.GetPosition();
}

//=========================================================================

void ComputePlatformMotion( const matrix4&  OldPlatformM, 
                            const matrix4&  NewPlatformM,
                            const vector3&  CurrentPos,
                                  vector3&  DeltaPos,
                                  radian&   DeltaYaw )
{
    //
    // Compute motion matrix
    //
    matrix4 DeltaM = OldPlatformM;
    DeltaM.InvertSRT();
    DeltaM = NewPlatformM * DeltaM;

    //
    // Compute DeltaPos
    //
	vector3 NewPos = DeltaM * CurrentPos;
            DeltaPos = NewPos - CurrentPos;

    //
    // Compute DeltaYaw
    //
    radian3 OldPlatformYaw = OldPlatformM.GetRotation();
    radian3 NewPlatformYaw = NewPlatformM.GetRotation();
            DeltaYaw       = NewPlatformYaw.Yaw - OldPlatformYaw.Yaw;
}

//===========================================================================

void character_physics::CatchUpWithRidingPlatform( f32 DeltaTime )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::CatchUpWithRidingPlatform", "" );
#endif

    if( !g_UpdateRidingPlatform )
        return;

    // Check that we are standing on a platform
    if( m_MovingPlatformGuid == 0 )
        return;

    // Get access to the previous surface.  Does it still exist?
    anim_surface* pPlatform = (anim_surface*)g_ObjMgr.GetObjectByGuid( m_MovingPlatformGuid );
    if( !pPlatform )
        return;

    // Get access to the actor
    actor* pActor = (actor*)g_ObjMgr.GetObjectByGuid( m_Guid );
    if( !pActor )
        return;

    //
    // We need to move the amount that the current platform did.
    //
    {
        // Get new matrix
        matrix4 NewMovingPlatformL2W;
        pPlatform->GetBoneL2W(m_MovingPlatformBone,NewMovingPlatformL2W);

        vector3 DeltaPos;
        radian  DeltaYaw;
        ComputePlatformMotion(  m_OldMovingPlatformL2W, 
                                NewMovingPlatformL2W, 
                                pActor->GetPosition(),
                                DeltaPos,
                                DeltaYaw );


        //
        // Remember velocity
        //
        m_OldMovingPlatformVelocity = DeltaPos / DeltaTime;

        //
        // Remember new platform L2W
        //
        m_OldMovingPlatformL2W = NewMovingPlatformL2W;

        //
        // Get current position and yaw
        //
        const matrix4& L2W = pActor->GetL2W();
        vector3 CurrPos = L2W.GetTranslation();
        radian  CurrYaw = L2W.GetRotation().Yaw;

        //
        // Update yaw
        //
        CurrYaw += DeltaYaw;

        //
        // Update position
        //
        m_PlatformCollisionIgnoreList[0] = m_MovingPlatformGuid;
        m_nPlatformsToIgnore=1;

        vector3 NewVector = DeltaPos;
        HandleMove( CurrPos, NewVector, 1.0f, m_SteepestSlide, FALSE );
        m_nPlatformsToIgnore=0;

        //
        // Set new player information
        //
        matrix4 NewL2W;
        NewL2W.Setup(vector3(1,1,1),radian3(0,CurrYaw,0),CurrPos);
        pActor->OnTransform(NewL2W);
    }
}

//===========================================================================

void character_physics::ResetRidingPlatforms( void )
{
    m_MovingPlatformGuid = 0;
    m_MovingPlatformBone = -1;
    m_OldMovingPlatformVelocity.Zero();
    m_OldMovingPlatformL2W.Identity();
}

//===========================================================================

void character_physics::WatchForRidingPlatform( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::WatchForRidingPlatform", "" );
#endif

    if( !g_UpdateRidingPlatform )
        return;

    anim_surface* pPlatform = NULL;
    s32           iPlatformBone = -1;
    guid          PlatformGuid = 0;

    //
    // Now check and see if we are still standing on any platform
    //
    if( m_GroundGuid )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_GroundGuid );
        if( pObject && (pObject->GetAttrBits() & object::ATTR_ACTOR_RIDEABLE) )
        {
            PlatformGuid    = m_GroundGuid;
            pPlatform       = (anim_surface*)pObject;
            iPlatformBone   = m_GroundBoneIndex;
        }
    }

    //
    // Handle transition if there is one
    //
    if( (PlatformGuid == 0) || 
        (PlatformGuid != m_MovingPlatformGuid) || 
        (iPlatformBone != m_MovingPlatformBone))
    {
        // Detach from old platform
        if( (PlatformGuid != m_MovingPlatformGuid) ||
            (iPlatformBone != m_MovingPlatformBone) )
        {
            // If previous platform is present inherit velocity
            if( m_MovingPlatformGuid != 0 )
            {
                m_Velocity += m_OldMovingPlatformVelocity;
            }

            m_MovingPlatformGuid = 0;
            m_MovingPlatformBone = -1;
            m_OldMovingPlatformVelocity.Zero();
            m_OldMovingPlatformL2W.Identity();
            //x_DebugMsg("DETTACHED FROM MOVING PLATFORM\n");
        }

        // Is new ground a moving platform?
        if( PlatformGuid != 0 )
        {
            // Attach to platform
            m_MovingPlatformGuid = PlatformGuid;
            m_MovingPlatformBone = iPlatformBone;
            pPlatform->GetBoneL2W(iPlatformBone,m_OldMovingPlatformL2W);
            //x_DebugMsg("ATTACHED TO MOVING PLATFORM\n");
        }
    }
}

//=========================================================================

xbool ComputeTriSphereMovement( const vector3& aP0,
                                const vector3& aP1,
                                const vector3& aP2,
                                const vector3& SphereCenter,
                                      f32      SphereRadius,
                                      f32      SphereHalfHeight,
                                      vector3& FinalMovement )
{
#ifdef X_ASSERT
    static xbool bCheckResults = TRUE;
#endif

    f32 SphereRadiusSquared = SphereRadius * SphereRadius;
    f32 WorldToSphereScale = SphereRadius / SphereHalfHeight;

    // Compute points in sphere space
    vector3 SphereSpacePt[4];
    SphereSpacePt[0] = aP0 - SphereCenter;
    SphereSpacePt[1] = aP1 - SphereCenter;
    SphereSpacePt[2] = aP2 - SphereCenter;
    SphereSpacePt[0].GetY() *= WorldToSphereScale;
    SphereSpacePt[1].GetY() *= WorldToSphereScale;
    SphereSpacePt[2].GetY() *= WorldToSphereScale;
    SphereSpacePt[3] = SphereSpacePt[0];
    plane SphereSpacePlane(SphereSpacePt[0],SphereSpacePt[1],SphereSpacePt[2]) ;

    //
    // Sphere center is now at (0,0,0) and the sphere has radius SphereRadius
    //

    // Skip if completely in front of the plane
    f32 SphereCenterDistFromPlane = SphereSpacePlane.D;
    if( SphereCenterDistFromPlane >= SphereRadius )
        return FALSE;

    // Skip if center is behind plane
    if( SphereCenterDistFromPlane < 0 )
        return FALSE;
    
    // Clear results
    vector3 BestPushDir;
    f32     BestPushDist = 0.0f;

    // Compute if sphere is inside edges
    xbool   bInsideEdge[3];
    vector3 ClosestPtOnPlaneToSphereCenter = - SphereSpacePlane.Normal * SphereCenterDistFromPlane;
    {
        for( s32 i = 0; i < 3; i++ )
        {
            vector3& PA         = SphereSpacePt[i];
            vector3& PB         = SphereSpacePt[i+1];
            vector3  EdgeDir    = PB - PA;
            vector3  EdgeNormal = SphereSpacePlane.Normal.Cross( EdgeDir );
            vector3  DeltaPoint = ClosestPtOnPlaneToSphereCenter - PA;
            f32      Dot        = EdgeNormal.Dot( DeltaPoint );
            bInsideEdge[i] = ( Dot >= 0.0f );
        }
    }
    
    // Push out of plane?
    if( bInsideEdge[0] && bInsideEdge[1] && bInsideEdge[2] )
    {
        BestPushDist = SphereRadius - SphereCenterDistFromPlane;
        BestPushDir = -ClosestPtOnPlaneToSphereCenter;
    }

    // Push out of edges/vertices?
    if( BestPushDist == 0.0f )
    {
        vector3 Zero(0,0,0);
        
        for( s32 i=0; i<3; i++ )
        {
            // Skip if on inside of the edge (only allow pushing outwards)
            if( bInsideEdge[i] )
                continue;
                
            // Get edge points
            vector3& PA = SphereSpacePt[i];
            vector3& PB = SphereSpacePt[i+1];

            // Get closest pt between edge and sphere center
            vector3 CP = Zero.GetClosestVToLSeg(PA,PB);
            f32 LenSquared = CP.LengthSquared();
            if( LenSquared < SphereRadiusSquared )
            {
                f32 PushDist = SphereRadius - x_sqrt(LenSquared);
                
                // Pt may intersect more than one valid edge - make sure to keep
                // the biggest push so that the sphere gets out of all the edges
                if( PushDist > BestPushDist )
                {
                    BestPushDist = PushDist;
                    BestPushDir = -CP;
                }
            }
        }
    }
    
    // If there were no close calls then bail
    if( BestPushDist == 0.0f )
        return FALSE;

    // Normalize the push direction, move it into world space and extend it 1mm
    ASSERT( BestPushDist > 0.0f );
    ASSERT( BestPushDist <= SphereRadius );
    BestPushDir.Normalize();
    BestPushDir *= BestPushDist;
    BestPushDir.GetY() /= WorldToSphereScale;
    f32 BPDLen = BestPushDir.Length();
    BestPushDir /= BPDLen;
    BestPushDir *= (BPDLen + 0.1f);

    FinalMovement = BestPushDir;

#ifdef X_ASSERT
    // Check to see if we no longer intersect the triangle!
    if( bCheckResults )
    {
        // Stop recursion in case of fail!
        bCheckResults = FALSE;
    
        // Check to make sure new position no longer intersects triangle
        vector3 CheckSphereCenter = SphereCenter + FinalMovement;
        vector3 CheckFinalMovement( 0.0f, 0.0f, 0.0f );
        ASSERT( ComputeTriSphereMovement( aP0, aP1, aP2, CheckSphereCenter, SphereRadius, SphereHalfHeight, CheckFinalMovement ) == FALSE );
    
        // Can now check again
        bCheckResults = TRUE;
    }
#endif

    return TRUE;
}

//=========================================================================

void character_physics::SolvePlatformCollisions ( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::SolvePlatformCollisions", "" );
#endif

    if( !g_ApplyPlatformCollision )
        return;

    s32 MaxLoops = 8;
    s32 nLoops;
    bbox MoveBounds;
    MoveBounds.Min.Zero();
    MoveBounds.Max.Zero();

    for( nLoops=0; nLoops < MaxLoops; nLoops++ )
    {
        s32 nPlatformsHit = 0;

        m_nPlatformsToIgnore = 0;
        vector3 AccumMoveDelta;
        AccumMoveDelta.Zero();

        //
        // Build sphere info
        //
        f32 SphereRadius = m_NavCollisionRadius;
        f32 SphereHalfHeight = m_NavCollisionCurentHeight * 0.5f;       // CJ: CROUCH - m_NavCollisionHeight
        vector3 SphereCenter = m_Position;
        SphereCenter.GetY() += SphereHalfHeight;

        //
        // Build bbox around character
        //
        bbox ActorBBox;
        ActorBBox.Max.GetZ() = ActorBBox.Max.GetX() = +SphereRadius;
        ActorBBox.Min.GetZ() = ActorBBox.Min.GetX() = -SphereRadius;
        ActorBBox.Max.GetY() = m_NavCollisionCurentHeight;              // CJ: CROUCH - m_NavCollisionHeight
        ActorBBox.Min.GetY() = 0;
        ActorBBox.Min += m_Position;
        ActorBBox.Max += m_Position;
        ActorBBox.Inflate(1,1,1);

        //
        // Gather factored out list of clusters in dynamic area
        //
        g_PolyCache.BuildClusterList( ActorBBox, object::TYPE_ALL_TYPES, object::ATTR_ACTOR_RIDEABLE, 0 );

        //
        // Process clusters
        //
        if( g_PolyCache.m_nClusters )
        {
            //
            // Loop through the clusters and process the triangles
            //
            for( s32 iCL=0; iCL<g_PolyCache.m_nClusters; iCL++ )
            {
                xbool bHitThisCluster=FALSE;
                poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

                s32 iQ = -1;
                while( 1 )
                {
                    // Do tight loop on bbox checks
                    {
                        iQ++;
                        while( iQ < (s32)CL.nQuads )
                        {
                            //#### This could be moved outside the loop
                            // and do a pointer incremenet after we
                            // optimized the vector3/bbox stuff
                            bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                            if( ActorBBox.Intersect( *pBBox ) ) 
                            {
                                break;
                            }
                            iQ++;
                        }
                        if( iQ==(s32)CL.nQuads )
                            break;
                    }

                    // Process this quad
                    poly_cache::cluster::quad& QD = CL.pQuad[iQ];

                    {
                        vector3  MoveDelta;
                        vector3* PT[4];
                        PT[0] = (vector3*)(&CL.pPoint[ QD.iP[0] ]);
                        PT[1] = (vector3*)(&CL.pPoint[ QD.iP[1] ]);
                        PT[2] = (vector3*)(&CL.pPoint[ QD.iP[2] ]);
                        PT[3] = (vector3*)(&CL.pPoint[ QD.iP[3] ]);

                        if( ComputeTriSphereMovement( *PT[0], *PT[1], *PT[2], SphereCenter, SphereRadius, SphereHalfHeight, MoveDelta ) )
                        {
                            nPlatformsHit++;
                            MoveBounds += MoveDelta;
                            AccumMoveDelta += MoveDelta;
                            bHitThisCluster = TRUE;
                        }

                        if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                        {
                            if( ComputeTriSphereMovement( *PT[0], *PT[2], *PT[3], SphereCenter, SphereRadius, SphereHalfHeight, MoveDelta ) )
                            {
                                nPlatformsHit++;
                                MoveBounds += MoveDelta;
                                AccumMoveDelta += MoveDelta;
                                bHitThisCluster = TRUE;
                            }
                        }
                    }
                }

                if( bHitThisCluster )
                {
                    s32 i;

                    for( i=0; i<m_nPlatformsToIgnore; i++ )
                    if( m_PlatformCollisionIgnoreList[i] == CL.Guid )
                        break;

                    if( i==m_nPlatformsToIgnore )
                    {
                        if( m_nPlatformsToIgnore < 8 )
                        {
                            m_PlatformCollisionIgnoreList[m_nPlatformsToIgnore] = CL.Guid;
                            m_nPlatformsToIgnore++;
                        }
                    }
                }
            }
        
        }

        // No collisions found so bail
        if( nPlatformsHit == 0 )
            break;

        vector3 FinalMoveDelta( MoveBounds.Min + MoveBounds.Max );

        #ifdef athyssen
        if( FinalMoveDelta.LengthSquared() > 0.001f )
        {
            x_DebugMsg("PLATFORM IS PUSHING!!!\n");
        }
        #endif

        // Move by FinalMoveDelta
        vector3 NewVector = FinalMoveDelta;
        HandleMove( m_Position, NewVector, 1.0f, 0, FALSE );
        m_nPlatformsToIgnore = 0;
    }

    //
    // Check if actor is trapped and crushed!!
    //

    if( nLoops == MaxLoops )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_Guid );
        if( pObj )
        {
            // Kill the character
            pain PainEvent;
            PainEvent.Setup("GENERIC_LETHAL",0,pObj->GetBBox().GetCenter());
            PainEvent.SetDirectHitGuid( pObj->GetGuid() );
            PainEvent.ApplyToObject( pObj );
        }
    }

}

//==============================================================================

void character_physics::SetDeltaPos( vector3 &DeltaPos )
{
    m_LstDeltaPos[ m_DeltaPosIndex ]  = DeltaPos;
    m_DeltaPosIndex                  += 1;

    if ( m_DeltaPosIndex == NUM_STILL_FRAMES )
    {
        m_DeltaPosIndex = 0 ;
    }
}
//==============================================================================

vector3 character_physics::GetRecentDeltaPos( void )
{
    vector3 vSum(0.f, 0.f, 0.f);
    for (s32 i =0 ; i < NUM_STILL_FRAMES; i++)
    {
        vSum += m_LstDeltaPos[i];
    }

    return vSum;
}

//===========================================================================

void character_physics::ResetDeltaPos( void )
{
    // Reset all of the positions in the delta pos storage.
    for (s32 i =0 ; i < NUM_STILL_FRAMES ; i++)
    {
        m_LstDeltaPos[i] = vector3( 100.f, 100.f, 100.f );
    }
    
    m_DeltaPosIndex = 0 ;
}

//=========================================================================

void character_physics::Push( const vector3& PushVector )
{
    xbool bPerm = m_bHandlePermeable;
    m_bHandlePermeable = FALSE;
    vector3 NewVector = PushVector;

    HandleMove( m_Position, NewVector, 1.0f, 0, FALSE );
    ResolvePenetrations();
    m_bHandlePermeable = bPerm;
}

//=========================================================================

void character_physics::SetCollisionIgnoreGuid( guid ignoreGuid )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::SetCollisionIgnoreGuid", "%08X:%08X",
                  ignoreGuid.GetHigh(), ignoreGuid.GetLow() );
#endif

    m_IgnoreGuid = ignoreGuid;
}

//=========================================================================

#define PENETRATION_TOLERANCE2  ( 0.1f * 0.1f )

void character_physics::ResolvePenetrations( void )
{
#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::ResolvePenetrations - Enter", "Pos (%.3f,%.3f,%.3f)", m_Position.GetX(), m_Position.GetY(), m_Position.GetZ() );
#endif

    // Skip if collision is turned off
    if( m_bLocoCollisionOn == FALSE )
    {
        return;
    }

    s32     MaxLoops = 8;
    s32     nLoops;
    vector3 DeepestMove;
#ifdef LOG_CHARACTER_PHYSICS
    vector3 OldPosition = m_Position;
#endif

    bbox MoveBounds;
    MoveBounds.Min.Zero();
    MoveBounds.Max.Zero();

    for( nLoops=0; nLoops < MaxLoops; nLoops++ )
    {
        ASSERT( IsValidPosition( m_Position ) );
    
        vector3 AccumMoveDelta;
        AccumMoveDelta.Zero();

        //
        // Build sphere info
        //
        f32 SphereRadius = m_NavCollisionRadius;
        f32 SphereHalfHeight = m_NavCollisionCurentHeight * 0.5f;           // CJ: CROUCH - m_NavCollisionHeight
        vector3 SphereCenter = m_Position;
        SphereCenter.GetY() += SphereHalfHeight;

        //
        // Build bbox around character
        //
        bbox ActorBBox;
        ActorBBox.Max.GetZ() = ActorBBox.Max.GetX() = +SphereRadius;
        ActorBBox.Min.GetZ() = ActorBBox.Min.GetX() = -SphereRadius;
        ActorBBox.Max.GetY() = m_NavCollisionCurentHeight;                  // CJ: CROUCH - m_NavCollisionHeight
        ActorBBox.Min.GetY() = 0;
        ActorBBox.Min += m_Position;
        ActorBBox.Max += m_Position;
        ActorBBox.Inflate(1,1,1);

        //
        // Gather factored out list of clusters in dynamic area
        //
        u32 Attr = object::ATTR_COLLIDABLE;
        object* pOurCharacter = g_ObjMgr.GetObjectByGuid( m_Guid );
        if( pOurCharacter && pOurCharacter->IsKindOf( player::GetRTTI() ) )
        {
            Attr = object::ATTR_BLOCKS_PLAYER;
        }
        else
        {
            Attr = object::ATTR_BLOCKS_CHARACTER;
        }

        g_PolyCache.BuildClusterList( ActorBBox, object::TYPE_ALL_TYPES, Attr, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

        //
        // Process clusters
        //
        if( g_PolyCache.m_nClusters )
        {
            // Clear deepest penetration found
            f32 DeepestPenetration2 = 0.0f;

            //
            // Loop through the clusters and process the triangles
            //
            for( s32 iCL=0; iCL<g_PolyCache.m_nClusters; iCL++ )
            {
                xbool bHitThisCluster=FALSE;
                poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

                s32 iQ = -1;
                while( 1 )
                {
                    // Do tight loop on bbox checks
                    {
                        iQ++;
                        while( iQ < (s32)CL.nQuads )
                        {
                            //#### This could be moved outside the loop
                            // and do a pointer incremenet after we
                            // optimized the vector3/bbox stuff
                            bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                            if( ActorBBox.Intersect( *pBBox ) ) 
                            {
                                break;
                            }
                            iQ++;
                        }
                        if( iQ==(s32)CL.nQuads )
                            break;
                    }

                    // Process this quad
                    poly_cache::cluster::quad& QD = CL.pQuad[iQ];

                    {
                        vector3  MoveDelta;
                        vector3* PT[4];
                        PT[0] = (vector3*)(&CL.pPoint[ QD.iP[0] ]);
                        PT[1] = (vector3*)(&CL.pPoint[ QD.iP[1] ]);
                        PT[2] = (vector3*)(&CL.pPoint[ QD.iP[2] ]);
                        PT[3] = (vector3*)(&CL.pPoint[ QD.iP[3] ]);

                        if( ComputeTriSphereMovement( *PT[0], *PT[1], *PT[2], SphereCenter, SphereRadius, SphereHalfHeight, MoveDelta ) )
                        {
                            if( MoveDelta.LengthSquared() > DeepestPenetration2 )
                            {
                                DeepestPenetration2 = MoveDelta.LengthSquared();
                                DeepestMove = MoveDelta;
                                bHitThisCluster = TRUE;
                            }
                        }

                        if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                        {
                            if( ComputeTriSphereMovement( *PT[0], *PT[2], *PT[3], SphereCenter, SphereRadius, SphereHalfHeight, MoveDelta ) )
                            {
                                if( MoveDelta.LengthSquared() > DeepestPenetration2 )
                                {
                                    DeepestPenetration2 = MoveDelta.LengthSquared();
                                    DeepestMove = MoveDelta;
                                    bHitThisCluster = TRUE;
                                }
                            }
                        }
                    }
                }
            }

            // Apply the move if it's above our tolerance
            if( DeepestPenetration2 > PENETRATION_TOLERANCE2 )
            {
                m_Position += DeepestMove;
                m_Velocity.Zero();
                
                ASSERT( IsValidPosition( m_Position ) );
                
#ifdef LOG_CHARACTER_PHYSICS
                LOG_WARNING( "character_physics::ResolvePenetrations", "Iteration %d Move (%.3f,%.3f,%.3f)", nLoops, DeepestMove.GetX(), DeepestMove.GetY(), DeepestMove.GetZ() );
#endif
/*
                if( x_sqrt(DeepestPenetration2) > 50.0f )
                {
                    LOG_ERROR( "physics", "Help" );
                }
*/
            }
            else
            {
                // Exit the iteration limiting for loop
                break;
            }
        }
    }

#ifdef LOG_CHARACTER_PHYSICS
    CLOG_MESSAGE( g_LogCharacterPhysics, "character_physics::ResolvePenetrations - Exit", "Pos (%.3f,%.3f,%.3f)", m_Position.GetX(), m_Position.GetY(), m_Position.GetZ() );

    if( m_Position != OldPosition )
    {
        LOG_WARNING( "character_physics::ResolvePenetrations - WEnter", "Pos (%.3f,%.3f,%.3f)", OldPosition.GetX(), OldPosition.GetY(), OldPosition.GetZ() );
        LOG_WARNING( "character_physics::ResolvePenetrations - WExit",  "Pos (%.3f,%.3f,%.3f)", m_Position.GetX(), m_Position.GetY(), m_Position.GetZ() );
    }
#endif
}

//=========================================================================
xbool character_physics::SetupPlayerCollisionCheck( const vector3& Start, const vector3&End )
{
    g_CollisionMgr.CylinderSetup( m_Guid, 
        Start,
        End,
        m_NavCollisionRadius,
        m_NavCollisionCurentHeight );      // CJ: CROUCH - m_NavCollisionHeight

    return TRUE; // placeholder return value in case we need to do 
                 // some error checking on Start and End
}

//=========================================================================

void character_physics::SetGroundTracking( xbool Track )
{
    m_bTrackingGround = Track;
}

//=========================================================================

void character_physics::InitialGroundCheck( const vector3& Position )
{
    m_Position = Position;
    m_bTrackingGround = UpdateGround( 50.0f );
}

//=========================================================================
