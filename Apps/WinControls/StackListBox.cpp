// StackListBox.cpp : implementation file
//

#include "stdafx.h"
#include "StackListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CStackListBox
//=========================================================================

CStackListBox::CStackListBox()
{
    m_iStackPtrIndex = 0;
}

//=========================================================================

CStackListBox::~CStackListBox()
{
}

//=========================================================================


BEGIN_MESSAGE_MAP(CStackListBox, CListBox)
	//{{AFX_MSG_MAP(CStackListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CStackListBox message handlers
//=========================================================================

void CStackListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    int         iItem       = lpDrawItemStruct -> itemID;
    int         iAction     = lpDrawItemStruct -> itemAction;
    int         iState      = lpDrawItemStruct -> itemState;
    CRect       rItemRect   ( lpDrawItemStruct -> rcItem );
    CRect       rBlockRect  ( rItemRect );
    CRect       rTextRect   ( rItemRect );
    CBrush      brFrameBrush;
    COLORREF    crNormal    = GetSysColor( COLOR_WINDOW );
    COLORREF    crSelected  = GetSysColor( COLOR_HIGHLIGHT );
    COLORREF    crText      = GetSysColor( COLOR_WINDOWTEXT );
    COLORREF    crPTR;
    CDC         dcContext;

    if( !dcContext.Attach( lpDrawItemStruct -> hDC ) )  // Attach CDC Object
    {
        ASSERT(FALSE);
        return;                                             // Stop If Attach Failed
    }

    brFrameBrush.CreateStockObject( BLACK_BRUSH );          // Create Black Brush

    if( iState & ODS_SELECTED )                 // If Selected
    {                                                       // Set Selected Attributes
        crText = ( 0x00FFFFFF & ~( crText ) );              // Set Inverted Text Color (With Mask)
        dcContext.SetBkColor( crSelected );                 // Set BG To Highlight Color
        dcContext.FillSolidRect( &rBlockRect, crSelected ); // Erase Item
        crPTR = ( 0x00FFFFFF & ~( crSelected ) );
    }
    else                                        // If Not Selected
    {                                                       // Set Standard Attributes
        dcContext.SetBkColor( crNormal );                   // Set BG Color
        dcContext.FillSolidRect( &rBlockRect, crNormal );   // Erase Item
        crPTR = crSelected;
    }

    if( iState & ODS_FOCUS )                    // If Item Has The Focus     
    {
        dcContext.DrawFocusRect( &rItemRect );              // Draw Focus Rect
    }

    //
    // Calculate Text Area
    //
    rTextRect.left += 12;                                   // Set Start Of Text
    rTextRect.top += 2;                                     // Offset A Bit

    //
    // Calculate PTR Area
    //
    rBlockRect.DeflateRect( CSize( 2, 5 ) );                // Reduce Block Size
    rBlockRect.right = 12;                                  // Set Width Of Block

    //
    // Draw Text And PTR
    //
    if( iItem != -1 )                               // If Not An Empty Item
    {
        CString strText;
        GetText( iItem, strText );                          // Get Text
        if( iState & ODS_DISABLED )                         // If Disabled
        {
            crText = GetSysColor( COLOR_INACTIVECAPTIONTEXT );
        }

        dcContext.SetTextColor( crText );                   // Set Text Color
        dcContext.SetBkMode( TRANSPARENT );                 // Transparent Background
        dcContext.TextOut( rTextRect.left, rTextRect.top, strText ); // Draw text      
        
        if (iItem == m_iStackPtrIndex)              // Draw stack pointer
        {
            dcContext.FillSolidRect( &rBlockRect, crPTR ); // Draw Color
            dcContext.FrameRect( &rBlockRect, &brFrameBrush );  // Draw Frame
        }
    }

    dcContext.Detach();                                     // Detach DC From Object
}

//=========================================================================

BOOL CStackListBox::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.style |= LBS_OWNERDRAWFIXED | LBS_HASSTRINGS;	
	return CListBox::PreCreateWindow(cs);
}

//=========================================================================

void CStackListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your code to determine the size of specified item
	
}