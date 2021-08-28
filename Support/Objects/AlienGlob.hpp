//=============================================================================
//  ALIENGLOB.HPP
//=============================================================================

#ifndef ALIENGLOB_HPP
#define ALIENGLOB_HPP

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "render/RigidInst.hpp"
#include "Characters\FloorProperties.hpp"
#include "Decals\DecalPackage.hpp"

//=============================================================================
// DEFINES
//=============================================================================

//=============================================================================
class alien_glob : public object
{
public:
    CREATE_RTTI( alien_glob, object, object )


//=============================================================================
						alien_glob          ();
	                   ~alien_glob         ();

    virtual void        OnInit              ( void );

    virtual bbox        GetLocalBBox        ( void ) const;

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnTransform         ( const matrix4& L2W );
    virtual void        OnRender            ( void );
    virtual void        OnRenderTransparent ( void );
    virtual void        OnPain              ( const pain& Pain );
    virtual void        OnColCheck          ( void );
    virtual const char* GetLogicalName      ( void );
    virtual void        OnActivate          ( xbool Flag );   
    virtual void        OnAddedToGroup      ( guid gGroup );

            //void        TeleportBones       ( const vector3& NewPos );
            void        Scatter             ( const vector3& Direction, xbool bDeleteSelf = TRUE, s32 nStagesToDrop = 1 );
            //void        Merge               ( alien_glob& Glob );
            //void        AddBone             ( void );
            //void        DelBone             ( void );
            xbool       UpdateAI                ( void );
            xbool       UpdateIdleAI            ( void );
            void        UpdatePhysics           ( void );
            void        UpdateConstraints       ( void );
            void        UpdateCollision         ( void );
            xbool       UpdateInAir             ( void );
            void        UpdateRenderPosition    ( void );
            void        BackupOldVelocities     ( void );
            void        CalculateNewVelocities  ( void );
            void        SolveCollisions         ( void );

            void        UpdateColor             ( void );

            void        Jump                ( const vector3& TargetPos, xbool bToward, xbool bNormalGravity );
            void        Land                ( const vector3& Position,
                                              const vector3& NewTractionNormal,
                                                    guid     GuidHit,
                                                    xbool    bPlayAudio = TRUE);

            virtual const object_desc&  GetTypeDesc ( void ) const;
    static  const object_desc&  GetObjectType       ( void );
    virtual s32                 GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void                OnEnumProp          ( prop_enum&    List );
    virtual xbool               OnProperty          ( prop_query&   I    );
            void                SetStage            ( s32 iNewStage );

            void                OnThink             ( f32 DeltaTime );

            xbool               IsLeader            ( void ) { return m_bIsLeader; }

            void                NotifyOnSpawn       ( guid gNotify ) { m_gNotifyOnSpawn = gNotify; }

protected:

    enum endoflife
    {
        EOL_ATTACK_IMPACT,
        EOL_DEATH,
        EOL_SPLITTING,
    };
            
            void        StartEndOfLife      ( endoflife EOLReason );
            void        SetupAttackData     ( void );
            vector3     GetAttackDestination( void );
            void        DestroySelf         ( void );


#ifdef X_EDITOR
            void        StickToNearestSurface( void );
#endif
//=============================================================================
protected:

    struct attack_data
    {
        s32     iBone;
        f32     tAlongBone;
        f32     AttackTimer;
        vector3 LeapOrigin;
    };

    enum state 
    {
        STATE_IDLE,                 // Do something interesting
        STATE_CHASE,                // Chase your target
        STATE_ATTACKING,            // Dive at target and cause pain
    };


    state       m_State;
    vector3     m_Position;
    vector3     m_OldPosition;
    vector3     m_DesiredPosition;
    f32         m_CollRadius;
    vector3     m_Velocity;
    xbool       m_bHasLanded;
    vector3     m_GroundNormal;
    f32         m_JumpDelay;

    vector3     m_Gravity;

    f32         m_DeltaTime;
    f32         m_LifeSpan;
    f32         m_Age;
    f32         m_AirTime;
    f32         m_JumpStartAltitude;
    guid        m_TargetGuid;

