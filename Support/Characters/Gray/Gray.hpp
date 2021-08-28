//=========================================================================
//
//  Gray.hpp
//
//=========================================================================

#ifndef __GRAY_HPP__
#define __GRAY_HPP__

#ifdef shird
#ifdef X_EDITOR
#define GRAY_AI_LOGGING
#endif // X_EDITOR
#endif

//=========================================================================
// INCLUDES
//=========================================================================

#include "Characters\Character.hpp"
#include "GrayLoco.hpp"
#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Attack_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"
#include "Characters\BaseStates\Character_Search_State.hpp"
#include "Characters\BaseStates\Character_Cover_State.hpp"
#include "Gray_Attack_State.hpp"

//=========================================================================
// Forward Declarations
//=========================================================================

//class character_idle_state;  
//class character_alert_state;  

//=========================================================================
// GRAY CLASS
//=========================================================================

class gray : public character
{
// Real time type information
public:
    CREATE_RTTI( gray, character, object );

friend class gray_idle_state;

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

public:

//=========================================================================
// Class functions
//=========================================================================
public:
                    gray ();
    virtual         ~gray();

//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:
    virtual void    OnEnumProp              ( prop_enum&    List );
    virtual xbool   OnProperty              ( prop_query&   I    );
    virtual void    OnPain                  ( const pain& Pain );
    virtual void    OnRender                ( void );

    virtual xbool   OnAnimEvent             ( const anim_event& Event, const vector3& WorldPos );

    virtual s32     GetNumberVoiceActors    ( void )    { return 1; }

    virtual void    OnInit                  ( void );

            void    SetShieldGuid           ( guid gShield )    { m_gShield = gShield; }
            guid    GetShieldGuid           ( void )            { return m_gShield; }
            xbool   IsShielded              ( void )            { return (m_gShield != NULL_GUID); }

//=========================================================================
// Data
//=========================================================================
protected:

    // Locomotion
    gray_loco               m_Loco;

    // States
    character_idle_state    m_Idle;
//    character_alert_state   m_Alert;
    //character_attack_state  m_Attack;
//    gray_attack_state       m_Attack;    
//    character_search_state  m_Search;    
//    character_cover_state   m_Cover;
    character_death_state   m_Death;

    guid                    m_gShield;              // Guid of shield protecting the gray
};

//=========================================================================


#endif
