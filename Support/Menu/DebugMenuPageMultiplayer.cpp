//==============================================================================
//  DebugMenuPageMultiplayer.cpp
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu multiplayer page.
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "DebugMenu2.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "e_Network.hpp"
#include "NetworkMgr/GameClient.hpp"
#include "NetworkMgr/GameServer.hpp"
#include "NetworkMgr/ClientProxy.hpp"
#include "InputMgr/GamePad.hpp"
#include "Ui/ui_font.hpp"

//==============================================================================
#if defined( ENABLE_DEBUG_MENU )
//==============================================================================

//==============================================================================
//  DATA
//==============================================================================

extern f32 HumanSpeedFactor;
extern f32 MutantSpeedFactor;

extern f32 TweakHeadPain;
extern f32 TweakBodyPain;
extern f32 TweakLimbPain;

extern f32 TweakCostParasite;
extern f32 TweakCostContagion;
extern f32 MPParasiteHealth;

extern f32 MPMutagenBurn;

//==============================================================================
//  FUNCTIONS
//==============================================================================

debug_menu_page_multiplayer::debug_menu_page_multiplayer( void ) 
{
    m_pTitle         = "Multiplayer";
    m_AvatarSkin     = -1;
    m_VoiceActor     = -1;
    m_bToggleNetInfo = FALSE;
    m_iClientIndex   = 0;
    
    AddItemFloat( "Speed Factor - Human",  HumanSpeedFactor,  0.1f, 2.0f, 0.01f );
    AddItemFloat( "Speed Factor - Mutant", MutantSpeedFactor, 0.1f, 2.0f, 0.01f );
    AddItemFloat( "Damage Factor - Head",  TweakHeadPain,     0.1f, 2.0f, 0.01f );
    AddItemFloat( "Damage Factor - Body",  TweakBodyPain,     0.1f, 2.0f, 0.01f );
    AddItemFloat( "Damage Factor - Limb",  TweakLimbPain,     0.1f, 2.0f, 0.01f );
    AddItemFloat( "Parasite - Health Return", MPParasiteHealth,   0.0f,  10.0f, 1.0f );
    AddItemFloat( "Parasite - Cost",          TweakCostParasite,  0.0f, 100.0f, 1.0f );
    AddItemFloat( "Contagion - Cost",         TweakCostContagion, 0.0f, 100.0f, 1.0f );
    AddItemFloat( "Burn Factor - Mutagen",    MPMutagenBurn,      0.1f, 2.0f, 0.05f );

    AddItemSeperator( "" );
    AddItemInt( "% Packet Sends Lost",       ISettings.PercSendsLost,      0, 100 );
    AddItemInt( "% Packet Received Lost",    ISettings.PercReceivesLost,   0, 100 );
//  AddItemInt( "% Packet Swaps",            ISettings.PercPacketsSwapped, 0, 100 );
//  AddItemInt( "% Packet Errors",           ISettings.PercPacketsDamaged, 0, 100 );

    AddItemSeperator( "" );
    m_pPlayerCount =
        AddItemInt( "Player count (for zoning)", GameMgr.m_Score.NPlayers,     1,  32 );
    m_pSkinItem =
        AddItemInt( "Force Player Skin",         m_AvatarSkin,                 -1,  SKIN_END_PLAYERS );
    
    // Spawn point teleport
    m_iSpawnPoint    = 0;
    s32 iLastSpawnPoint = g_ObjMgr.GetNumInstances( object::TYPE_SPAWN_POINT ) -1;
    m_pSpawnPoint = AddItemInt( "Teleport to spawn point", m_iSpawnPoint, 0, iLastSpawnPoint );

    // Net info
    AddItemSeperator( "" );
    AddItemBool( "Toggle NetInfo",   m_bToggleNetInfo );
    m_pClientIndex =
        AddItemInt ( "ClientToWatch",    m_iClientIndex, 0, 31 );

//  m_pVoiceActorItem =
//      AddItemInt( "Force Player Voice",        m_VoiceActor,                 -1,  3 );
}

//==============================================================================

