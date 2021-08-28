#ifndef MANIPULATOR_HPP
#define MANIPULATOR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Animation\AnimPlayer.hpp"

//=========================================================================
// CLASS
//=========================================================================

class manipulator : public object
{
public:

    CREATE_RTTI( manipulator, object, object )
    
                            manipulator     ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return bbox( vector3(0,0,0), 100 ); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnActivate      ( xbool Flag );  
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 

    virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_hAnimGroup; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================

protected:

    enum
    {
        MAX_ANIMATED_OBJECTS = 64
    };

    enum
    {
        FLAGS_ACTIVE                = (1<<0),
        FLAGS_DESTROY_AFTER_PLAYING = (1<<1),
        FLAGS_LOOP                  = (1<<2),
        //FLAGS_LOOP, // etc.
    };

protected:

    virtual void            OnAdvanceLogic	( f32 DelaTime );
    virtual void            OnInit          ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender   ( void );
#endif // X_RETAIL

            void            UpdateObjects   ( void );
            void            UpdateAnimL2W   ( void );
    
protected:

    anim_group::handle          m_hAnimGroup;
    rhandle<char>               m_hAudioPackage;
    simple_anim_player          m_Anim;

    s32                         m_nGuids;
    guid                        m_Guid[ MAX_ANIMATED_OBJECTS ];
    u32                         m_Flags;
};

//=========================================================================
// END
//=========================================================================
#endif
