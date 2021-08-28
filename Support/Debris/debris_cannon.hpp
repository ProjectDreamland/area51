//=============================================================================
//  DEBRIS
//=============================================================================

#ifndef __DEBRIS_CANNON_HPP__
#define __DEBRIS_CANNON_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "..\objects\Render\RigidInst.hpp"
#include "..\PainMgr\Pain.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "..\Objects\Player.hpp"

//=============================================================================
class debris_cannon : public object
{
public:
    CREATE_RTTI( debris_cannon, object, object )


        //=============================================================================
                         debris_cannon             ( void );
    virtual				~debris_cannon             ( void );

    virtual void        OnInit              ( void );

    virtual void        Create              ( const char*       pMeshName,
                                              const vector3&    Pos,
                                              const vector3&    Dir,
                                              s32               nFragments );

    virtual bbox        GetLocalBBox        ( void ) const;
    virtual s32         GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );

    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnRender            ( void );
    virtual void        OnRenderTransparent ( void );

    //==-----------------------------------------------------------------------
    //  STRUCTURES
    //==-----------------------------------------------------------------------
protected:

    enum
    {
        MAX_FRAGMENTS                   = 32,
    };

    enum sound
    {
        SOUND_KICKOFF,
        SOUND_IMPACT,
    };
    
    struct fragment
    {
        fragment();
        ~fragment();

        void Reset( void );

        vector3             m_Position;             // current position        
        vector3             m_Velocity;             // current velocity
        quaternion          m_Orientation;
        vector3             m_RotationAxis;
        radian              m_RotationAmount;
        vector3             m_OldPosition;          // position last frame
        
        u64                 m_MeshMask;             // mask for submesh selection
        fx_handle           m_FX;                   // trailing fx, not mandatory
        s32                 m_BounceCount;          // How many bounces?
        
        u8                  m_bInUse:1,             // Available or not        
                            m_bInactive:1,
                            m_bSuspendFXOnRest:1;            // Inactive = in use, but not moving.
    };

    //==-----------------------------------------------------------------------
    //  FUNCTIONS
    //==-----------------------------------------------------------------------
protected:
            void        DoCollisions    ( f32 DeltaTime );
            s32         GetFreeFragment ( void );
            void        ReleaseFragment ( s32 iFragment );
            void        SetupFlyby      ( s32 iFragment, const vector3& PlayerPos, player* pPlayer );
            void        UpdateFlyby     ( f32 DeltaTime );

            void        InitializeFragmentsForPlayerDirectedExplosion ( const vector3&  Pos,
                                                                        const vector3&  Dir,
                                                                        s32             nFragments,
                                                                        f32             MinSpeed,
                                                                        f32             MaxSpeed,
                                                                        f32             FaceShotPercentage,
                                                                        f32             SlowTrailerThreshold,
                                                                        f32             ABPercentage,
                                                                        xbool           bTypeASuspendOnRest,
                                                                        s32             MaxTypeA,
                                                                        const char*     TypeAFXName,
                                                                        xbool           bTypeBSuspendOnRest,
                                                                        s32             MaxTypeB,
                                                                        const char*     TypeBFXName,
                                                                        f32             FastTrailerThreshold,
                                                                        f32             FastPercentage,
                                                                        xbool           bFastSuspendOnRest,
                                                                        s32             MaxFastTrailer,
                                                                        const char*     FastFXName );

            void        CauseShellshock ( f32 MinDist, f32 MaxDist );

    //==-----------------------------------------------------------------------
    //  OVERRIDABLES
    //
    // UpdateFragments - override if you want to handle how they move through
    //                   worldspace without worrying about collision
    //
    // OnFragmentCollide - override if you want to handle everything about
    //                     how a fragment responds to collision with the world
    //
    // OnBounce - override if you want to be notified when a fragment has bounced
    //
    // PlaySound - override if you want to change how sounds are played
    //
    //==-----------------------------------------------------------------------
protected:

    virtual void        UpdateFragments     ( f32 DeltaTime );
    virtual void        OnFragmentCollide   ( s32 iFragment, collision_mgr::collision& Col );
    virtual void        OnBounce            ( s32 iFragment );
    virtual void        PlaySound           ( s32 iFragment, sound iSound, f32 Volume = 1.0f );
        
    //==-----------------------------------------------------------------------
    //  DATA
    //==-----------------------------------------------------------------------
protected:

    fragment            m_Fragment[ MAX_FRAGMENTS ];
    s32                 m_nFragmentsUsed;
    f32                 m_TotalTime;
    bbox                m_LocalBBox;
    rigid_inst          m_RigidInst;
    voice_id            m_FlybyVoiceID;
    vector3             m_FlybyStart;
    vector3             m_FlybyEnd;
    f32                 m_FlybyTimer;           // How far into sound
    f32                 m_FlybyLength;          // Length of sound
    f32                 m_FadeTimer;            // Counts down to death
    u8                  m_Alpha;                // < 255 = fading out
    u32                 m_bFlybyPerformed:1,
                        m_bFlybyActive:1;

};


#endif // __DEBRIS_CANNON_HPP__