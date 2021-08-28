// ResourceBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ResourceBrowserDlg.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceBrowserDlg dialog


CResourceBrowserDlg::CResourceBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResourceBrowserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResourceBrowserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CResourceBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResourceBrowserDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_TREE_RESOURCE_SELECTOR, m_rscTree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResourceBrowserDlg, CDialog)
	//{{AFX_MSG_MAP(CResourceBrowserDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_RESOURCE_SELECTOR, OnSelchangedTreeResourceSelector)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCLEAR, OnBnClickedClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceBrowserDlg message handlers

void CResourceBrowserDlg::OnOK() 
{
	// TODO: Add extra validation here
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        xhandle hHandle = m_rscTree.GetItemData(hItem);
        if (hHandle != HNULL)
        {
            CString& strData = m_xaPaths(hHandle);
            m_strPath = strData;
            m_strName = m_rscTree.GetItemText(hItem);
        }
    }
	
	CDialog::OnOK();
}

BOOL CResourceBrowserDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CRect rc;
    GetDlgItem(IDC_PREVIEW_FRAME)->GetWindowRect(&rc);
    ScreenToClient(&rc);

    if (!m_wndPreview.Create(_T("STATIC"),"PreviewWnd", WS_CHILD | WS_VISIBLE, rc, this, IDC_STATIC))
    {
        ASSERT(FALSE);
    }

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_rscTree.SetImageList(&m_imageList, TVSIL_NORMAL);

    //iterate through the resources
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
        
        if (strType.CompareNoCase(m_strType)==0)
        {
            CString strName(Node.pDesc->GetName());
            CString strPath(Node.pDesc->GetPath());

            AddPathToTree(strPath, strName);
        }
    }
	
    m_btnOk.EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResourceBrowserDlg::AddPathToTree(CString strPath, CString strName)
{
    xhandle hHandle = HNULL;
    CString& strPathAdded = m_xaPaths.Add(hHandle);
    strPathAdded = strPath;
    CFileSearch::FormatPath(strPath); //strPath now in form "c:\gamedata\a51\temp"
    strPath += "\\"; //ensure a final slash

    char cThemePath[MAX_PATH];
    CString strThemePath;
    for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
    {
        if (strPath.Find(cThemePath) != -1)
        {
            //found base path
            CString strNewPath(cThemePath);
            if (strNewPath.GetLength() > strThemePath.GetLength())
            {
                //we want the longest match here
                strThemePath = strNewPath;
            }
        }
    }

    if (!strThemePath.IsEmpty())
    {
        int nThemeLen = strThemePath.GetLength();
        CFileSearch::FormatPath(strThemePath);

        CString strThemeName;
        strThemeName = strThemePath.Left(strThemePath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        strPath = strThemeName + strPath.Right( strPath.GetLength() - nThemeLen);
    }

    ASSERT(hHandle != HNULL);

    int nIndex = strPath.Find('\\');
    HTREEITEM hParent = TVI_ROOT;
    while (nIndex!=-1)
    {
        CString strCurrent = strPath.Left(nIndex);
        strPath = strPath.Right ( strPath.GetLength() - nIndex - 1);

        //does strCurrent exist at this level?
        HTREEITEM hCurrent = DoesChildExist(strCurrent, hParent);
        if ( hCurrent )
        {
            hParent = hCurrent;
        }
        else
        {
            //need to insert this item
            hParent = m_rscTree.InsertItem(strCurrent, 0, 1, hParent);
            m_rscTree.SetItemData(hParent, HNULL);
        }

        nIndex = strPath.Find('\\');
    }

    HTREEITEM hItem = m_rscTree.InsertItem(strName, 2, 3, hParent);
    m_rscTree.SetItemData(hItem, hHandle);
}

HTREEITEM CResourceBrowserDlg::DoesChildExist(CString strCurrent, HTREEITEM hParent)
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem;
    
    if (hParent == TVI_ROOT)
    {
        hChildItem = m_rscTree.GetRootItem();
        if (!hChildItem)
        {
            return NULL;
        }
    }
    else
    {
        if (m_rscTree.ItemHasChildren(hParent))
        {   
            hChildItem = m_rscTree.GetChildItem(hParent);
        }
        else
        {
            return NULL;
        }
    }

    while (hChildItem != NULL)
    {
        hNextItem = m_rscTree.GetNextSiblingItem(hChildItem);
        if (strCurrent.CompareNoCase(m_rscTree.GetItemText(hChildItem)) == 0)
        {
            //Found it!
            return hChildItem;
        }
        hChildItem = hNextItem;
    }

    return NULL;
}

BOOL CResourceBrowserDlg::DestroyWindow() 
{
	m_imageList.DeleteImageList();
	
	return CDialog::DestroyWindow();
}

void CResourceBrowserDlg::OnSelchangedTreeResourceSelector(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
    xhandle hHandle = m_rscTree.GetItemData(pNMTreeView->itemNew.hItem);

    if (hHandle  != HNULL)
    {
        CWaitCursor wc;

        CString& strData = m_xaPaths(hHandle);
        CString strPath = strData + m_rscTree.GetItemText(pNMTreeView->itemNew.hItem);

        if (m_strType.CompareNoCase("RigidGeom") == 0)
        {
            m_wndPreview.LoadGeom(m_strType, strPath);
        }
        else if (m_strType.CompareNoCase("SkinGeom") == 0)
        {
            m_wndPreview.LoadGeom(m_strType, strPath);
        }
        else if (m_strType.CompareNoCase("audiopkg") == 0)
        {
            
        }

        m_btnOk.EnableWindow( TRUE);
        m_wndPreview.OnStartTimer();
    }
    else
    {
        m_btnOk.EnableWindow( FALSE);
        m_wndPreview.OnStopTimer();
    }

	*pResult = 0;
}

//=========================================================================
void CResourceBrowserDlg::OnBnClickedClear()
{
    m_strPath = "";
    m_strName = "<null>";

    EndDialog( IDCLEAR );
}
