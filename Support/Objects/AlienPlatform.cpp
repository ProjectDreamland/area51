
#include "AlienPlatform.hpp"
#include "AlienPlatformDock.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "..\Support\Objects\Player.hpp"
#include "InputMgr\GamePad.hpp"
#include "Objects\Path.hpp"
#include "Objects\Tracker.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================
static const f32 k_ORB_ARRIVE_AT_DISTANCE               = 11.0f;
static const s32 k_MAX_ORB_TARGETS                      = 16;
//=============================================================================
// SHARED
//=============================================================================
static guid     g_OrbTargetValidList    [ k_MAX_ORB_TARGETS ];
static guid     g_OrbTargetPotentialList[ k_MAX_ORB_TARGETS ];

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_platform_desc : public object_desc
{
    alien_platform_desc( void ) : object_desc( 
        object::TYPE_ALIEN_PLATFORM, 
        "Alien Platform", 
        "PROPS",
        object::ATTR_COLLIDABLE             |         
        object::ATTR_BLOCKS_ALL_PROJECTILES |         
        object::ATTR_BLOCKS_ALL_ACTORS      |         
        object::ATTR_BLOCKS_RAGDOLL         |          
        object::ATTR_BLOCKS_CHARACTER_LOS   |         
        object::ATTR_BLOCKS_PLAYER_LOS      |         
        object::ATTR_BLOCKS_PAIN_LOS        |         
        object::ATTR_BLOCKS_SMALL_DEBRIS    |         
        object::ATTR_RENDERABLE             |
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_ACTOR_RIDEABLE         |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new alien_platform; }

} s_AlienOrb_Desc;

//=============================================================================

alien_platform::alien_platform()
{
    m_Tracker               = NULL_GUID;
    m_CurrentDock           = NULL_GUID;
    m_DestinationDock       = NULL_GUID;
    m_DestinationPath       = NULL_GUID;
    m_bInTransit            = FALSE;

    m_HighlightedDock       = NULL_GUID;
    m_ActivateOnDeparture   = NULL_GUID;
    m_ActivateOnArrival     = NULL_GUID;
    m_QueuedDestinationDock = NULL_GUID;

    m_DragThisAroundWithMe  = NULL_GUID;

    m_bPlayerOnMe           = FALSE;

    m_State                 = PLATFORM_STATE_IDLE;

#if (defined X_EDITOR) && (defined X_DEBUG)
    m_DbgEyePos.Set(0,0,0);
    m_DbgEyeDir.Set(0,0,1);
#endif  
    
}

//=============================================================================

alien_platform::~alien_platform()
{
}

//=============================================================================

const object_desc& alien_platform::GetTypeDesc( void ) const
{
    return s_AlienOrb_Desc;
}

//=============================================================================

const object_desc& alien_platform::GetObjectType( void )
{
    return s_AlienOrb_Desc;
}

//=============================================================================

void alien_platform::OnColCheck( void )
{    
    anim_surface::OnColCheck();   
}

//=============================================================================

void alien_platform::OnEnumProp      ( prop_enum&    List )
{
    anim_surface::OnEnumProp( List );

    List.PropEnumHeader( "Alien Platform",           "Alien Platform Properties", 0 );

    List.PropEnumGuid( "Alien Platform\\Tracker", "The tracker this platform is attached to", PROP_TYPE_EXPOSE );
    List.PropEnumGuid( "Alien Platform\\Current Dock", "The dock this platform is currently docked at", PROP_TYPE_EXPOSE );

    List.PropEnumGuid( "Alien Platform\\Attached Object", "The platform will move this object to it's current position all the time.", 0 );

    List.PropEnumGuid( "Alien Platform\\Activate On Arrival",    "Object to activate when the platform arrives at a dock.", PROP_TYPE_EXPOSE );
    List.PropEnumGuid( "Alien Platform\\Activate On Departure",  "Object to activate when the platform departs from a dock.", PROP_TYPE_EXPOSE );

    List.PropEnumGuid( "Alien Platform\\Queued Destination Dock", "The dock this platform should travel to next.", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW );    
}

