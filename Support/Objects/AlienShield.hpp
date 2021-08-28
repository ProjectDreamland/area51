//=============================================================================
//  ALIENSHIELD.HPP
//=============================================================================

#ifndef __ALIEN_SHIELD_HPP__
#define __ALIEN_SHIELD_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "audiomgr\AudioMgr.hpp"

//=============================================================================
// DEFINES
//=============================================================================
#define MAX_SHIELD_STAGES           8
#define MAX_SHIELD_ORBS             8
#define MAX_SHIELD_PAIN_FX          8

//#define ALIEN_SHIELD_CAN_FIRE

//=============================================================================
class alien_shield: public object
{
public:
    CREATE_RTTI( alien_shield, object, object )


        //=============================================================================

     alien_shield         ();
    ~alien_shield         ();

    virtual void        OnInit              ( void );

    virtual bbox        GetLocalBBox        ( void ) const;

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnRender            ( void );
    virtual void        OnRenderTransparent ( void );
    virtual void        OnPain              ( const pain& Pain );
    virtual void        OnColCheck          ( void );
    virtual void        OnActivate          ( xbool Flag );            
    
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    
    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual const char*         GetLogicalName  ( void )   { return "ALIEN_SHIELD"; }
            vector3             GetAimAtPoint   ( const vector3& AimFromHere );
            void                PlayPainFX      ( const vector3& PainPos );
#ifdef X_EDITOR
    virtual void                EditorPreGame   ( void );
#endif// X_EDITOR

    //=============================================================================

protected:
    struct stage
    {
        rhandle<char>           m_hParticleEffect;    
        fx_handle               m_hFX;
        f32                     m_ThinkTimer; 
        f32                     m_ActivePitch;
        xcolor                  m_PainFXColor;
    };

    struct orb
    {
        orb()
        {
            m_gOrb        = 0;
            m_OrbitRadius = 0;
        }
        guid                    m_gOrb;
        f32                     m_OrbitRadius;
    };

    enum gray_anim
    {
        GRAY_ANIM_DAMAGE            = 0,
        GRAY_ANIM_RELEASE_ORB       = 1,
        GRAY_ANIM_IDLE              = 2,
        GRAY_ANIM_TELEPORT_IN       = 3,
        
        GRAY_ANIM_COUNT
    };

    struct gray_anim_data
    {
        gray_anim_data()
        {
            m_AnimName      = -1;
            m_AnimPlayTime  = 0;
            m_AnimFlags     = 0;
            m_AnimGroupName = -1;
        }
        s32                 m_AnimGroupName;
        s32                 m_AnimName;         // Name of animation to play        
        f32                 m_AnimPlayTime;     // Number of cycles/seconds to play
        u32                 m_AnimFlags;        // Animation control flags
    };

protected:

            void                SwitchStage             ( s32 iStage );
            void                DeactivateCurrentStage  ( void );
            void                ActivateCurrentStage    ( void );
            
            void                CreateOrb               ( s32 iSlot );
            xbool               ValidHostsExist         ( void );

            void                PlayGrayAnim            ( gray_anim Anim );
            

#ifdef ALIEN_SHIELD_CAN_FIRE
protected:
            struct combat_info
            {
                f32             m_AttackTime;           // How often attack checks are made
                s32             m_AttackChance;         // [0-100] chance of firing every time the timer cycles
                s32             m_AttackAccuracy;   
            };

            enum combat_mode
            {
                COMBAT_WITH_HOSTS   = 0,                // There are available orb hosts
                COMBAT_NO_HOSTS     = 1,                // There are no available orb hosts
            };

            enum shot_state
            {
                SHOT_IDLE,
                SHOT_CHARGING,
                SHOT_FIRING,
            };

            void                SwitchShotState         ( shot_state State );
            void                AcquireTarget           ( void );
            void                Shoot                   ( xbool bAimToHit );

#endif // ALIEN_SHIELD_CAN_FIRE        


    //
    //  Basic properties
    //
    f32                 m_Radius;
    f32                 m_Height;
    s32                 m_nStages;
    s32                 m_iCurStage;
    s32                 m_nDeployedOrbsMax;
    guid                m_gGray;
    f32                 m_StageHealth;
    f32                 m_OrbMaintenanceTimer;  // Used to limit how often the shield evaluates it's orbs
                                                // and combat mode
    f32                 m_OrbDelayTime;
    s32                 m_iOrbBeingSpawned;
    guid                m_gActivateOnDestroy;

    rhandle<char>       m_hPain;    
    fx_handle           m_hPainFX[ MAX_SHIELD_PAIN_FX ];
    u8                  m_iNextPainFX;

#ifdef ALIEN_SHIELD_CAN_FIRE

    //
    //  Combat
    //    
    combat_mode         m_CurCombatMode;        // Used to determine which combat_info
                                                // to use.
    f32                 m_AttackTimer;          // Current value of attack timer
    combat_info         m_CombatInfo[2];        // Data for each combat state
    shot_state          m_ShotState;

    rhandle<char>       m_hShotChargeup;    
    fx_handle           m_hShotChargeupFX;
    rhandle<char>       m_hShoot;    
    fx_handle           m_hShootFX;

    //
    //  Last shot taken
    //
    vector3             m_LastShotStart;        // Origin of shot
    vector3             m_LastShotEnd;          // End of shot
    f32                 m_LastShotTimer;        // How much time is left on the fade
    guid                m_gLastShotTarget;      // Who was the target    
    vector3             m_RenderVert[ 18 ];

    //
    //  Audio for attack system
    //
    s32                 m_SoundIDChargeup;
    s32                 m_SoundIDAttack;

#endif // ALIEN_SHIELD_CAN_FIRE


    //
    //  Data fed into spawned orbs
    //
    guid                m_SpawnTubeLimitVolume;
    guid                m_TurretLimitVolume;
    guid                m_HostGroup;
    u8                  m_SpawnTubeProb;            // 0-100 probability of choosing spawntube over turret

    //
    //  Audio
    //
    rhandle<char>       m_hAudioPackage;
    s32                 m_SoundIDActive;
    s32                 m_SoundIDStageDestroyed;
    s32                 m_SoundIDFinalDestruction;
    s32                 m_SoundIDPain;
    voice_id            m_ActiveVoiceID;
    f32                 m_ActivePitchTimer;
    f32                 m_ActivePitchStart;
    f32                 m_ActivePitchEnd;
    f32                 m_ActivePitchCurrent;

    f32                 m_ActivePitchBaseline;
    f32                 m_PitchEnvelopeTime;


    //
    //  Gray control
    //  
    gray_anim_data      m_GrayAnimData[ GRAY_ANIM_COUNT ];
    

    //
    //  Core
    //
    stage               m_Stage     [ MAX_SHIELD_STAGES ];
    stage               m_DestructionFX;
    orb                 m_Orb       [ MAX_SHIELD_ORBS   ];
    u32                 m_bActive:1,
                        m_bShotCharging:1,
                        m_bLastShotToHit:1,
                        m_bNeedToBeDestroyed:1;         // Set when shield has been destroyed
};

//=============================================================================
// END
//=============================================================================
#endif// __ALIEN_SHIELD_HPP__
