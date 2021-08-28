//=========================================================================
//
//  ui_joinlist.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_listbox.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_font.hpp"
#include "ui_joinlist.hpp"

#include "StateMgr/StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"

#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=========================================================================
//  Defines
//=========================================================================

#define SPACE_TOP       4
#define SPACE_BOTTOM    4
#define TEXT_OFFSET     -2

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_joinlist_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_joinlist* pList = new ui_joinlist;
    pList->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pList;
}

//=========================================================================
//  ui_listbox
//=========================================================================

ui_joinlist::ui_joinlist( void )
{
    // load mutation mode icons
    MutationIcon[ICON_MUTANT]   = g_UiMgr->LoadBitmap( "MutantIcon", "UI_MutantIcon.xbmp" );
    MutationIcon[ICON_HUMAN]    = g_UiMgr->LoadBitmap( "HumanIcon",  "UI_HumanIcon.xbmp"  );
    MutationIcon[ICON_VS]       = g_UiMgr->LoadBitmap( "VersusIcon", "UI_VersusIcon.xbmp" );
    MutationIcon[ICON_CYCLE]    = g_UiMgr->LoadBitmap( "CycleIcon",  "UI_CycleIcon.xbmp"  );
}

//=========================================================================

ui_joinlist::~ui_joinlist( void )
{
#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmaps
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif
    // unload mutation mode icons
    g_UiMgr->UnloadBitmap( "MutantIcon" );
    g_UiMgr->UnloadBitmap( "HumanIcon"  );
    g_UiMgr->UnloadBitmap( "VersusIcon" );
    g_UiMgr->UnloadBitmap( "CycleIcon"  );
}

//=========================================================================

void ui_joinlist::Render( s32 ox, s32 oy )
{
    s32     i;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1  = XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;
        s32     State       = ui_manager::CS_NORMAL;

        // Calculate rectangle
        irect   br;
        irect   r;
        irect   r2;
        br.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );
        r = br;
        r2 = r;
        r.r -= 14;
        r2.l = r.r;

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            TextColor1  = XCOLOR_GREY;
            TextColor2  = xcolor(0,0,0,0);
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }

        // Add Highlight to list
        if( m_Flags & WF_HIGHLIGHT )
            m_pManager->AddHighlight( m_UserID, br, !(m_Flags & WF_SELECTED) );

        if (g_UiMgr->IsWipeActive())
        {
            irect wipePos;
            g_UiMgr->GetWipePos(wipePos);

            s32 XRes;
            s32 YRes;
            eng_GetRes( XRes, YRes );
            irect clipRect = r;

            if( clipRect.l < 0 )
            {
                clipRect.l = 0;
                clipRect.r = XRes;
            }

            if ( wipePos.b > r.t )
            {
                if ( wipePos.b > r.b )
                {
#ifdef TARGET_PS2
                    gsreg_Begin( 1 );
                    gsreg_SetScissor( clipRect.l, clipRect.t, clipRect.r, clipRect.b );
                    gsreg_End();
#endif
                }
                else
                {
#ifdef TARGET_PS2
                    gsreg_Begin( 1 );
                    gsreg_SetScissor( clipRect.l, clipRect.t, clipRect.r, wipePos.b );
                    gsreg_End();
#endif
                }
            }
        }
        else
        {
            m_pManager->PushClipWindow( r );
        }


        irect   rb = r;

        if (m_ShowFrame)
            rb.Deflate( 1, 1 );

        // render header bar
        if( m_ShowHeaderBar )
        {
            irect hb = rb;
            hb.SetHeight( 22 );          

            m_pManager->RenderRect( hb, m_HeaderBarColor, FALSE );

            // render the header text
            hb.l += (s32)(8.0f * m_pManager->GetScaleX());
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, xwstring( xstring( (char)0x88 ) ) );   // Headset enabled
            hb.l += (s32)(14.0f * m_pManager->GetScaleX());