//=============================================================================

xbool alien_platform::OnProperty      ( prop_query&   I    )
{
    if (anim_surface::OnProperty( I ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Attached Object", m_DragThisAroundWithMe ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Activate On Arrival", m_ActivateOnArrival ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Activate On Departure", m_ActivateOnDeparture ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Queued Destination Dock", m_QueuedDestinationDock ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Tracker", m_Tracker ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Platform\\Current Dock", m_CurrentDock ))
    {
        return TRUE;
    }
   /* else if (I.IsVar( "Alien Platform\\Destination Dock" ))
    {
        if (I.IsRead())
        {
            I.SetVarGUID( m_DestinationDock );
            return TRUE;
        }

        // Can't do anything if we are moving
        if (m_bInTransit)
            return TRUE;

        // We need to see if there is even a path to that location
        m_DestinationPath = NULL;
        m_DestinationDock = I.GetVarGUID();

        object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentDock );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( alien_platform_dock::GetRTTI() ))
            {
                alien_platform_dock& APD = alien_platform_dock::GetSafeType( *pObj );

                s32 nDest = APD.GetDestinationCount();
                s32 i;
                for (i=0;i<nDest;i++)
                {
                    guid Dock, Path;
                    if (APD.GetDestination( i, Dock, Path ))
                    {
                        if (Dock == m_DestinationDock)
                        {
                            m_DestinationPath = Path;
                            break;
                        }
                    }
                }
            }
        }

        if (NULL != m_DestinationPath)
        {
            ConfigureTrackerAndLaunch();
        }
        return TRUE;
    }*/
   return FALSE;
}

//=============================================================================

void alien_platform::OnAdvanceLogic  ( f32 DeltaTime )
{
    anim_surface::OnAdvanceLogic( DeltaTime );

    switch(m_State)
    {
    case PLATFORM_STATE_IDLE:
        {
            HandleIdleLogic( DeltaTime );
        }
        break;
    case PLATFORM_STATE_ACTIVATING:
        {
            HandleActivatingLogic( DeltaTime );
        }
        break;
    case PLATFORM_STATE_ACTIVE:
        {
            HandleActiveLogic( DeltaTime );
        }
        break;
    case PLATFORM_STATE_DEACTIVATING:
        {
            HandleDeactivatingLogic( DeltaTime );
        }
        break;
    }
}

//=============================================================================

xbool alien_platform::GetNearestDockInCone ( const vector3& Pos,
                                             const vector3& Dir,
                                                   guid&    OutDock,
                                                   guid&    OutPath )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentDock );
    if (NULL == pObj)
        return FALSE;

    if (!pObj->IsKindOf( alien_platform_dock::GetRTTI() ))
        return FALSE;

    alien_platform_dock& Dock = alien_platform_dock::GetSafeType( *pObj );

    s32         i;
    s32         nDocks      = Dock.GetDestinationCount();    
    vector3     EyeDir      = Dir;
    s32         BestIndex   = -1;
    f32         BestDist    = 1e30f;

    EyeDir.Normalize();

    for (i=0;i<nDocks;i++)
    {
        guid    gDestDock,gPath;

        if (!Dock.GetDestination( i, gDestDock, gPath ))
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid( gDestDock );
        if (NULL == pObj)
            continue;

        
        sphere  Sphere  = pObj->GetBBox();
        vector3 DockPos = pObj->GetL2W().GetTranslation();
        vector3 Delta   = DockPos - Pos;

        f32     Dist    = Delta.Length();

        EyeDir.NormalizeAndScale( Dist + Sphere.R );

        f32     T0,T1;
        s32     nIsect = Sphere.Intersect( T0, T1, Pos, Pos + EyeDir );

        if (nIsect > 0)
        {
            Delta.Scale( T1 );
            Dist = ((Pos+EyeDir) - Sphere.Pos).Length();

            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestIndex = i;
            }
        }
    }

    if (BestIndex == -1)
        return FALSE;

    Dock.GetDestination( BestIndex, OutDock, OutPath );

    return TRUE;
}

