//=========================================================================
// INCLUDES
//=========================================================================
#include "Tracker.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct tracker_desc : public object_desc
{
    tracker_desc( void ) : object_desc( 
            object::TYPE_TRACKER, 
            "Tracker", 
            "SCRIPT",
            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new tracker; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        // Make sure position is updated because user maybe
        // editing the path the tracker is attached too!
        object_ptr<tracker> pTracker(Object.GetGuid()) ;
        if (pTracker)
            pTracker->Update(FALSE) ;

        // Call default render
        object_desc::OnEditorRender( Object );
        //return -1 ;
        return EDITOR_ICON_TRACKER; 
    }

#endif // X_EDITOR

} s_tracker_Desc;

//=========================================================================

const object_desc& tracker::GetTypeDesc( void ) const
{
    return s_tracker_Desc;
}

//=========================================================================

const object_desc& tracker::GetObjectType( void )
{
    return s_tracker_Desc;
}
//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

tracker::tracker( void )
{
    // Init playback
    m_PlayType  = PLAY_TYPE_PING_PONG ;
    m_PrevTime  = 0.0f ;
    m_Time      = 0.0f ;
    m_Speed     = 1.0f ;
    m_Direction = 1.0f ;
    m_PathGuid  = 0 ;
    m_CurrentKey.SetDefaults() ;
    m_SizeMult  = 1.0f ;
}

//=============================================================================

bbox tracker::GetLocalBBox( void ) const
{ 
    return bbox( vector3(0,0,0), 100 * m_SizeMult ); 
}

//=============================================================================

void tracker::OnEnumProp( prop_enum& List )
{
    // Call base class
    object::OnEnumProp( List );

    // Add properties
    List.PropEnumHeader("Tracker", "Properties for the tracker object", 0 );
    List.PropEnumBool  ("Tracker\\Active",    "Does this tracker start active (ie.moving?)",        PROP_TYPE_EXPOSE) ;
    List.PropEnumGuid  ("Tracker\\PathGuid",  "Guid of path to track",                              PROP_TYPE_EXPOSE) ;
    List.PropEnumEnum  ("Tracker\\PlayType",  "ONCE\0LOOP\0PING_PONG\0", "Playback type.",          PROP_TYPE_EXPOSE) ;
    List.PropEnumFloat ("Tracker\\Time",      "Time along path.",                                   PROP_TYPE_EXPOSE) ;
    List.PropEnumFloat ("Tracker\\Speed",     "Speed along path. 1 = normal.",                      PROP_TYPE_EXPOSE) ;
    List.PropEnumEnum  ("Tracker\\Direction", "FORWARDS\0BACKWARDS\0", "Direction along the path.", PROP_TYPE_EXPOSE) ;
    List.PropEnumFloat ("Tracker\\SizeMult",  "Increases BBox Size to encompass attached objects. This can be used to help move large coupled objects through zones properly", 0);
}

//=============================================================================

