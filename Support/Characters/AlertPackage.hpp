#ifndef __ALERT_PACKAGE_HPP
#define __ALERT_PACKAGE_HPP

#include <x_types.hpp>
#include "factions.hpp"

struct alert_package
{
    enum alert_type
    {
        ALERT_TYPE_NULL,
        ALERT_TYPE_PLAYER_PROXIMITY,
        ALERT_TYPE_NPC_SHOUT,
        ALERT_TYPE_EXPLOSION,
        ALERT_TYPE_GRENADE,
        ALERT_TYPE_SOUND,
        ALERT_TYPE_ACTOR_DIED,
        ALERT_TYPE_PLAYER_TURNED,
        ALERT_TYPE_REQUEST_COVER_FIRE_DIALOG_DONE,
    };

    enum alert_targets
    {
        ALERT_TARGET_NPC = 0
    };

    vector3             m_Position;        // 
    f32                 m_AlertRadius;
    alert_type          m_Type;
    alert_targets       m_Target;
    guid                m_Origin;
    guid                m_Cause;
    factions            m_FactionsSpecific;
    xtick               m_Time;
    s16                 m_ZoneID;    
    
    alert_package       ( void )
    {
        m_Type = ALERT_TYPE_NULL;
    }
};

#endif