#ifdef TARGET_XBOX
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, xwstring( xstring( (char)0x80 ) ) );   // Buddy on server
#else
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, xwstring( xstring( (char)0x85 ) ) );   // Buddy on server
#endif
            hb.l += (s32)(14.0f * m_pManager->GetScaleX());
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, xwstring( xstring( (char)0x16 ) ) );   // Server has password            
            hb.l += (s32)(16.0f * m_pManager->GetScaleX());
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, g_StringTableMgr( "ui", "IDS_HEADER_SERVER" ) );
            irect hp = hb;
            hp.l += (s32)(91.0f * m_pManager->GetScaleX());
            hp.r = hp.l + (s32)(84.0f * m_pManager->GetScaleX());
            RenderTitle( hp, ui_font::h_right|ui_font::v_center, g_StringTableMgr( "ui", "IDS_HEADER_PLAYERS" ) );
            hb.l += (s32)(122.0f * m_pManager->GetScaleX());
            hb.l += (s32)(60.0f * m_pManager->GetScaleX());
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, g_StringTableMgr( "ui", "IDS_HEADER_MUTANT_MODE" ) );
            hb.l += (s32)(66.0f * m_pManager->GetScaleX());
            RenderTitle( hb, ui_font::h_left|ui_font::v_center, g_StringTableMgr( "ui", "IDS_HEADER_MAPNAME" ) );
            

            rb.t += 22;
            r2.t += 22;
        }
        
        // Render background color       
        m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
        
        
        // Render Text & Selection Marker
        irect rl = rb;//r;
        rl.SetHeight( m_LineHeight );
        rl.Deflate( 2, 0 );
        rl.r -= 2;
        rl.Translate( 0, SPACE_TOP );

        // check for empty list
        if ( m_Items.GetCount() == 0 )
        {
            if( m_Flags & (WF_SELECTED) )
            {
                // render cursor bar
                s32 alpha = 128 + (g_UiMgr->GetHighlightAlpha(8) * 8); // 64<->192
                m_pManager->RenderRect( rl, xcolor(79,214,60,alpha), FALSE );

                if( m_Flags & WF_HIGHLIGHT )
                    m_pManager->AddHighlight( m_UserID, rl );
            }
        }
        else
        {
            for( i=0 ; i<m_nVisibleItems ; i++ )
            {
                s32 iItem = m_iFirstVisibleItem + i;

                if( (iItem >= 0) && (iItem < m_Items.GetCount()) )
                {
                    // Render Selection Rectangle
                    if( (iItem == m_iSelection)  && m_ShowBorders )
                    {
                        if( m_Flags & (WF_SELECTED) )
                        {
                            s32 alpha = 128 + (g_UiMgr->GetHighlightAlpha(8) * 8); // 64<->192
                            m_pManager->RenderRect( rl, xcolor(79,214,60,alpha), FALSE );

                            if( m_Flags & WF_HIGHLIGHT )
                                m_pManager->AddHighlight( m_UserID, rl );
                        }
                        else
                            m_pManager->RenderRect( rl, xcolor(66,158,11,192), FALSE );
                    }
    #ifdef TARGET_PC
                    // Let the hight light track the mouse cursor.
                    if( iItem == m_TrackHighLight )
                    {
                        m_pManager->AddHighlight( m_UserID, rl );
                    }
    #endif

                    // Render Text
                    xcolor c1 = m_Items[iItem].Color;
                    xcolor c2 = TextColor2;
                    if( !m_Items[iItem].Enabled )
                    {
                        c1 = XCOLOR_GREY;
                        c2 = xcolor(0,0,0,0);
                    }
                    else if (iItem == m_iSelection)
                    {
                        if ( m_Flags & WF_SELECTED )
                        {
                            c1 = xcolor(0,0,0,255);
                        }
                        else
                        {
                            c1 = xcolor(126,220,60,255);
                        }
                        c2 = xcolor(0,0,0,0);
                    }
                    else
                    {
                        // check for server full
                        const server_info* pServerInfo = g_MatchMgr.GetServerInfo(m_Items[iItem].Data[0]);

                        // set display color based on ping and max players
                        if( pServerInfo->Players >= pServerInfo->MaxPlayers )
                        {
                            // server is maxed out- display in grey!
                            c1 = XCOLOR_GREY;
                        }
                        else
                        {
                            g_UiMgr->PingToColor( pServerInfo->PingDelay, c1 );
                        }
                    }
                    irect rl2 = rl;

    //				m_pManager->PushClipWindow( rl2 );

                    RenderItem( rl2, m_Items[iItem], c1, c2 );

				    // Clear the clip window
    //				m_pManager->PopClipWindow();

                }
                rl.Translate( 0, m_LineHeight );
            }
        }

        if (g_UiMgr->IsWipeActive())
        {
#ifdef TARGET_PS2
            // restore correct scissor
            irect wipePos;
            g_UiMgr->GetWipePos(wipePos);

            irect screen;
            g_UiMgr->GetScreenSize(screen);
            
            s32 XRes;
            s32 YRes;
            eng_GetRes( XRes, YRes );
            
            if( screen.l < 0 )
            {
                screen.l = 0;
                screen.r = XRes;
            }

            gsreg_Begin( 1 );
            gsreg_SetScissor( screen.l, screen.t, screen.r, wipePos.b );
            gsreg_End();
#endif
        }
        else
        {
            m_pManager->PopClipWindow();
        }

        if (m_ShowBorders)
        {
            // Render Frame
            if (m_ShowFrame)
                m_pManager->RenderElement( m_iElementFrame, r, 0 );

            irect r3 = r2;
            irect r4 = r2;
            r3.b = r3.t + 16;
            r4.t = r4.b - 16;
            r2.t = r3.b;
            r2.b = r4.t;

#ifdef TARGET_PC
            m_UpArrow = r3;
            m_DownArrow = r4;
#endif
            m_pManager->RenderElement( m_iElement_sb_container, r2, State );
            m_pManager->RenderElement( m_iElement_sb_arrowup,   r3, State );
            m_pManager->RenderElement( m_iElement_sb_arrowdown, r4, State );

            // Render thumb background
            r2.Deflate( 1, 1 );
            r2.l += 1;
            m_pManager->RenderRect( r2, xcolor(20,80,13,128), FALSE );


            // Render Thumb
            r2.Deflate( 1, 1 );
            r2.l += 1;
         
			s32 itemcount;

			itemcount = m_Items.GetCount(); //0;
			//for (s32 i=0;i<m_Items.GetCount();i++)
			//{
			//	if (m_Items[i].Enabled)
			//		itemcount++;
			//}

            if( itemcount > m_nVisibleItems )
            {
                s32 ThumbSize = (s32)(r2.GetHeight() * ((f32)m_nVisibleItems / itemcount));
                if( ThumbSize < 16 )
                    ThumbSize = 16;

                s32 ThumbPos  = (s32)((r2.GetHeight()-ThumbSize) * ((f32)m_iFirstVisibleItem / (itemcount - m_nVisibleItems)));

                r2.Set( r2.l, r2.t + ThumbPos, r2.r, r2.t + ThumbPos + ThumbSize );
            }

            m_pManager->RenderElement( m_iElement_sb_thumb,     r2, State );
#ifdef TARGET_PC
            m_ScrollBar = r2;
#endif
        }

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

