//=============================================================================
//  MUTAGEN_RESERVOIR.HPP
//=============================================================================

#ifndef MUTAGEN_RESERVOIR_HPP
#define MUTAGEN_RESERVOIR_HPP

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "objects\render\rigidinst.hpp"

//=============================================================================
// DEFINES
//=============================================================================

//=============================================================================
class mutagen_reservoir : public object
{
public:
    CREATE_RTTI( mutagen_reservoir, object, object );

//=============================================================================
						        mutagen_reservoir   ();
	                           ~mutagen_reservoir   ();

    virtual bbox                GetLocalBBox        ( void ) const;      
    virtual s32                 GetMaterial         ( void ) const;
    virtual void                OnInit              ( void );

    virtual void                OnAdvanceLogic      ( f32 DeltaTime );
    virtual void                OnRender            ( void );

    virtual const object_desc&  GetTypeDesc         ( void ) const;
    static  const object_desc&  GetObjectType       ( void );

//=============================================================================
protected:

    rigid_inst      m_RigidInst;
};

//=============================================================================
// END
//=============================================================================
#endif// mutagen_reservoir_HPP
