///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  spawn_point.hpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef spawn_point_hpp
#define spawn_point_hpp

///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Circuit.hpp"

#include "Obj_mgr\obj_mgr.hpp"
#include "Dictionary\global_dictionary.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////////////////////////////

class spawn_point : public object
{
public:

    CREATE_RTTI( spawn_point, object, object )

                                spawn_point     ( void );
                               ~spawn_point     ( void );
                            
    virtual         bbox        GetLocalBBox    ( void ) const;
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    virtual         s32         GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
                    void        OnEnumProp      ( prop_enum&  List );
                    xbool       OnProperty      ( prop_query& Query );

                    circuit&    GetCircuit     ( void ) { return m_Circuit; }

                    guid        GetSpawnInfo    ( guid     ActorGuid, 
                                                  vector3& Position, 
                                                  radian3& Rotation, 
                                                  u16&     Zone1, 
                                                  u16&     Zone2 ) const;

#ifdef X_EDITOR
                    virtual         s32         OnValidateProperties    ( xstring& ErrorMsg );
#endif

protected:
                    circuit     m_Circuit;   
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif//spawn_point_hpp