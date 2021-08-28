//==============================================================================
//
//  InputSetting.hpp
//
//==============================================================================

#ifndef INPUTSETTING_HPP
#define INPUTSETTING_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_mgr\Obj_mgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class input_setting : public object
{
public:
    
    CREATE_RTTI( input_setting, object, object )

//==============================================================================

                        input_setting         ( void );
                        ~input_setting        ( void );
                                                                
    virtual void        OnInit			    ( void );

    virtual bbox        GetLocalBBox        ( void ) const;
    virtual s32         GetMaterial         ( void ) const;
    virtual void        OnEnumProp          ( prop_enum&     List     );
    virtual xbool       OnProperty          ( prop_query&    I        );

    virtual const   object_desc&  GetTypeDesc     ( void ) const;
    static  const   object_desc&  GetObjectType   ( void );
                                                                
protected:
   
};

//==============================================================================
#endif // INPUTSETTING_HPP
//==============================================================================

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================

//==============================================================================
inline
bbox input_setting::GetLocalBBox( void ) const
{
    return bbox( vector3(0.0f,0.0f,0.0f), 50.0f );
}
//==============================================================================
inline
s32 input_setting::GetMaterial( void ) const
{
    return MAT_TYPE_NULL;
}
//==============================================================================