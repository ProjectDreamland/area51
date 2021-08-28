#ifndef SIMPLE_SOUND_EMITTER_HPP
#define SIMPLE_SOUND_EMITTER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// CLASS
//=========================================================================
class simple_sound_emitter : public object
{
public:

    CREATE_RTTI( simple_sound_emitter, object, object )

                            simple_sound_emitter                ( void );
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnAdvanceLogic                      ( f32 DeltaTime );      
    virtual void            OnMove                              ( const vector3& NewPos   );      
    virtual void            OnTransform                         ( const matrix4& L2W      );
    virtual bbox            GetLocalBBox                        ( void ) const;      
    virtual void            OnActivate                          ( xbool Flag );
    virtual void            OnKill                              ( void );               

    virtual void            OnEnumProp                          ( prop_enum& List );
    virtual xbool           OnProperty                          ( prop_query& I );
#ifdef X_EDITOR
    virtual s32             OnValidateProperties                ( xstring& ErrorMsg );
#endif

    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );

protected:
            
#ifndef X_RETAIL
    virtual void            OnDebugRender                       ( void );
#endif // X_RETAIL

//=========================================================================


    
    char            m_DescriptorName[64];
    rhandle<char>   m_hAudioPackage;
    xarray<s32>     m_pAsyncVoiceID;
    xbool           m_bIsStreamed;
    s32             m_VoiceID;
    u16             m_Flags;
    f32             m_ReleaseTime;
        
// Make friends here
};
//=========================================================================
// END
//=========================================================================
#endif