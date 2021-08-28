// ConnectionAttributeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "worldeditor.hpp"
#include "ConnectionAttributeDialog.h"
#include "Ai_editor.hpp"
#include "navigation\ng_connection2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ConnectionAttributeDialog dialog


ConnectionAttributeDialog::ConnectionAttributeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ConnectionAttributeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(ConnectionAttributeDialog)
	//}}AFX_DATA_INIT
}


void ConnectionAttributeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConnectionAttributeDialog)
	DDX_Control(pDX, IDC_COMBO_CONNECTION_HINT_SNEAKY, m_HintSneaky);
	DDX_Control(pDX, IDC_COMBO_CONNECTION_HINT_PATROL_ROUTE, m_HintPatrolRoute);
	DDX_Control(pDX, IDC_COMBO_CONNECTION_HINT_ONE_WAY, m_HintOneWay);
	DDX_Control(pDX, IDC_COMBO_CONNECTION_HINT_DARK, m_HintDark);
	DDX_Control(pDX, IDC_COMBO_CONNECTION_HINT_COVER, m_HintCover);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ConnectionAttributeDialog, CDialog)
	//{{AFX_MSG_MAP(ConnectionAttributeDialog)
	ON_WM_CLOSE()
	ON_COMMAND(ID_BUTTON_BATCH_FLAG, OnButtonBatchFlag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ConnectionAttributeDialog message handlers

void ConnectionAttributeDialog::OnClose() 
{
	// TODO: Add your message handler code here and/or call default


	CDialog::OnClose();
}

void ConnectionAttributeDialog::OnButtonBatchFlag() 
{
	// TODO: Add your command handler code here
	ConnectionAttributeDialog aDialog;
    aDialog.DoModal();



}

void ConnectionAttributeDialog::OnOK() 
{
	// TODO: Add extra validation here
		
    switch(m_HintSneaky.GetCurSel() )
    {
    case 1:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_SNEAKY, ai_editor::k_Toggle ) ;
        }
        break;

    case 2:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_SNEAKY,ai_editor::k_AllTrue ) ;
        }
        break;

    case 3:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_SNEAKY, ai_editor::k_AllFalse ) ;
        }
        break;
    }

    //=================================================================================================
    switch(m_HintPatrolRoute.GetCurSel() )
    {
    case 1:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_PATROL_ROUTE, ai_editor::k_Toggle ) ;
        }
        break;

    case 2:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_PATROL_ROUTE,ai_editor::k_AllTrue ) ;
        }
        break;

    case 3:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_PATROL_ROUTE, ai_editor::k_AllFalse ) ;
        }
        break;
    }


    //=================================================================================================
    switch(m_HintOneWay.GetCurSel() )
    {
    case 1:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_ONE_WAY, ai_editor::k_Toggle ) ;
        }
        break;

    case 2:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_ONE_WAY,ai_editor::k_AllTrue ) ;
        }
        break;

    case 3:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_ONE_WAY, ai_editor::k_AllFalse ) ;
        }
        break;
    }

    //=================================================================================================
    switch(m_HintDark.GetCurSel() )
    {
    case 1:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_DARK, ai_editor::k_Toggle ) ;
        }
        break;

    case 2:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_DARK,ai_editor::k_AllTrue ) ;
        }
        break;

    case 3:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_DARK, ai_editor::k_AllFalse ) ;
        }
        break;
    }

    //=================================================================================================
    switch(m_HintCover.GetCurSel() )
    {
    case 1:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_COVER, ai_editor::k_Toggle ) ;
        }
        break;

    case 2:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_COVER,ai_editor::k_AllTrue ) ;
        }
        break;

    case 3:
        {
            ai_editor::GetAIEditor()->SetFlagsInSelectedObjects( ng_connection2::HINT_COVER, ai_editor::k_AllFalse ) ;
        }
        break;
    }



	CDialog::OnOK();
}
