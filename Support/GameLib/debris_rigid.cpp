//=============================================================================
//  DEBRIS_RIGID.CPP 
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================

#include "Debris\debris_rigid.hpp"
#include "e_Draw.hpp"


//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_rigid_desc : public object_desc
{
    debris_rigid_desc( void ) : object_desc( 
        object::TYPE_DEBRIS_RIGID, 
        "Debris_Rigid",
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC )
    {}
    
    virtual object* Create          ( void )
    {
        return new debris_rigid;
    }
    
} s_debris_rigid_desc ;

//=============================================================================================

const object_desc&  debris_rigid::GetTypeDesc ( void ) const
{
    return s_debris_rigid_desc;
}

//=============================================================================================

const object_desc&  debris_rigid::GetObjectType ( void )
{
    return s_debris_rigid_desc;
}

//=============================================================================

debris_rigid::debris_rigid(void) :
    debris()
{
    m_VMeshMask = 0;
    m_VMeshMask = ~m_VMeshMask;
}

//=============================================================================

debris_rigid::~debris_rigid()
{
}

//=============================================================================

void debris_rigid::OnInit          ( void )
{
//    m_RigidInst.OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pFileName ) );
}

//=============================================================================

void debris_rigid::Create( const char*            rigidFileName,
                           const vector3&         startingPosition,
                           const vector3&         startingVelocity,
                           f32                    lifeSpan,
                           xbool                  bBounces )
{
    m_RigidInst.SetUpRigidGeom( rigidFileName );  //.SetUpRigidGeom( rigidFileName  );

    static radian InitSpinRate = R_60;
    m_Spin = radian3(x_frand(-InitSpinRate,InitSpinRate), x_frand(-InitSpinRate,InitSpinRate),x_frand(-InitSpinRate,InitSpinRate));

    debris::Create( NULL, startingPosition, startingVelocity, lifeSpan, bBounces );
}

//=============================================================================

void debris_rigid::Create( rigid_inst&            Inst,
                           const vector3&         startingPosition,
                           const vector3&         startingVelocity,
                           f32                    lifeSpan,
                           xbool                  bBounces )
{
    xarray<prop_container>  ContainerList;
    Inst.OnCopy( ContainerList );
    m_RigidInst.OnPaste( ContainerList );


    m_Spin = radian3(x_frand(-R_20,R_20), x_frand(-R_20,R_20),x_frand(-R_20,R_20));

    debris::Create( NULL, startingPosition, startingVelocity, lifeSpan, bBounces );
}

//=============================================================================

bbox debris_rigid::GetLocalBBox ( void ) const
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    
    return( bbox( vector3( 20.0f, 20.0f, 20.0f),
                  vector3(-20.0f,-20.0f,-20.0f) ) );
}

//=============================================================================

void debris_rigid::OnRender ( void )
{
    CONTEXT("debris_rigid::OnRender");
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        const matrix4& L2W = GetL2W();

        ASSERT( pRigidGeom->m_nBones <= 1 );
        m_RigidInst.SetVMeshMask( m_VMeshMask );
        m_RigidInst.Render( &L2W, Flags, m_RigidInst.GetLODMask(L2W) );
    }
}

//=============================================================================

void debris_rigid::UpdatePhysics   ( f32 DeltaTime )
{
    if(m_Inactive )
        return;
    CONTEXT("debris_rigid::UpdatePhysics");
    //update the spin
    f32 fTimeSpeed = DeltaTime * 4.0f;

    if(m_Velocity.LengthSquared() > 35.0f*35.0f )
    {
        m_TotalSpin += radian3(m_Spin.Pitch * fTimeSpeed, m_Spin.Yaw * fTimeSpeed, m_Spin.Roll * fTimeSpeed);
    }
    
    debris::UpdatePhysics(DeltaTime);
}

//=============================================================================




