xbool tracker::OnProperty( prop_query& I )
{
    // Call base class
    if( object::OnProperty( I ) )
    {
        // Initialize the zone tracker
        if( I.IsVar( "Base\\Position" )) 
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }
    
        return TRUE ;
    }
    
    // Active?
    if (I.IsVar("Tracker\\Active"))
    {
        if (I.IsRead())
            I.SetVarBool((GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME) != 0) ;
        else
            OnActivate(I.GetVarBool()) ;
        return TRUE ;
    }

    // Path guid?
    if (I.VarGUID("Tracker\\PathGuid", m_PathGuid))
        return TRUE ;

    // Play type?
    if (I.IsVar("Tracker\\PlayType"))
    {
        if( I.IsRead() )
        {
            switch(m_PlayType)
            {
                case PLAY_TYPE_ONCE:        I.SetVarEnum("ONCE") ;      break ;
                case PLAY_TYPE_LOOP:        I.SetVarEnum("LOOP") ;      break ;
                case PLAY_TYPE_PING_PONG:   I.SetVarEnum("PING_PONG") ; break ;
            }
        }
        else
        {
            if (!x_stricmp(I.GetVarEnum(), "ONCE"))
                m_PlayType = PLAY_TYPE_ONCE ;
            else if (!x_stricmp(I.GetVarEnum(), "LOOP"))
                m_PlayType = PLAY_TYPE_LOOP ;
            else if (!x_stricmp(I.GetVarEnum(), "PING_PONG"))
                m_PlayType = PLAY_TYPE_PING_PONG ;
        }
        return TRUE ;
    }

    // Time?
    if (I.VarFloat("Tracker\\Time", m_Time, 0.0f, F32_MAX))
    {
        // Update previous time also
        m_PrevTime = m_Time ;

        // Update position if user has entered a value
        if (I.IsRead() == FALSE)
            OnAdvanceLogic(0) ;

        return TRUE ;
    }

    // Speed?
    if (I.VarFloat("Tracker\\Speed", m_Speed, 0.0f, 100.0f))
        return TRUE ;

    // Direction?
    if( I.IsVar("Tracker\\Direction" ) )
    {
        if( I.IsRead() )
        {
            if (m_Direction > 0)
                I.SetVarEnum("FORWARDS") ;
            else
                I.SetVarEnum("BACKWARDS") ;
        }
        else
        {
            if (!x_stricmp(I.GetVarEnum(), "FORWARDS"))
                m_Direction = 1.0f ;
            else if (!x_stricmp(I.GetVarEnum(), "BACKWARDS"))
                m_Direction = -1.0f ;
            else
                m_Direction = 1.0f ;
        }
        return TRUE ;
    }

    if (I.VarFloat("Tracker\\SizeMult", m_SizeMult, 0.1f, 100.0f))
    {
        if (!I.IsRead())
        {
            OnTransform(GetL2W());
        }
        return TRUE;
    }

    return FALSE ;
}

//=========================================================================

void tracker::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove(NewPos) ;

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, NewPos );
}

//=========================================================================

void tracker::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform(L2W) ;

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, L2W.GetTranslation() );
}
    
//=========================================================================

void tracker::OnInit( void )
{
}

//=========================================================================

