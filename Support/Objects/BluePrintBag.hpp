//==============================================================================
//
//  BluePrintBag.hpp
//
//==============================================================================

#ifndef BLUEPRINT_BAG_HPP
#define BLUEPRINT_BAG_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class blueprint_bag : public object
{
public:

    CREATE_RTTI( blueprint_bag, object, object )

                                blueprint_bag       ( void );
                               ~blueprint_bag       ( void );
                                     
virtual         bbox            GetLocalBBox    ( void ) const;
virtual const   object_desc&    GetTypeDesc     ( void ) const;
static  const   object_desc&    GetObjectType   ( void );
virtual         s32             GetMaterial     ( void ) const;
                void            OnEnumProp      ( prop_enum&  rPropList  );
                xbool           OnProperty      ( prop_query& rPropQuery );

protected:

                s32             m_TemplateIndex[32];
                s32             m_NItems;
};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 blueprint_bag::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // BLUEPRINT_BAG_HPP
//==============================================================================
