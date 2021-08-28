///////////////////////////////////////////////////////////////////////////
//
//  action_ai_dialog_line.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_dialog_line.hpp"
#include "Characters\Character.hpp"
#include "Loco\LocoUtil.hpp"
#include "ConversationMgr\ConversationMgr.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_dialog_line::action_ai_dialog_line ( guid ParentGuid ) 
:   action_ai_base( ParentGuid ),
    m_AnimGroupName(-1),
    m_AnimName(-1),
    m_AnimFlags(0),
    m_SoundName(-1),
    m_InterruptAI(TRUE),
    m_BlockOnDialog(TRUE),
    m_KillAnim(TRUE),
    m_SoundFlags(0),
    m_BlendTime(0.25f),
    m_PreDialogDelay(0.0f),
    m_PostDialogDelay(0.0f),
    m_DelayPreCountDown(0.0f),
    m_DelayPostCountDown(0.0f),
    m_NextAiState(character_state::STATE_IDLE)
{
}
//=============================================================================

void action_ai_dialog_line::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );
    
    // Audio resource.
    rPropList.PropEnumExternal( "Audio Package", "Resource\0audiopkg\0", "The audio package associated with this task.", 0 );
    rPropList.PropEnumExternal( "SoundName",     "Sound\0soundexternal","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );
    rPropList.PropEnumBool    ( "Interrupt AI",  "Are we interrupting the flow of the AI to play the audio?", 0 ) ;

    if( m_InterruptAI )
    {    
        rPropList.PropEnumBool    ( "Block on Dialog","If true we block until Dialog done, if false we block on Anim done.", 0 ) ;   
    }
    if( m_BlockOnDialog )
    {    
        rPropList.PropEnumBool( "Kill Anim",  "Should we cut off the anim when the dialog finishes?", 0 ) ;
    }
    if( !m_BlockOnDialog || !m_KillAnim )
    {
        rPropList.PropEnumFloat( "Anim Blend Time", "Time to blend out of the lipsync anim. (Currrently non-functional)", 0 ) ;
    }

    rPropList.PropEnumBool    ( "Play2D",        "Play 2D sound, such as over a headset.", 0 ) ;
    rPropList.PropEnumFloat    ( "PreDelay",		"Delay before playing dialog in seconds", 0 ) ;
    rPropList.PropEnumFloat    ( "PostDelay",	"Delay after playing dialog in seconds", 0 ) ;

    // Add loco animation properties
    LocoUtil_OnEnumPropAnimFlags(   rPropList,
                                    loco::ANIM_FLAG_END_STATE_ALL        |
                                    loco::ANIM_FLAG_INTERRUPT_BLEND      |
                                    loco::ANIM_FLAG_TURN_OFF_AIMER       |
                                    loco::ANIM_FLAG_MASK_TYPE_ALL        |
                                    loco::ANIM_FLAG_RESTART_IF_SAME_ANIM,
                                    m_AnimFlags) ;

    // Misc
    if( m_TriggerGuid )
    {   
        rPropList.PropEnumEnum    ( "Next State" ,    character::GetStatesEnum(), "State to transition the AI to after the action is complete.", 0 );
    }
}

//=============================================================================

xbool action_ai_dialog_line::OnProperty	( prop_query& rPropQuery )
{
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;

    // Check for loco animation property
    if (LocoUtil_OnPropertyAnimFlags(rPropQuery, 
                                     m_AnimGroupName, 
                                     m_AnimName, 
                                     m_AnimFlags)) 
    {
        return TRUE ;
    }

    // Next ai state
    else if ( rPropQuery.IsVar( "Next State" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( character::GetStateName(m_NextAiState) ); 
            return TRUE;
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            m_NextAiState = (s32) character::GetStateByName  ( pString) ;
            return TRUE;
        }
    }
    
    // External
    else if( rPropQuery.IsVar( "Audio Package" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return TRUE;
    }

    // Sound name
    else if( SMP_UTIL_IsAudioVar( rPropQuery, "SoundName", m_hAudioPackage, m_SoundName ) )
    {
        if( m_SoundName == -1 )
            m_hAudioPackage.SetName( "" );

        return( TRUE );
    }
    else if( rPropQuery.VarBool("Interrupt AI", m_InterruptAI) )
    {    
        if( !m_InterruptAI )
        {
            m_BlockOnDialog = TRUE;
        }
        return TRUE;
    }
    else if( rPropQuery.VarBool("Block on Dialog", m_BlockOnDialog) )
    {    
        return TRUE;
    }
    else if( rPropQuery.VarBool("Kill Anim", m_KillAnim) )
    {    
        return TRUE;
    }
    else if( rPropQuery.VarFloat("Anim Blend Time", m_BlendTime) )
    {    
        return TRUE;
    }
    else if( rPropQuery.IsVar("Play2D") )
    {    
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( (m_SoundFlags & PLAY_2D) );
        }
        else
        {
            if( rPropQuery.GetVarBool() )
                m_SoundFlags |= PLAY_2D;
            else
                m_SoundFlags &= ~PLAY_2D;
        }
        return TRUE;
    }

    else if( rPropQuery.VarFloat("PreDelay", m_PreDialogDelay) )
    {    
        return TRUE;
    }
    else if( rPropQuery.VarFloat("PostDelay", m_PostDialogDelay) )
    {    
        return TRUE;
    }


    return FALSE;
}

//=============================================================================

#ifdef X_EDITOR
s32* action_ai_dialog_line::GetAnimRef( xstring& Desc, s32& AnimName  )
{
    // Only validate if an animation is specified
    if( m_AnimGroupName != -1 )
    {
        Desc = "Animation: "; 
        AnimName  = m_AnimName; 
        return &m_AnimGroupName;
    }

    return NULL;
}
#endif


//=============================================================================

const char* action_ai_dialog_line::GetDescription( void )
{
    static big_string   Info;
    static med_string   SoundLine;

    if (m_SoundName == -1 )
    {
        SoundLine.Set("Unknown Sound");
    }
    else
    {
        SoundLine.Set(g_StringMgr.GetString(m_SoundName));
    }
    
    Info.Set(xfs("%s Say Line %s", GetAIName(), SoundLine.Get()));          
    return Info.Get();
}