static void GetMapNameString( xwchar* pBuffer, const xwchar* GameType, const xwchar* MissionName )
{
    x_wstrcpy( pBuffer, GameType );
    x_mstrcat( pBuffer, ":" );
    x_wstrcat( pBuffer, MissionName );
}

//=========================================================================

void ui_joinlist::RenderString( irect r, u32 Flags, const xcolor& c1, const xcolor& c2, const char* pString )
{
    m_pManager->RenderText( m_Font, r, Flags, c2, pString );
    r.Translate( -1, -1 );
    m_pManager->RenderText( m_Font, r, Flags, c1, pString );
}

//=========================================================================

void ui_joinlist::RenderString( irect r, u32 Flags, const xcolor& c1, const xcolor& c2, const xwchar* pString )
{
    m_pManager->RenderText( m_Font, r, Flags, c2, pString );
    r.Translate( -1, -1 );
    m_pManager->RenderText( m_Font, r, Flags, c1, pString );
}

//=========================================================================

void ui_joinlist::RenderTitle( irect r, u32 Flags, const xwchar* pString )
{
    m_pManager->RenderText( m_Font, r, Flags, XCOLOR_BLACK, pString );
    r.Translate( -1, -1 );
    m_pManager->RenderText( m_Font, r, Flags, m_HeaderColor, pString );
}

//=========================================================================

