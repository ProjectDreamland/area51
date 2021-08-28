// TransactionStackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TransactionStackDlg.h"
#include "transaction_mgr.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//=========================================================================
// CTransactionStackDlg dialog
//=========================================================================


CTransactionStackDlg::CTransactionStackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransactionStackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTransactionStackDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


//=========================================================================

void CTransactionStackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTransactionStackDlg)
	DDX_Control(pDX, IDC_LIST_TRANSACTIONS, m_lbStack);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}

//=========================================================================


BEGIN_MESSAGE_MAP(CTransactionStackDlg, CDialog)
	//{{AFX_MSG_MAP(CTransactionStackDlg)
	ON_LBN_SELCHANGE(IDC_LIST_TRANSACTIONS, OnSelchangeListTransactions)
	ON_LBN_DBLCLK(IDC_LIST_TRANSACTIONS, OnDblclkListTransactions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// CTransactionStackDlg message handlers
//=========================================================================

BOOL CTransactionStackDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_btnOk.EnableWindow(FALSE);
    int iIndex = m_lbStack.AddString("Top of Stack");
    ASSERT(iIndex != LB_ERR);
    m_lbStack.SetItemData( iIndex, 0 );

    m_nCurrentPos = 0;

    transaction_mgr::Transaction()->SetFirstTransactionDescription();

    CString strData;

    do {
        BOOL bPtr = FALSE;
        strData = CString(transaction_mgr::Transaction()->GetNextTransactionDescription(bPtr));
        if (!strData.IsEmpty())
        {
            iIndex = m_lbStack.AddString(strData);
            ASSERT(iIndex != LB_ERR);

            if (m_nCurrentPos==0)
            {
                //this is an undo
                m_lbStack.SetItemData( iIndex, 1 );
            }
            else
            {
                //this is a redo
                m_lbStack.SetItemData( iIndex, 2 );
            }

            if (bPtr)
            {
                ASSERT(m_nCurrentPos == 0); //should only be changed once
                m_nCurrentPos = iIndex;
                m_lbStack.SetStackPtrIndex(iIndex);
            }
        }
    } while (!strData.IsEmpty());

    if (m_nCurrentPos == 0)
    {
        //stack ptr never changes, all items are redo's except first
        for (int i=1; i<m_lbStack.GetCount(); i++)
        {
            m_lbStack.SetItemData( i, 2 );
        }
    }

    m_lbStack.SetCurSel(m_nCurrentPos);
    OnSelchangeListTransactions();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CTransactionStackDlg::OnOK() 
{
	int nSelected = m_lbStack.GetCurSel();
    int nData = m_lbStack.GetItemData(nSelected);

    if (nData == 1)
    {
        //undo
        int nLevels = m_nCurrentPos - nSelected + 1;
        transaction_mgr::Transaction()->Undo(nLevels);
	    CDialog::OnOK();
    }
    else if (nData == 2)
    {
        //redo
        int nLevels = nSelected - m_nCurrentPos;
        transaction_mgr::Transaction()->Redo(nLevels);
	    CDialog::OnOK();
    }	
}

//=========================================================================

void CTransactionStackDlg::OnSelchangeListTransactions() 
{
	int nSelected = m_lbStack.GetCurSel();
    int nData = m_lbStack.GetItemData(nSelected);

    if (nData == 0)
    {
        //top of stack
        m_btnOk.SetWindowText("");
    	m_btnOk.EnableWindow(FALSE);
    }
    else if (nData == 1)
    {
        //undo
        int nLevels = m_nCurrentPos - nSelected + 1;
        CString strText;
        strText.Format("Undo(%d)",nLevels);
        m_btnOk.SetWindowText(strText);
    	m_btnOk.EnableWindow(TRUE);
    }
    else if (nData == 2)
    {
        //redo
        int nLevels = nSelected - m_nCurrentPos;
        CString strText;
        strText.Format("Redo(%d)",nLevels);
        m_btnOk.SetWindowText(strText);
        m_btnOk.EnableWindow(TRUE);
    }
}

//=========================================================================

void CTransactionStackDlg::OnDblclkListTransactions() 
{
	int nSelected = m_lbStack.GetCurSel();
    int nData = m_lbStack.GetItemData(nSelected);

    if (nData != 0)
    {
        //not the top of stack
	    OnOK();
    }
}
