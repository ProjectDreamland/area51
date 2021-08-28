//=========================================================================
//
//  God.hpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "God.hpp"
#include "Characters\Character.hpp"
#include "Objects\Corpse.hpp"
#include "Gamelib\StatsMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "MusicStateMgr\MusicStateMgr.hpp"

//=========================================================================
// DATA
//=========================================================================

static const f32    s_KeepActiveAfterRendering      =    2.0f;
static const s32    k_MaxNumPlayers                 =    5;
const f32           k_MinActiveDistance             =    10000.0f;
f32 g_MinGodTimeTalk    = 10.0f;
f32 g_MaxGodTimeTalk    = 20.0f;


//=========================================================================
// DEBUGGING DATA
//=========================================================================

#ifndef X_RETAIL

// Set this guid in the debugger so only that character is active
static guid DEBUG_ACTIVE_THINK_GUID  = 0;       // Guid of npc to make active
static s32  DEBUG_ACTIVE_THINK_COUNT = 10;      // Frames in between think call

#endif

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct god_desc : public object_desc
{
        god_desc( void ) : object_desc( 
            object::TYPE_GOD, 
            "god", 
            "AI",

            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_NO_EDITOR_RENDERABLE )   { }

    //-------------------------------------------------------------------------

    virtual object* Create          ( void ) { return new god; } 
    virtual void    OnEnumProp      ( prop_enum& List ) ;
    virtual xbool   OnProperty      ( prop_query& I ) ;
#ifdef X_EDITOR
    virtual s32     OnEditorRender  ( object& Object ) const;
#endif // X_EDITOR

} s_god_Desc;

//=============================================================================

const object_desc& god::GetTypeDesc( void ) const
{
    return s_god_Desc;
}

//=============================================================================

#ifdef X_EDITOR
s32 god_desc::OnEditorRender( object& Object ) const
{
    Object.OnDebugRender();
    return -1;
}
#endif // X_EDITOR

//=============================================================================

void god_desc::OnEnumProp( prop_enum&   List )
{
    // Call base class
    object_desc::OnEnumProp( List ) ;

    // Character debug
    List.PropEnumHeader  ("CharacterDebug", "Character debug settings.", 0 );
    List.PropEnumBool    ("CharacterDebug\\Loco",  "Render debug for loco lookat, moveat", 0 );
    List.PropEnumBool    ("CharacterDebug\\AI",    "Render debug for sight etc", 0 );
    List.PropEnumBool    ("CharacterDebug\\Path",  "Render debug for pathfinding", 0 );
    List.PropEnumBool    ("CharacterDebug\\Stats", "Shows # of meshes, verts, faces, bones etc", 0 );
}


//=============================================================================

xbool god_desc::OnProperty( prop_query& I )
{
    // Call base class
    if ( object_desc::OnProperty( I ) )
        return TRUE ;
 
#ifndef X_RETAIL
    // Character debug render
    if ( I.VarBool( "CharacterDebug\\Loco",   character::s_bDebugLoco ) )
        return TRUE ;
    
    // Character debug render
    if ( I.VarBool( "CharacterDebug\\AI",     character::s_bDebugAI ) )
        return TRUE ;
    
    // Character debug render
    if ( I.VarBool( "CharacterDebug\\Path",   character::s_bDebugPath ) )
        return TRUE ;
    
    // Character mesh,vert,face,bone counts etc
    if ( I.VarBool( "CharacterDebug\\Stats",  character::s_bDebugStats ) )
        return TRUE ;
#endif // X_RETAIL

    return FALSE ;
}


//=============================================================================
//=============================================================================

god::TargettingData::TargettingData(guid targetter, guid targetGuid, f32 distanceSqr ):
    m_Targetter(targetter),
    m_TargetGuid(targetGuid),
    m_DistanceSqr(distanceSqr)
{
}

