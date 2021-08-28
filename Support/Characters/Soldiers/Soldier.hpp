//=========================================================================
//
//  Soldier.cpp
//
//=========================================================================

#ifndef __SOLDIER_HPP__
#define __SOLDIER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Characters\Character.hpp"
#include "SoldierLoco.hpp"
#include "Characters\BaseStates\Character_Idle_State.hpp"
#include "Characters\BaseStates\Character_Alert_State.hpp"
#include "Characters\BaseStates\Character_Search_State.hpp"
#include "Characters\BaseStates\Character_Cover_State.hpp"
#include "Characters\BaseStates\Character_Alarm_State.hpp"
#include "Characters\BaseStates\Character_Turret_State.hpp"
#include "Characters\BaseStates\Character_Death_State.hpp"
#include "Soldier_Attack_State.hpp"
#include "BlackOp_Attack_State.hpp"
#include "BlackOp_Cover_State.hpp"

//=========================================================================
// Soldier CLASS
//=========================================================================

class soldier : public character
{
// Real time type information
public:
    CREATE_RTTI( soldier, character, object )

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Defines
//=========================================================================
public:

    enum eSoldierSubtypes
    {
        SUBTYPE_HAZMAT,
        SUBTYPE_SPEC4,
        SUBTYPE_BLACKOPS,
        SUBTYPE_BLACKOP_LEADER,
    };
//=========================================================================
// Class functions
//=========================================================================
public:
                    soldier();
    virtual        ~soldier();

//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:

    virtual void        OnInit                  ( void );
    virtual s32         GetNumberVoiceActors    ( void )    { return 4; }       
    virtual void        OnDeath                 ( void );
    virtual void        SetOrbGuid              ( guid OrbGuid );
    virtual void        OnThink                 ( void );    
    virtual xbool       GetHasDrainableCorpse   ( void );
            guid        GetAllyCorpseGuid       ( void )    { return m_AllyCorpse; }
            void        BecomeLeader            ( void );
    virtual xbool       CoverRetreatWhenDamaged ( void );
//=========================================================================
// Data
//=========================================================================
protected:

    // Locomotion
    soldier_loco                    m_Loco;

    // States
    character_idle_state            m_Idle;
    character_alert_state           m_Alert;
    character_search_state          m_Search;
    character_alarm_state           m_Alarm;
    character_turret_state          m_Turret;
    character_death_state           m_Death;
    //    soldier_attack_state            m_Attack;

    // added as a pointer since BOs will have a 
    // different attack and cover states than other soldiers.
    character_cover_state          *m_Cover;
    character_attack_state         *m_Attack;
    guid                            m_AllyCorpse;

    
public:

    s32                             m_BlackOppsType;    // The type used to determin which sound to play.
    f32                             m_LastBabyCry;      // Last time this guy whinned about pain


//=========================================================================
// Editor
//=========================================================================
protected:
    
    virtual void                    OnEnumProp      ( prop_enum&    List );
    virtual xbool                   OnProperty      ( prop_query&   I    );
    rigid_inst                      m_GrenadeInst;
    
protected:
    
//=========================================================================
// Weapons
//=========================================================================

    struct weapon_info
    {
        weapon_info( void )
        { 
            m_WeaponName        = NULL;
            m_SkinGeomFileName  = NULL;
            m_AnimFileName      = NULL; 
        }
        
        weapon_info( const char* AnimFileName, const char*SkinGeomFileName, const char*WeaponName ) :
            m_AnimFileName(AnimFileName),
            m_SkinGeomFileName(SkinGeomFileName),
            m_WeaponName(WeaponName)
        { }
        
        const char* m_AnimFileName;
        const char* m_SkinGeomFileName;
        const char* m_WeaponName;
    };


protected:
    
    virtual xbool                   SetupShoulderLight      ( void );

    guid                            m_ParticleGuid;
    vector3                         m_vFlashlightBoneOffset;
    s32                             m_nFlashlightBoneIndex;
    radian3                         m_FlashLightBindRot;
    xbool                           m_bFlashLightInited;

protected:    
    guid                            m_OrbGuid;      

};

class hazmat : public soldier
{
    // Real time type information
public:
    CREATE_RTTI( hazmat, character, object )

        virtual const object_desc&  GetTypeDesc     ( void ) const;
        static  const object_desc&  GetObjectType   ( void );
        
                        hazmat();
        virtual        ~hazmat();

};

#endif // __SOLDIER_HPP__

