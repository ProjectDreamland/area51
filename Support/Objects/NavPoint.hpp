//=========================================================================
//
// NavPoint
//
//=========================================================================

#ifndef NAV_POINT_HPP
#define NAV_POINT_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"


//=========================================================================
// CLASS
//=========================================================================

class nav_point : public object
{
public:

    //---------------------------------------------------------------------
    CREATE_RTTI( nav_point, object, object )

                            nav_point                   ( void );
    virtual                 ~nav_point                  ( void );
    virtual s32             GetMaterial                 ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnRender                    ( void );
    virtual bbox            GetLocalBBox                ( void ) const;      
    virtual void            OnActivate                  ( xbool Flag );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
	virtual	void	            OnEnumProp		        ( prop_enum& list );
	virtual	xbool	            OnProperty		        ( prop_query& rPropQuery );

protected:
    xbool                   m_Active;
    guid                    m_TargetGuid;
};

//=========================================================================
// END
//=========================================================================
#endif