//=============================================================================

#ifdef X_EDITOR

s32 alien_platform::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = anim_surface::OnValidateProperties( ErrorMsg );

    return nErrors;
}

#endif


//=============================================================================

#ifndef X_RETAIL
void alien_platform::OnDebugRender( void )
{
    anim_surface::OnDebugRender();
}
#endif // X_RETAIL

//=============================================================================

xbool alien_platform::IsPlayerOn( void )
{
    // ASSUMING THIS WILL NEVER BE IN MULTIPLAYER
    // since it's a moving platform
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( !pPlayer )
        return FALSE;
        
    bbox Box = pPlayer->GetBBox();

    Box.Inflate( 10,10,10 );

    object* pObjToGetBBoxFrom = this;

    if ( NULL_GUID != m_DragThisAroundWithMe )
    {
        object* pTemp = g_ObjMgr.GetObjectByGuid( m_DragThisAroundWithMe );
        if (NULL != pTemp)
        {
            pObjToGetBBoxFrom = pTemp;
        }
    }

    // Safety!
    if (NULL == pObjToGetBBoxFrom)
        return FALSE;

    if (Box.Intersect( pObjToGetBBoxFrom->GetBBox() ))
        return TRUE;

    return FALSE;
}

//=============================================================================

xbool alien_platform::IsUsePressed( void )
{
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( !pPlayer )
        return FALSE;

    f32 WasVal = g_IngamePad[ pPlayer->GetActivePlayerPad() ].GetLogical( ingame_pad::ACTION_USE ).WasValue;
    f32 IsVal  = g_IngamePad[ pPlayer->GetActivePlayerPad() ].GetLogical( ingame_pad::ACTION_USE ).IsValue;
    xbool WasPressed = WasVal > 0.0f;
    xbool IsPressed  = IsVal  > 0.0f;

    if (!WasPressed && IsPressed)
        return TRUE;

    return FALSE;

}

//=============================================================================

void alien_platform::Highlight( guid Guid )
{
    if (NULL_GUID == Guid)
        return;
    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    if (NULL == pObj)
        return;
    if (!pObj->IsKindOf(alien_platform_dock::GetRTTI()))
        return;
    alien_platform_dock& Dock = alien_platform_dock::GetSafeType( *pObj );

    Dock.Highlight();
}

//=============================================================================

void alien_platform::Unhighlight( guid Guid )
{
    if (NULL_GUID == Guid)
        return;
    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    if (NULL == pObj)
        return;
    if (!pObj->IsKindOf(alien_platform_dock::GetRTTI()))
        return;
    alien_platform_dock& Dock = alien_platform_dock::GetSafeType( *pObj );

    Dock.Unhighlight();
}

//=============================================================================

void alien_platform::OnRender( void )
{
    anim_surface::OnRender();

#if defined(X_EDITOR) && defined(X_DEBUG) && defined(shird)
    {
        if (IsPlayerOn())
        {
            bbox Box = GetBBox();
            Box.Inflate( 50,50,50 );

            draw_BBox( Box, XCOLOR_YELLOW );
        }

        vector3 Dir = m_DbgEyeDir;
        Dir.NormalizeAndScale( 2000 );
        draw_Line( m_DbgEyePos, m_DbgEyePos + Dir, XCOLOR_YELLOW );
    }
#endif
}

//=============================================================================

