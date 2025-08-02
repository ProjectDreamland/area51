//=========================================================================
//
//  ui_combo.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"

#include "ui_combo.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"
#include "ui_listbox.hpp"
#include "ui_dlg_list.hpp"
//#include "ui_dlg_combolist.hpp"

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_combo_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_combo* pcombo = new ui_combo;
    pcombo->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pcombo;
}

//=========================================================================
//  ui_combo
//=========================================================================

ui_combo::ui_combo( void )
{
}

//=========================================================================

ui_combo::~ui_combo( void )
{
    Destroy();
}

//=========================================================================

xbool ui_combo::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize data
    
    m_iElement1 = m_pManager->FindElement( "button_combo1" );
    //m_iElement2 = m_pManager->FindElement( "button_combo2" );
    ASSERT( m_iElement1 != -1 );
    //ASSERT( m_iElement2 != -1 );

    m_NavFlags          = 0;
    m_iSelection        = -1;
    m_LabelWidth        = 0;
    m_Font              = g_UiMgr->FindFont("small");

    return Success;
}

//=========================================================================

void ui_combo::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  LabelColor1 = XCOLOR_WHITE;
        xcolor  LabelColor2 = XCOLOR_BLACK;
        xcolor  TextColor1  = XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;
        s32     State       = 0;

        // Calculate rectangle
        irect    br;
        irect    r, r2;
        br.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );
        r = br;
        r2 = r;
        r.r = r.l + m_LabelWidth;
        r2.l = r.r;

        // set item color
        if( m_iSelection != -1 )
        {
            LabelColor1 = m_Items[m_iSelection].Color; 
            TextColor1  = m_Items[m_iSelection].Color; 
        }
        else
        {
            LabelColor1 = xcolor(255,252,204,255);
            TextColor1  = xcolor(255,252,204,255);
        }

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            LabelColor1 = XCOLOR_GREY;
            LabelColor2 = XCOLOR_BLACK;
            TextColor1  = XCOLOR_GREY;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            LabelColor2 = XCOLOR_BLACK;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            LabelColor2 = XCOLOR_BLACK;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            LabelColor2 = XCOLOR_BLACK;
            TextColor2  = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            LabelColor2 = XCOLOR_BLACK;
            TextColor2  = XCOLOR_BLACK;
        }

        if( m_iSelection != -1 )
        {
            if( !m_Items[m_iSelection].Enabled )
            {
                TextColor1  = XCOLOR_GREY;
                TextColor2  = XCOLOR_BLACK;
            }

            // Add Highlight to list
            if( m_Flags & WF_HIGHLIGHT )
                m_pManager->AddHighlight( m_UserID, br );
        }

        // Render bitmap (if any)
        if( m_iSelection != -1 )
        {
            if( m_Items[m_iSelection].BitmapID != -1 )
            {
                m_pManager->RenderBitmap( m_Items[m_iSelection].BitmapID, r2 );
                // Render single field Combo
                //r2.Inflate( 12, 0 );
                //r2.Translate( 0, 1 );
                //m_pManager->RenderElement( m_iElement2, r2, 0 );
            }
            else
            {
                // Render single field Combo
                m_pManager->RenderElement( m_iElement1, r2, State );
            }
        }
        else
        {
            // Render single field Combo
            m_pManager->RenderElement( m_iElement1, r2, State );
        }

        // Render Selection Text
        if( m_iSelection != -1 )
        {
#if defined(TARGET_PC)
            r2.Translate( 0, -22 );
#else
            r2.Translate( 0, -10 );
#endif
            m_pManager->RenderText( m_Font, r2, ui_font::h_center|ui_font::v_center, TextColor1, m_Items[m_iSelection].Label );
        }

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_combo::OnCursorEnter( ui_win* pWin )
{
    ui_control::OnCursorEnter( pWin );
}

//=========================================================================

void ui_combo::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    xbool bHandled = FALSE;

    if ( m_NavFlags & CB_CHANGE_ON_NAV )
    {
        // check 
        if (Code == ui_manager::NAV_LEFT)
        {
            // Move Back in List
            OnPadShoulder( pWin, -1 );
            bHandled = TRUE;
        }
        else if (Code == ui_manager::NAV_RIGHT)
        {
            // Move Forward in List
            OnPadShoulder( pWin, 1 );
            bHandled = TRUE;
        }

        // Pass up chain
        if( m_pParent )
        {
            if( ( m_NavFlags & CB_NOTIFY_PARENT ) || ( !bHandled ) )
            {
                m_pParent->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
            }
        }
    }
    else
    {
        // Pass up chain
        if( m_pParent )
            m_pParent->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
    }
}