void debug_menu_page_multiplayer::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pSkinItem )
    {
        if( IN_RANGE( 0, pItem->GetValueInt(), SKIN_END_PLAYERS ) )
        {
            GameMgr.DebugSetSkin( pItem->GetValueInt() );
        }
    }
    
    if( pItem == m_pVoiceActorItem )
    {
        if( IN_RANGE( 0, pItem->GetValueInt(), 3 ) )
        {
            GameMgr.DebugSetVoiceActor( pItem->GetValueInt() );
        }
    }

    if( pItem == m_pPlayerCount )
    {
        GameMgr.m_ZoneTimer = 3.0f;
    }

    m_iClientIndex = MAX( m_iClientIndex,  0 );
    m_iClientIndex = MIN( m_iClientIndex, 31 );
    
    // Teleport to spawn point?
    if( pItem == m_pSpawnPoint )
    {
        // Update range
        s32 iLastSpawnPoint = g_ObjMgr.GetNumInstances( object::TYPE_SPAWN_POINT ) -1;
        m_pSpawnPoint->SetIntLimits( 0, iLastSpawnPoint );
    
        // Lookup spawn point ID
        slot_id SpawnPointID = g_ObjMgr.GetFirst( object::TYPE_SPAWN_POINT );
        s32     iSpawnPoint  = 0;
        while( ( iSpawnPoint < m_iSpawnPoint ) && ( SpawnPointID != SLOT_NULL ) )
        {
            // Goto next spawn point
            iSpawnPoint++;
            SpawnPointID = g_ObjMgr.GetNext( SpawnPointID );
        }
        
        // Lookup spawn point and player
        spawn_point* pSpawnPoint = (spawn_point*)g_ObjMgr.GetObjectBySlot( SpawnPointID );
        player*      pPlayer     = SMP_UTIL_GetActivePlayer();
        
        // Teleport?
        if( pSpawnPoint && pPlayer )
        {
            // Update item text
            m_SpawnPointName.Format( "Spawn: %s", 
                                     (const char*)guid_ToString( pSpawnPoint->GetGuid() ) );
            m_pSpawnPoint->SetName( (const char*)m_SpawnPointName );                                     
            
            //  Lookup spawn point info
            vector3 Position;
            radian3 Rotation;
            u16     Zone1;
            u16     Zone2;
            pSpawnPoint->GetSpawnInfo( pPlayer->GetGuid(), Position, Rotation, Zone1, Zone2 );

            // Teleport the player just like in "logic_base::MoveToSpawnPt"
            pPlayer->Teleport( Position, FALSE );
            pPlayer->SetPitch( Rotation.Pitch );
            pPlayer->SetYaw  ( Rotation.Yaw   ); 
            pPlayer->SetZone1( Zone1 );
            pPlayer->SetZone2( Zone2 );
            pPlayer->InitZoneTracking();

            // Force player to update internal view members so view updates
            g_IngamePad[ pPlayer->GetActivePlayerPad() ].ClearAllLogical();
            g_ObjMgr.AdvanceAllLogic( 1.0f / 30.0f );
        }
        else
        {
            // Invalid spawn point - clear info
            m_pSpawnPoint->SetName( "Teleport to spawn point" );
        }
    }
}

//==============================================================================

