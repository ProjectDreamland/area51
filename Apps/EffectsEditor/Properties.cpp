// Properties.cpp : implementation file
//

#include "stdafx.h"
#include "mainfrm.h"
#include "PartEd.h"
#include "Properties.h"
//#include "controller.hpp"
//#include "element.hpp"
#include "Auxiliary/fx_core/effect.hpp"
#include "Auxiliary/fx_core/element_spemitter.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#define WIDTH   256

/////////////////////////////////////////////////////////////////////////////
// CProperties

CProperties::CProperties()
{
}

CProperties::~CProperties()
{
}

BEGIN_MESSAGE_MAP(CProperties, CXTDockWindow)
	//{{AFX_MSG_MAP(CProperties)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
    ON_MESSAGE( WM_USER_MSG_GRID_ITEM_CHANGE, OnItemChange )
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProperties message handlers

BOOL CProperties::OnEraseBkgnd(CDC* pDC) 
{
//    pDC->SelectStockObject( LTGRAY_BRUSH );
//    RECT Rect;
//    GetClientRect( &Rect );
//    pDC->Rectangle( &Rect );

	return TRUE;//CControlBar::OnEraseBkgnd(pDC);
}

CGridItemInfo* CProperties::AddGridDataElement( CString strName,
                                                CString strValue,
                                                CString strComment,
                                                CGridItemInfo::CONTROLTYPE type,
                                                COLORREF fieldColor,
                                                int iXaIndex,
									            BOOL bReadOnly,
                                                BOOL bMustEnum,
                                                BOOL bHeader )
{
	CGridItemInfo* lp = new CGridItemInfo();
	lp->SetItemText(strName);
	lp->AddSubItemText(strValue);// 1
	lp->SetReadOnly(bReadOnly);
	lp->SetControlType(type,0); //first sub column
	lp->SetListData(0, &m_PropList);
    lp->SetIsHeader(bHeader);
    lp->SetDataIndex(iXaIndex);

    CGridTreeItem *pTreeItemInserted = NULL;

    if (strName.Find('\\') == -1)
    {
        //Must be a header
		pTreeItemInserted = m_PropertyGrid.InsertRootItem(lp);
    }
    else
    {
        int iIndex = strName.ReverseFind('\\');
        if (iIndex == -1) 
        {
            //nothing inserted
            delete lp;
            lp = NULL;
        }
        else
        {
            CGridTreeItem *pParentTreeItem = NULL;
            CString strShortName = strName.Right(strName.GetLength()-(iIndex+1));
            CString strParentName = strName.Left(iIndex);

            pParentTreeItem = m_PropertyGrid.FindTreeItemWithIdentifier(strParentName);
            
            if (pParentTreeItem)
            {
		        if (pParentTreeItem->m_lpNodeInfo && 
                    pParentTreeItem->m_lpNodeInfo->IsReadOnly())
		        {
			        //roll down the read only flag
			        lp->SetReadOnly(TRUE);
		        }
       	        lp->SetItemText(strShortName);

		        pTreeItemInserted = m_PropertyGrid.InsertItem(pParentTreeItem, lp, FALSE /*expand*/);
            }
            else
            {
                //nothing inserted
                delete lp;
                lp = NULL;
                x_throw( xfs("Error: Unable to find parent for property [%s]", (const char*)strName) );
            }
        }
    }

    if ((lp == NULL) || (pTreeItemInserted == NULL) )
    {
        //invalid format in object, can not edit!
        m_PropertyGrid.DeleteAll();
    }
    else
    {
        pTreeItemInserted->m_strIdentifier      = strName;
        pTreeItemInserted->m_strComment         = strComment;
        pTreeItemInserted->m_bMustReloadData    = bMustEnum;
        pTreeItemInserted->m_BackgroundColor    = fieldColor;
    }

	return lp;
}

void CProperties::ExpandAll( void )
{
    int nScroll;
    int nRoot = m_PropertyGrid.GetRootCount();
    for( int i=0 ; i<nRoot ; i++ )
        m_PropertyGrid.ExpandAll( m_PropertyGrid.GetRootItem(i), nScroll );
}

int CProperties::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTDockWindow::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    CRect r;
    GetInsideRect( r );
//    GetClientRect( &r );
	m_PropertyGrid.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS|WS_CLIPCHILDREN, r, this, IDC_PROP_GRID_ITEM );
	m_PropertyGrid.InitializeGrid();
//	m_PropertyGrid.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	m_PropertyGrid.RecalcSize();

    AddGridDataElement( "EFFECT", "", "The Effect", CGridItemInfo::GCT_STRING_EDIT, RGB(192,192,192), 0, FALSE, FALSE, TRUE );

	m_PropertyGrid.SetFirstColWidth( 128 );

	SetXTBarStyle(CBRS_XT_DEFAULT|CBRS_XT_CLIENT_OUTLINE);

	return 0;
}

