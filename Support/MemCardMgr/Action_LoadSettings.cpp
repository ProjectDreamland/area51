///////////////////////////////////////////////////////////////////////////////
//
//  Action_LoadSettings.cpp
//  Wed Feb 26 11:43:28 2003
//
//  Quick note on the state machine:
//
//      The queue_machine class is an improvement over the old array and switch
//      mechanism used in the Hobbit. What it does is maintain an array of ptrs
//      to member functions inside MemCardMgr. These methods all follow similar
//      naming conventions to the previous implementation.
//
//      The only difference between the paradigms are we constantly execute the
//      top of the stack. This gets rid of any requirement for a switch. I have
//      arranged each of the actions into separate files for readability.
//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
//  Includes
//
///////////////////////////////////////////////////////////////////////////////

#include "MemCardMgr.hpp"
#include "e_Memcard.hpp"
#include "StringMgr/StringMgr.hpp"
#include "Dialogs/dlg_MCMessage.hpp"



///////////////////////////////////////////////////////////////////////////////
//
//  Memory card state methods
//
///////////////////////////////////////////////////////////////////////////////

void MemCardMgr::MC_STATE_LOAD_SETTINGS( void )
{
    //condition& Pending = GetPendingCondition(m_iCard);

    //if( Pending.ErrorCode )
    //{
    //    // don't try to load the settings if we hit an error already
    //    PopState();
    //}
    //else
    {
#ifdef TARGET_XBOX
        g_MemcardMgr.AsyncSetDirectory( "Game Settings" );
#elif defined(TARGET_PC)
        g_MemcardMgr.AsyncSetDirectory( "" ); //We dont using settings folders on PC.
#endif
        ChangeState( __id MC_STATE_LOAD_SETTINGS_SET_DIR_WAIT );
    }
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_SETTINGS_SET_DIR_WAIT( void )
{
    // Check status ***********************************************************
    condition& Pending = GetPendingCondition(m_iCard);

    switch( GetMcResult())
    {
    case kPENDING:
        return;
        break;

    case kSUCCESS:
        {
            // set found settings flag (even if there is an error because bad data can be overwritten)
            m_bFoundSettings = TRUE;
            // set settings found for this card
            m_bCardHasSettings[m_iCard] = TRUE;
            // attempt to read the settings file
            s32 RoundedSize = AllocBuffer( sizeof(global_settings) );
            g_MemcardMgr.AsyncReadFile( xfs("%s%s", m_SavePrefix, m_OptionsPostfix), m_pLoadBuffer, RoundedSize );
            ChangeState( __id MC_STATE_LOAD_SETTINGS_READ_WAIT );
            return;
        }

    case kFAILURE:
    case kRESET:
        // We don't care if the settings are not found.
        #ifdef TARGET_XBOX
        Pending.ErrorCode = FALSE;
        #else
        Pending.bFileNotFound = 0;
        #endif
        break;

    default:
        ASSERT(0);
    }
    PopState();
}

//==---------------------------------------------------------------------------

void MemCardMgr::MC_STATE_LOAD_SETTINGS_READ_WAIT( void )
{
    condition& Pending = GetPendingCondition(m_iCard);
    // Check status ***********************************************************
    switch( GetMcResult())
    {
    case kPENDING:
        return;

    case kSUCCESS:
        {
            global_settings* pSettings = (global_settings*) m_pLoadBuffer;
            ASSERT( m_pLoadBuffer );
            global_settings& Active = g_StateMgr.GetActiveSettings();
            
            if( pSettings->Validate() )
            {
                // We want to load the settings if the settings on the card are newer than the current
                // settings or the current settings have changed. This gives us a way to revert settings
                // to the default and makes sure we load the most recent version of the settings
                // during the initial boot check.
                if( (pSettings->GetDateStamp() > Active.GetDateStamp()) || (Active.HasChanged()) )
                {
#ifdef TARGET_XBOX
                    f32 Brightness = (f32(pSettings->GetBrightness())/100.0f);
                    xbox_SetBrightness( Brightness );
#endif
                    g_StateMgr.SetSettingsCardSlot( 0 );
                    
                    // update the settings
                    Active = *pSettings;
                    Active.Commit();
                }
                else
                {
                    LOG_WARNING( "MemCardMgr::MC_STATE_LOAD_SETTINGS_READ_WAIT", "Did not restore settings due to a later datestamp" );
                }
            }
            else
            {
                LOG_ERROR( "MemCardMgr::MC_STATE_LOAD_SETTINGS_READ_WAIT", "Unable to load settings due to a checksum failure." );                
            }
            break;
        }

    case kFAILURE:
    case kRESET:
        // We don't care if the settings are not found.
        #ifdef TARGET_XBOX
        Pending.ErrorCode = FALSE;
        #else
        Pending.bFileNotFound = 0;
        #endif
        break;


    default:
        ASSERT(0);
    }
    FreeBuffer();
    PopState();
}