#ifndef X_RETAIL
void tracker::OnDebugRender  ( void )
{
#ifdef X_EDITOR
    CONTEXT("tracker::OnDebugRender" );

    // Make sure position is up to date
    Update(FALSE) ;

    // Call base class
    object::OnDebugRender() ;
#endif // X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

void tracker::OnAdvanceLogic( f32 DeltaTime )
{
    // Keep previous time
    m_PrevTime = m_Time ;

    //
    xbool bUsePathZones = FALSE;

    // Get path
    object_ptr<path> pPath(m_PathGuid) ;
    if (pPath && pPath->IsPathOn())
    {
        // Lookup total time
        f32 TotalTime = pPath->GetTotalTime( FALSE );

        // Update tracker time
        m_Time += DeltaTime * m_Direction * m_Speed ;
        if (m_Time < 0)
        {
            switch(m_PlayType)
            {
                default:
                    ASSERTS(0, "Add new type!") ;

                case PLAY_TYPE_ONCE:
                    m_Time = 0.0f ;
                    break ;

                case PLAY_TYPE_LOOP:

                    // Send events for last key
                    m_Time = 0 ;
                    Update(TRUE) ;

                    // If we were deactivated - bail!
                    if (!IsActive())
                        return ;

                    // Now put to end
                    m_PrevTime = TotalTime ;
                    m_Time     += TotalTime ;
                    break ;

                case PLAY_TYPE_PING_PONG:
                    m_Time = 0 ;
                    m_Direction *= -1.0f ;
                    break ;
            }
        }
        else
        if (m_Time > TotalTime)
        {
            switch(m_PlayType)
            {
                default:
                    ASSERTS(0, "Add new type!") ;

                case PLAY_TYPE_ONCE:
                    m_Time = TotalTime ;
                    break ;

                case PLAY_TYPE_LOOP:

                    // Send events for last key
                    m_Time = TotalTime ;
                    Update(TRUE) ;

                    // If we were deactivated - bail!
                    if (!IsActive())
                        return ;

                    // Now put to start
                    m_PrevTime = 0 ;
                    m_Time     -= TotalTime ;

                    // Just in case we have passed through any portals,
                    // copy the zone info from the parent path
                    bUsePathZones = TRUE;
                    break ;

                case PLAY_TYPE_PING_PONG:
                    m_Time = TotalTime ;
                    m_Direction *= -1.0f ;
                    break ;
            }
        }
    }

    // Make sure position is up to date
    Update(TRUE) ;

    if (bUsePathZones)
    {
        SetZone1( pPath->GetZone1() );
        SetZone2( pPath->GetZone2() );

        m_ZoneTracker.SetMainZone( (u8)GetZone1() );
    }
}

//=========================================================================

void tracker::OnActivate( xbool bFlag )
{
    // Call base class
    object::OnActivate(bFlag) ;
}

//=========================================================================

void tracker::Update( xbool bSendKeyEvents )
{
    // Get path
    object_ptr<path> pPath(m_PathGuid) ;
    if (pPath)
    {
        // Make sure the path is on
        if (!pPath->IsPathOn())
            return;

        // Lookup current key
        if (bSendKeyEvents)
            pPath->GetInterpKey(m_PrevTime, m_Time, m_CurrentKey) ;
        else
            pPath->GetInterpKey(m_Time, m_Time, m_CurrentKey) ;

        // Update position and rotation?
        if (pPath->GetFlags() & path::FLAG_KEY_ROTATION)
        {
            // Constructor new L2W
            matrix4 L2W ;
            L2W.Identity() ;
            L2W.SetTranslation(m_CurrentKey.m_Position) ;
            L2W.SetRotation(m_CurrentKey.m_Rotation) ;
            OnTransform(L2W) ;
        }
        else
        {
            // Just update position
            OnMove(m_CurrentKey.m_Position) ;
        }

        // Send key events?
        if (bSendKeyEvents)
        {
            // Object events?
            if (pPath->GetFlags() & path::FLAG_KEY_OBJECT_GUID)
            {
                // Does key have an object?
                object_ptr<object> pObject(m_CurrentKey.m_ObjectGuid) ;
                if (pObject)
                {
                    // Activate the object?
                    if (m_CurrentKey.m_Flags & path::key::FLAG_ACTIVATE_OBJECT)
                        pObject->OnActivate(TRUE) ;

                    // De-activate the object?
                    if (m_CurrentKey.m_Flags & path::key::FLAG_DEACTIVATE_OBJECT)
                        pObject->OnActivate(FALSE) ;
                }
            }

            // Deactivate tracker?
            if (m_CurrentKey.m_Flags & path::key::FLAG_DEACTIVATE_TRACKER)
                OnActivate(FALSE) ;
        }
    }
    else
    {
        // Attached to an object?
        object_ptr<object> pObject(m_PathGuid) ;
        if (pObject)
        {
            m_CurrentKey.m_Position = pObject->GetPosition() ;
            m_CurrentKey.m_Rotation.Setup(pObject->GetL2W()) ;
            OnTransform(pObject->GetL2W()) ;
        }
    }
}

//=========================================================================

path* tracker::GetPath( void )
{
    // Get path
    object_ptr<path> pPath(m_PathGuid) ;
    return pPath.m_pObject ;
}

//=========================================================================

xbool tracker::IsEditorSelected( void )
{
    // Is this object selected?
    if (GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT))
        return TRUE ;

    // Attached to an object?
    object_ptr<path> pObject(m_PathGuid) ;
    if (pObject)
    {
        // Is path selected?
        if (pObject->GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT))
            return TRUE ;
    }

    // Not selected
    return FALSE ;
}

//=========================================================================

void tracker::SetTime( f32 Time )
{
    m_PrevTime = m_Time = Time;
    OnAdvanceLogic(0);
}

//=========================================================================

void tracker::SetDirection( f32 Direction )
{
    if (Direction < 0)
        Direction = -1;
    else 
        Direction = 1;

    m_Direction = Direction;
}

//=========================================================================

f32 tracker::GetTime( void )
{
    return m_Time;
}

//=========================================================================

void tracker::SetPath( guid gPath )
{
    m_PathGuid = gPath;
}

//=========================================================================

