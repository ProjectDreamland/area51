//=========================================================================
// GameTextMgr
//=========================================================================

//=========================================================================
// Includes
//=========================================================================
#include "GameTextMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Objects\HudObject.hpp"
#include "Sound\SimpleSoundEmitter.hpp"
#include "MiscUtils\SimpleUtils.hpp"

#ifndef X_EDITOR
#include "NetworkMgr\MsgMgr.hpp"
#endif

//=========================================================================
// GLOBALS
//=========================================================================
game_text_mgr g_GameTextMgr;


//=========================================================================

void game_text_mgr::DisplayMessage( const char* pTableName, const char* pTitleString )
{
    (void)pTableName;
    (void)pTitleString;

    if( g_ConverseMgr.IsActive( m_Message.SoundID ) )
    {
        g_ConverseMgr.Stop( m_Message.SoundID );
        m_Message.SoundID = 0;
    }

    m_Message.pMainString       = (xwchar*)g_StringTableMgr( pTableName, pTitleString );
    m_Message.pSubTitleString   = (xwchar*)g_StringTableMgr.GetSubTitleSpeaker( pTableName, pTitleString );
    m_Message.pSoundDescString  = (xwchar*)g_StringTableMgr.GetSoundDescString( pTableName, pTitleString );
    
    
    // If there is a sound specified start warming it up.
    if( x_wstrlen( m_Message.pSoundDescString ) > 0 )
    {
        m_Message.Guid = g_ObjMgr.CreateObject( simple_sound_emitter::GetObjectType() );
        object* pObj = g_ObjMgr.GetObjectByGuid( m_Message.Guid );
        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if( pObj && pPlayer )
        {
            xwstring WideString( m_Message.pSoundDescString );
            xstring  String( WideString );
        
            pObj->OnTransform( pPlayer->GetL2W() );
            
            // Use the player's position but pass in zone 0 so we don't try to propagate the sound.
            m_Message.SoundID = g_ConverseMgr.PlayStream( (const char*)String, 
                                                          pPlayer->GetPosition(), 
                                                          m_Message.Guid, 
                                                          pObj->GetZone1(), 
                                                          IMMEDIATE_PLAY, 
                                                          FALSE );
        }
        
        if( m_Message.SoundID )
        {
            m_Message.State = 0;
            m_Message.State |= AUDIO_WARMING;
        }
    }
    else
    {
        
        // Functionality moved to MsgClient
#ifndef X_EDITOR
        MsgMgr.Message( MSG_GOAL_STRING, 0, (s32)m_Message.pMainString ); 
#endif
    }


} 

//=========================================================================

void game_text_mgr::Init( void )
{   
    m_Message.pMainString       = NULL;
    m_Message.pSoundDescString  = NULL;
    m_Message.pSubTitleString   = NULL;
    m_Message.SoundID           = 0;
    m_Message.Guid              = 0;
    m_Message.State             = 0;
}

//=========================================================================

void game_text_mgr::Update( f32 DeltaTime )
{
    (void)DeltaTime;

    if( m_Message.pSoundDescString == NULL )
        return;

    if( x_wstrlen( m_Message.pSoundDescString ) > 0 )
    {
        // Once the audio start we are no longer dependent on it.
        if( (m_Message.State & AUDIO_PLAYING) || (m_Message.State & AUDIO_FINISHED) )
        {
            ;
        }
        else if( m_Message.State & AUDIO_WARMING )
        {
            // Still warming up.
            if( g_ConverseMgr.IsReadyToPlay( m_Message.SoundID ) == FALSE )
                return;

            // The sound is warmed up so lets start rendering the text and playing the audio.
            m_Message.State = 0;
            m_Message.State |= AUDIO_PLAYING;
            
            g_ConverseMgr.StartSound( m_Message.SoundID );
        }

        if( m_Message.State & AUDIO_PLAYING )
        {
            // If the sound is dead, destory the object.  If its still active update its position to the player position.
            if( g_ConverseMgr.IsActive( m_Message.SoundID ) == FALSE )
            {
                g_ObjMgr.DestroyObject( m_Message.Guid );
                m_Message.Guid  = 0;
                m_Message.State = 0;
                m_Message.State |= AUDIO_FINISHED;
            }
            else
            {
                object* pObj    = g_ObjMgr.GetObjectByGuid( m_Message.Guid );
                player* pPlayer = SMP_UTIL_GetActivePlayer();
                if( pObj && pPlayer )
                    pObj->OnTransform( pPlayer->GetL2W() );
            }
        }   
    }
    else
    {
        ;
    }
}

//=========================================================================

void game_text_mgr::Kill( void )
{
}
        
//=========================================================================
