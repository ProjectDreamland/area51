//=============================================================================
//  DEBRIS
//=============================================================================

#ifndef __DEBRIS_FRAG_EXPLOSION_HPP__
#define __DEBRIS_FRAG_EXPLOSION_HPP__

//=============================================================================
// INCLUDES
//=============================================================================
#include "obj_mgr\obj_mgr.hpp"
#include "debris_cannon.hpp"

//=============================================================================
class debris_frag_explosion : public debris_cannon
{
public:
    CREATE_RTTI( debris_frag_explosion, debris_cannon, object )


        //=============================================================================
                         debris_frag_explosion( void );
    virtual				~debris_frag_explosion( void );

    virtual void        Create                ( const char*       pMeshName,
                                                const vector3&    Pos,
                                                u32               Zones,
                                                const vector3&    Dir,
                                                s32               nFragments );


    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );


    //==-----------------------------------------------------------------------
    //  STRUCTURES
    //==-----------------------------------------------------------------------
protected:

    //==-----------------------------------------------------------------------
    //  FUNCTIONS
    //==-----------------------------------------------------------------------
protected:

    //==-----------------------------------------------------------------------
    //  OVERRIDABLES
    //==-----------------------------------------------------------------------
protected:

    //==-----------------------------------------------------------------------
    //  DATA
    //==-----------------------------------------------------------------------
protected:
    static rhandle<decal_package>   s_hDecalPkg;
};


#endif // __DEBRIS_CANNON_HPP__