void ui_joinlist::RenderItem( irect r, const item& Item, const xcolor& c1, const xcolor& c2 )
{
    //r.Deflate( 4, 0 );
    //r.Translate( 1, -2 );

    if (m_ShowFrame)
        r.Deflate( 1, 1 );

    r.Translate( 0, -2 );

    irect rIcons   = r;
    irect rName    = r;
    irect rPlayers = r;
    irect rMode    = r;
    irect rMap     = r;

    rIcons.l   = (s32)((f32)(r.l +   5) * m_pManager->GetScaleX());
    rIcons.r   = (s32)((f32)(r.l +  47) * m_pManager->GetScaleX());
    rName.l    = (s32)((f32)(r.l +  49) * m_pManager->GetScaleX());
    rName.r    = (s32)((f32)(r.l + 172) * m_pManager->GetScaleX());
    rPlayers.l = (s32)((f32)(r.l + 172) * m_pManager->GetScaleX());
    rPlayers.r = (s32)((f32)(r.l + 225) * m_pManager->GetScaleX());
    rMode.l    = (s32)((f32)(r.l + 230) * m_pManager->GetScaleX());
    rMode.r    = rMode.l + 16;
    rMap.l     = (s32)((f32)(r.l + 297) * m_pManager->GetScaleX());
    rMap.r     = (s32)((f32)(r.r - 4  ) * m_pManager->GetScaleX());

    const server_info* pServerInfo = g_MatchMgr.GetServerInfo(Item.Data[0]);

    ASSERT( pServerInfo );
    {
        xstring Name;
        xstring Players;
        xwchar MapName[128];

        Name     = pServerInfo->Name;
        Players  = xfs( "%d/%d", pServerInfo->Players, pServerInfo->MaxPlayers );
        GetMapNameString( MapName, pServerInfo->ShortGameType, pServerInfo->MissionName );

        if( pServerInfo->Flags & SERVER_VOICE_ENABLED )
        {
            RenderString( rIcons  , ui_font::h_left  |ui_font::v_center,                        c1, c2, xstring( (char)0x88 ) );
        }

        rIcons.l += (s32)(14.0f * m_pManager->GetScaleX());
        if( pServerInfo->Flags & SERVER_HAS_BUDDY )
        {
#ifdef TARGET_XBOX
            RenderString( rIcons  , ui_font::h_left  |ui_font::v_center,                        c1, c2, xstring( (char)0x80 ) );
#else
            RenderString( rIcons  , ui_font::h_left  |ui_font::v_center,                        c1, c2, xstring( (char)0x85 ) );
#endif
        }

        rIcons.l += (s32)(14.0f * m_pManager->GetScaleX());
        if( pServerInfo->Flags & SERVER_HAS_PASSWORD )
        {
            RenderString( rIcons  , ui_font::h_left  |ui_font::v_center,                        c1, c2, xstring( (char)0x16 ) );
        }


        RenderString( rName   , ui_font::h_left  |ui_font::v_center|ui_font::clip_ellipsis, c1, c2, (const char*)Name );
        RenderString( rPlayers, ui_font::h_center|ui_font::v_center|ui_font::clip_ellipsis, c1, c2, (const char*)Players );
        
        // set mutant icon color
        xcolor iconColor;
        if( c1 == XCOLOR_GREY )
        {
            iconColor = XCOLOR_GREY;
        }
        else if( c1 == xcolor(0,0,0,255) )
        {
            iconColor = XCOLOR_GREY;
        }
        else
        {
            iconColor = XCOLOR_WHITE;
        }

        //render icons based on mutation mode
        switch( pServerInfo->MutationMode )
        {
            case MUTATE_CHANGE_AT_WILL:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_CYCLE],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                break;

            case MUTATE_HUMAN_LOCK:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                break;

            case MUTATE_MUTANT_LOCK:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                break;

            case MUTATE_HUMAN_VS_MUTANT:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_VS],     rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                break;

            case MUTATE_MAN_HUNT:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                break;

            case MUTATE_MUTANT_HUNT:
                g_UiMgr->RenderBitmap( MutationIcon[ICON_MUTANT], rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                rMode.Translate( 16, 0 );
                g_UiMgr->RenderBitmap( MutationIcon[ICON_HUMAN],  rMode, iconColor );
                break;
        }
        
        RenderString( rMap    , ui_font::h_left  |ui_font::v_center|ui_font::clip_ellipsis, c1, c2, MapName );
    }
}

//=========================================================================