void CProperties::EraseAll( void )
{
//	m_PropertyGrid.InitializeGrid();
    m_PropertyGrid.DeleteAll();
//	m_PropertyGrid.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
//	m_PropertyGrid.RecalcSize();
}

afx_msg LRESULT CProperties::OnItemChange(WPARAM WParam, LPARAM)
{
    s32 T = (s32)g_pMainFrame->GetGlobalTime();

    CGridTreeItem *pSelItem;

    // get info about the modified item
    pSelItem = (CGridTreeItem*)WParam;
    CString Value = pSelItem->m_lpNodeInfo->GetSubItem(0);

    // Find the element this thing is referencing
    fx_core::base* pObject = (fx_core::base*)pSelItem->m_lpNodeInfo->GetDataIndex();

    if ( pObject )
    {
        xstring ID, Val;
        ID = pSelItem->m_strIdentifier;
        Val = Value;

        if ( x_strcmp( pObject->GetType(), "Effect" ) == 0 )
        {
            fx_core::effect* pEffect = (fx_core::effect*)pObject;

            // arg
            if( x_strcmp( ID, "EFFECT\\Instanceable" ) == 0 )
            {
                if( x_strcmp( Val, "true" ) == 0 )
                {
                    // Display a warning to the user that it will change all their Emitter to Local Space
                    s32 DoIt    = ::AfxGetMainWnd()->MessageBox( "This will cause all Emitter's 'World Space' flag to be set to false\nAre you sure you want to do that?", "Warning!", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL );
                
                    if( DoIt != IDYES )
                    {
                        // They don't wanna do it, so change value back to false
                        pSelItem->m_lpNodeInfo->SetSubItemText( 0, "false" );
                        return 0;
                    }
                }   
            }

            // update and check to see if it's dirty
            if ( pEffect->OnPropertyChanged( T, ID, Val ) == TRUE )
                g_pMainFrame->Invalidate( FALSE );
        }
        else   
        {
            fx_core::element* pElem = (fx_core::element*)pObject;

            // Special Case stuff for Spemitter
            if( x_strcmp( pElem->GetType(), "Spemitter" ) == 0 )
            {
                fx_core::element_spemitter*  pSpemitter = (fx_core::element_spemitter*)pElem;

                if( x_strstr( ID, "Object\\Immortal" )  && ( x_strcmp( Val, "true" ) == 0 ) )
                {
                    if( pSpemitter->GetBurstMode() )
                    {
                        // Display a warning to the user that this will turn Burst Mode off
                        s32 DoIt    = ::AfxGetMainWnd()->MessageBox( "This will cause \"Burst Mode\" to be turned off\nAre you sure you want to do that?", "Warning!", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL );

                        if( DoIt != IDYES )
                        {
                            // They don't wanna do it, so change value back to false
                            pSelItem->m_lpNodeInfo->SetSubItemText( 0, "false" );
                            return 0;
                        }
                    }
                }

            }

            // update and check to see if it's dirty
            if ( pElem->OnPropertyChanged( T, ID, Val ) == TRUE )
            {
                g_pMainFrame->Invalidate( FALSE );

                // Send an update message to parent, so it can update other controls
                LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
                g_pMainFrame->SendMessage   ( WM_USER_MSG_PROPERTIES_PROPERTYCHANGED, controlID, 0 );

                // SPECIAL CASE: Update "Name" field...otherwise further edits on this object will fail!
                if( x_strstr( ID, "Object\\Name" ) )
                {
                    g_pMainFrame->PopulatePropertyControl();
                }

                // SPECIAL CASE: When opening a Matx file, if the file doesn't exist, show a MessageBox
                else if( x_strstr( ID, "Mesh\\Mesh" ) )
                {
                    X_FILE* pFile = x_fopen( Val , "r" );

                    if( pFile )
                    {
                        x_fclose( pFile );
                    }
                    else
                    {
                        ::AfxGetMainWnd()->MessageBox( "That file does not exist", "Warning!", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL );
                    }
                }
            }
        }
    }

    return 1;
}

void CProperties::OnWindowPosChanged(WINDOWPOS FAR* lpWP) 
{
	CXTDockWindow::OnWindowPosChanged(lpWP);
	
	CRect rc;
	GetInsideRect(rc);
	rc.DeflateRect(1,1);

	// Resize controls
	if( m_PropertyGrid.GetSafeHwnd() )
    {
		m_PropertyGrid.MoveWindow( rc.left, rc.top, rc.Width(), rc.Height() );
	}
}