void alien_platform::ConfigureTrackerAndLaunch( void )
{
    if ((NULL_GUID == m_DestinationPath) || (NULL_GUID == m_DestinationDock))
        return;

    object* pDock = g_ObjMgr.GetObjectByGuid( m_DestinationDock );
    object* pPath = g_ObjMgr.GetObjectByGuid( m_DestinationPath );

    if ((NULL == pDock) || (NULL == pPath))
        return;

    if (!(pDock->IsKindOf( alien_platform_dock::GetRTTI() )) ||
        !(pPath->IsKindOf( path::GetRTTI() )))
        return;

    path&                Path = path::GetSafeType( *pPath );

    // Figure out which end of the path we are currently on.
    vector3 Pos = GetL2W().GetTranslation();

    s32 nKeys = Path.GetKeyCount();

    path::key& Key0 = Path.GetKey( 0 );
    path::key& Key1 = Path.GetKey( nKeys-1 );

    f32 Dist0 = (Key0.m_Position - Pos).Length();
    f32 Dist1 = (Key1.m_Position - Pos).Length();

    object* pTracker = g_ObjMgr.GetObjectByGuid( m_Tracker );
    if (NULL == pTracker)
        return;
    if (!pTracker->IsKindOf( tracker::GetRTTI() ))
        return;

    tracker& Tracker = tracker::GetSafeType( *pTracker );
    
    Tracker.SetPath( m_DestinationPath );

    if (Dist0 < Dist1)
    {
        // We are on the T=0 end
        Tracker.SetTime(0);
        Tracker.SetDirection(1);
        Tracker.OnActivate(TRUE);
        m_TrackerTargetTime = Path.GetTotalTime(FALSE);
    }
    else
    {   
        // We are on the T=1 end
        Tracker.SetTime(Path.GetTotalTime(FALSE));
        Tracker.SetDirection(-1);
        Tracker.OnActivate(TRUE);
        m_TrackerTargetTime = 0;
    }

    m_bInTransit = TRUE;
    m_CurrentDock = NULL_GUID;

    if (m_ActivateOnDeparture)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ActivateOnDeparture );
        if (NULL != pObj)
        {
            pObj->OnActivate( TRUE );
        }
    }    
}

//=============================================================================

void alien_platform::HandleTransit( void )
{
    object* pTracker = g_ObjMgr.GetObjectByGuid( m_Tracker );
    if (NULL == pTracker)
        return;
    if (!pTracker->IsKindOf( tracker::GetRTTI() ))
        return;

    tracker& Tracker = tracker::GetSafeType( *pTracker );  
    f32 TrackerTime = Tracker.GetTime();

    f32 dT = m_TrackerTargetTime - TrackerTime;

    if (x_abs(dT) < 0.001f)
    {
        //At End
        m_CurrentDock = m_DestinationDock;
        m_DestinationDock = NULL_GUID;
        m_DestinationPath = NULL_GUID;
        m_bInTransit      = FALSE;

        // Clear the queued destination, if we have finally arrived at it
        if (m_CurrentDock == m_QueuedDestinationDock)
        {
            m_QueuedDestinationDock = NULL_GUID;
        }

        // We drop out of ACTIVE state only if there is no queued destination
        if (NULL_GUID == m_QueuedDestinationDock)
        {
            SwitchState( PLATFORM_STATE_DEACTIVATING );

            if (m_ActivateOnArrival)
            {
                object* pObj = g_ObjMgr.GetObjectByGuid(m_ActivateOnArrival );
                if (NULL != pObj)
                {
                    pObj->OnActivate( TRUE );
                }
            }    
        }
        else
        {
            // We are docked, and we are not in transit.
            // Plus there is a movement in the queue
            // We need to see if there is even a path to that location
            m_DestinationPath = NULL_GUID;
            m_DestinationDock = NULL_GUID;
            
            ResolveQueuedDestination();            

            // If the queued destination was invalid, drop out and deactivate
            if (NULL_GUID == m_DestinationPath)
            {
                SwitchState( PLATFORM_STATE_DEACTIVATING );
            }
            else
            {
                // Otherwise, the queued destination was valid, so we need to start moving again
                ASSERT( NULL_GUID != m_DestinationPath );
                
                ConfigureTrackerAndLaunch();                               
            }
        }
    }
}

