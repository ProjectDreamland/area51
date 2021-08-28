#ifndef EVENT_SOUND_EMITTER_HPP
#define EVENT_SOUND_EMITTER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// CLASS
//=========================================================================
class event_sound_emitter : public object
{
public:

    CREATE_RTTI( event_sound_emitter, object, object )

    enum sound_type{ FIRST_TYPE,
                     
                     SINGLE_SHOT = FIRST_TYPE, 
                     LOOPED, 
                     CONVERSATION,
                     CONTACT,

                     LAST_TYPE = CONTACT
                      };

                            event_sound_emitter                ( void );
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnAdvanceLogic                      ( f32 DeltaTime );      
    virtual void            OnMove                              ( const vector3& NewPos   );      
    virtual bbox            GetLocalBBox                        ( void ) const;      

            void            PlayEmitter                         ( const char* pDescriptor, vector3& Position, u16 ZoneID,
                                                                  sound_type Type, guid ParentGuid, u32 Flags = 0, 
                                                                  f32 Delay = 0.0f, xbool UseRadius = FALSE, 
                                                                  f32 Radius = 1.0f, xbool Play2D = FALSE );
            
    virtual guid            GetParentGuid                       ( void ) { return m_ParentGuid; }
            const char*     GetDescriptorName                   ( void ) { return m_DescriptorName; }
            const char*     GetMaterialType                     ( void );
            const char*     GetMaterialTypeFromActor            ( guid Guid );
            const char*     GetMaterialName                     ( s32 MatType );
            s32             GetVoiceID                          ( void ) { return m_VoiceID; }

    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );
            
            
    

protected:
            void            StartEmitter                        ( void );
            void            StartEmitter2D                      ( void );

//=========================================================================

protected:
    
    char        m_DescriptorName[64];
    char        m_ObjectName[64];
    char        m_ActionName[64];
    s32         m_VoiceID;
    guid        m_ParentGuid;
    xbool       m_Active;
    sound_type  m_SoundType;
    f32         m_Delay;
    u32         m_Flags;
    xbool       m_SphereTest;
    f32         m_Radius;
    xbool       m_b2D;
        
// Make friends here
};
//=========================================================================
// END
//=========================================================================
#endif