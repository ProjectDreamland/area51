//=============================================================================
//
//  MsgClient.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc. All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../GameTextMgr/GameTextMgr.hpp"

#include "MsgClient.hpp"
#include "GameMgr.hpp"
#include "Objects\HudObject.hpp"
#include "NetworkMgr.hpp"
#include "Inventory/Inventory2.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

const u32 COLOR_FLAG    =  0xFF00;    
const u32 ALLY_COLOR    =  0x51995100;
const u32 ENEMY_COLOR   =  0x99515100;

const u32 YOU_COLOR     =  0x01FF0100;
const u32 TEXT_COLOR    =  0xFFFF0000;

const u32 URGENT_COLOR  =  0xFFFF0000;
const u32 NEUTRAL_COLOR =  0xAAAAAA00;
const u32 GOOD_COLOR    =  0xAAFFAA00;
const u32 BAD_COLOR     =  0xFFAAAA00;

msg_client  MsgClient;

static xwchar  s_NoString  = 0;
static xwchar* s_pNoString = &s_NoString;

// Performance tracking code.
/*
s32 g_NumLost = 0; 
*/

//==============================================================================

msg_client::msg_client( void )
{
    Init();
}

//==============================================================================

msg_client::~msg_client( void )
{
}

//==============================================================================

void msg_client::Init( void )
{
    m_LastAcked = -1;
}

//==============================================================================

void msg_client::AcceptMsgs( bitstream& BS )
{
    msg Msg;

    while( Msg.Read( BS ) )
    {
        if( Msg.m_MsgSeq > m_LastAcked + 1 )
        {
            LOG_WARNING( "MsgClient::AcceptMsgs", 
                "Missing %d message(s)! Got m_MsgSeq=%d when I expected m_MsgSeq=%d!", 
                Msg.m_MsgSeq - (m_LastAcked + 1), Msg.m_MsgSeq, m_LastAcked + 1  );

            //          g_NumLost += Msg.m_MsgSeq - (m_LastAcked + 1);  // For performance tracking
        }

        AcceptMsg( Msg );
    }
}

//==============================================================================

void msg_client::AcceptMsg( msg& Msg )
{
    ASSERT( MsgTable[ Msg.m_MsgID ].m_Active );

    if( MsgTable[ Msg.m_MsgID ].m_Active )
    {
        if( (Msg.m_MsgSeq > m_LastAcked) || Msg.m_bIsLocal )
        {
            if( (Msg.m_MsgSeq > m_LastAcked) && !Msg.m_bIsLocal )
            {
                m_LastAcked = Msg.m_MsgSeq;
            }

            DisplayMsg( Msg );
        }
    }
}

//==============================================================================

