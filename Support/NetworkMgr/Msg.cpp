//==============================================================================
//
//  Msg.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Messages.hpp"
#include "Msg.hpp"
#include "GameMgr.hpp"
#include "NetworkMgr.hpp"

#include "x_log.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

xbool msg::Write( bitstream& BS, xbool bIncludeInfo )
{
    //LOG_MESSAGE( "msg::Write", "Message:%d", Index );

    if( !BS.OpenSection() )
        return( FALSE );
        
    BS.WriteRangedS32( m_MsgID, MSG_NULL, NUM_MESSAGES-1 ); 

    // Note that this must come after writing the MsgID because
    // otherwise the client won't know which slot to override.
    if( BS.WriteFlag( bIncludeInfo ) )
    {
        BS.WriteMarker();
        MsgTable[ m_MsgID ].WriteMessageInfo( BS );
        BS.WriteMarker();
    }

    BS.WriteS32( m_MsgSeq );   // DMT: Explore means to use fewer bits.
    BS.WriteRangedS32( m_Target, 0, 31 );

    if( MsgTable[ m_MsgID ].m_MsgType == GOAL )
    {
        BS.WriteFlag( m_Enabled    );
        BS.WriteS32 ( m_MsgGoalNum );
    }

    if( MsgTable[ m_MsgID ].m_MsgType == GOAL || MsgTable[ m_MsgID ].m_MsgType == BONUS )
    {
        BS.WriteF32( m_Time );
    }

    BS.WriteMarker();

    s32 i;
    for( i = 0; i < MAX_MSG_ARGS; i++ )
    {
        if( MsgTable[ m_MsgID ].m_Types[ i ] == ARG_STRING )
        {
            WriteWString( BS, m_StringData[ i ] );
        }
        else if( MsgTable[ m_MsgID ].m_Types[ i ] != ARG_NONE )
        {
            // 95% of the values passed will probably be within this range.
            if( BS.WriteFlag( IN_RANGE(0, m_Data[i], 31) ) )
            {
                BS.WriteRangedS32( m_Data[ i ], 0, 31 );
            }
            else
            {
                BS.WriteS32( m_Data[ i ] );
            }
        }
    }

    BS.WriteMarker();

    return( BS.CloseSection() );
}

//==============================================================================

xbool msg::Read( const bitstream& BS )
{
    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( m_MsgID, MSG_NULL, NUM_MESSAGES-1 );

        // If this is true this is a dynamic message and we need the info 
        // about it (so the server thinks).
        if( BS.ReadFlag() )
        {
            BS.ReadMarker();
            MsgTable[ m_MsgID ].ReadMessageInfo( BS );
            BS.ReadMarker();
        }
        
        BS.ReadS32( m_MsgSeq );
        BS.ReadRangedS32( m_Target, 0, 31 );

        if( MsgTable[ m_MsgID ].m_MsgType == GOAL )
        {
            BS.ReadFlag( m_Enabled    );
            BS.ReadS32 ( m_MsgGoalNum );
        }

        if( MsgTable[ m_MsgID ].m_MsgType == GOAL || MsgTable[ m_MsgID ].m_MsgType == BONUS )
        {
            BS.ReadF32( m_Time );
        }

        m_Audience = MsgTable[ m_MsgID ].m_Audience;

        BS.ReadMarker();

        s32 i;
        for( i = 0; i < MAX_MSG_ARGS; i++ )
        {
            if( MsgTable[ m_MsgID ].m_Types[ i ] == ARG_STRING )
            {
                ASSERT( i < MAX_MSG_STRINGS );
                const xwchar* pString = ReadWString( BS );

                const xwchar* pCurrPos = pString;

                for( s32 k = 0; k < MAX_STRING_LENGTH; k++ )
                {
                    m_StringData[ i ][ k ] = *pCurrPos;
                    pCurrPos++;
                    if( m_StringData[ i ][ k ] == 0 )
                    {
                        break;
                    }
                }
                m_StringData[ i ][ MAX_STRING_LENGTH - 1 ] = 0;

                delete pString;
                pString = NULL;
            }
            else if( MsgTable[ m_MsgID ].m_Types[ i ] != ARG_NONE )
            {
                // 95% of the values passed will probably be within this range.
                if( BS.ReadFlag() )
                {
                    BS.ReadRangedS32( m_Data[ i ], 0, 31 );
                }
                else
                {
                    BS.ReadS32( m_Data[ i ] );
                }
            }
        }

        m_bIsLocal = FALSE;

        BS.ReadMarker();

        return( TRUE );
    }
    else
    {
        // Set anything indicating this message is dead?
        // m_MsgID = NULL;
    }

    return( FALSE );
}