    f32         m_RenderRadius;
    vector3     m_DesiredRenderPosition;
    vector3     m_RenderPosition;
    f32         m_RadiusAnimRate;
    f32         m_JumpDelayBaseTime;

    f32         m_InvulnerableTime;
    s32         m_Stage;   

    guid        m_gGroupContainer;


    vector3     m_TargetPosOnPlane;
    vector3     m_JumpDest;

    zone_mgr::tracker   m_ZoneTracker;
    attack_data         m_AttackData;
    static s32          m_kInAirCount;

    floor_properties    m_FloorProperties;
    //DECAL rhandle<decal_package>  m_hDecalPackage;
    //
    //  Audio
    //
    s32                 m_SoundIDActive;
    s32                 m_SoundIDChargeup;
    s32                 m_SoundIDAttack;
    s32                 m_SoundIDStageDestroyed;
    s32                 m_SoundIDFinalDestruction;
    voice_id            m_ActiveVoiceID;

    //
    //  its thinking...
    //
    guid                m_LeaderGuid;
    vector3             m_LastLeaderPosition;    

    //
    //  Rendering data
    //
    fx_handle           m_hPainSplatFX;
    fx_handle           m_hNucleusFX;
    //rigid_inst          m_MembraneGeom;
    fx_handle           m_hEndOfLifeFX;             // one of Detah, attack 
    xcolor              m_Color;
    xcolor              m_DesiredColor;

    u32                 m_bCommitedToAttack:1,
                        m_bMustJumpImmediately:1,
                        m_bConnectedWithAttack:1,
                        m_bActive:1,
                        m_bFleeFromTarget:1,
                        m_bThinking:1,
                        m_bIsLeader:1,
                        m_bEndOfLife:1,             // If set, the glob is dead and is playing a death effect
                        m_bInvulnerable:1;             


    guid                m_gNotifyOnSpawn;
};

//=============================================================================
//=============================================================================
//=============================================================================

class alien_glob_mgr
{
public:

    //==-------------------------------------------------------------

    enum glob_audio
    {
        GLOB_AUDIO_ALERT        = 0,
        GLOB_AUDIO_IDLE,
        GLOB_AUDIO_SPAWN,
        GLOB_AUDIO_ATTACK,
        GLOB_AUDIO_DEATH,
        GLOB_AUDIO_SCATTER,
        GLOB_AUDIO_FLEE,
        GLOB_AUDIO_LAND,
        GLOB_AUDIO_JUMP,
        GLOB_NUM_AUDIO_TYPES,        
    };

    //==-------------------------------------------------------------
    //==-------------------------------------------------------------

    alien_glob_mgr();

    //==-------------------------------------------------------------
    
    void        Advance                 ( f32 DeltaTime     );

    xbool       RequestJumpPermission   ( alien_glob* pGlob );
    void        ConfirmLanding          ( alien_glob* pGlob );
    void        RequestAudio            ( alien_glob* pGlob, glob_audio Type );

    //==-------------------------------------------------------------
    //==-------------------------------------------------------------


    static const char*      m_pAudioIdentifier[ GLOB_NUM_AUDIO_TYPES ];

    enum
    {
        MAX_AUDIO_VOICES            = 3,
    };


    //==-------------------------------------------------------------
    //==-------------------------------------------------------------
    //==-------------------------------------------------------------

    s32             m_nMaxJumping;
    alien_glob*     m_pJumping[ 32 ];


    //==-------------------------------------------------------------
    //  AUDIO MEMBERS
    //==-------------------------------------------------------------
    struct voice_data
    {
        voice_id    VoiceID[ MAX_AUDIO_VOICES ];
        vector3     Position[ MAX_AUDIO_VOICES ];
    };
    voice_data  m_VoiceData[ GLOB_NUM_AUDIO_TYPES ];

    //==-------------------------------------------------------------

    guid            m_CurrentThinker;
};

extern alien_glob_mgr g_AlienGlobMgr;

//=============================================================================
// END
//=============================================================================
#endif// ALIEN_GLOB_HPP