//=========================================================================

void ui_combo::OnPadSelect( ui_win* pWin )
{
    if ( m_NavFlags & CB_CHANGE_ON_SELECT )
    {
        // Move Forward in List
        OnPadShoulder( pWin, 1 );

        // Pass up chain
        if( m_pParent )
        {
            if( m_NavFlags & CB_NOTIFY_PARENT ) 
            {
                m_pParent->OnPadSelect( pWin );
            }
        }
    }
    else
    {
        if ( m_pParent )
            m_pParent->OnPadSelect ( pWin );
    }
}

//=========================================================================

void ui_combo::OnPadShoulder( ui_win* pWin, s32 Direction )
{
    (void)pWin;

    // Apply movement
    if( Direction != 0 )
    {
        // check that we have > 1 item
        if( m_Items.GetCount() < 2 )
        {
            g_AudioMgr.Play( "InvalidEntry" ); 
        }
        else
        {
            s32 OldSelection = m_iSelection;

            m_iSelection += Direction;
            if( m_iSelection < 0 )
                m_iSelection = m_Items.GetCount()-1;
            if( m_iSelection >= m_Items.GetCount() )
                m_iSelection = 0;
            if( m_Items.GetCount() == 0 )
                m_iSelection = -1;

            if( (m_iSelection != OldSelection) && m_pParent )
            {
                m_pParent->OnNotify( m_pParent, this, WN_COMBO_SELCHANGE, (void*)m_iSelection );
                g_AudioMgr.Play( "Toggle" ); 
            }
        }
    }
}

//=========================================================================

void ui_combo::SetLabelWidth( s32 Width )
{
    m_LabelWidth = Width;
}

//=========================================================================

s32 ui_combo::AddItem( const xwstring& Label, s32 Data1, s32 Data2 )
{
    item& Item      = m_Items.Append();
    Item.Label      = Label;
    Item.Enabled    = TRUE;
    Item.Data[0]    = Data1;
    Item.Data[1]    = Data2;
    Item.BitmapID   = -1;
    Item.Color      = xcolor(255,252,204,255);

    return m_Items.GetCount()-1;
}

//=========================================================================

s32 ui_combo::AddItem( const xwchar* Label, s32 Data1, s32 Data2 )
{
    item& Item      = m_Items.Append();
    Item.Label      = Label;
    Item.Enabled    = TRUE;
    Item.Data[0]    = Data1;
    Item.Data[1]    = Data2;
    Item.BitmapID   = -1;
    Item.Color      = xcolor(255,252,204,255);

    return m_Items.GetCount()-1;
}

//=========================================================================

void ui_combo::SetItemEnabled( s32 iItem, xbool State )
{
    (void)iItem;
    (void)State;

    m_Items[iItem].Enabled = State;
}

//=========================================================================

void ui_combo::SetItemBitmap( s32 iItem, s32 ID )
{
    m_Items[iItem].BitmapID = ID;
}

//=========================================================================

void ui_combo::SetItemColor( s32 iItem, xcolor Color )
{
    m_Items[iItem].Color = Color;
}

//=========================================================================

void ui_combo::DeleteAllItems( void )
{
    m_iSelection = -1;
    m_Items.Delete( 0, m_Items.GetCount() );

    if( m_pParent )
        m_pParent->OnNotify( m_pParent, this, WN_COMBO_SELCHANGE, (void*)m_iSelection );
}

//=========================================================================

void ui_combo::DeleteItem( s32 iItem )
{
    s32 OldSelection = m_iSelection;

    if( iItem < m_iSelection ) m_iSelection--;
    if( m_iSelection < 0 ) m_iSelection = 0;
    if( m_iSelection > m_Items.GetCount() ) m_iSelection = m_Items.GetCount() - 1;
    if( m_Items.GetCount() == 0 ) m_iSelection = -1;
    m_Items.Delete( iItem );

    if( (m_iSelection != OldSelection) && m_pParent )
        m_pParent->OnNotify( m_pParent, this, WN_COMBO_SELCHANGE, (void*)m_iSelection );
}

//=========================================================================

s32 ui_combo::GetItemCount( void ) const
{
    return m_Items.GetCount();
}

//=========================================================================