// Do all the formatting for the message in here.
void msg_client::DisplayMsg( const msg& Msg )
{
    // If this message has been deactivated we don't want to display it
    if( MsgTable[ Msg.m_MsgID ].m_Active == FALSE)
    {
        return;
    }

    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        hud_object& Hud = hud_object::GetSafeType( *pObj );

        if( !Hud.m_Initialized )
        {
            return;
        }

        s32 HudIndex;
        xbool bSoundPlayed = FALSE;

        for( HudIndex = 0; HudIndex < g_NetworkMgr.GetLocalPlayerCount(); HudIndex++ )
        {
            slot_id PlayerSlot = Hud.m_PlayerHuds[ HudIndex ].m_PlayerSlot;
            f32 Pitch = 1.0f / g_PerceptionMgr.GetAudioTimeDialation();
            s32 Sound;

            if( (PlayerSlot != SLOT_NULL) && 
                Msg.IsValidTarget( Hud.m_PlayerHuds[ HudIndex ].m_NetSlot ) )
            {
                if( (MsgTable[ Msg.m_MsgID ].m_pMsgSound) && !bSoundPlayed )
                {
                    Sound = g_AudioMgr.Play( MsgTable[ Msg.m_MsgID ].m_pMsgSound );
                    g_AudioMgr.SetPitch( Sound, Pitch );
                    g_AudioMgr.SetPitchLock( Sound, TRUE );
                }

                if( (MsgTable[ Msg.m_MsgID ].m_pMsgSound2) && !bSoundPlayed )
                {
                    Sound = g_AudioMgr.Play( MsgTable[ Msg.m_MsgID ].m_pMsgSound2 );
                    g_AudioMgr.SetPitch( Sound, Pitch );
                    g_AudioMgr.SetPitchLock( Sound, TRUE );
                }

                bSoundPlayed = TRUE;

                ASSERT( Hud.m_PlayerHuds[ HudIndex ].m_LocalSlot != -1 );

                xwstring MsgString =
                    GetFormattedString( Msg, Hud.m_PlayerHuds[ HudIndex ].m_NetSlot );

                // If this message is Empty don't display anything!
                if(  MsgString.GetLength() == 0 )
                {
                    return;
                }

                // Now ship it off to the HUD.  Notice I'm using HudIndex not PlayerSlot,
                // as the huds are 0 - 4 and the playerslot could be 0 - 32,
                // and I don't really want to have to convert it back later on.
                switch( MsgTable[ Msg.m_MsgID ].m_MsgType )
                {

                case NORMAL: 
                    Hud.m_PlayerHuds[ HudIndex ].m_Text.AddLine( (const xwchar*)MsgString );
                    break;

                case GOAL:
                    Hud.m_PlayerHuds[ HudIndex ].m_Text.UpdateGoal( Msg.m_MsgGoalNum, Msg.m_Enabled, (const xwchar*)MsgString, Msg.m_Time );
                    break;

                case BONUS:
                    Hud.m_PlayerHuds[ HudIndex ].m_Text.SetBonus( (const xwchar*)MsgString, Msg.m_Time );
                    break;

                case WEAPON_INFO:
                    Hud.m_PlayerHuds[ HudIndex ].m_Text.SetWeaponInfo( (const xwchar*)MsgString, Msg.m_Time );
                    break;

                case SCORE_MSG:
                    {
                        // Break the message into three parts.

                        #define SCORE_COL_SIZE 32

                        xwchar Columns[ 3 ][ SCORE_COL_SIZE ];

                        x_memset( Columns, 0, sizeof( Columns ) );

                        s32 Part = 0;
                        s32 CharInOrig = 0;
                        s32 CharInPart = 0;
                        while( (Part < 3) && (CharInOrig < MsgString.GetLength()) )
                        {
                            if( MsgString.GetAt( CharInOrig ) == '\\' )
                            {
                                Columns[ Part ][ CharInPart ] = '\0';
                                CharInPart = 0;
                                Part++;
                            }
                            else
                            {
                                Columns[ Part ][ CharInPart ] = MsgString.GetAt( CharInOrig );
                                CharInPart++;
                                ASSERT( CharInPart < SCORE_COL_SIZE );
                            }

                            CharInOrig++;
                        }
                        
                        Hud.m_PlayerHuds[ HudIndex ].m_InfoBox.SetScoreInfo( Columns[ 0 ], Columns[ 1 ], Columns[ 2 ], Msg.m_Data[ 3 ] );
                    }
                    break;

                default:
                    ASSERTS( FALSE, "Unknown Message Type" );
                    break;
                }
            }
        }
    }
}

//==============================================================================

