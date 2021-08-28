///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_PLAY_CONVERSATION
#define _TRIGGER_ACTIONS_PLAY_CONVERSATION

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// PLAY_SOUND
//=========================================================================

class  play_conversation : public actions_base
{
public:
    
                    play_conversation               ( guid ParentGuid );

    
    virtual         const char*         GetTypeName ( void )    { return "Play Conversation"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Plays a conversation (streamed sound)."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_PLAY_CONVERSATION;}

protected:
    
    char            m_Descriptor[64];               // Sound ID to play
    rhandle<char>   m_hAudioPackage;                // Audio resource that contains the sound id.
    f32             m_StartDelay;                   // Delay till we start the audio sample.
    u8              m_Flags;
};

#endif
