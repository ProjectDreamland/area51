//=============================================================================
//  DEBRIS GLASS CLUSTER
//=============================================================================

#ifndef __DEBRIS_GLASS_CLUSTER_HPP__
#define __DEBRIS_GLASS_CLUSTER_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "..\objects\Render\RigidInst.hpp"
#include "..\PainMgr\Pain.hpp"
#include "Dictionary\Global_Dictionary.hpp"

class play_surface;

//=============================================================================
class debris_glass_cluster : public object
{
public:
    CREATE_RTTI( debris_glass_cluster, object, object )

    enum
    {
        MAX_SHARDS          = 200,
    };

        //=============================================================================
             debris_glass_cluster           ( void );
    virtual	~debris_glass_cluster           ( void );

    virtual void        OnInit              ( void );

    virtual void        CreateFromRigidGeom ( play_surface* pPlaySurface, const pain* pPain );

    virtual bbox        GetLocalBBox        ( void ) const;
    virtual s32         GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}

    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        UpdatePhysics       ( f32 DeltaTime );

    virtual void        OnMove				( const vector3& rNewPos );
    virtual void        OnRender            ( void );
    virtual void        OnRenderTransparent ( void );

    static  const object_desc&  GetObjectType   ( void );

protected:
    void                ProcessCollisions   ( f32 DeltaTime );
    void                TesselateIntoShards ( const play_surface*       pPlaySurface,                                               
                                              object::detail_tri*       pTriStack,
                                              s32                       iRead,
                                              s32                       iWrite,
                                              s32                       iMaxStackSize,
                                              f32*                      pRawTriStartTime );

    virtual const object_desc&  GetTypeDesc     ( void ) const;    

    //=============================================================================
protected:
    struct shard
    {
        vector3         m_Pos;
        radian3         m_SpinRate;        
        radian3         m_TotalSpin;
        vector3         m_Velocity;
        f32             m_Variation;
        f32             m_StartTime;
        u8              m_bCheckAudioCollision:1;
    };

    shard               m_Shards[ MAX_SHARDS ];
    s32                 m_nShards;
    f32                 m_TotalTime;
    pain                m_SourcePain;
    f32                 m_MinStartTime;
    f32                 m_MaxStartTime;

    // Render data
    vector3             m_pLocalPos[ MAX_SHARDS*3 ];
    vector3             m_pPos     [ MAX_SHARDS*3 ];  
    vector2             m_pUV      [ MAX_SHARDS*3 ];
// TODO: ?  u32                 m_pColor   [ MAX_SHARDS*3 ];

    bbox                m_BBox;
};


#endif  // __DEBRIS_GLASS_CLUSTER_HPP__