inline xbool msg_client::ReplaceArg( const msg&      Msg,
                                   s32       TargetPlayer,
                                   s32       ArgNum,
                                   xwchar*   pMessage,
                                   s32&      CursorPos )
{
    const game_score&   Score    = GameMgr.GetScore();
    s32                 ArgType  = MsgTable[ Msg.m_MsgID ].m_Types[ ArgNum ];
    s32                 ArgVal   = Msg.m_Data[ ArgNum ];

    ASSERT( IN_RANGE( 0, TargetPlayer, 31 ) );
    ASSERT( IN_RANGE( 0, ArgNum, MAX_MSG_ARGS-1 ) );
    if( !IN_RANGE( 0, TargetPlayer, 31 ) ||
        !IN_RANGE( 0, ArgNum, MAX_MSG_ARGS-1 ) )
    {
        return FALSE;
    }

    // Override "you" substitution if this is not in English.
    if( (ArgType == ARG_PLAYER) && (x_GetLocale() != XL_LANG_ENGLISH) )
    {
        ArgType = ARG_PLAYER_NAME;
    }

    switch( ArgType )
    {
    case ARG_PLAYER:
        if( ArgVal == TargetPlayer )
        {
            InsertColor( YOU_COLOR, pMessage, CursorPos );

            ASSERT( m_SelfName.GetLength() != 0 );
            if( m_SelfName.GetLength() == 0 )
            {
                return FALSE;
            }

            // inserts "you" - loaded with messages to ensure localized version.
            InsertWord( m_SelfName, pMessage, CursorPos );
            break;
        }

        // Intentional fall through.

    case ARG_PLAYER_NAME:
        ASSERT( IN_RANGE( 0, ArgVal, 31 ) );
        if( !IN_RANGE( 0, ArgVal, 31 ) )
        {
            return FALSE;
        }

        if( (Score.IsTeamBased) &&
            (Score.Player[ ArgVal ].Team == Score.Player[ TargetPlayer ].Team) )
        {
            InsertColor( ALLY_COLOR, pMessage, CursorPos );
        }
        else if( ArgVal == TargetPlayer )
        {
            InsertColor( YOU_COLOR, pMessage, CursorPos );
        }
        else
        {
            InsertColor( ENEMY_COLOR, pMessage, CursorPos );
        }
        InsertString( Score.Player[ ArgVal ].Name, pMessage, CursorPos );

        break;

    case ARG_TEAM:
        if( ArgVal == Score.Player[ TargetPlayer ].Team )
        {
            InsertColor( ALLY_COLOR, pMessage, CursorPos );
        }
        else
        {
            InsertColor( ENEMY_COLOR, pMessage, CursorPos );
        }

        InsertString( Score.Team[ ArgVal ].Name, pMessage, CursorPos );
        break;

    case ARG_STRING:
        InsertColor( TEXT_COLOR, pMessage, CursorPos );
        InsertString( Msg.m_StringData[ ArgNum ], pMessage, CursorPos );
        break;

    case ARG_NUMBER_SIGNED:
        if( ArgVal > 0 )
        {
            InsertCharacter( (xwchar)'+', pMessage, CursorPos );
        }
        // Fall through.
    case ARG_NUMBER:
        InsertNumber( ArgVal, pMessage, CursorPos );
        break;

    case ARG_ITEM:
        InsertString( (const xwchar*)xwstring( inventory2::ItemToDisplayName((inven_item)ArgVal) ), pMessage, CursorPos );
        break;

    case ARG_PLAYER_SCORE:
        {
            ASSERT( IN_RANGE( 0, ArgVal, 31 ) );
            if( !IN_RANGE( 0, ArgVal, 31 ) )
            {
                return FALSE;
            }

            if( ArgVal == TargetPlayer )
            {
                InsertColor( YOU_COLOR, pMessage, CursorPos );
            }
            if( (Score.IsTeamBased) &&
                (Score.Player[ ArgVal ].Team == Score.Player[ TargetPlayer ].Team) )
            {
                InsertColor( ALLY_COLOR, pMessage, CursorPos );
            }
            else if( ArgVal == TargetPlayer )
            {
                InsertColor( YOU_COLOR, pMessage, CursorPos );
            }
            else
            {
                InsertColor( ENEMY_COLOR, pMessage, CursorPos );
            }
        }
        InsertNumber( Score.Player[ ArgVal ].Score, pMessage, CursorPos  );
        break;

    case ARG_TEAM_SCORE:
        {
            if( ArgVal == Score.Player[ TargetPlayer ].Team )
            {
                InsertColor( ALLY_COLOR, pMessage, CursorPos );
            }
            else
            {
                InsertColor( ENEMY_COLOR, pMessage, CursorPos );
            }
        }       
        InsertNumber( Score.Team[ ArgVal ].Score, pMessage, CursorPos  );
        break;

    // Insert 1st, 2nd, 3rd, etc.
    case ARG_ORDINAL:
        if( ArgVal > 0 )
        {
            InsertNumber( ArgVal, pMessage, CursorPos );

            if( x_GetLocale() == XL_LANG_ENGLISH ) 
            {
                if(  IN_RANGE(  1, ArgVal % 10,   3 ) &&
                    !IN_RANGE( 11, ArgVal % 100, 13 ) )
                {
                    switch( ArgVal % 10 )
                    {
                    case 1:
                        InsertString( (const xwchar*)xwstring( "st" ), pMessage, CursorPos );
                        break;
                    case 2:
                        InsertString( (const xwchar*)xwstring( "nd" ), pMessage, CursorPos );
                        break;
                    case 3:
                        InsertString( (const xwchar*)xwstring( "rd" ), pMessage, CursorPos );
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    InsertString( (const xwchar*)xwstring( "th" ), pMessage, CursorPos );
                }  
            }
            else
            {
                InsertCharacter( '.', pMessage, CursorPos );
            }
        }
        break;

    default:
        break;
    }
    return TRUE;
}

//==============================================================================

xwstring msg_client::GetFormattedString( const msg& Msg, s32 TargetPlayer )
{
    const xwchar*   pRead;

    xwchar          Message[ MAX_DISPLAY_LENGTH ];
    s32             CursorPos   = 0;
    xwstring        MsgString   = GetMsgString( Msg, TargetPlayer );

    if( MsgString.GetLength() == 0 )
    {
        return MsgString;
    }

    pRead = (const xwchar*) MsgString;

#if 0 //message number/source printout
    {   
        InsertColor( 255, 0, 0, Message, CursorPos );

        if( Msg.m_bIsLocal )
        {
            InsertString( (const xwchar*)xwstring( "L" ), Message, CursorPos );
        }
        else
        {
            InsertString( (const xwchar*)xwstring( "R" ), Message, CursorPos );
        }

        InsertNumber( Msg.m_MsgSeq, Message, CursorPos );
        InsertString( (const xwchar*)xwstring( ". " ), Message, CursorPos );
    }
#endif

    if( MsgTable[ Msg.m_MsgID ].m_MsgType == GOAL )
    {
        static xwchar SpecialChar = (xwchar)24;//31;
        InsertColor( 0, 200, 0, Message, CursorPos );
        InsertCharacter( SpecialChar, Message, CursorPos);
        InsertCharacter( (xwchar)' ', Message, CursorPos);
    }

    xcolor MsgBaseColor( 0 );

    msg_impact MsgImpact = GetGoodOrBad( TargetPlayer, Msg );

    switch( MsgImpact )
    {
    case IMPACT_GOOD:    
        MsgBaseColor = xcolor( GOOD_COLOR );
        InsertColor( MsgBaseColor, Message, CursorPos );
        //  InsertString( (const xwchar*)xwstring( "[+] " ), Message, CursorPos );
        break;
    case IMPACT_BAD:
        MsgBaseColor = xcolor( BAD_COLOR );
        InsertColor( MsgBaseColor, Message, CursorPos );
        //  InsertString( (const xwchar*)xwstring( "[-] " ), Message, CursorPos );
        break;
    case IMPACT_NEUTRAL:
        MsgBaseColor = xcolor( NEUTRAL_COLOR );
        break;
    case IMPACT_URGENT:
        MsgBaseColor = xcolor( URGENT_COLOR );
        break;
    default:
        break;
    }

    InsertColor( MsgBaseColor, Message, CursorPos );

    // Process rest of string.
    xbool   Error   = FALSE;
    while( *pRead && !Error )
    {
        if( *pRead != '<' )
        {
            Message[ CursorPos ] = *pRead;
            CursorPos++;
            pRead++;
            continue;
        }

        pRead++;        // Skip '<'.
        xwchar  Key = *pRead++; // Read key.
        pRead++;        // Skip '>'.

        s32 ArgNum  = Key - '1';

        if( MsgTable[ Msg.m_MsgID ].m_MsgType == SCORE_MSG )
        {
            InsertColor( MsgBaseColor, Message, CursorPos );
        }

        if( !ReplaceArg( Msg, TargetPlayer, ArgNum, Message, CursorPos ) )
        {
            return xwstring("");
        }

        InsertColor( MsgBaseColor, Message, CursorPos );
    }

    Message[ CursorPos ] = 0;
    ASSERT( CursorPos < MAX_DISPLAY_LENGTH );

    return xwstring( Message );
}                   

//==============================================================================

void msg_client::InsertNumber( s32 ArgVal, xwchar* pMessage, s32& CursorPos )
{
    s32         Len         = 0;
    s32         Num         = ( s32 ) ArgVal;
    const s32   BuffSize    = 16;

    xwchar      NumBuf[ BuffSize ];
    NumBuf[ BuffSize - 1 ] = 0;

    if( Num < 0 )
    {
        Num *= -1;
        pMessage[ CursorPos ] = '-';
        CursorPos += 1;
    }

    do
    {
        NumBuf[ BuffSize - 2 - Len ] = ( Num % 10 ) + '0';
        Num = Num / 10;
        Len++;
    }
    while( Num > 0 );

    InsertString( &NumBuf[ BuffSize - 1 - Len ], pMessage, CursorPos );
}

//==============================================================================

void msg_client::InsertString( const xwchar* pString,
                              xwchar* pMessage,
                              s32& CursorPos )
{
    s32 Len = x_wstrlen( pString );
    x_wstrcpy( pMessage + CursorPos, pString );
    CursorPos += Len;
}

//==============================================================================

void msg_client::InsertWord( const xwchar* pString,
                            xwchar* pMessage,
                            s32& CursorPos )
{
    s32 StartChar = 0;

    // See if the word could use capitalization to begin with.
    if( IN_RANGE( 'a', pString[ 0 ], 'z' ) )
    {
        // Now check to see if the previous non-space char was a period.
        xbool StartSentence = TRUE;

        s32 i;
        for( i = 0; i < CursorPos; i++ )
        {
            // Look for a color code first and ignore.
            if( (pMessage[ i ] & 0xFF00) == 0xFF00 )
            {
                i++; // Because colors take up 32 bytes, not 16 like xwchars.
                continue;
            }
            else if( pMessage[ i ] == (xwchar)'.' )
            {
                StartSentence = TRUE;
            }
            else if( pMessage[ i  ] != (xwchar)' ' )
            {
                StartSentence = FALSE;
            }
        }

        if( StartSentence )
        {
            StartChar = 1;
            pMessage[ CursorPos ] = (xwchar)(pString[ 0 ] + ((xwchar)'A' - (xwchar)'a'));
            CursorPos++;
        }
    }

    if( pString[ StartChar ] != 0 )
    {
        InsertString( pString + StartChar, pMessage, CursorPos );
    }
}


//==============================================================================

void msg_client::InsertColor( u32 Color, xwchar* pMessage, s32& CursorPos )
{
    u16 TmpRed  = ( Color & 0xFF000000 ) >> 24;
    u16 TmpGrn  = ( Color & 0x00FF0000 ) >>  8;
    u16 TmpBlu  = ( Color & 0x0000FF00 ) >>  8;

    *( pMessage + CursorPos++ ) = COLOR_FLAG | TmpRed | 0x0101;
    *( pMessage + CursorPos++ ) = TmpGrn     | TmpBlu | 0x0101;
}

//==============================================================================

void msg_client::InsertColor( u8 Red,
                             u8 Green,
                             u8 Blue,
                             xwchar* pMessage,
                             s32& CursorPos )
{

    u16 TmpRed  = Red & 0x00FF;
    u16 TmpGrn  = Green << 8;
    u16 TmpBlu  = Blue & 0x00FF;

    *( pMessage + CursorPos++ ) = COLOR_FLAG | TmpRed | 0x0101;
    *( pMessage + CursorPos++ ) = TmpGrn     | TmpBlu | 0x0101;
}

void msg_client::InsertCharacter( xwchar Char,
                                 xwchar* pMessage,
                                 s32& CursorPos )
{
    *( pMessage + CursorPos++ ) = Char;
}


//==============================================================================

const xwchar* msg_client::GetMsgString( msg Msg, s32 TargetPlayer )
{
    s32 MsgNum = 0;

    if( (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_PLAYER) &&
        (Msg.m_Data[ 0 ] == TargetPlayer) &&
        (MsgTable[ Msg.m_MsgID ].m_pMsgString[ 1 ] != NULL) )
    {
        MsgNum = 1;
    }
    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_PLAYER) && 
            (Msg.m_Data[ 1 ] == TargetPlayer) &&
            (MsgTable[ Msg.m_MsgID ].m_pMsgString[ 2 ] != NULL) )
    {
        MsgNum = 2;
    }

    if( MsgTable[ Msg.m_MsgID ].m_pMsgString[ MsgNum ] == NULL )
        return( s_pNoString );
    else
        return( MsgTable[ Msg.m_MsgID ].m_pMsgString[ MsgNum ] );
}

