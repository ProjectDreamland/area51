//==============================================================================
//  DebugMenuPage.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu page.
//  
//==============================================================================

#include "DebugMenu2.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

static s32 DEBUG_MENU_ITEM_START       = 34;
static s32 DEBUG_MENU_SEPERATOR_HEIGHT = 14;
static s32 DEBUG_MENU_TEXT_HEIGHT      = 18;

//==============================================================================

debug_menu_page::debug_menu_page( void )
{
    m_pTitle        = "<null>";
    m_iActiveItem   = 0;
}

//==============================================================================

debug_menu_page::~debug_menu_page( void )
{
}

//==============================================================================

debug_menu_item& debug_menu_page::GetActiveItem( void )
{
    ASSERT( m_Items.GetCount() > 0 );

    return *m_Items[m_iActiveItem];
}

//==============================================================================

void debug_menu_page::NextItem( void )
{
    ASSERT( m_Items.GetCount() > 0 );

    s32 m_iOldItem = m_iActiveItem;
    do
    {
        m_iActiveItem = (m_iActiveItem + 1);
        if( m_iActiveItem >= m_Items.GetCount() )
            m_iActiveItem = 0;
    } while( (m_iOldItem != m_iActiveItem) && (m_Items[m_iActiveItem]->GetType() == debug_menu_item::TYPE_SEPERATOR) );
}

//==============================================================================

void debug_menu_page::PrevItem( void )
{
    ASSERT( m_Items.GetCount() > 0 );

    s32 m_iOldItem = m_iActiveItem;
    do
    {
        m_iActiveItem = (m_iActiveItem - 1);
        if( m_iActiveItem < 0 )
            m_iActiveItem = m_Items.GetCount()-1;
    } while( (m_iOldItem != m_iActiveItem) && (m_Items[m_iActiveItem]->GetType() == debug_menu_item::TYPE_SEPERATOR) );
}

//==============================================================================

s32 debug_menu_page::GetItemCount( void )
{
    return m_Items.GetCount();
}

//==============================================================================

debug_menu_item* debug_menu_page::GetItem( s32 ItemNum )
{
    ASSERT( (ItemNum >= 0) && (ItemNum < m_Items.GetCount()) );

    return m_Items[ItemNum];
}

//==============================================================================
debug_menu_item* debug_menu_page::FindItem( const char* pName )
{
    // look for the page with a given title string
    for (s32 iItem = 0; iItem < m_Items.GetCount(); iItem++ )
    {
        if( strcmp(m_Items[iItem]->GetName(), pName) == 0 ) return m_Items[iItem];
    }
    return NULL;
}

//==============================================================================

const char* debug_menu_page::GetTitle( void )
{
    return m_pTitle;
}

//==============================================================================

void debug_menu_page::Render( void )
{
    const s32 PAGE_TITLE_FONT = 0;
    s32 CursorX = 150;
    s32 CursorY = 30;

    // Display page title.
    irect TextPos;
    // TODO: make this better.
    TextPos.l = CursorX;
    TextPos.t = CursorY;
    TextPos.r = TextPos.l + 400;
    TextPos.b = TextPos.t + 20;

    ASSERT( m_pTitle != NULL );
    debug_menu2::RenderLine( PAGE_TITLE_FONT, m_pTitle, TextPos );

    // Update Cursor position for first menu item.
    CursorX = 50;
    CursorY += DEBUG_MENU_ITEM_START;  //TODO: fix this too... 

    for( s32 i=0; i<m_Items.GetCount(); i++ )
    {
        debug_menu_item* pItem = m_Items[i];
        ASSERT( pItem );
        
        pItem->Render( CursorX, CursorY, (i == m_iActiveItem) );
        
        // Blank line?
        if(     ( pItem->GetType() == debug_menu_item::TYPE_SEPERATOR )
            &&  ( !pItem->GetName() || !x_strlen( pItem->GetName() ) ) )
        {
            CursorY += DEBUG_MENU_SEPERATOR_HEIGHT;   // Blank line
        }            
        else
        {
            CursorY += DEBUG_MENU_TEXT_HEIGHT;  //TODO: make line height...
        }            
    }
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemSeperator( const char* pName )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitSeperator( pName );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemButton( const char* pName )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitButton( pName );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemBool( const char* pName, xbool& Var, const char** pValues )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitBool( pName, Var, pValues );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemInt( const char* pName, s32& Var, s32 Min, s32 Max )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitInt( pName, Var, Min, Max );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemFloat( const char* pName, f32& Var, f32 Min, f32 Max, f32 Increment )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitFloat( pName, Var, Min, Max, Increment );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

debug_menu_item* debug_menu_page::AddItemEnum( const char* pName, s32& Var, const char** pValues, s32 nValues )
{
    m_Items.Append() = new debug_menu_item();
    m_Items[m_Items.GetCount()-1]->InitEnum( pName, Var, pValues, nValues );
    return m_Items[m_Items.GetCount()-1];
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
