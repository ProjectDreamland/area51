///////////////////////////////////////////////////////////////////////////////
//
//  action_play_2d_sound.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_play_2d_sound_
#define _action_play_2d_sound_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// action_play_2d_sound
//=========================================================================

class action_play_2d_sound : public actions_ex_base
{
public:
    CREATE_RTTI( action_play_2d_sound, actions_ex_base, actions_ex_base )

                    action_play_2d_sound          ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_PLAY_2D_SOUND;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Play a 2d sound. (blockable)"; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );
    virtual         void                OnActivate      ( xbool Flag );
                    s32                 GetStreamed     ( void ) { return m_Streamed; }

protected:

    char            m_DescriptorName[64];
    rhandle<char>   m_hAudioPackage;
    xbool           m_bBlock;
    s32             m_Streamed;
    s32             m_VoiceID;
};


#endif
