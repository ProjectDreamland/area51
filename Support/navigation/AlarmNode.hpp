#ifndef ALARM_NODE_HPP
#define ALARM_NODE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\Obj_mgr.hpp"
#include "miscutils\PropertyEnum.hpp"
#include "Nav_Map.hpp"
#include "inventory\inventory2.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Characters\factions.hpp"

//=========================================================================
// AlarmNode
//=========================================================================
class alarm_node : public object
{
public:
    enum eAlarmAnimPackageType
    {
        ALARM_PACKAGE_NONE = -1,
        ALARM_PACKAGE_CIVILIAN,
        ALARM_PACKAGE_SOLDIER,
        ALARM_PACKAGE_GRAY,
        ALARM_PACKAGE_COUNT,
    };

public:

    CREATE_RTTI( alarm_node, object, object )

                                alarm_node      ( void );
    virtual                    ~alarm_node      ( void );

    virtual const object_desc&  GetTypeDesc     ( void ) const ;

    virtual         bbox        GetLocalBBox    ( void ) const;
    virtual	void	            OnEnumProp	    ( prop_enum& rList );
    virtual	xbool	            OnProperty	    ( prop_query& rPropQuery );
    virtual void                OnInit          ( void );     

    virtual s32                 GetMaterial     ( void ) const ;

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

    inline  xbool               IsOccupied      ( void );
    inline  const char*         GetAnimGroup    ( void );
    inline  const char*         GetAnimName     ( void );                       
    inline  f32                 GetAnimPlayTime ( void ) { return m_AnimPlayTime ; }                       
    inline  u32                 GetAnimFlags    ( void ) { return m_AnimFlags ;    }
           
    virtual void                OnActivate      ( xbool Flag );            
            void                InvalidateNode  ( void );
            void                Activate        ( xbool AIActivated );
            void                Deactivate      ( void );
    virtual void                OnAdvanceLogic  ( f32 DeltaTime );      
            s32                 GetAlarmGroup   ( void )            { return m_AlarmGroup; }
            void                ReserveNode     ( guid NewUser, xbool AIReserved ) ;
            xbool               IsReserved      ( guid Requester ) ;
            xbool               HasValidAnims2  ( object::type characterType, inven_item WeaponItem );
            const char*         GetAnimGroupName( object::type characterType );
            const char*         GetAnimGroupNameByType( s32 packageType );
    inline  xbool               IsFriendlyFaction( factions Faction );

protected:

            void                ReserveAllInGroup( void );
            void                ActivateAllInGroup( void );

    enum flags
    {
        FLAGS_OCCUPIED     = (1<<0),
        FLAGS_DEBUG_RENDER = (1<<1),
    };

protected:

    u32             m_Flags;
    xtick           m_ReserveTimer;
    xbool           m_bFirstReservation;
    guid            m_ReservedGuid;
    s32             m_AlarmGroup;
    guid            m_ActivateGuid;
    guid            m_DeactivateGuid;
    f32             m_TurnOffTime;
    f32             m_CountDownTimer;
    xbool           m_AIActivated;

    factions        m_Faction ;             //  which faction this object belongs to
    u32             m_FriendFlags ;         //  Flags of our friends.
    
    s32             m_AnimName;         // Name of animation to play
    s32             m_AnimGroupName;    // Name of animation group
    f32             m_AnimPlayTime ;    // Number of cycles/seconds to play
    u32             m_AnimFlags ;       // Animation control flags
};

//=========================================================================

inline
xbool alarm_node::IsOccupied( void )
{
    return (m_Flags&FLAGS_OCCUPIED)!=0 ;
}

//=========================================================================
inline const char* alarm_node::GetAnimGroup( void ) 
{ 
    if (m_AnimGroupName != -1)
        return g_StringMgr.GetString(m_AnimGroupName); 
    else
        return "";
}

//=========================================================================

inline const char* alarm_node::GetAnimName( void ) 
{ 
    if (m_AnimName != -1)
        return g_StringMgr.GetString(m_AnimName); 
    else
        return "";
}
inline

//=========================================================================

xbool alarm_node::IsFriendlyFaction( factions Faction )
{
    if ( Faction == FACTION_NOT_SET)
        return FALSE;

    return( (m_FriendFlags & Faction) ||
            (Faction = FACTION_NEUTRAL) || 
            (m_FriendFlags = FACTION_NEUTRAL) );
}

//=========================================================================
// END
//=========================================================================
#endif