//=============================================================================

xbool alien_platform::SwitchState( platform_state State )
{
    // Verify that it is ok to exit the current state
    xbool bOkToSwitch = TRUE;
    switch(m_State )
    {
    case PLATFORM_STATE_IDLE:
        break;
    case PLATFORM_STATE_ACTIVATING:
        if (!m_AnimPlayer.IsAtEnd())
        {
            bOkToSwitch = FALSE;
        }
        break;
    case PLATFORM_STATE_ACTIVE:
        break;
    case PLATFORM_STATE_DEACTIVATING:
        if (!m_AnimPlayer.IsAtEnd())
        {
            bOkToSwitch = FALSE;
        }
        break;
    }

    if (!bOkToSwitch)
    {
        return FALSE;
    }

    //
    //  Handle switching to the new state
    //
    xbool bSwitchedSuccessfully = FALSE;
    switch( State )
    {
    case PLATFORM_STATE_IDLE:
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( "IDLE" );
            if (-1 == Index)
                break;
            m_AnimPlayer.SetSimpleAnim( Index );
            bSwitchedSuccessfully = TRUE;

            // Manually set the "player on me" 
            // now that the player has arrived.
            m_bPlayerOnMe = FALSE;
        }
        break;
    case PLATFORM_STATE_ACTIVATING:  
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( "ACTIVATE" );
            if (-1 == Index)
                break;
            m_AnimPlayer.SetSimpleAnim( Index );
            bSwitchedSuccessfully = TRUE;
        }
        break;
    case PLATFORM_STATE_ACTIVE:
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( "ACTIVE" );
            if (-1 == Index)
                break;
            m_AnimPlayer.SetSimpleAnim( Index );
            bSwitchedSuccessfully = TRUE;
        }
        break;
    case PLATFORM_STATE_DEACTIVATING:  
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( "DEACTIVATE" );
            if (-1 == Index)
                break;
            m_AnimPlayer.SetSimpleAnim( Index );
            bSwitchedSuccessfully = TRUE;
        }
        break;
    }

    if (bSwitchedSuccessfully)
    {
        m_State = State;
    }

    return bSwitchedSuccessfully;
}


//=============================================================================

void alien_platform::HandleIdleLogic( f32 DeltaTime )
{
    (void)DeltaTime;
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( !pPlayer )
        return;

    xbool bPlayerOn = IsPlayerOn();

    if ((!!m_bPlayerOnMe) != bPlayerOn )
    {
        // Did player step on or off?
        if (bPlayerOn)
        {
            // Player stepped on
            ActivateDocks( TRUE );
        }
        else
        {
            // Player stepped off
            ActivateDocks( FALSE );
        }
    }
    m_bPlayerOnMe = bPlayerOn;
    
    if (   m_bPlayerOnMe 
        && IsUsePressed() 
        && !m_bInTransit
        && (NULL_GUID != m_HighlightedDock) )
    {
        // We are docked, the player is standing on us, use has been pressed,
        // and the player is looking at a valid destination dock.
        // Lets go!

        m_DestinationDock = m_HighlightedDock;

        ActivateDocks( FALSE );

        SwitchState( PLATFORM_STATE_ACTIVATING );
        return;
    }

    if (   (NULL_GUID != m_QueuedDestinationDock) 
        && !m_bInTransit )
    {
        // We have a destination in the queue
        ResolveQueuedDestination();
        if (NULL_GUID != m_DestinationPath)
        {
            SwitchState( PLATFORM_STATE_ACTIVATING );
            return;
        }
    }

    if (!m_bPlayerOnMe)
    {
        Unhighlight( m_HighlightedDock );
        m_HighlightedDock = NULL_GUID;
        return;
    }

    if (!m_bInTransit && (NULL_GUID != m_CurrentDock))
    {
        // We are docked, and we are not in transit.
        // Check to see if the player is looking at something
        vector3 Pos = pPlayer->GetEyesPosition();    
        radian  Pitch,Yaw;

        pPlayer->GetEyesPitchYaw( Pitch, Yaw );

        vector3 EyeDir(0,0,1);
        EyeDir.Rotate( radian3(Pitch,Yaw,0) );

        guid LookingAt;
        guid PathTo;

#if defined(X_EDITOR) && defined(X_DEBUG)
        m_DbgEyeDir = EyeDir;
        m_DbgEyePos = Pos;
#endif


        if (GetNearestDockInCone( Pos, EyeDir, LookingAt, PathTo ))
        {
            // We are looking at a dock.
            if (LookingAt != m_HighlightedDock)
            {
                Unhighlight( m_HighlightedDock );

                m_HighlightedDock = LookingAt;
                m_DestinationPath = PathTo;

                Highlight( m_HighlightedDock );
            }
        }
        else
        {
            Unhighlight( m_HighlightedDock );
            m_HighlightedDock = NULL_GUID;
        }
    }    
}

