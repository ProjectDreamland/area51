//=========================================================================
// MUTANT CONTAGION CHARGE PROJECTILE
//=========================================================================

#ifndef MUTANT_CONTAGION_PROJECTILE_HPP
#define MUTANT_CONTAGION_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"
#include "Objects/NetProjectile.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//=========================================================================

#define MAX_CONTAGION 16

class contagion;
class mutant_contagion_projectile;

//=========================================================================

class contagion
{
public:
    contagion   ( void );
    ~contagion  ( void );

    void    BeginArming     ( const vector3& SourcePos, const vector3& ArmedPos, const vector3& LaunchNormal );
    void    BeginArming     ( contagion* pBranch, f32 BranchT );
    xbool   IsArmed         ( void );
    void    SetTarget       ( guid TargetGuid );
    void    SetTarget       ( const vector3& TargetPos );
    void    BeginAttack     ( f32 ContagionAttackTime );
    void    AdvanceLogic    ( f32 DeltaTime );
    const vector3& GetPosition( void ) { return m_CurrentPos; }
    vector3 ComputeArmPoint ( const vector3& StartPos, const vector3& Normal );
    vector3 ComputePositionAtT  ( f32 T );
    vector3 GetBaseDirection    ( void );
    void    Render          ( void );

protected:
    void    UpdateArming    ( f32 DeltaTime );
    void    BeginArmedIdle  ( void );
    void    UpdateArmedIdle ( f32 DeltaTime );
    void    UpdateAttack    ( f32 DeltaTime );
    void    ComputeSplinePathFromNormal( const vector3& StartPos, const vector3& EndPos, const vector3& Normal );
    void    UpdateTargetPosition( void );

public:
    
    enum contagion_state
    {
        STATE_INACTIVE,
        STATE_ARMING,
        STATE_WAITING_TO_BRANCH,
        STATE_ARMED_IDLE,
        STATE_ATTACKING,
    };

    contagion_state     m_State;
    vector3             m_StartPos;
    vector3             m_EndPos;
    vector3             m_StartVelocity;
    vector3             m_EndVelocity;
    f32                 m_SplineRate;
    f32                 m_SplineT;
    vector3             m_CurrentPos;
    vector3             m_PrevPos;
    guid                m_TargetGuid;
    vector3             m_TargetPos;
    contagion*          m_pBranch;
    f32                 m_BranchT;
    mutant_contagion_projectile*    m_pContagionProjectile;
    fx_handle           m_FXHandle;
    xcolor              m_Color;
};

//=========================================================================

class mutant_contagion_projectile : public net_proj
{
public:
	CREATE_RTTI( mutant_contagion_projectile , net_proj , object )

	mutant_contagion_projectile();
	virtual ~mutant_contagion_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================

                void            Setup               ( guid              OriginGuid,
                                                      s32               OriginNetSlot,
                                                      const vector3&    Position,
                                                      const radian3&    Orientation,
                                                      const vector3&    Velocity,
                                                      s32               Zone1,
                                                      s32               Zone2,
                                                      pain_handle       PainHandle);

virtual         void            SetStart            ( const vector3& Position,
                                                      const radian3& Orientation,
                                                      const vector3& Velocity,
                                                            s32      Zone1,
                                                            s32      Zone2,
                                                            f32      Gravity = 0.0f );



                xbool           LoadEffect          ( const char* pFileName, const vector3& InitPos, const radian3& InitRot );
                void            DestroyParticles    ( void );
                void            UpdateParticles     ( const vector3& Position );
virtual	        bbox	        GetLocalBBox		( void ) const;

virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );

virtual	        void	        OnMove				( const vector3& NewPosition );
virtual         void            OnRender            ( void );
virtual         void            OnRenderTransparent ( void );

virtual         void            OnExplode           ( void );
                xbool	        LoadInstance		( const char* pFileName );
                void            ReleaseProjectile   ( xbool bHoming, f32 scalar );
                void            UpdateProjectile    ( const matrix4& L2W, f32 time );

                void            UpdateTravel        ( void );
                void            BeginArming         ( void );
                void            UpdateArming        ( void );
                void            BeginArmedIdle      ( void );
                void            UpdateArmedIdle     ( void );
                void            BeginAttack         ( f32 ContagionAttackTime );
                void            BeginAttack         ( f32 ContagionAttackTime, guid TargetGuid );
                void            UpdateAttack        ( void );

                xbool           IsArmed             ( void );
                void            TriggerFromWeapon   ( void );
                void            TriggerFromTheta    ( f32 ContagionAttackTime, guid TargetGuid );
static          xbool           IsTargeted          ( guid OriginGuid, guid TargetGuid );

#ifndef X_EDITOR
virtual         void            net_Deactivate      ( void );
#endif

public:

protected:
    enum contagion_projectile_state
    {
        STATE_TRAVEL,
        STATE_ARMING,
        STATE_ARMED_IDLE,
        STATE_ATTACK,
        NUM_STAGES
    };

    contagion_projectile_state m_State;
    f32             m_TimeInState;
    f32             m_DeltaTime;

    xbool           m_bFromTheta;
    guid            m_ThetaTarget;
    f32             m_ThetaContagionAttackTime;

    vector3         m_ImpactNormal;
    contagion       m_Contagion[ MAX_CONTAGION ];

    bbox            m_LocalBBox;
    voice_id        m_FlyVoiceID;

    fx_handle       m_FXHandle;
};

//=========================================================================
// END
//=========================================================================
#endif