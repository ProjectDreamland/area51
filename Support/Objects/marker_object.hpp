///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  marker_object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef marker_object_hpp
#define marker_object_hpp

///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Obj_mgr\obj_mgr.hpp"

#ifdef X_EDITOR
#include "Objects\Render\SkinInst.hpp"
#include "Loco\LocoCharAnimPlayer.hpp"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////////////////////////////

class marker_object : public object
{
public:

    CREATE_RTTI( marker_object, object, object )

                                marker_object        ( void );
                                ~marker_object       ( void );
                            
    virtual         bbox        GetLocalBBox    ( void ) const;
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    virtual         s32         GetMaterial     ( void ) const { return MAT_TYPE_NULL; }

#ifdef X_EDITOR
    virtual			void	    OnEnumProp		( prop_enum& List );
    virtual			xbool	    OnProperty		( prop_query& I );
    virtual         void        OnMove          ( const vector3& NewPos   );      
    virtual         void        OnTransform     ( const matrix4& L2W      );
    virtual         void        OnRender        ( void );
    
    virtual         render_inst*        GetRenderInstPtr      ( void );
    virtual         anim_group::handle* GetAnimGroupHandlePtr ( void );
    
                    void        ResetAnimPlayer  ( f32 Frame );
                    void        AdvanceAnimPlayer( f32 DeltaTime );
    
#endif

protected:

#ifdef X_EDITOR
    // Animation debug info
    skin_inst               m_SkinInst;         // Render instance
    loco_char_anim_player   m_AnimPlayer;       // Animation player
    anim_group::handle      m_hAnimGroup;       // Anim group handle
    s32                     m_AnimGroupName;    // Name of animation group
    s32                     m_AnimName;         // Name of animation to play
    s32                     m_AnimFrame;        // Frame of animation to display
    xbool                   m_bAnimate;         // Animate or show static frame
#endif

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif//marker_object_hpp