//=========================================================================
// MUTANT PARASITE CHARGE PROJECTILE
//=========================================================================

#ifndef MUTANT_PARASITE_PROJECTILE_HPP
#define MUTANT_PARASITE_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "ProjectileHoming.hpp"
#include "e_audio.hpp"
#include "Objects/NetProjectile.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//=========================================================================

class mutant_parasite_projectile : public net_proj
{
public:
	CREATE_RTTI( mutant_parasite_projectile , net_proj , object )
    
	mutant_parasite_projectile();
	virtual ~mutant_parasite_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void    Initialize          ( const vector3&    InitPos,
                                          const radian3&    InitRot,
                                          const vector3&    InheritedVelocity,
                                                f32         Speed,
                                                guid        OwnerGuid,
                                                pain_handle PainID,
                                                s32         OwnerNetSlot,
                                                xbool       bContagious );
    virtual void    SetStart            ( const vector3& Position,
                                          const radian3& Orientation,
                                          const vector3& Velocity,
                                                s32      Zone1,
                                                s32      Zone2,
                                                f32      Gravity = 0.0f );

    virtual void    SetTarget           ( guid Target );
    virtual void    OnImpact            ( collision_mgr::collision& Coll, object* pTarget );  
    virtual void    OnExplode           ( void );
    
//=========================================================================

    virtual void    OnAdvanceLogic      ( f32 DeltaTime );
	virtual	void	OnRender			( void );
    virtual	void	OnRenderTransparent ( void );

//------------------------------------------------------------------------------
#ifndef X_EDITOR
//------------------------------------------------------------------------------

    virtual void    net_AcceptActivate  ( const bitstream& BitStream );
    virtual void    net_ProvideActivate (       bitstream& BitStream );

    virtual void    net_AcceptStart     ( const bitstream& BitStream );
    virtual void    net_ProvideStart    (       bitstream& BitStream );

            void    ResyncGhosts        ( void );

//------------------------------------------------------------------------------
#endif // X_EDITOR
//------------------------------------------------------------------------------


protected:
    
            void    SetupType               ( void );
            xbool   HandleCollision         ( collision_mgr::collision& Collision, object* pObject );   
    virtual void    OnColCheck              ( void );              

            void    UpdateTarget            ( f32 DeltaTime     );
            void    GetNewTarget            ( void              );
            f32     ScorePotentialTarget    ( object* pObject   );            

            void    UpdateTrajectory        ( f32 DeltaTime     );
            xbool   UpdateTargetPoint       ( void );               // Returns TRUE if point is an indirect
                                                                    // point to the target

            f32     GetWanderScale          ( void );
            void    CheckInternalCollision  ( f32 DeltaTime );
            vector3 GetWanderVector         ( void );
        
    rhandle<char>   m_hFlyParticles;
    rhandle<char>   m_hLaunchParticles;
    rhandle<char>   m_hInfectParticles;
    rhandle<char>   m_hCollisionParticles;

    enum { AUDIO_IDENTIFIER_LENGTH = 32 };
    rhandle<char>   m_hAudioPackage;    // Audio package for the weapon.
    s32             m_FlyAudioID;
    s32             m_LaunchAudioID;
    s32             m_InfectAudioID;
    s32             m_CollisionAudioID;

    guid            m_LaunchFXGuid;

    voice_id        m_FlyVoiceID;

    xbool           m_CalledSetupType;
    
    ///////////////

    guid            m_Target;               // Who are we trying to hurt
    vector3         m_TargetPoint;          // Where should we be flying to RIGHT NOW    
    vector3         m_DesiredPosition;
    vector3         m_WanderPosition;       // Where the fx should be after accounting for wander
    vector3         m_OldWanderPosition;    
    fx_handle       m_FXFly;    
    f32             m_Age;
    f32             m_MaxAliveTime;
    f32             m_WanderPhase;
    f32             m_WanderOffset;
    xbool           m_bContagious;
    s32             m_TypeIndex;            // Used as an array index. 0=parasite, 1=contagion
    radian          m_MaxRotationSpeed;
    f32             m_BaseSpeed;
    f32             m_ActualSpeed;
    f32             m_DesiredSpeed;

    f32             m_RetargetingTimer;

    f32             m_LastLogicDeltaTime;

    xbool           m_ExplodeEffectCreated;
    f32             m_ExplodeFadeTimer;

    // Flight characteristics
    f32             m_SlowestSpeedMultiplier;
    
};

//=========================================================================
// END
//=========================================================================
#endif //MUTANT_PARASITE_PROJECTILE_HPP