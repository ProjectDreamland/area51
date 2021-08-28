//=============================================================================
//  DEBRIS_RIGID.HPP 
//=============================================================================
#ifndef DEBRIS_RIGID_HPP
#define DEBRIS_RIGID_HPP

//=============================================================================
// INCLUDES
//=============================================================================
#include "debris\debris.hpp"

//=============================================================================
class debris_rigid : public debris
{
public:
    CREATE_RTTI( debris_rigid, debris, object )

//=============================================================================
						debris_rigid(void);
	virtual				~debris_rigid();

    virtual void        OnInit          ( void );

    virtual bbox        GetLocalBBox    ( void ) const;
    virtual s32         GetMaterial     ( void ) const { return MAT_TYPE_CONCRETE;}
    virtual void        Create          ( const char*            rigidFileName,
                                          const vector3&         startingPosition,
                                          const vector3&         startingVelocity,
                                          f32                    lifeSpan,
                                          xbool                  bBounces = FALSE );

    virtual void        Create          ( rigid_inst&            Inst,
                                          const vector3&         startingPosition,
                                          const vector3&         startingVelocity,
                                          f32                    lifeSpan,
                                          xbool                  bBounces = FALSE );

            void        SetVMeshMask    ( u32 MeshMask );

    virtual void        OnRender        ( void );
    virtual void        UpdatePhysics   ( f32 DeltaTime );

	virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual render_inst* GetRenderInstPtr       ( void ) { return &m_RigidInst; }

//=============================================================================
protected:

    rigid_inst          m_RigidInst;
    u32                 m_VMeshMask;
};

//=============================================================================

inline
void debris_rigid::SetVMeshMask( u32 VMeshMask )
{
    m_VMeshMask = VMeshMask;
}

//=============================================================================
// END
//=============================================================================
#endif//DEBRIS_RIGID_HPP
 