//==============================================================================

void msg::DeliverToClient( s32& ClientNum, xbool bDeliver )
{
    u32 BitMask;
    if( bDeliver )
    {
        // Should make sure the bit is set.
        BitMask = 1 << ClientNum;
        m_BitfieldDelivered = m_BitfieldDelivered | BitMask;
    }
    else
    {
        // Should make sure the bit is unset.
        BitMask = ~(1 << ClientNum);
        m_BitfieldDelivered = m_BitfieldDelivered & BitMask;
    }
}

//==============================================================================

xbool msg::NeedsDelivering( s32& ClientNum ) const
{
    u32 BitMask = 1 << ClientNum;

    if( m_BitfieldDelivered & BitMask )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

//==============================================================================

xbool msg::DeliveredToAll( void ) const
{
    if( m_BitfieldDelivered == 0 )
    {
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

//==============================================================================

void msg::SetStringData( s32 ArgNum, const xwchar* pStringData )
{
    ASSERT( x_wstrlen(pStringData)<MAX_STRING_LENGTH );

    s32 i = -1;
    do
    {
        i++;
        m_StringData[ ArgNum ][ i ] = pStringData[ i ];
    }
    while( (pStringData[ i ] != 0) && (i < MAX_STRING_LENGTH) );

    m_StringData[ ArgNum ][ MAX_STRING_LENGTH - 1 ] = 0;
}

//==============================================================================

void msg::GetStringData( s32 ArgNum, xwchar* pStringData )  const
{
    s32 i   = -1;
    do
    {
        i++;
        pStringData[ i ] = m_StringData[ ArgNum ][ i ];
    }
    while( m_StringData[ ArgNum ][ i ] != 0 && i < MAX_STRING_LENGTH );
    pStringData[ MAX_STRING_LENGTH - 1 ] = 0;
}

//==============================================================================

void msg::SetArg( s32 ArgNum, s32 ArgVal )
{
    // ArgNum is in the range [1..MAX_MSG_ARGS].  This matches up with the 
    // <1> <2> <3> and so on in the message source strings.  But, the args
    // stored within the message are in a good old fashioned C array which
    // is indexed [0..MAX_MSG_ARGS-1].  So, subtract one from the parameter
    // to get the index.
    ArgNum--; 

    ASSERT( IN_RANGE( 0, ArgNum, MAX_MSG_ARGS-1 ) );

    if( MsgTable[ m_MsgID ].m_Types[ ArgNum ] == ARG_STRING )
    {
        SetStringData( ArgNum, (const xwchar*)ArgVal );
    }
    else if( MsgTable[ m_MsgID ].m_Types[ ArgNum ] != ARG_NONE )
    {
        m_Data[ ArgNum ] = ArgVal;
    }
}

//==============================================================================

xbool msg::IsValidTarget( s32 TestTarget ) const
{
    msg_hear            Audience    = m_Audience;
    s32                 MsgTarget   = m_Target;
    const game_score&   Score       = GameMgr.GetScore();

    if( !GameMgr.GetScore().Player[ TestTarget ].IsConnected )
    {
        return( FALSE );
    }

    switch( Audience )
    {
    case HEAR_ALL:
        return( TRUE );
        break;

    case HEAR_LOCAL_MACHINE:
        return( TRUE );
        break;

    case HEAR_NONE:
        return( FALSE );
        break;

    case HEAR_PLAYER:
    case HEAR_LOCAL_PLAYER:
        return( MsgTarget == TestTarget );
        break;

    case HEAR_OTHER_PLAYER:
        return( MsgTarget != TestTarget );
        break;

    case HEAR_OTHER_CLIENT:
        return( Score.Player[ TestTarget ].ClientIndex != MsgTarget );
        break;

    case HEAR_TEAM:
        return( Score.Player[ TestTarget ].Team == MsgTarget );
        break;

    case HEAR_CLIENT:
        if( g_NetworkMgr.IsServer() )
            return( Score.Player[ TestTarget ].ClientIndex == MsgTarget );
        else
            return( TRUE );
        break;

    default:
        return( FALSE );
        break;
    }
}

//==============================================================================