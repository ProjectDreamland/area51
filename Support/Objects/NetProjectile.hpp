//==============================================================================
//
//  NetProjectile.hpp
//
//==============================================================================

#ifndef NET_PROJECTILE_HPP
#define NET_PROJECTILE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "NetworkMgr\NetObj.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "ZoneMgr/ZoneMgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class net_proj : public netobj
{
public:

    CREATE_RTTI( net_proj, object, object )

                                net_proj                ( void );
                               ~net_proj                ( void );

                void            SetOrigin               ( guid OriginGuid,
                                                          s32  OriginNetSlot );

virtual         void            SetStart                ( const vector3& Position,
                                                          const radian3& Orientation,
                                                          const vector3& Velocity,
                                                                s32      Zone1,
                                                                s32      Zone2,
                                                                f32      Gravity = 0.0f );

                void            SetOrientation          ( const radian3& Orientation );
                void            SetPosition             ( const vector3& Position,
                                                                s32      Zone1,
                                                                s32      Zone2 );

                void            Setup                   (       guid     OriginGuid,
                                                                s32      OriginNetSlot,
                                                          const vector3& Position,
                                                          const radian3& Orientation,
                                                          const vector3& Velocity,
                                                                s32      Zone1,
                                                                s32      Zone2,
                                                          pain_handle    PainHandle );

                void            SetSticky               ( xbool IsSticky = TRUE );

        const   vector3&        GetStartVelocity        ( void );
                void            ComputePosAndVelAtTimeT ( f32 T );

                void            SetPainHandle           ( pain_handle    PainHandle );
                pain_handle     GetPainHandle           ( void ) { return m_PainHandle; }

virtual         bbox            GetLocalBBox            ( void ) const;
virtual         s32             GetMaterial             ( void ) const;

                u32             GetRenderMode           ( void );
virtual         void            OnColCheck              ( void );
virtual         void            OnMove                  ( const vector3& NewPos );
                void            UpdateZoneTrack         ( void );

virtual         void            OnRender                ( void );
virtual         void            OnRenderTransparent     ( void );
virtual         void            OnAdvanceLogic          ( f32 DeltaTime );

virtual         xbool           GetDoCollisions         ( void )        { return TRUE; }
virtual         void            OnImpact                ( collision_mgr::collision& Coll, object* pTarget );
virtual         void            OnAttachToObject        ( collision_mgr::collision& Coll, object* pTarget );
virtual         void            OnExplode               ( void );

virtual         xbool           CheckHitValidObject     ( object *pObject );
virtual         xbool           CheckHitThreatObject    ( object *pObject );

                guid            GetOriginGuid           ( void ) { return m_OriginGuid; }

//------------------------------------------------------------------------------
#ifndef X_EDITOR
//------------------------------------------------------------------------------

virtual         void            net_Logic               ( f32 DeltaTime );

//rtual         void            net_Activate            ( void );
virtual         void            net_Deactivate          ( void );
                
virtual         void            net_AcceptUpdate        ( const bitstream& BitStream );
virtual         void            net_ProvideUpdate       (       bitstream& BitStream, 
                                                                u32&       DirtyBits );

virtual         void            net_AcceptActivate      ( const bitstream& BitStream );
virtual         void            net_ProvideActivate     (       bitstream& BitStream );

virtual         void            net_AcceptStart         ( const bitstream& BitStream );
virtual         void            net_ProvideStart        (       bitstream& BitStream );

                vector3         net_GetBlendOffset      ( void );

//------------------------------------------------------------------------------
#endif // X_EDITOR
//------------------------------------------------------------------------------

protected:
                matrix4             m_RenderL2W;

                guid                m_OriginGuid;
                s32                 m_OriginNetSlot;
                zone_mgr::tracker   m_ZoneTracker;

                vector3             m_StartPos;
                vector3             m_StartVel;                
                f32                 m_Gravity;                  // If 0, trajectory is linear.
                f32                 m_Age;                      // Overall age.
                f32                 m_TimeT;                    // Time t for movement from 'start'.

                radian3             m_Orientation;

    static      vector3             m_OldPos;   
    static      vector3             m_NewPos;
    static      vector3             m_Velocity;
    static      f32                 m_DeltaTime;

                xbool               m_AtRest;

                xbool               m_Exploded;                 // The projectile has exploded and is waiting to time out
                f32                 m_ExplodedTimer;            // Cooldown timer after explode

                xbool               m_Impact;
                s32                 m_ImpactCount;              // Number of impacts
                vector3             m_ImpactPoint;              // Used for decal generation
                vector3             m_ImpactNormal;             // Used for decal generation

                vector3             m_BlendVector;
                f32                 m_BlendTimer;
                f32                 m_BlendTotalTime;

                xbool               m_Finished;                 // Object is done, wait for FX.
                xbool               m_RenderGeom;
                xbool               m_RenderEffect;
                xbool               m_PlayAudio;

                pain_handle         m_PainHandle;

                // Members for attaching projectiles to objects
                xbool               m_bIsSticky;
                xbool               m_bIsAttached;
                s32                 m_AttachedBoneIndex;
                vector3             m_AttachedLocalPos;
                guid                m_AttachedObjectGuid;

                xbool               m_bIsLargeProjectile;

                f32                 m_BounceFactorRise;
                f32                 m_BounceFactorRun;

                xbool               m_bThroughGlass;            // does this projectile destroy glass and continue on?

            //  render_inst         m_RenderInst; ?
            //  f32                 m_Elasticity;
            //                      m_Audio;
            //  fx_handle           m_Effect;

    enum
    {
        DIRTY_POSITION = 0x00000001,    // Only used when at rest.
        DIRTY_START    = 0x00000002,    // Only used when NOT at rest.
        DIRTY_TIME_T   = 0x00000004,    // Only used when NOT at rest.
        DIRTY_ATTACH   = 0x00000008,    // Only used when NOT at rest.
        DIRTY_EXPLODED = 0x00000010,    // The projectile has exploded
        DIRTY_ALL      = 0x0000001F,
    }; 

};

//==============================================================================
#endif // NET_PROJECTILE_HPP
//==============================================================================