void god::TargettingData::Clear()
{
    m_TargetGuid = NULL_GUID;
    m_Targetter = NULL_GUID;
    m_DistanceSqr = -1.0f;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

god::god()
{
    m_ActiveThinkID = -1 ;  // Current ID of character that can think
    m_SoundTimer = 0;
    m_GrenadeTimer = 0;

    m_TimeDeltaToTalk = x_frand( g_MinGodTimeTalk, g_MaxGodTimeTalk );
}

//=========================================================================

god::~god()
{
}

//=============================================================================

void god::OnActivate( xbool bActive )
{
    ( void ) bActive ;
    m_ActiveThinkID = -1 ;  // Current ID of character that can think

    m_AStarPathFinder.ResetNumNodes() ;

}

//=============================================================================

void god::OnKill( void )
{

}

//=============================================================================

xbool IntersectBBoxes( bbox ActiveBBox[k_MaxNumPlayers], object* pObject, s32 MaxIndex )
{
    if ( MaxIndex < 0 )
        return FALSE;

    s32 i = 0;
    
    while( i < MaxIndex )
    {
        if ( ActiveBBox[i].Intersect( pObject->GetBBox() ) )
            return TRUE;
        
        i++;
    }
    
    return FALSE;
}

//=============================================================================

xbool god::GetCanMeleePlayer( guid reqestNPC )
{
    s32 c;
    for(c=0;c<k_MaxMeleeingPlayer;c++)
    {
        if( reqestNPC == m_MeleeingPlayerGuids[c] || 
            m_MeleeingPlayerGuids[c] == NULL_GUID )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

void god::OnAdvanceLogic( f32 DeltaTime )
{
    (void)DeltaTime ;

    STAT_LOGGER( temp, k_stats_AI_Think );

    CONTEXT( "god::OnAdvanceLogic" ) ;

#ifdef X_EDITOR
    // rmb: I'm pulling this out because this code is poo!
    //extern s32 s_PS2MemorySize;
    //x_printfxy( 1, 15, xfs( "Audio Memory Free %d K", (s_PS2MemorySize/1024) ) );
#endif

    // Create infinite bbox incase a player is not found
    bbox ActiveBBox[k_MaxNumPlayers];

    for (s32 i = 0; i < k_MaxNumPlayers; i++)
    {
        ActiveBBox[i].Clear();
    }

    s32 CurrentBBox = -1;

    // Grab active bbox around all players
    slot_id PlayerID = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
        
    while ( PlayerID != SLOT_NULL )
    {  
        CurrentBBox++;

        if (CurrentBBox >= k_MaxNumPlayers)
            break;

        object* pObject = g_ObjMgr.GetObjectBySlot( PlayerID );
        
        if (pObject == NULL)
        {
            PlayerID = g_ObjMgr.GetNext(PlayerID);
            continue;
        }

        ActiveBBox[CurrentBBox] = pObject->GetBBox();
        ActiveBBox[CurrentBBox].Inflate( 2000.f, 2000.f, 2000.f ) ;
      
        PlayerID = g_ObjMgr.GetNext(PlayerID);
    }

    /*
    if (PlayerID != SLOT_NULL)
    {
        // Lookup player
        player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot(PlayerID) ;
        ASSERT(pPlayer) ;
        ASSERT(pPlayer->IsKindOf( player::GetRTTI() ) ) ;

        // Create a bbox around the player that has z far limits
        ActiveBBox = pPlayer->GetBBox() ;
        ActiveBBox.Inflate( 3000.f, 2000.f, 3000.f ) ;

    }*/


#ifndef X_RETAIL
    
    // Debug override?
    object_ptr<character> pDebugThinker( DEBUG_ACTIVE_THINK_GUID );
    
#endif

    // Loop through all characters, setting up their active flag and 
    // records which character should think next (if any)
    character* pActiveThink = NULL ;
    s32        ActiveCount = 0 ;

    //=============================================================================
    //
    //  re-wrote this logic Jim
    //
    //  Used to ask object manager for a list of all objects with an attribute
    //  and object manager would check every object in the world and see what flags
    //  it had.  I changed it to walk the list of object types and chack the first
    //  object in the list and just check it.  If it has the flag then we assume
    //  all objects of that type have the flag and walk through just the objects
    //  of that type
    //
    //      -CDS
    //=============================================================================
 
    s32 c;
    for(c=0;c<k_MaxMeleeingPlayer;c++)
    {    
        m_MeleeingPlayerGuids[c] = NULL_GUID;
    }

    u32 objectTypeCount;
    s32 HighestAwareness = 0;
    for(objectTypeCount = 0; objectTypeCount < TYPE_END_OF_LIST; objectTypeCount++)
    {
        slot_id SlotID = g_ObjMgr.GetFirst((object::type)objectTypeCount);

        if( SlotID != SLOT_NULL )
        {
            object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);

            if(tempObject->GetAttrBits() & object::ATTR_CHARACTER_OBJECT )
            {
                while(SlotID != SLOT_NULL)
                {
                    // Lookup the actual character
                    character* pCharacter = (character*)g_ObjMgr.GetObjectBySlot(SlotID) ;
                    ASSERT(pCharacter) ;
                    ASSERT(pCharacter->IsKindOf( character::GetRTTI() ) ) ;

                    if( pCharacter->IsMeleeingPlayer() )
                    {
                        for(c=0;c<k_MaxMeleeingPlayer;c++)
                        {
                            if( m_MeleeingPlayerGuids[c] == NULL_GUID )
                            {                            
                                m_MeleeingPlayerGuids[c] = pCharacter->GetGuid();
                                break;
                            }
                        }
                    }

                    xbool bShouldBeActive = FALSE;

                    // Setup active flag
                    if (pCharacter->m_pActiveState && pCharacter->m_pActiveState->m_State == character_state::STATE_HOLD)
                    {
                        // Force active if character should be killed now, otherwise force inactive
                        bShouldBeActive = pCharacter->GetAutoRagdoll();
                    }
                    /*else if (pCharacter->m_pActiveState && pCharacter->m_pActiveState->m_State == character_state::STATE_GOAL)
                    {
                        //goal state is always active
                        pCharacter->SetIsActive( TRUE );
                    }*/
                    else if (pCharacter->m_pActiveState && pCharacter->m_pActiveState->m_State == character_state::STATE_TRIGGER)
                    {
                        //goal state is always active
                        bShouldBeActive = TRUE;
                    }                    
                    else if ( pCharacter->IsDead() )
                    {
                        //dead, so must be active to finish death anim
                        bShouldBeActive = TRUE;
                    }
                    else if ( pCharacter->TimeSinceLastRender() < s_KeepActiveAfterRendering )
                    {
                        //keep active for a few seconds after last rendering
                        bShouldBeActive = TRUE;
                    }
                    else if ( IntersectBBoxes(ActiveBBox, pCharacter, CurrentBBox) )
                    {
                        //ok are we in an active zone
                        bShouldBeActive = TRUE;
                    }
                    else if (IsActiveZone( pCharacter->GetZone1() ))
                    {
                        //in an adjacent zone
                        bShouldBeActive = TRUE;
                    }
                    else if( GetMinDistanceToPlayersThroughZones(pCharacter->GetPosition(), pCharacter->GetZone1()) < (k_MinActiveDistance*k_MinActiveDistance) )
                    {
                        //in an adjacent zone
                        bShouldBeActive = TRUE;
                    }

                    #ifndef X_RETAIL
                    
                        // Debug override?
                        if ( ( pDebugThinker ) && ( pCharacter != pDebugThinker.m_pObject ) )
                            bShouldBeActive = FALSE;
                        
                    #endif

                    // Finally set the actual value
                    pCharacter->SetIsActive( bShouldBeActive );

                    // Do?
                    if( pCharacter->IsActive() &&
                        pCharacter->GetDoRunLogic() )
                    {
                        if( pCharacter->GetAwarenessLevel() > HighestAwareness )
                        {
                            HighestAwareness = pCharacter->GetAwarenessLevel();
                        }

                        // NOTE: Be careful if you tell a character to do anything
                        //       which will perform collision checks etc since
                        //       currently the object manager loops cannot be nested!
                        //       That's why I have to keep a pointer to the active thinker
                        //       and call the OnThink functions outside this loop.

                        // Keep this character for later so we can make it think
                        if (ActiveCount == m_ActiveThinkID)
                            pActiveThink = pCharacter ;

                        // Update count
                        ActiveCount++ ;
                    }

                    // Check next
                    SlotID = g_ObjMgr.GetNext(SlotID) ;
                }
            }
        }
    }
    
    g_MusicStateMgr.SetAwarenessLevel( HighestAwareness, DeltaTime );

    // update targetting info. 
    for(c=0;c<k_NumTargettingData;c++)
    {
        m_LastTickTargettingData[c] = m_CurrentTargettingData[c];
        m_CurrentTargettingData[c].Clear();
    }

#ifndef X_RETAIL
    
    // Debug override?
    if ( pDebugThinker )
    {
        // Think every "DEBUG_ACTIVE_THINK_COUNT" frames
        ActiveCount = DEBUG_ACTIVE_THINK_COUNT;
        if ( m_ActiveThinkID == 0 )
            pActiveThink = pDebugThinker.m_pObject;
        else
            pActiveThink = 0;
    }
    
#endif

    // Let one character do some thinking this frame
    if (pActiveThink)
    {
        CONTEXT("god::OnThink") ;

        // Flag as thinking and then think!
        pActiveThink->m_bThinking = TRUE ;
        pActiveThink->OnThink() ;
        pActiveThink->m_bThinking = FALSE ;
    }

    // Goto next thinking ID
    if (++m_ActiveThinkID >= ActiveCount)
        m_ActiveThinkID = 0 ;
}

//=============================================================================

#ifndef X_RETAIL
void god::OnDebugRender( void )
{
}
#endif // X_RETAIL

//=========================================================================

s32 god::GetMaterial( void ) const
{
    // Not really sure what "god" is made of, but this will do!
    return MAT_TYPE_FLESH ;
}

//=============================================================================

bbox god::GetLocalBBox( void ) const
{
    bbox BBox ;
    BBox.Set(vector3(0,0,0), 0) ;
    return BBox ;
}

//=============================================================================

//=========================================================================
// Editor
//=========================================================================

void god::OnEnumProp( prop_enum& List )
{
    (void)List ;
}

//=========================================================================

xbool god::OnProperty( prop_query& I )
{
    (void)I ;
    return FALSE ;
}

//=========================================================================

void god::AddTargettingData( TargettingData newTargetData )
{
    // don't add targetting null.
    if( newTargetData.m_TargetGuid == NULL_GUID )
        return;

    s32 c;
    for (c=0;c<k_NumTargettingData;c++)
    {
        // search for the first empty one. once found add us. 
        if( m_CurrentTargettingData[c].m_TargetGuid == NULL_GUID )
        {
            m_CurrentTargettingData[c] = newTargetData;
            return;
        }
    }
    // we are full. Oh well...
}

//=========================================================================

s32 god::GetNumTargettingCloser( TargettingData newTargetData )
{
    if( newTargetData.m_TargetGuid == NULL_GUID )
        return 0;

    s32 numCloser = 0;
    // check against last ticks data.
    s32 c;
    for(c=0;c<k_NumTargettingData;c++)
    {
        // look for data on this target
        if( m_LastTickTargettingData[c].m_TargetGuid == newTargetData.m_TargetGuid )
        {
            // only add if not us and it's closer.
            if( m_LastTickTargettingData[c].m_Targetter != newTargetData.m_Targetter &&
                m_LastTickTargettingData[c].m_DistanceSqr < newTargetData.m_DistanceSqr )
            {
                numCloser++;
            }
        }
        else if( m_LastTickTargettingData[c].m_TargetGuid == NULL_GUID )
        {
            // we reached the end of the list break;
            break;
        }
    }
    // we reached the end of the list and not found, so no targetters.
    return numCloser;
}


//=========================================================================
/*
xbool   god::RequestPath( const s32 SourceNodeIndex, const s32 DestNodeIndex, const guid RequestorGuid , s32* PathList , s32 PathCount )
{ 
    CONTEXT("god::RequestPath1") ;

    //get the references to the nodes.
    ng_node2& SourceNode = g_NavMap.GetNodeByID( SourceNodeIndex );
    ng_node2& DestNode   = g_NavMap.GetNodeByID( DestNodeIndex );

    xbool RetVal;
    if ( SourceNode.GetGridID() != DestNode.GetGridID() )
    {
        RetVal = FALSE;
    }
    else
    if ( SourceNodeIndex == DestNodeIndex )
    {
        RetVal = FALSE;
    }
    else
    {
        //generate the path.
        RetVal = m_AStarPathFinder.GeneratePath( &SourceNode, &DestNode, RequestorGuid, PathList, PathCount );
    }

    return RetVal;
}

//=========================================================================

xbool   god::RequestPath( vector3 SourcePos ,vector3 DestPos ,guid RequestorGuid, s32* PathList , s32 PathCount )
{
    CONTEXT("god::RequestPath2") ;

    //get the nearest node to SourcePos and to DestPos
    s32 SourceID = g_NavMap.GetNearestNode( SourcePos );
    s32 DestID   = g_NavMap.GetNearestNode( DestPos );
    return RequestPath( SourceID , DestID , RequestorGuid, PathList, PathCount );
}

//=========================================================================

xbool   god::RequestPath( s32 SourceID ,vector3 DestPos ,guid RequestorGuid, s32* PathList , s32 PathCount )
{
    CONTEXT("god::RequestPath3") ;

    //get the nearest node to SourcePos and to DestPos
    s32 DestID   = g_NavMap.GetNearestNode( DestPos );
    return RequestPath( SourceID , DestID , RequestorGuid, PathList, PathCount );
}
*/
//=========================================================================

s32 god::PlayAlertSound( const char* pObjectName, const char* pAction, s32 State, guid ObjGuid, s16 ZoneID, vector3& Pos )
{
    s32 iId = 0;
    
    f32 TimePassed = g_ObjMgr.GetGameDeltaTime( m_SoundTimer );

    if( TimePassed > m_TimeDeltaToTalk )
    {
        m_TimeDeltaToTalk = x_frand( g_MinGodTimeTalk, g_MaxGodTimeTalk );
        m_LastTalkState = 0;
    }

    if(  m_LastTalkState != State )
    {
        iId = g_ConverseMgr.PlayStream( pObjectName, pAction, ObjGuid, ZoneID, Pos );
        m_LastTalkState = State;

        m_SoundTimer = g_ObjMgr.GetGameTime();
    }

    return iId;
}

//=========================================================================

f32 god::GetMinDistanceToPlayersThroughZones( const vector3& position, u16 zoneID )
{
    (void)zoneID;

    // Grab all players
    slot_id PlayerID = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
    f32 minDistSquared = 100000.0f * 100000.0f;    

    while ( PlayerID != SLOT_NULL )
    {  
        object* pObject = g_ObjMgr.GetObjectBySlot( PlayerID );

        if (pObject == NULL)
        {
            PlayerID = g_ObjMgr.GetNext(PlayerID);
            continue;
        }

        f32 zoneDistSquared = (position - pObject->GetPosition()).LengthSquared();
        if( zoneDistSquared < minDistSquared )
        {
            minDistSquared = zoneDistSquared;
        }
        PlayerID = g_ObjMgr.GetNext(PlayerID);
    }
    return minDistSquared;    
}

//=========================================================================

xbool god::IsActiveZone( u16 ZoneID )
{
    //query the zonemngr for all active zones .... (ASK TOMAS TO IMPLEMENT)

    // Grab all players
    slot_id PlayerID = g_ObjMgr.GetFirst(object::TYPE_PLAYER) ;
        
    while ( PlayerID != SLOT_NULL )
    {  
        object* pObject = g_ObjMgr.GetObjectBySlot( PlayerID );

        if (pObject == NULL)
        {
            PlayerID = g_ObjMgr.GetNext(PlayerID);
            continue;
        }

        u16 PlayerZoneID = pObject->GetZone1();

        if ( ZoneID == PlayerZoneID )
            return TRUE;    

        if ( g_ZoneMgr.IsAdjacentZone(PlayerZoneID, ZoneID) )
            return TRUE;

        PlayerID = g_ObjMgr.GetNext(PlayerID);
    }

    return FALSE;
}

//===========================================================================
static s32      g_pPathList[ 50 ];
static s32      g_PathCount = 50;

xbool god::RequestPathWithEdges( object*                pRequestObject, 
                                 const vector3&         vDestination, 
                                 path_find_struct&      rPathStruct, 
                                 s32                    nMaxEdgeListSize,
                                 const pathing_hints*   pPathingHints,
                                 xbool                  bDestInSameGrid )                                 
{
    (void)nMaxEdgeListSize;

    pathing_hints   DefaultHints;
    if ( NULL == pPathingHints )
        pPathingHints = &DefaultHints;

    // Let's clear out the path list.
    x_memset( g_pPathList, -1, g_PathCount * sizeof( s32 ) );
    rPathStruct.Clear();

    // If not using nav map, go straight to destination
    if( pPathingHints->bUseNavMap == FALSE )
    {
        rPathStruct.m_vStartPoint             = vDestination;
        rPathStruct.m_vEndPoint               = vDestination;
        rPathStruct.m_bStartPointOnConnection = TRUE;
        rPathStruct.m_bEndPointOnConnection   = TRUE;
        rPathStruct.m_bStraightPath           = TRUE;
        rPathStruct.m_StartConnectionSlotID   = g_NavMap.GetNearestConnection( pRequestObject->GetPosition() );
        rPathStruct.m_EndConnectionSlotID     = g_NavMap.GetNearestConnection( vDestination );
        
        rPathStruct.m_StepData[0].m_CurrentConnection = rPathStruct.m_StartConnectionSlotID;
        rPathStruct.m_StepData[0].m_DestConnection    = rPathStruct.m_EndConnectionSlotID;
        rPathStruct.m_StepData[0].m_NodeToPassThrough = NULL_NAV_SLOT;
        rPathStruct.m_nSteps = 0;
        return TRUE;
    }

    if (g_NavMap.GetConnectionCount() == 0)
    {
        // There are no nav connections to work with.
        return FALSE;
    }

    vector3 vStartPos = pRequestObject->GetPosition();

    rPathStruct.m_vStartPoint = vStartPos;
    rPathStruct.m_vEndPoint   = vDestination;

    // Find the start connection
    nav_connection_slot_id StartConnectionSlot = g_NavMap.GetNearestConnection( vStartPos );

    // We cannot build a path if the start point isn't even inside the navmap.
    // A base state should be directing the NPC to step back into the navmap before it
    // even bothers with pathfinding.
    if (NULL_NAV_SLOT == StartConnectionSlot)
        return FALSE;

    ng_connection2&        StartConnection     = g_NavMap.GetConnectionByID( StartConnectionSlot );

    // Find the end connection.
    nav_connection_slot_id EndConnectionSlot;
    if( bDestInSameGrid )
    {    
        EndConnectionSlot = g_NavMap.GetNearestConnectionInGrid( vDestination, StartConnection.GetGridID() );
    }
    else
    {
        EndConnectionSlot = g_NavMap.GetNearestConnection( vDestination );
    }
    ng_connection2&        EndConnection     = g_NavMap.GetConnectionByID( EndConnectionSlot );
    
    // if not in same grid, can't path to.
    if( StartConnection.GetGridID() != EndConnection.GetGridID() )
        return FALSE;

    // Get connection info for path end pts
    rPathStruct.m_bStartPointOnConnection = g_NavMap.GetConnectionContainingPoint( rPathStruct.m_StartConnectionSlotID, vStartPos );
    rPathStruct.m_bEndPointOnConnection   = g_NavMap.GetConnectionContainingPoint( rPathStruct.m_EndConnectionSlotID, vDestination );
    
    // If the connections are the same, don't worry about pathfinding because we're there
    if(         ( rPathStruct.m_StartConnectionSlotID == rPathStruct.m_EndConnectionSlotID )
            ||  ( g_NavMap.DoesStraightPathExist( rPathStruct ) ) )    
    {        
        rPathStruct.m_bStraightPath = TRUE;
        rPathStruct.m_nSteps = 0;
        rPathStruct.m_StepData[0].m_CurrentConnection = StartConnectionSlot;
        rPathStruct.m_StepData[0].m_DestConnection    = EndConnectionSlot;
        rPathStruct.m_StepData[0].m_NodeToPassThrough = NULL_NAV_SLOT;
        
        return TRUE;        
    }

    // Let's get a path from the pathfinder.    
    s32 nStepsInPath = 0;
    if ( !m_AStarPathFinder.GeneratePath( &StartConnection, 
                                          &EndConnection,
                                          vDestination,
                                          pRequestObject->GetGuid(),
                                          *pPathingHints,
                                          g_pPathList,
                                          g_PathCount,
                                          nStepsInPath ))
    {
        // No path was found
        return FALSE;
    }

    rPathStruct.m_nSteps = nStepsInPath;

    s32 i;
    for (i=0;i<g_PathCount;i++)
    {
        nav_connection_slot_id iNextConnection;
        if (i < (g_PathCount-1))
            iNextConnection = g_pPathList[i+1];
        else
            iNextConnection = NULL_NAV_SLOT;

        rPathStruct.m_StepData[i].m_CurrentConnection = g_pPathList[i];
        rPathStruct.m_StepData[i].m_DestConnection    = g_pPathList[i+1];
        
        nav_node_slot_id iOverlap = NULL_NAV_SLOT;
        g_NavMap.DoOverlap( g_pPathList[i], iNextConnection, &iOverlap );

        rPathStruct.m_StepData[i].m_NodeToPassThrough = iOverlap;
    }
    
    return TRUE;
}

//===========================================================================
//
//  Retreat uses 2 planes.
//  1) Reference plane
//  2) Termination plane
//
//  The reference plane is a plane through the current position with a normal
//  pointing toward the threat.  This plane can be used to examine nodes
//  from the current connection and determine if they are toward or away from
//  the threat.                                 
//
//  The termination plane is locked down the very first time that we have
//  nodes on the back side of the reference plane.  Once the termination
//  plane is locked down, the retreat pathfinding will be forced to stop
//  if it has no option but to cross the plane.
//  
//  For each step of retreat:
//  1) Evaluate and score all nodes leading out of the connection
//     Ignoring the current node (1st cycle doesn't have a current node)
//
//  2) If there are nodes on the back side of the reference plane,
//     choose the best of those.  Otherwise choose the best front side
//     node.
//
//  3) Lather, rinse, repeat until a termination condition is met
//  
//  Score for nodes is a combination of distance away from the current
//  node, and the number of nodes in the remote connection (remote
//  connection is the connection on the other side of a node - the one
//  that the npc would be travelling into should it choose that node)
//
//  When the connection being evaluated also contains the threat,
//  scores are multiplied by a scalar.  The scalar is some relative
//  measure of the distance the threat is from the line segment
//  defined by the current node and the node being evaluated.
//  Greate distance from the threat is favoured.
//
//
//
//
//

xbool god::RequestRetreatPath( object*                  pRequestObject, 
                                 const vector3&         vRetreatFrom, 
                                 f32                    DesiredMinDistance,
                                 path_find_struct&      rPathStruct, 
                                 s32                    nMaxEdgeListSize,
                                 const pathing_hints*   pPathingHints )
{
    (void)nMaxEdgeListSize;

    pathing_hints   DefaultHints;
    if ( NULL == pPathingHints )
        pPathingHints = &DefaultHints;

    rPathStruct.Clear();


    vector3 vStartPos = pRequestObject->GetPosition();
    nav_connection_slot_id StartConnectionSlot      = g_NavMap.GetNearestConnection( vStartPos );
    //ng_connection2&        StartConnection          = g_NavMap.GetConnectionByID( StartConnectionSlot );

    //nav_connection_slot_id RetreatConnectionSlot    = g_NavMap.GetNearestConnection( vRetreatFrom );

    s32 i;

    //
    //  2 Scores are kept.
    //
    //  [0] is the back halfspace defined by the partitioning plane
    //      that lies perpendicular to the direction vector from the
    //      requesting object -> the point to retreat from.
    //  [1] is in the front halfspace.
    //
    //  We want to use the best in the back halfspace, if there is on.
    //  If there isn't, then we use the best in the front halfspace
    //

    nav_connection_slot_id      CurrentConnectionSlot   = NULL_NAV_SLOT;
    nav_node_slot_id            CurrentNodeSlot         = NULL_NAV_SLOT;
    vector3                     vCurrentPos             = vStartPos;
    xbool                       bDonePathing            = FALSE;
    plane                       TerminationPlane;
    plane                       ReferencePlane;
    f32                         DistanceTravelled       = 0;
    
    vector3                     vToThreat = vRetreatFrom - vStartPos;
    vToThreat.GetY() = 0;
    vToThreat.Normalize();
    
    ReferencePlane.Setup( vStartPos, vToThreat );
    TerminationPlane = ReferencePlane;

    CurrentConnectionSlot = StartConnectionSlot;

    if (NULL_NAV_SLOT == CurrentConnectionSlot)
        return FALSE;

    //
    //  Setup the pathfindstruct
    //
    rPathStruct.m_nSteps = 0;

    //
    //  Loop until we're done
    //  or we discover a special case
    //
    xbool   bNoRetreatAvailable = FALSE;

    while (!bDonePathing)
    {
        //
        //  2 Scores are kept.
        //
        //  [0] is the back halfspace defined by the partitioning plane
        //      that lies perpendicular to the direction vector from the
        //      requesting object -> the point to retreat from.
        //  [1] is in the front halfspace.
        //
        //  For now, we are using the NoRetreatAvailable special condition
        //  to short circuit out of the retreat pathing.
        //  
        //  With this special case in place, we are guaranteed that we will
        //  always have back halfspace nodes available, so technically
        //  we dont' need to track the front halfspace nodes.
        //
        //  I'm leaving it in for now, just in case the NoRetreatAvailable
        //  case ever goes away.
        //

        s32     iBest[2];
        f32     BestScore[2];   

        iBest[0] = -1;
        iBest[1] = -1;

        BestScore[0] = -1;
        BestScore[1] = -1;

        ng_connection2& CurrentConnection = g_NavMap.GetConnectionByID( CurrentConnectionSlot );

        vector3 vToThreat = vRetreatFrom - vCurrentPos;
        vToThreat.Normalize();
        ReferencePlane.Setup( vCurrentPos, vToThreat );

        // Evaluate all nodes of this connection
        for (i=0;i<CurrentConnection.GetOverlapCount();i++)
        {
            nav_node_slot_id        iOverlap          = CurrentConnection.GetOverlapNodeID( i );
            nav_connection_slot_id  iRemoteConnection = CurrentConnection.GetOverlapRemoteConnectionID( i );

            if (iOverlap == NULL_NAV_SLOT)
                continue;
            if (iOverlap == CurrentNodeSlot)
                continue;

            ng_node2&       Node           = g_NavMap.GetNodeByID      ( iOverlap          );
            ng_connection2& RemoteConn     = g_NavMap.GetConnectionByID( iRemoteConnection );
            s32             nRemoteOptions = RemoteConn.GetOverlapCount() - 1;

            vector3 Pos = Node.GetPosition();

            s32 iSlot = 0;              
            if (ReferencePlane.InFront( Pos ))
            {
                iSlot = 1;
            }
                
            vector3 Delta      = Pos - vCurrentPos;
            f32     LenSquared = Delta.LengthSquared();
        
            f32     Score = LenSquared * (nRemoteOptions+1);       
        
            if (Score > BestScore[ iSlot ])
            {
                BestScore[ iSlot ] = Score;
                iBest    [ iSlot ] = i;
            }           
        }

        //  We have a special case if, on the very first attempt we 
        //  discover no nodes on the back size of the ref plane.
        //  In this case, we just retreat to the edge of the connection
        if ((rPathStruct.m_nSteps == 0) && ( iBest[0] == -1))
        {
            bDonePathing        = TRUE;
            bNoRetreatAvailable = TRUE;
            break;
        }

        s32    iRetreatTo = -1;

        if (iBest[0] != -1)
            iRetreatTo = iBest[0];
        else 
        if (iBest[1] != -1)
            iRetreatTo = iBest[1];

        // If we have nothing, we're done
        if (iRetreatTo == -1)
            bDonePathing = TRUE;
        else if (iBest[0] == -1)
            bDonePathing = TRUE;
        else
        {
            // Otherwise, we retreat to overlap #(iRetreat)
            nav_node_slot_id        iOverlap          = CurrentConnection.GetOverlapNodeID( iRetreatTo );
            nav_connection_slot_id  iRemoteConnection = CurrentConnection.GetOverlapRemoteConnectionID( iRetreatTo );
            ng_node2&               Node              = g_NavMap.GetNodeByID( iOverlap );
    
            s32 iCurStep = rPathStruct.m_nSteps++;

            rPathStruct.m_vStartPoint               = vStartPos;
            rPathStruct.m_vEndPoint                 = Node.GetPosition();
            rPathStruct.m_bStartPointOnConnection   = TRUE;
            rPathStruct.m_bEndPointOnConnection     = TRUE;
            rPathStruct.m_StepData[iCurStep].m_CurrentConnection   = CurrentConnectionSlot;
            rPathStruct.m_StepData[iCurStep].m_DestConnection      = iRemoteConnection;
            rPathStruct.m_StepData[iCurStep].m_NodeToPassThrough   = iOverlap;

            rPathStruct.m_StepData[iCurStep+1].m_CurrentConnection   = iRemoteConnection;
            rPathStruct.m_StepData[iCurStep+1].m_DestConnection      = NULL_NAV_SLOT;
            rPathStruct.m_StepData[iCurStep+1].m_NodeToPassThrough   = NULL_NAV_SLOT;


            DistanceTravelled += (vCurrentPos - rPathStruct.m_vEndPoint).Length();

            vCurrentPos = rPathStruct.m_vEndPoint;

            CurrentConnectionSlot = iRemoteConnection;
            //CurrentConnection = g_NavMap.GetConnectionByID( CurrentConnectionSlot );

            CurrentNodeSlot = iOverlap;
        }

        if ( DistanceTravelled >= DesiredMinDistance)
            bDonePathing = TRUE;

    }// while done pathing
    

    if (bNoRetreatAvailable)
        return FALSE;

    return TRUE;
}

                                 
     /*                            
xbool god::RequestRetreatPath( object*                  pRequestObject, 
                                 const vector3&         vRetreatFrom, 
                                 path_find_struct&      rPathStruct, 
                                 s32                    nMaxEdgeListSize,
                                 const pathing_hints*   pPathingHints )
{
    (void)nMaxEdgeListSize;

    pathing_hints   DefaultHints;
    if ( NULL == pPathingHints )
        pPathingHints = &DefaultHints;


    vector3 vStartPos = pRequestObject->GetPosition();
    nav_connection_slot_id StartConnectionSlot = g_NavMap.GetNearestConnection( vStartPos );
    ng_connection2&        StartConnection     = g_NavMap.GetConnectionByID( StartConnectionSlot );

    s32 i;

    //
    //  2 Scores are kept.
    //
    //  [0] is the back halfspace defined by the partitioning plane
    //      that lies perpendicular to the direction vector from the
    //      requesting object -> the point to retreat from.
    //  [1] is in the front halfspace.
    //
    //  We want to use the best in the back halfspace, if there is on.
    //  If there isn't, then we use the best in the front halfspace
    //
    
    s32     iBest[2];
    f32     BestScore[2];   

    iBest[0] = -1;
    iBest[1] = -1;

    BestScore[0] = -1;
    BestScore[1] = -1;

    plane Plane;
    
    vector3 DeltaThreat = vRetreatFrom - vStartPos;
    DeltaThreat.Normalize();

    Plane.Setup( vStartPos, DeltaThreat );
    
    for (i=0;i<StartConnection.GetOverlapCount();i++)
    {
        nav_node_slot_id        iOverlap          = StartConnection.GetOverlapNodeID( i );
        nav_connection_slot_id  iRemoteConnection = StartConnection.GetOverlapRemoteConnectionID( i );

        if (iOverlap == NULL_NAV_SLOT)
            continue;

        ng_node2&       Node           = g_NavMap.GetNodeByID      ( iOverlap          );
        ng_connection2& RemoteConn     = g_NavMap.GetConnectionByID( iRemoteConnection );
        s32             nRemoteOptions = RemoteConn.GetOverlapCount() - 1;

        vector3 Pos = Node.GetPosition();

        s32 iSlot = 0;              
        if (Plane.InFront( Pos ))
        {
            iSlot = 1;
        }
                
        vector3 Delta      = Pos - vStartPos;
        f32     LenSquared = Delta.LengthSquared();
        
        f32     Score = LenSquared * (nRemoteOptions+1);       
        
        if (Score > BestScore[ iSlot ])
        {
            BestScore[ iSlot ] = Score;
            iBest    [ iSlot ] = i;
        }           
    }

    s32    iRetreatTo = -1;

    if (iBest[0] != -1)
        iRetreatTo = iBest[0];
    else 
    if (iBest[1] != -1)
        iRetreatTo = iBest[1];

    // If we have nothing, return a failure
    if (iRetreatTo == -1)
        return FALSE;

    // Otherwise, we retreat to overlap #(iRetreat)
    nav_node_slot_id        iOverlap          = StartConnection.GetOverlapNodeID( iRetreatTo );
    nav_connection_slot_id  iRemoteConnection = StartConnection.GetOverlapRemoteConnectionID( iRetreatTo );
    ng_node2&               Node              = g_NavMap.GetNodeByID( iOverlap );
    
    rPathStruct.m_nSteps                    = 1;
    rPathStruct.m_vStartPoint               = vStartPos;
    rPathStruct.m_vEndPoint                 = Node.GetPosition();
    rPathStruct.m_bStartPointOnConnection   = TRUE;
    rPathStruct.m_bEndPointOnConnection     = TRUE;
    rPathStruct.m_StepData[0].m_CurrentConnection   = StartConnectionSlot;
    rPathStruct.m_StepData[0].m_DestConnection      = iRemoteConnection;
    rPathStruct.m_StepData[0].m_NodeToPassThrough   = iOverlap;

    return TRUE;
}

*/

