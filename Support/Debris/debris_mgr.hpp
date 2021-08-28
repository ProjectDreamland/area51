//=============================================================================
//  DEBRIS_MGR.HPP 
//=============================================================================

#ifndef DEBRIS_MGR_HPP
#define DEBRIS_MGR_HPP

//=============================================================================
// INCLUDES
//=============================================================================

#include "Debris\debris.hpp"
#include "Objects\Render\RigidInst.hpp" 

//=============================================================================
// EXTERNALS
//=============================================================================
class prop_surface;
class play_surface;

//=============================================================================
class debris_mgr
{
public:

    //-------------------------------------------------------------------------
    enum debris_set
    {
        DEBRIS_SET_FLESH = 0,
        DEBRIS_SET_MECHANICAL,
        DEBRIS_SET_GLASS,
        DEBRIS_SET_ELECTRONICS,

        DEBRIS_SET_LAST        
    
    };

    //-------------------------------------------------------------------------

						debris_mgr              ( void );
	virtual				~debris_mgr             ( void );

    static  debris_mgr* GetDebrisMgr            ( void );
    static  void        ClearData               ( void );

    //-------------------------------------------------------------------------
    
    virtual void        CreateDebrisFromObject  ( guid          thisGuid, 
                                                  s32           numberOfDebrisParticles, 
                                                  f32           energyLevel );

    virtual void        CreateDebrisFromObject  ( rigid_inst&   Inst,
                                                  guid          thisGuid, 
                                                  s32           numberOfDebrisParticles, 
                                                  f32           energyLevel );

    virtual void        CreateGlassFromRigidGeom( play_surface* thisRigidGeom, 
                                                  const pain*   thisPain );
    
    //-------------------------------------------------------------------------

    virtual void        CreateDebris            ( const vector3&    InitPos, 
                                                  u32               Zones,
                                                  const vector3&    InitVelocity, 
                                                  const char*       rigidFileName, 
                                                  f32               Life = 6.0f,
                                                  xbool             Bounce = TRUE, 
                                                  u32               VMeshMask = U32_MAX,
                                                  s32               BounceSoundID = -1 );

    virtual void        CreateDebris            ( const vector3&    InitPos, 
                                                  u32               Zones,
                                                  const vector3&    InitVelocity, 
                                                  rigid_inst&       Inst, 
                                                  f32               Life = 6.0f,
                                                  xbool             Bounce = TRUE, 
                                                  u32               VMeshMask = U32_MAX );

    virtual void        CreateDebris            ( const vector3&    InitPos, 
                                                  u32               Zones,
                                                  const vector3&    InitVelocity, 
                                                  const radian3&    RandRot,       
                                                  rigid_inst&       Inst, 
                                                  f32               Life,
                                                  xbool             Bounce = TRUE, 
                                                  u32               VMeshMask = U32_MAX );

    virtual void        CreateShell             ( const vector3&    InitPos, 
                                                  u32               Zones,
                                                  const radian3&    InitRot, 
                                                  const char*       rigidFileName, 
                                                  f32               Life = 10.0f,
                                                  u32               VMeshMask = U32_MAX );

            void        CreateSpecializedDebris ( const vector3&    InitPos, 
                                                  const vector3&    InitVelocity, 
                                                  object::type      Type,
                                                  u32               Zones,
                                                  guid              OwnerGuid );

    //-------------------------------------------------------------------------
//    virtual debris*     CreateDebris(   matrix4     baseMatrix,
//                                        f32         baseSpeed,
//                                        radian      baseRotationalSpeed,
//                                        const char* debrisMatx,
//                                        const char* boundSound  );
    //-------------------------------------------------------------------------
protected:
    
    void                CapVelocity             ( vector3&  Velocity );

    static debris_mgr*  m_sThis;

};

//=============================================================================
// INLINES
//=============================================================================
inline
debris_mgr* debris_mgr::GetDebrisMgr( void )
{ 
    if(!m_sThis) new debris_mgr; 
    return m_sThis; 
}

inline
void debris_mgr::ClearData( void )
{ 
    if( m_sThis )
    {
        delete m_sThis;
        m_sThis = NULL;
    }
}

//=============================================================================
// END.
//=============================================================================
#endif//DEBRIS_MGR_HPP
