#ifndef NAV_CONNECTION2_ANCHOR_HPP
#define NAV_CONNECTION2_ANCHOR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "nav_connection2_editor.hpp"

//=========================================================================
// CLASS
//=========================================================================
class nav_connection2_anchor : public object
{
public:

    CREATE_RTTI( nav_connection2_anchor, object, object )

    enum flags
    {
        FLAG_NONE       = 0x00000000,
        FLAG_DYING      = BIT(0),
        FLAG_MOVING     = BIT(1),

        FLAG_ALL        = 0xFFFFFFFF
    };


                            nav_connection2_anchor                     ( void );
    virtual                 ~nav_connection2_anchor                    ( );
    virtual bbox            GetLocalBBox                        ( void ) const;
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }

    virtual void            OnInit                              ( void );       

    virtual void            Reset                               ( void );

    virtual void            OnEnumProp                          ( prop_enum& List );
    virtual xbool           OnProperty                          ( prop_query& I );

    virtual void            OnMove                              ( const vector3& NewPos );
    
    virtual xbool           HasConnection                       ( guid Connection );
    virtual void            SetConnection                       ( guid thisConnection );

    virtual guid            GetConnection                       ( void );

    virtual u32             GetFlags            ( void )         { return m_Flags;    }
    virtual void            SetFlags            ( u32 flags)     { m_Flags = flags;   }
    
    xbool                   IsDying             ( void )        { return m_Flags & FLAG_DYING; }
    xbool                   IsMoving            ( void )        { return m_Flags & FLAG_MOVING; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );


    virtual void            SetIndexInSavedList( s32 connectionIndex ) { m_IndexInSavedList = connectionIndex;    }
    virtual s32             GetIndexInSavedList( void ) { return m_IndexInSavedList;  }


protected:

    virtual void            OnRender            ( void );
    virtual void            OnLoad              ( text_in& TextIn );
    virtual void            OnKill              ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

//    virtual void            ValidateConnections ( void );

//=========================================================================
//  DATA MEMBERS
//=========================================================================


//  Members saved
    guid                    m_Connection;
    

//  Members not saved
    sphere                  m_Sphere;
    xbool                   m_FirstUpdate;   
    s32                     m_IndexInSavedList;
    u32                     m_Flags;
};

//=========================================================================
// END
//=========================================================================
#endif//NAV_CONNECTION2_ANCHOR_HPP