//==============================================================================

msg_impact msg_client::GetGoodOrBad( s32 TargetPlayer, msg Msg )
{
    s32 Affiliation = 1;
    const game_score&   Score           = GameMgr.GetScore();

    if( Score.IsTeamBased &&
        (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_PLAYER) &&
        (Score.Player[ TargetPlayer ].Team == Score.Player[ Msg.m_Data[ 0 ] ].Team) )
    {
        Affiliation = 1;
    }

    else if( Score.IsTeamBased &&
        (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_PLAYER) &&
        (Score.Player[ TargetPlayer ].Team == Score.Player[ Msg.m_Data[ 1 ] ].Team) )
    {
        Affiliation = 2;
    }

    else if( Score.IsTeamBased &&
        (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_PLAYER) &&
        (Score.Player[ TargetPlayer ].Team != Score.Player[ Msg.m_Data[ 0 ] ].Team) )
    {
        Affiliation = 2;
    }

    else if( Score.IsTeamBased &&
        (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_PLAYER) &&
        (Score.Player[ TargetPlayer ].Team != Score.Player[ Msg.m_Data[ 1 ] ].Team) )
    {
        Affiliation = 1;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_PLAYER) && 
        (TargetPlayer == Msg.m_Data[ 0 ]) )
    {
        Affiliation = 1;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_PLAYER) &&
        (TargetPlayer == Msg.m_Data[ 1 ]) )
    {
        Affiliation = 2;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_TEAM ) &&
        (Score.Player[ TargetPlayer ].Team == Msg.m_Data[ 0 ]) )
    {
        Affiliation = 1;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 0 ] == ARG_TEAM ) &&
        (Score.Player[ TargetPlayer ].Team != Msg.m_Data[ 0 ]) )
    {
        Affiliation = 2;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_TEAM) &&
        (Score.Player[ TargetPlayer ].Team == Msg.m_Data[ 1 ]) )
    {
        Affiliation = 1;
    }

    else if( (MsgTable[ Msg.m_MsgID ].m_Types[ 1 ] == ARG_TEAM) &&
        (Score.Player[ TargetPlayer ].Team != Msg.m_Data[ 1 ]) )
    {
        Affiliation = 2;
    }

    // Now determine what the affiliation says the actual impact of this message is.
    msg_impact Impact = MsgTable[ Msg.m_MsgID ].m_Impact;

    if( Impact == IMPACT_URGENT ) // Urgent ignores affiliation.
    {
        return IMPACT_URGENT;    
    }
    else if( ((Affiliation == 1) && (Impact == IMPACT_GOOD)) ||
             ((Affiliation == 2) && (Impact == IMPACT_BAD )) )
    {
        return IMPACT_GOOD;
    }
    else if(    ((Affiliation == 2) && (Impact == IMPACT_GOOD)) ||
                ((Affiliation == 1) && (Impact == IMPACT_BAD )) )
    {
        return IMPACT_BAD;
    }

    else
    {
        return IMPACT_NEUTRAL;
    }
}

//==============================================================================
