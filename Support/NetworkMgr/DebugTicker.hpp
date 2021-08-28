//==============================================================================
//
// DebugTicker.hpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#ifndef __DEBUG_TICKER_HPP
#define __DEBUG_TICKER_HPP

#include "x_Files.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"


//#define VERBOSE_DEBUG_TICKS

class debug_ticker
{
public:
        void                Init                ( s32 Port );
        void                Kill                ( void );
        void                Update              ( f32 DeltaTime );
        xbool               ReceiveUpdate       ( const net_address& Remote, bitstream& BitStream );
        void                ProvideUpdate       ( bitstream& BitStream );

private:
        net_socket          m_LocalSocket;
        net_address         m_RemoteAddress;
        f32                 m_TickTimer;
        f32                 m_LastTickTimer;
        xbool               m_SkipTickCheck;
        xbool               m_RemoteIsListening;
        xbool               m_WasNotified;
};


#endif