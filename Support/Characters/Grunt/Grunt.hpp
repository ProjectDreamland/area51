//=========================================================================
//
//  grunt.cpp
//
//=========================================================================

#ifndef __GRUNT_HPP__
#define __GRUNT_HPP__


//=========================================================================
// INCLUDES
//=========================================================================
#include "Characters\Character.hpp"
#include "Characters\God.hpp"
#include "GruntLoco.hpp"

#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Search_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"
#include "Grunt_Cover_State.hpp"
#include "Grunt_Attack_State.hpp"
#include "Leaper_Attack_State.hpp"


//=========================================================================
// grunt CLASS
//=========================================================================

class grunt : public character
{
// Real time type information
public:
    CREATE_RTTI( grunt, character, object )
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Defines
//=========================================================================

    enum eGruntSubtypes
    {
        SUBTYPE_GRUNT_HAZMAT,
        SUBTYPE_GRUNT_SPEC4,
        SUBTYPE_GRUNT_SCIENTIST,
        SUBTYPE_LEAPER,
    };

//=========================================================================
// Class functions
//=========================================================================
public:
                    grunt ();
    virtual         ~grunt();

//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:

    virtual void    OnInit                  ( void );

    virtual xbool   OnAnimEvent             ( const anim_event& Event, const vector3& WorldPos );

    virtual xbool   HandleSpecialImpactAnim ( const eHitType hitType );

    virtual s32     GetNumberVoiceActors    ( void )    { return 1; }
    virtual xbool   OnProperty              ( prop_query&   I    ) ;
    virtual xbool   UseSplines              ( void )    { return ( m_Subtype != SUBTYPE_LEAPER ); }
    virtual xbool   AlwaysAllowMoveBackwards( void )    { return ( m_Subtype == SUBTYPE_LEAPER ); }
    

//=========================================================================
// Data
//=========================================================================
protected:

    // Locomotion
    grunt_loco              m_Loco;

    //special for grunt
    xbool                   m_bPlayedRage;
    
    // States

    character_idle_state                m_Idle;
    character_alert_state               m_Alert;
    character_search_state              m_Search;
    grunt_cover_state                   m_Cover;
    character_death_state               m_Death;

    // this is a pointer because grunts and leapers share the same object
    // typebut have different attack states.
    character_attack_state             *m_Attack;

public:
    static  char          s_pArmedBoneMaskFileName[256];                  // Trying to data drive the bone mask information.
    static  char          s_pSearchBoneMaskFileName[256];                  // Trying to data drive the bone mask information.
};


#endif