//=============================================================================

void alien_platform::HandleActivatingLogic( f32 DeltaTime )
{
    (void)DeltaTime;
    if (m_AnimPlayer.IsAtEnd())
    {
        if ((NULL_GUID == m_DestinationPath) || (NULL_GUID == m_DestinationDock))
        {
            SwitchState( PLATFORM_STATE_DEACTIVATING );
            return;
        }

        ConfigureTrackerAndLaunch();

        SwitchState( PLATFORM_STATE_ACTIVE );
    }
}

//=============================================================================

void alien_platform::HandleActiveLogic( f32 DeltaTime )
{
    (void)DeltaTime;

    ASSERT(m_bInTransit);
    
    HandleTransit();   
}

//=============================================================================

void alien_platform::HandleDeactivatingLogic( f32 DeltaTime )
{
    (void)DeltaTime;

    if (m_AnimPlayer.IsAtEnd())
    {
        SwitchState( PLATFORM_STATE_IDLE );
    }
}

//=============================================================================

void alien_platform::OnMove( const vector3& NewPos   )
{
    anim_surface::OnMove( NewPos );

    UpdateAttachedObject();
}

//=============================================================================

void alien_platform::OnMoveRel( const vector3& DeltaPos )
{
    anim_surface::OnMoveRel( DeltaPos );

    UpdateAttachedObject();
}

//=============================================================================

void alien_platform::OnTransform( const matrix4& L2W      )
{
    anim_surface::OnTransform( L2W );

    UpdateAttachedObject();
}

//=============================================================================

void alien_platform::UpdateAttachedObject( void )
{
    if (NULL_GUID == m_DragThisAroundWithMe)
        return;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_DragThisAroundWithMe );
    if (NULL == pObj)
        return;

    pObj->OnTransform( GetL2W() );
    pObj->SetZones( GetZones() );
}
//=============================================================================

guid alien_platform::SelectNextDestinationOnWayToGoal( guid GoalDock )
{
    xarray<pathing> Nodes;
    guid            gRet = NULL_GUID;

    // Seed the list with docs connected to the current one
    object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentDock );
    if (NULL != pObj)
    {
        alien_platform_dock& APD = alien_platform_dock::GetSafeType( *pObj );

        s32 nDest = APD.GetDestinationCount();
        s32 i;
        for (i=0;i<nDest;i++)
        {
            guid    gDock,gPath;
            APD.GetDestination(i, gDock, gPath );

            if (GoalDock == gDock)
            {
                // It is adjacent to the current dock
                return GoalDock;
            }

            pathing& P = Nodes.Append();

            P.Source = gDock;
            P.This   = gDock;
        }
    }
    
    s32 iCur = 0;
    while (iCur < Nodes.GetCount())
    {
        pathing& P          = Nodes[iCur];
        s32      iFoundHere = -1;

        AppendGuidsFromDock( P, GoalDock, Nodes, iFoundHere );

        if ((iFoundHere >= 0) && (iFoundHere < Nodes.GetCount()))
        {
            gRet = Nodes[ iFoundHere ].Source;
            break;
        }
        iCur++;
    }

    return gRet;
}

