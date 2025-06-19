#include "AlarmNode.hpp"
#include "entropy\e_draw.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "ng_node2.hpp"
#include "Loco\LocoUtil.hpp"

const f32 k_MinPreferedReserveTime = 0.0f;
const f32 k_MinReserveTime = 2.0f;
const f32 k_Sphere_Radius = 50.0f;
//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct alarm_node_desc : public object_desc
{
        alarm_node_desc( void ) : object_desc( 
            object::TYPE_ALARM_NODE, 
            "Alarm Node Object", 
            "AI",
            object::ATTR_NEEDS_LOGIC_TIME,             
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC )   {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new alarm_node; } 

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        EditorIcon_Draw( EDITOR_ICON_COVER_NODE, Object.GetL2W(), FALSE, XCOLOR_RED );
        EditorIcon_Draw( EDITOR_ICON_MARKER, Object.GetL2W(), FALSE, XCOLOR_RED );
        return -1;
    }

#endif // X_EDITOR

} s_alarm_node_desc;

//===============================================================================

alarm_node::alarm_node() :
    m_Flags( 0 ),
    m_ReserveTimer(0),
    m_bFirstReservation(TRUE),
    m_ReservedGuid(0),
    m_AlarmGroup(0),
    m_ActivateGuid(0),
    m_DeactivateGuid(0),
    m_TurnOffTime(10.0f),
    m_CountDownTimer(0.0f),
    m_AIActivated(FALSE)
{
    m_AnimGroupName = -1;
    m_AnimName = -1;
    m_AnimFlags = 0;
    m_AnimPlayTime = 0.0f;
    m_Faction = FACTION_NONE;
    m_FriendFlags = FACTION_BLACK_OPS;
}

//===============================================================================

alarm_node::~alarm_node()
{
}

//===============================================================================

void alarm_node::OnInit( )
{
    SetAttrBits( GetAttrBits() &  ~object::ATTR_NEEDS_LOGIC_TIME );
    object::OnInit();
}

//===============================================================================

const object_desc& alarm_node::GetTypeDesc( void ) const
{
    return s_alarm_node_desc;
}

//===============================================================================

bbox alarm_node::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), k_Sphere_Radius);
}
//===============================================================================

s32 alarm_node::GetMaterial( void ) const
{
    return MAT_TYPE_NULL ;
}



//===============================================================================

#ifndef X_RETAIL
void alarm_node::OnDebugRender( void )
{
}
#endif // X_RETAIL

//===============================================================================


void alarm_node::InvalidateNode()
{
    ReserveNode(GetGuid(),TRUE);
}

//=========================================================================================================

void alarm_node::ReserveNode( guid NewUser, xbool AIReserved ) 
{ 
    m_bFirstReservation = FALSE;
    m_ReserveTimer = g_ObjMgr.GetGameTime();
    m_ReservedGuid = NewUser;
    if( AIReserved )
    {    
        ReserveAllInGroup();
    }
}

//=========================================================================================================

void alarm_node::ReserveAllInGroup()
{
    const s32 kMAX_CONTACTS = 256;
    s32 contactCount = 0;
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_ALARM_NODE );
    while( contactCount < kMAX_CONTACTS && SlotID != SLOT_NULL )
    {    
        object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);
        if(tempObject && tempObject->IsKindOf(alarm_node::GetRTTI()) )
        {
            alarm_node &tempAlarm = alarm_node::GetSafeType( *tempObject );        
            // if in our group then reserve it!
            if( tempAlarm.GetAlarmGroup() == m_AlarmGroup && tempAlarm.GetGuid() != GetGuid() )
            {            
                tempAlarm.ReserveNode(m_ReservedGuid,FALSE);
            }
        }
        // Check next
        SlotID = g_ObjMgr.GetNext(SlotID);
    }    
}

//=========================================================================================================

void alarm_node::ActivateAllInGroup()
{
    const s32 kMAX_CONTACTS = 256;
    s32 contactCount = 0;
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_ALARM_NODE );
    while( contactCount < kMAX_CONTACTS && SlotID != SLOT_NULL )
    {    
        object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);
        if(tempObject && tempObject->IsKindOf(alarm_node::GetRTTI()) )
        {
            alarm_node &tempAlarm = alarm_node::GetSafeType( *tempObject );        
            // if in our group then reserve it!
            if( tempAlarm.GetAlarmGroup() == m_AlarmGroup && tempAlarm.GetGuid() != GetGuid() )
            {            
                tempAlarm.Activate(FALSE);
            }
        }
        // Check next
        SlotID = g_ObjMgr.GetNext(SlotID);
    }    
}

//=========================================================================================================

xbool alarm_node::IsReserved( guid Requester ) 
{ 
    //if first time used
    if (m_bFirstReservation)
        return FALSE;

    f32 TimePassed = g_ObjMgr.GetGameDeltaTime( m_ReserveTimer );

    //if used by same requester, then allow use after shorter wait
    if (m_ReservedGuid == Requester)
    {
        return ( TimePassed < k_MinPreferedReserveTime);
    }

    //no reserved guid
    if (!m_ReservedGuid)
    {
        return FALSE;
    }

    //if reservation holder is no longer in existence, then allow use
    if (!g_ObjMgr.GetObjectByGuid(m_ReservedGuid))
    {
        m_ReservedGuid = NULL_GUID;
        return FALSE;
    }

    //reserves for 8 seconds
    return ( TimePassed < k_MinReserveTime );
}


