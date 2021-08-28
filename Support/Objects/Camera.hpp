#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Entropy.hpp"
#include "Tracker.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"

//=========================================================================
// CLASS
//=========================================================================

class camera : public tracker
{
//=====================================================================
// DEFINES
//=====================================================================
public:


//=====================================================================
// PUBLIC BASE CLASS FUNCTIONS
//=====================================================================
public:

    CREATE_RTTI( camera, tracker, object )
    
                            camera         ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return bbox( vector3(0,0,0), 100 ); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=====================================================================
// PRIVATE BASE CLASS FUNCTIONS
//=====================================================================

protected:
    virtual void            OnInit              ( void );     
    virtual void            OnRender            ( void );
    virtual void            OnAdvanceLogic	    ( f32 DeltaTime );

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

//=====================================================================
// PRIVATE FUNCTIONS
//=====================================================================
            
    virtual void            Update              ( xbool bSendKeyEvents ) ;

            void            RenderViewBegin     ( const irect& Viewport, 
                                                        s32    VramID    = -1 ) ;

            void            RenderViewEnd       ( const irect& Viewport, 
                                                        s32    VramID    = -1,
                                                        s32    TexWidth  = -1,
                                                        s32    TexHeight = -1 ) ;

//=====================================================================
// PUBLIC FUNCTIONS
//=====================================================================
public:

            void            RenderView          ( const irect& Viewport, 
                                                        s32    VramID    = -1,
                                                        s32    TexWidth  = -1,
                                                        s32    TexHeight = -1 ) ;

            view&           GetView             ( void ) { return m_View ; }
            xbool           IsEditorSelected    ( void );
            
#ifdef X_EDITOR
            void            RenderEditorView    ( void );
#endif            

            // Pip ear access
            f32             GetPipEarVolume     ( void ) const { return m_PipEarVolume ;   }
            f32             GetPipEarNearClip   ( void ) const { return m_PipEarNearClip ; }
            f32             GetPipEarFarClip    ( void ) const { return m_PipEarFarClip ;  }

//=====================================================================
// DATA
//=====================================================================

protected:

    object_affecter     m_ObjectAffecter;

    f32                 m_FieldOfView ;     // Field of view
    f32                 m_FarClip ;         // Far clipping plane
    view                m_View ;            // View to use
    vector3             m_TargetPos ;       // Target position to look at
    f32                 m_PipEarVolume ;    // PIP ear volume
    f32                 m_PipEarNearClip ;  // PIP ear near clip
    f32                 m_PipEarFarClip ;   // PIP ear near clip
};

//=========================================================================
// END
//=========================================================================
#endif