const xwstring& ui_combo::GetItemLabel( s32 iItem ) const
{
    ASSERT( (iItem >= 0) && (iItem < m_Items.GetCount()) );

    return m_Items[iItem].Label;
}

//=========================================================================

s32 ui_combo::GetItemBitmap( s32 iItem ) const
{
    return m_Items[iItem].BitmapID;
}

//=========================================================================

s32 ui_combo::GetItemData( s32 iItem, s32 Index ) const
{
    ASSERT( (iItem >= 0) && (iItem < m_Items.GetCount()) );
    ASSERT( (Index >= 0) && (Index < COMBO_DATA_FIELDS) );

    return m_Items[iItem].Data[Index];
}

//=========================================================================

const xwstring& ui_combo::GetSelectedItemLabel( void ) const
{
    ASSERT( (m_iSelection >= 0) && (m_iSelection < m_Items.GetCount()) );

    return m_Items[m_iSelection].Label;
}

//=========================================================================

s32 ui_combo::GetSelectedItemData( s32 Index ) const
{
    ASSERT( (m_iSelection >= 0) && (m_iSelection < m_Items.GetCount()) );
    ASSERT( (Index >= 0) && (Index < COMBO_DATA_FIELDS) );

    return m_Items[m_iSelection].Data[Index];
}

//=========================================================================

xbool ui_combo::GetItemEnabled( s32 iItem ) const
{
    ASSERT( (iItem >= 0) && (iItem < m_Items.GetCount()) );
    return (m_Items[iItem].Enabled);
}

//=========================================================================

xbool ui_combo::GetSelectedItemEnabled( void ) const
{
    ASSERT( (m_iSelection >= 0) && (m_iSelection < m_Items.GetCount()) );
    return (m_Items[m_iSelection].Enabled);
}

//=========================================================================

s32 ui_combo::FindItemByLabel( const xwstring& Label )
{
    s32     i;
    s32     iFound = -1;

    for( i=0 ; i<m_Items.GetCount() ; i++ )
    {
        if( m_Items[i].Label == Label )
        {
            iFound = i;
            break;
        }
    }

    return iFound;
}

//=========================================================================

s32 ui_combo::FindItemByData( s32 Data, s32 Index )
{
    ASSERT( (Index >= 0) && (Index < COMBO_DATA_FIELDS) );

    s32     i;
    s32     iFound = -1;

    for( i=0 ; i<m_Items.GetCount() ; i++ )
    {
        if( m_Items[i].Data[Index] == Data )
        {
            iFound = i;
            break;
        }
    }

    return iFound;
}

//=========================================================================

s32 ui_combo::GetSelection( void ) const
{
    return m_iSelection;
}

//=========================================================================

void ui_combo::SetSelection( s32 iSelection )
{
    ASSERT( (iSelection >= -1) && (iSelection < m_Items.GetCount()) );

    m_iSelection = iSelection;

    if( m_pParent )
        m_pParent->OnNotify( m_pParent, this, WN_COMBO_SELCHANGE, (void*)m_iSelection );
}

//=========================================================================

void ui_combo::ClearSelection( void )
{
    m_iSelection = -1;
}

//=========================================================================

void ui_combo::OnLBDown ( ui_win* pWin )
{
    (void)pWin;
#ifdef TARGET_PC
    // Throw up default config selection dialog
    irect r = GetPosition();
    r.Translate( -r.l, -r.t + r.GetHeight() );
    LocalToScreenCreate( r );

    // Scale the list box to a resonable size.
    s32 Items = m_Items.GetCount()-1;
    if( Items < 3 )
        Items = 3;
    r.b = r.t + 26+18*(Items);
    s32 x, y;
    eng_GetRes( x, y );
    
    // Keep the list box inside the viewport.
    if( r.b > y )
    {
        s32 diff = r.b - y;
        r.Translate( 0, -diff );
    }

    // Open a list box with all the items listed in it.
    ui_dlg_list* pListDialog = (ui_dlg_list*)m_pManager->OpenDialog( m_UserID, "ui_list", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL );
    ui_listbox* pList = pListDialog->GetListBox();
    pListDialog->SetResultPtr( &m_iSelection );

    // Set the items label.
    for( s32 i=0 ; i < m_Items.GetCount(); i++ )
    {
        pList->AddItem( m_Items[i].Label, i );
    }

    pList->SetSelection( 0 );
#endif
}

//=========================================================================
