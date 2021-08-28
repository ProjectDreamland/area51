//=========================================================================
//
//  generic.cpp
//
//=========================================================================

#ifndef __GENERIC_HPP__
#define __GENERIC_HPP__


//=========================================================================
// INCLUDES
//=========================================================================
#include "Characters\Character.hpp"
#include "Characters\God.hpp"
#include "GenericNPCLoco.hpp"

#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Search_State.hpp"
#include "Characters\BaseStates\Character_Attack_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"
#include "Characters\BaseStates\Character_Cover_State.hpp"

//=========================================================================
// generic CLASS
//=========================================================================

class genericNPC : public character
{
// Real time type information
public:
    CREATE_RTTI( genericNPC, character, object )
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Class functions
//=========================================================================
public:
                    genericNPC();
    virtual         ~genericNPC();

//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:

    virtual void    OnInit                  ( void );

    virtual void    CreateDamageEffects     ( const pain& Pain )            { (void)Pain; }    
    
    virtual s32     GetNumberVoiceActors    ( void )    { return 1; }
    virtual xbool   CanTargetGlobs          ( void )    { return TRUE; }

//=========================================================================
// Data
//=========================================================================
protected:

    // Locomotion
    generic_loco              m_Loco;

    // States
    character_idle_state                m_Idle;
    character_alert_state               m_Alert;
    character_search_state              m_Search;
    character_attack_state              m_Attack;
    character_cover_state               m_Cover;
    character_death_state               m_Death;
};


#endif