void debug_menu_page_multiplayer::OnPreRender( void )
{
#if !defined(X_RETAIL) || defined(X_QA)
    if( m_bToggleNetInfo )
    {
        s32 font = g_UiMgr->FindFont("small");
        s32 XRes,YRes;
        eng_GetRes(XRes,YRes);

        irect Rect;
        s32 x = 10;
        s32 y = 50;

        xwstring strPackets, strUpdates, strGameMgr, strVoiceMgr, strUpdateMgr;
        xwstring strPlayerBits, strGhostBits, strOtherBits, strTotalBits;
        xwstring strPains, strPainQueue;

        if( g_NetworkMgr.IsServer() )
        {
            s32          Index      = m_iClientIndex;
            conn_mgr&    ConnMgr    = g_NetworkMgr.GetServerObject().m_pClients[Index].m_ConnMgr;
            conn_stats&  Stats      = ConnMgr.m_StatsDebug;
            f32          fStatsTime = ConnMgr.m_StatsClockDebug;
            s32          Packets    = (s32)(Stats.Packets / fStatsTime);

            strPackets    = (const xstring&)xfs( "%04d Packets per second", Packets);
            strUpdates    = (const xstring&)xfs( "%04d P%03d [%03d] Updates", 
                                (s32) (Stats.Updates / fStatsTime),
                                (s32)((Stats.Updates / fStatsTime) / Packets),
                                Stats.MaxUpdates );
            strGameMgr    = (const xstring&)xfs( "%04d P%03d [%03d] GameMgr Bytes", 
                                (s32) ((Stats.BitsGameMgr    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsGameMgr    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsGameMgr >> 3));
            strVoiceMgr   = (const xstring&)xfs( "%04d P%03d [%03d] VoiceMgr Bytes", 
                                (s32) ((Stats.BitsVoiceMgr    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsVoiceMgr    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsVoiceMgr >> 3));
            strUpdateMgr  = (const xstring&)xfs( "%04d P%03d [%03d] UpdateMgr Bytes", 
                                (s32) ((Stats.BitsUpdateMgr    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsUpdateMgr    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsUpdateMgr >> 3));
            strPlayerBits = (const xstring&)xfs( "%04d P%03d [%03d] Update(player) Bytes", 
                                (s32) ((Stats.BitsPlayerUpdates    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsPlayerUpdates    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsPlayerUpdates >> 3));
            strGhostBits  = (const xstring&)xfs( "%04d P%03d [%03d] Update(ghost) Bytes", 
                                (s32) ((Stats.BitsGhostUpdates    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsGhostUpdates    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsGhostUpdates >> 3));
            strOtherBits  = (const xstring&)xfs( "%04d P%03d [%03d] Update(other) Bytes", 
                                (s32) ((Stats.BitsOtherUpdates    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsOtherUpdates    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsOtherUpdates >> 3));
            strTotalBits  = (const xstring&)xfs( "%04d P%03d [%03d] Total Packet Bytes", 
                                (s32) ((Stats.BitsTotal    >> 3) / fStatsTime),
                                (s32)(((Stats.BitsTotal    >> 3) / fStatsTime) / Packets),
                                (s32)  (Stats.MaxBitsTotal >> 3));

            Rect.Set( x - 4, y, x + 320, y + g_UiMgr->GetLineHeight(font) * 10 );
            draw_Rect( Rect, xcolor(0,0,0,128), FALSE );
        }
        
        if( g_NetworkMgr.IsClient() )
        {
            f32 fStatsTime = g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsClockDebug;
            s32 Packets = (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.Packets / fStatsTime);
            strPackets    = (const xstring&)xfs( "%04d Packets per second", Packets);
            strUpdates    = (const xstring&)xfs( "%04d P%03d [%03d] Updates", 
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.Updates / fStatsTime),
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.Updates / fStatsTime)/ Packets),
                g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxUpdates);
            strGameMgr    = (const xstring&)xfs( "%04d P%03d [%03d] GameMgr Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsGameMgr >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsGameMgr >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsGameMgr >> 3));
            strVoiceMgr   = (const xstring&)xfs( "%04d P%03d [%03d] VoiceMgr Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsVoiceMgr >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsVoiceMgr >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsVoiceMgr >> 3));
            strUpdateMgr  = (const xstring&)xfs( "%04d P%03d [%03d] UpdateMgr Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsUpdateMgr >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsUpdateMgr >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsUpdateMgr >> 3));
            strPlayerBits = (const xstring&)xfs( "%04d P%03d [%03d] Update(player) Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsPlayerUpdates >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsPlayerUpdates >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsPlayerUpdates >> 3));
            strGhostBits  = (const xstring&)xfs( "%04d P%03d [%03d] Update(ghost) Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsGhostUpdates >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsGhostUpdates >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsGhostUpdates >> 3));
            strOtherBits  = (const xstring&)xfs( "%04d P%03d [%03d] Update(other) Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsOtherUpdates >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsOtherUpdates >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsOtherUpdates >> 3));
            strTotalBits  = (const xstring&)xfs( "%04d P%03d [%03d] Total Packet Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsTotal >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsTotal >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsTotal >> 3));
            strPains      = (const xstring&)xfs( "%04d P%03d [%03d] Pains sent", 
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.Pains / fStatsTime),
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.Pains / fStatsTime)/ Packets),
                g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxPains);
            strPainQueue  = (const xstring&)xfs( "%04d P%03d [%03d] Pain Queue Bytes", 
                (s32)((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsPainQueue >> 3) / fStatsTime),
                (s32)(((g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.BitsPainQueue >> 3) / fStatsTime)/ Packets),
                (s32)(g_NetworkMgr.GetClientObject().m_ConnMgr.m_StatsDebug.MaxBitsPainQueue >> 3));

            Rect.Set( x - 4, y, x + 320, y + g_UiMgr->GetLineHeight(font) * 12 );
            draw_Rect( Rect, xcolor(0,0,0,128), FALSE );
        }

        RenderText( strPackets   , x, y, font, Rect );   y += 10;
        RenderText( strUpdates   , x, y, font, Rect );
        RenderText( strGameMgr   , x, y, font, Rect );
        RenderText( strVoiceMgr  , x, y, font, Rect );
        RenderText( strUpdateMgr , x, y, font, Rect );
        RenderText( strPlayerBits, x, y, font, Rect );
        RenderText( strGhostBits , x, y, font, Rect );
        RenderText( strOtherBits , x, y, font, Rect );

        if( g_NetworkMgr.IsClient() )
        {
            RenderText( strPains    , x, y, font, Rect );
            RenderText( strPainQueue, x, y, font, Rect );
        }

        RenderText( strTotalBits , x, y, font, Rect );   
    }
#endif
}


//==============================================================================

void debug_menu_page_multiplayer::RenderText ( const xwstring& strOutput, s32& x, s32& y, const s32& font, irect& Rect )
{
    g_UiMgr->TextSize( font, Rect, strOutput, strOutput.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), strOutput );
    y += g_UiMgr->GetLineHeight(font);
}

//==============================================================================
#endif // defined( ENABLE_DEBUG_MENU )
//==============================================================================