//=============================================================================

s32 alien_platform::AppendGuidsFromDock( alien_platform::pathing&           Dock, 
                                         guid                               WatchForThis,
                                         xarray<alien_platform::pathing>&   Array,
                                         s32&                               WatchLocatedHere )
{
    s32 nRet = 0;

    WatchLocatedHere = -1;

    object* pObj = g_ObjMgr.GetObjectByGuid( Dock.This );
    if (NULL != pObj)
    {
        if (pObj->IsKindOf( alien_platform_dock::GetRTTI() ))
        {
            alien_platform_dock& APD = alien_platform_dock::GetSafeType( *pObj );

            s32 nDest = APD.GetDestinationCount();
            s32 i;
            for (i=0;i<nDest;i++)
            {
                guid    gDock,gPath;
                xbool   bAdd = TRUE;

                APD.GetDestination(i, gDock, gPath );

                object* pOtherDock = g_ObjMgr.GetObjectByGuid( gDock );
                if (NULL == pOtherDock)
                    bAdd = FALSE;
                else if (!pOtherDock->IsKindOf( alien_platform_dock::GetRTTI() ))
                    bAdd = FALSE;
                                
                if (bAdd)
                {
                    s32 j;
                    for (j=0;j<Array.GetCount();j++)
                    {
                        if (Array[j].This == gDock)
                        {
                            bAdd = FALSE;
                            break;
                        }
                    }
                }

                if (bAdd)
                {
                    pathing& P = Array.Append();
                    P.Source = Dock.Source;
                    P.This   = gDock;

                    if (gDock == WatchForThis)
                    {
                        WatchLocatedHere = Array.GetCount() - 1;
                    }
                    nRet++;
                }                
            }
        }
    }

    return nRet;
}
//=============================================================================

void alien_platform::ResolveQueuedDestination( void )
{    
    m_DestinationPath = NULL_GUID;
    m_DestinationDock = m_QueuedDestinationDock;

    if (m_QueuedDestinationDock == m_CurrentDock)
    {
        m_QueuedDestinationDock = NULL_GUID;
        m_DestinationDock = NULL_GUID;
        return;
    }

    guid gNext = SelectNextDestinationOnWayToGoal( m_QueuedDestinationDock );

    object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentDock );
    if (NULL != pObj)
    {
        if (pObj->IsKindOf( alien_platform_dock::GetRTTI() ))
        {
            alien_platform_dock& APD = alien_platform_dock::GetSafeType( *pObj );

            s32 nDest = APD.GetDestinationCount();
            s32 i;
            for (i=0;i<nDest;i++)
            {
                guid Dock, Path;
                if (APD.GetDestination( i, Dock, Path ))
                {
                    if (Dock == gNext)
                    {
                        m_DestinationPath       = Path;
                        m_DestinationDock       = Dock;                        
                        break;
                    }
                }
            }
        }
    }
}

//=============================================================================

void alien_platform::ActivateDocks( xbool bActive )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentDock );
    if (NULL != pObj)
    {
        if (pObj->IsKindOf( alien_platform_dock::GetRTTI() ))
        {
            alien_platform_dock& APD = alien_platform_dock::GetSafeType( *pObj );
            //SH - removed until we decide if we are keeping ring fx on docks
            //APD.ActivateDock( bActive );
            s32 nDest = APD.GetDestinationCount();
            s32 i;
            for (i=0;i<nDest;i++)
            {
                guid gDock,gPath;
                APD.GetDestination( i, gDock, gPath );
                
                object_ptr<alien_platform_dock> pDock( gDock );

                if (pDock)
                {
                    pDock->ActivateDock( bActive );
                }
            }
        }
    }
}

//=============================================================================

//=============================================================================

//=============================================================================
