//=========================================================================
// MUTANT TENDRIL PROJECTILE
//=========================================================================

#ifndef MUTANT_TENDRIL_PROJECTILE_HPP
#define MUTANT_TENDRIL_PROJECTILE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "BaseProjectile.hpp"
#include "Objects/NetProjectile.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Animation\SMemMatrixCache.hpp"


#define MAX_SEGMENTS 9

class actor;

//=========================================================================

class mutant_tendril_projectile : public net_proj
{
public:
	CREATE_RTTI( mutant_tendril_projectile , net_proj , object )

	mutant_tendril_projectile();
	virtual ~mutant_tendril_projectile();

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//============================================================   =============

                void            Setup               ( guid              OriginGuid,
                                                      s32               OriginNetSlot,
                                                      const vector3&    Position,
                                                      const radian3&    Orientation,
                                                      const vector3&    Velocity,
                                                      s32               Zone1,
                                                      s32               Zone2,
                                                      pain_handle       PainHandle,
                                                      xbool             bLeft);

virtual         void            SetStart            ( const vector3& Position,
                                                      const radian3& Orientation,
                                                      const vector3& Velocity,
                                                            s32      Zone1,
                                                            s32      Zone2,
                                                            f32      Gravity = 0.0f );



                void            RetractTendril      ( const guid& Owner, xbool bLeft );
                xbool           LoadEffect          ( const char* pFileName, const vector3& InitPos, const radian3& InitRot );
                void            DestroyParticles    ( void );
                void            UpdateParticles     ( const vector3& Position );
virtual	        bbox	        GetLocalBBox		( void ) const;

virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
virtual         void            OnImpact            ( collision_mgr::collision& Coll, object* pTarget );

virtual         void            OnRender            ( void );
virtual         void            OnMove              ( const vector3& NewPos );

virtual         void            OnExplode           ( void );
                xbool	        LoadInstance		( const char* pFileName ); 

virtual	        void	        OnEnumProp		    ( prop_enum& PropEnumList );
virtual	        xbool	        OnProperty		    ( prop_query& rPropQuery );

                void            SetTarget           ( guid TargetGuid ) { m_Target = TargetGuid; } 
                void            SetCorpseGuid       ( guid CorpseGuid ) { m_CorpseGuid = CorpseGuid; }
                guid            GetTarget           ( void )            { return m_Target; }
                guid            GetCorpseGuid       ( void )            { return m_CorpseGuid; }
                xbool           GetBeginRetract     ( void )            { return m_bRetractTendrils; }                

                xbool           GetTendrilAttachLocation( actor *pFromActor, matrix4 &L2W, vector3 &BonePos, s32 &iBone, xbool bLeft );
                xbool           DidHitBody          ( void ) { return m_bHitBody; }                

#ifndef X_EDITOR
virtual         void            net_Deactivate      ( void );
#endif

protected:
    virtual vector3 GetAimAtPosition    ( void );
    virtual vector3 GetTargetPoint      ( object* pTarget );
    virtual void    CausePain           ( collision_mgr::collision& Coll, object* pObject );
            void    FakePain            ( object* pObject );
    virtual void    OnColCheck          ( void );
            void    UpdateSegments      ( object *pOwner, f32 DeltaTime );

protected:
    guid            m_FlyFXGuid;
    guid            m_LaunchFXGuid;
    guid            m_AimAtBone;
    f32             m_DeltaTime;

    vector3         m_ImpactNormal;
    bbox            m_LocalBBox;
    voice_id        m_FlyVoiceID;
    fx_handle       m_FXHandle;

    // tendrils
    f32             m_CollapseTime;
    xbool           m_bEnterBody;
    xbool           m_bRetractTendrils;
    skin_inst       m_SkinInst;
    vector3         m_SegmentPositions[MAX_SEGMENTS];
    s32             m_AttachBone;
    s32             m_iBone;
    guid            m_Target;
    guid            m_CorpseGuid;    
    xbool           m_bLeft;                // are we the left tendril?    
    vector3         m_BonePosition;         // where is our bone in the world
    xbool           m_bHitBody;             // has this tendril hit a target yet?
    guid            m_HitEffect;            // Guid of the effect trail
    f32             m_LastDistance;         // Keep track of distance in case we move closer to victim so we can recoil tendril
    f32             m_SplineT;
    f32             m_SplineRate;
    f32             m_SplineTime;           // Keep track of deltatime

    rhandle<char>   m_hParticleHit;

    f32             m_HackTendrilTime;      // this is a hack to time tendril retraction event
    xbool           m_HackTendrilRetract;   // flag to know hack is in effect (overrides initial m_RetractTendrils flag)
    
    rhandle<char>   m_hInfectParticles;
    smem_matrix_cache m_SMEM_Mat_Cache;
};

//=========================================================================
// END
//=========================================================================
#endif