//=============================================================================
//  DEBRIS
//=============================================================================

#ifndef __DEBRIS_ALIEN_GRENADE_EXPLOSION_HPP__
#define __DEBRIS_ALIEN_GRENADE_EXPLOSION_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "debris_cannon.hpp"
#include "Tracers\TracerMgr.hpp"

//=============================================================================
class debris_alien_grenade_explosion : public debris_cannon
{
public:
    CREATE_RTTI( debris_alien_grenade_explosion, debris_cannon, object )


        //=============================================================================
                         debris_alien_grenade_explosion( void );
    virtual				~debris_alien_grenade_explosion( void );

    virtual void        Create                ( const char*       pMeshName,
                                                const vector3&    Pos,
                                                u32               Zones,
                                                const vector3&    Dir,
                                                s32               nFragments, 
                                                guid              OriginGuid );


    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    virtual void        OnRenderTransparent ( void );

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );

    //==-----------------------------------------------------------------------
    //  STRUCTURES
    //==-----------------------------------------------------------------------
protected:

    //==-----------------------------------------------------------------------
    //  FUNCTIONS
    //==-----------------------------------------------------------------------
protected:
    void        ReinitializeFragmentsForExplode(    s32           nFragments,
                                                    f32           MinSpeed,
                                                    f32           MaxSpeed,
                                                    f32           FaceShotPercentage,
                                                    f32           SlowTrailerThreshold,
                                                    f32           ABPercentage,
                                                    xbool         bTypeASuspendOnRest,
                                                    s32           MaxTypeA,
                                                    const char*   TypeAFXName,
                                                    xbool         bTypeBSuspendOnRest,
                                                    s32           MaxTypeB,
                                                    const char*   TypeBFXName,
                                                    f32           FastTrailerThreshold,
                                                    f32           FastPercentage,
                                                    xbool         bFastSuspendOnRest,
                                                    s32           MaxFastTrailer,
                                                    const char*   FastFXName );

    //==-----------------------------------------------------------------------
    //  OVERRIDABLES
    //==-----------------------------------------------------------------------
protected:
    virtual void        UpdateFragments     ( f32 DeltaTime );
    virtual void        OnFragmentCollide   ( s32 iFragment, collision_mgr::collision& Col );

    //==-----------------------------------------------------------------------
    //  DATA
    //==-----------------------------------------------------------------------
protected:
    enum mode 
    {
        MODE_EXPAND     = 0,
        MODE_HOLD       = 1,
        MODE_COLLAPSE   = 2,
        MODE_EXPLODE    = 3,
    };

    struct fragment_ex
    {
        f32         m_CollapseTimer;
        f32         m_MaxDist;
        tracer_id   m_TracerID;
    };

    fx_handle   m_CoreFX[ 4 ];
    
    f32         m_CoreSize;
    fragment_ex m_FragmentEx[ MAX_FRAGMENTS ];
    mode        m_Mode;
    f32         m_Speed;
    f32         m_ClampDistance;
    f32         m_CurrentDistance;
    vector3     m_Origin;
    vector3     m_HitPlaneNormal;
    f32         m_ModeTimer;
    f32         m_SpinRateController;
    s32         m_iCollisionSetupIndex;         // Where we left off doing the collision setups
    guid        m_OriginGuid;

};


#endif // __DEBRIS_ALIEN_GRENADE_EXPLOSION_HPP__