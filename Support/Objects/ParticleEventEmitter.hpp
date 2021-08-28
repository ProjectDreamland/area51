#ifndef PARTICLE_EVENT_EMITTER_HPP
#define PARTICLE_EVENT_EMITTER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// CLASS
//=========================================================================
class particle_event_emitter : public object
{
public:

    CREATE_RTTI( particle_event_emitter, object, object )

                            particle_event_emitter              ( void );
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnAdvanceLogic                      ( f32 DeltaTime );      
    virtual void            OnMove                              ( const vector3& NewPos   );      
    virtual void            OnTransform                         ( const matrix4& L2W      );
    virtual bbox            GetLocalBBox                        ( void ) const;      

            void            StartEmitter                        ( const char*    pFx,
                                                                  const vector3& Position, 
                                                                  const vector3& Rotation,
                                                                  u16            ZoneID,
                                                                  guid           ParentGuid, 
                                                                  s32            EventID,
                                                                  xbool          EventActive );
            
    virtual guid            GetParentGuid                       ( void ) { return m_ParentGuid; }
            guid            GetParticleGuid                     ( void ) { return m_ParticleGuid; }
            const char*     GetFxName                           ( void ) { return m_FxName; }
            s32             GetEventID                          ( void ) { return m_EventID; }
            void            EnableUpdate                        ( void ) { m_LogicRunning = TRUE; }

    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );
            
    
//=========================================================================

protected:
    
    char        m_FxName[64];
    guid        m_ParentGuid;
    guid        m_ParticleGuid;
    s32         m_EventID;
    xbool       m_EventActive;
    xbool       m_LogicRunning;
        
};
//=========================================================================
// END
//=========================================================================
#endif