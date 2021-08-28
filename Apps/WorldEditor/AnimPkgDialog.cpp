// AnimPkgDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AnimPkgDialog.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\LocoEditor\LocoEditor.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\AudioEditor\AudioEditor.hpp"
#include "..\Editor\Project.hpp"
#include ".\animpkgdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimPkgDialog dialog


CAnimPkgDialog::CAnimPkgDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimPkgDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimPkgDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_DescLoaded = FALSE;
}


void CAnimPkgDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimPkgDialog)
	DDX_Control(pDX, IDC_ANIMPKG_DESC, m_rscTree);
	//}}AFX_DATA_MAP

//    DDX_Control(pDX, IDC_ANIMPKG_DESC,     m_DescListBox);

}


BEGIN_MESSAGE_MAP(CAnimPkgDialog, CDialog)
	//{{AFX_MSG_MAP(CAnimPkgDialog)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCLEAR, OnBnClickedBtnClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimPkgDialog message handlers

BOOL CAnimPkgDialog::OnInitDialog() 
{
    x_try;

	CDialog::OnInitDialog();

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_rscTree.SetImageList(&m_imageList, TVSIL_NORMAL);
    
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
    
        if (strType.CompareNoCase("anim")==0)
        {
            animation_desc* pAnimDesc = ( animation_desc* )Node.pDesc;
            OnLoadAnimPackage( pAnimDesc );
        }
    }

    m_DescLoaded = TRUE;


    x_catch_display;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//===========================================================================

void CAnimPkgDialog::OnLoadAnimPackage( animation_desc* pAnimDesc )
{
    HTREEITEM hParent = TVI_ROOT;

    // What is the name of the animation package?
    hParent = m_rscTree.InsertItem(pAnimDesc->GetName(), 0, 1, hParent);
    m_rscTree.SetItemData(hParent, HNULL);

    for( s32 i = 0; i < pAnimDesc->m_lAnimInfo.GetCount(); i++ )
    {
        m_rscTree.InsertItem( pAnimDesc->m_lAnimInfo.GetAt(i).Name, 16, 17, hParent );
    }
}

//===========================================================================

void CAnimPkgDialog::OnOK() 
{
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        xhandle hHandle = m_rscTree.GetItemData(hItem);
        if (hHandle != HNULL)
        {
            m_DescName = m_rscTree.GetItemText(m_rscTree.GetParentItem(hItem));
            m_DescName += "\\";
            m_DescName += m_rscTree.GetItemText(hItem);
        }
    }
	
	CDialog::OnOK();
}

//===========================================================================

void CAnimPkgDialog::OnBnClickedBtnClear()
{
    m_DescName.Empty();

    EndDialog( IDCLEAR );
}
