#ifndef EVENT_EDITOR_HPP
#define EVENT_EDITOR_HPP

#include "../EDRscDesc\RSCDesc.hpp"

//=========================================================================
// EVENT DESCRIPTION
//=========================================================================
struct event_description : public rsc_desc
{
    CREATE_RTTI( event_description, rsc_desc, rsc_desc )

    struct particle_event
    {
        char    m_Name[128];           

        f32     m_Velocity;
        f32     m_Speed;

        particle_event(){ m_Velocity = 0.0f; m_Speed = 0.0f; m_Name[0] = 0; }
        void Clear( void );
    };

    struct audio_event
    {
        char    m_Name[128];           
        
        f32     m_Volume;
        f32     m_Pitch;

        audio_event(){ m_Volume = 1.0f; m_Pitch = 1.0f; m_Name[0] = 0; }
        void Clear( void );//{ m_Volume = 1.0f; m_Pitch = 1.0f; m_Name[0] = 0; }
    };

    struct event
    {
        char                        m_EventName[128];
        xarray<audio_event>         m_EventAudioList;
        xarray<particle_event>      m_EventParticleList;

        event(){ m_EventName[0] = 0; m_EventAudioList.Clear(); m_EventParticleList.Clear(); }
        void Clear( void );
    };
                        event_description   ( void );
    virtual void        OnEnumProp          ( prop_enum&    List    );
    virtual xbool       OnProperty          ( prop_query&   I              );
    virtual void        OnGetDependencies   ( xarray<xstring>& List     );
    virtual void        OnGetCompilerRules  ( xstring& CompilerRules     );
    virtual void        OnCheckIntegrity    ( void );
    
    xarray<event>       m_EventList;
};

//=========================================================================
// EVENT EDITOR
//=========================================================================
class event_editor : public prop_interface
{
//=========================================================================
public:


//=========================================================================
public:
                                       event_editor      ( void );

                               void    AddEvent          ( const char* pEvent      );
    event_description::audio_event*    AddAudioEvent     ( s32 EventIndex          );
 event_description::particle_event*    AddParticleEvent  ( s32 EventIndex          );
                       virtual void    OnEnumProp        ( prop_enum&   List        );
                      virtual xbool    OnProperty        ( prop_query&  I           );
                               void    Save              ( void );
                               void    Load              ( const char* pFileName );
                               void    New               ( void );
                               xbool   NeedSave          ( void );
                               void    Edit              ( event_description& EventDesc );


//=========================================================================
public:
    event_description* m_pDesc;
};

//=========================================================================
// 
// "pepe\jack1"
// "pepe\jack2"
// "pepe\jack3\rita"
// 
// + pepe
//   * jack1
//   * jack2
//   + jack3
//     * Rita
// 
//=========================================================================
// END
//=========================================================================
#endif
