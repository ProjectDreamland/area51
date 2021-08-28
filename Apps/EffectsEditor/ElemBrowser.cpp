// ElemBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "Auxiliary/fx_core/controller.hpp"
#include "Auxiliary/fx_core/element.hpp"
#include "Auxiliary/fx_core/effect.hpp"
#include "PartEdDoc.h"
#include "ElemBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CElemBrowser dialog


CElemBrowser::CElemBrowser(CWnd* pParent /*=NULL*/)
	: CDialog(CElemBrowser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CElemBrowser)
	m_ShouldHide = FALSE;
	//}}AFX_DATA_INIT

    m_pDoc = NULL;
    m_pData = NULL;
}

CElemBrowser::~CElemBrowser()
{
    if ( m_pData )
        delete[] m_pData;
}

void CElemBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CElemBrowser)
	DDX_Control(pDX, IDC_DOWN, m_Down);
	DDX_Control(pDX, IDC_UP, m_Up);
	DDX_Control(pDX, IDC_ELEM_LIST, m_ElemList);
	DDX_Check(pDX, IDC_HIDE, m_ShouldHide);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CElemBrowser, CDialog)
	//{{AFX_MSG_MAP(CElemBrowser)
	ON_LBN_SELCHANGE(IDC_ELEM_LIST, OnSelchangeElemList)
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_ELEM_LIST, OnDblclkElemList)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CElemBrowser message handlers

void CElemBrowser::PopulateList( void )
{
    m_ElemList.SetRedraw( FALSE );

    // Populate List
    m_ElemList.ResetContent();
    s32 NumElems = m_pDoc->m_Effect.GetNumElements();
    for ( s32 i = 0; i < NumElems; i++ )
    {
        char ID[256];

        fx_core::element* pElem = m_pDoc->m_Effect.GetElement( i );
        pElem->GetElementID( ID );

        s32 ListIdx;
        if ( pElem->IsHidden() )
        {
            CString Tmp;
            Tmp.Format( "[ %s ]   <--- Hidden", ID );
            ListIdx = m_ElemList.AddString( Tmp );           // Add the text, get the list index
        }
        else
        {
            ListIdx = m_ElemList.AddString( ID );           // Add the text, get the list index
        }

        m_ElemList.SetItemData( ListIdx, (DWORD)pElem );    // set the item data to be the elem ptr

        // select the ones from the current selset
        POSITION Pos = m_pDoc->m_SelSet.GetHeadPosition();
        while( Pos )
        {
            fx_core::element* pElem2 = m_pDoc->m_SelSet.GetNext( Pos );

            if ( pElem2 == pElem )
                m_ElemList.SetSel( ListIdx, TRUE );
        }
    }

    m_ElemList.SetRedraw( TRUE );
}

BOOL CElemBrowser::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_Up  .SetBitmap( CSize(15,15), IDB_UP   );
    m_Down.SetBitmap( CSize(15,15), IDB_DOWN );

    // Make sure it's been set up
    ASSERT( m_pDoc );

    // Populate List
    PopulateList();

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CElemBrowser::OnSelchangeElemList() 
{
    
}



void CElemBrowser::OnDestroy() 
{
	CDialog::OnDestroy();
	
    if( ::IsWindow( m_ElemList.GetSafeHwnd() ) )
    {
        m_SelCount = m_ElemList.GetSelCount();           // Find out how many new items selected    
    
        s32* pSel = new s32[m_SelCount];                 // allocate an array of indices
        m_pData =   new DWORD[m_SelCount];
    
        m_ElemList.GetSelItems( m_SelCount, pSel );      // Fetch the selected items    	

        for ( s32 i = 0; i < m_SelCount; i++ )
        {
            m_pData[i] = m_ElemList.GetItemData( pSel[i] );
        }

        delete[] pSel; 
    }
}

void CElemBrowser::OnDblclkElemList() 
{
	EndDialog(IDOK);
}

void CElemBrowser::OnUp() 
{
    s32 iItem = m_ElemList.GetCaretIndex();
    if( iItem > 0 )
    {
        m_pDoc->m_Effect.MoveElement( iItem, iItem-1 );
        PopulateList();
        m_ElemList.SetCaretIndex( iItem-1 );
    }
}

void CElemBrowser::OnDown() 
{
    s32 iItem = m_ElemList.GetCaretIndex();
    if( iItem < (m_pDoc->m_Effect.GetNumElements()-1) )
    {
        m_pDoc->m_Effect.MoveElement( iItem, iItem+1 );
        PopulateList();
        m_ElemList.SetCaretIndex( iItem+1 );
    }
}