//=============================================================================

void alarm_node::OnAdvanceLogic( f32 DeltaTime )
{
    m_CountDownTimer += DeltaTime;
    if( m_CountDownTimer >= m_TurnOffTime )
    {
        Deactivate();
    }
}

//=============================================================================

void alarm_node::OnActivate( xbool Flag )
{
    if( Flag )
    {
        Activate(TRUE);
    }
    else
    {
        Deactivate();
    }
}

//=============================================================================

void alarm_node::Activate( xbool AIActivated )
{
    // just return if already active.
    if( GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME )
    {
        return;
    }

    SetAttrBits( GetAttrBits() |  object::ATTR_NEEDS_LOGIC_TIME );
    m_CountDownTimer = 0.0f;
    m_AIActivated = AIActivated;

    if( AIActivated )
    {    
        object *activateObj = g_ObjMgr.GetObjectByGuid(m_ActivateGuid);
        if( activateObj )
        {
            activateObj->OnActivate(TRUE);
        }
        ActivateAllInGroup();
    }
}

//=============================================================================

void alarm_node::Deactivate()
{
    SetAttrBits( GetAttrBits() &  ~object::ATTR_NEEDS_LOGIC_TIME );
    
    if( m_AIActivated )
    {    
        object *deactivateObj = g_ObjMgr.GetObjectByGuid(m_DeactivateGuid);
        if( deactivateObj )
        {
            deactivateObj->OnActivate(TRUE);
        }
    }
}

//=========================================================================================================

void alarm_node::OnEnumProp( prop_enum& rPropList )
{
    object::OnEnumProp ( rPropList ) ;
    rPropList.PropEnumHeader( "Alarm Node", "Information for the alarm node." , PROP_TYPE_HEADER ) ;
    // Animation file
    s32 SubID = rPropList.PushPath( "Alarm Node\\" );
    LocoUtil_OnEnumPropAnimFlags(rPropList,
        loco::ANIM_FLAG_PLAY_TYPE_ALL        | 
        loco::ANIM_FLAG_END_STATE_ALL        |
        loco::ANIM_FLAG_INTERRUPT_BLEND      |
        loco::ANIM_FLAG_TURN_OFF_AIMER       |
        loco::ANIM_FLAG_RESTART_IF_SAME_ANIM,
        m_AnimFlags) ;

    rPropList.PropEnumInt("Alarm Group", "Which group the alrm belongs to", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumGuid("Activate Guid", "Object to activate on alarm",PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumGuid("Deactivate Guid", "Object to activate on alarm deactivation", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumFloat("Turn Off Time", "How long till the alarm auto-shuts off", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumHeader("UsableByFactions", "List of factions that use this alarm.", 0 );
    
    rPropList.PropEnumHeader("Factions", "General Faction Info", 0 );
    s32 ID = rPropList.PushPath( "Factions\\" );
    factions_manager::OnEnumProp( rPropList );
    rPropList.PopPath( ID );

    rPropList.PropEnumBool( "DebugRender", "Turns on/off the debugging features", PROP_TYPE_DONT_SAVE ) ;
    rPropList.PopPath( SubID );
}

//=============================================================================

xbool alarm_node::OnProperty( prop_query& rPropQuery )
{
    //
    // When ever we get here we are going to assume we are changing something
    // that will affect our frusturm. This function is not performace critical
    // so we can get away with it.
    //
    s32 Id = rPropQuery.PushPath( "Alarm Node\\" );
    if (LocoUtil_OnPropertyAnimFlags(rPropQuery, 
        m_AnimGroupName, 
        m_AnimName, 
        m_AnimFlags, 
        m_AnimPlayTime))
    {
        return TRUE ;
    }

    if( rPropQuery.VarInt("Alarm Group",m_AlarmGroup))
    {
        return TRUE;
    }
    if( rPropQuery.VarGUID("Activate Guid",m_ActivateGuid))
    {
        return TRUE;
    }
    if( rPropQuery.VarGUID("Deactivate Guid",m_DeactivateGuid))
    {
        return TRUE;
    }
    if( rPropQuery.VarFloat("Turn Off Time",m_TurnOffTime))
    {
        return TRUE;
    }

    //
    // Handle the properties
    //
    if ( rPropQuery.IsVar( "DebugRender" ) )
    {
        if( rPropQuery.IsRead() )        
        {
            rPropQuery.SetVarBool( (m_Flags&FLAGS_DEBUG_RENDER)!=0 );
        }
        else
        {
            if( rPropQuery.GetVarBool() )
            {
                m_Flags |= FLAGS_DEBUG_RENDER;
            }
            else
            {
                m_Flags &= ~FLAGS_DEBUG_RENDER;
            }
        }
        return TRUE;
    }

    s32 ID = rPropQuery.PushPath( "Factions\\" );
    if ( factions_manager::OnProperty( rPropQuery, m_Faction, m_FriendFlags ) )
    {
        return TRUE;
    }
    rPropQuery.PopPath( ID );
    rPropQuery.PopPath( Id );

    return object::OnProperty(rPropQuery);
}
