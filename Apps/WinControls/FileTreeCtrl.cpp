// FileTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "FileTreeCtrl.h"
#include "FileSearch.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl

CFileTreeCtrl::CFileTreeCtrl() :
m_bUsePrev(FALSE)
{
    m_bInit = FALSE;
}

CFileTreeCtrl::~CFileTreeCtrl()
{
    if (m_bInit)
	    m_imageList.DeleteImageList();
}


BEGIN_MESSAGE_MAP(CFileTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CFileTreeCtrl)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(IDM_FBC_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FBC_FILE_DELETE, OnFileDelete)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl message handlers

void CFileTreeCtrl::BuildTreeFromPath(CString strRootPath, CString strWildcard, CString strForcedExt)
{
    if (!m_bInit)
    {
	    // Create the image list used by the tree control.
	    if (!m_imageList.Create (IDB_TREE_DRIVE_ICONS, 16, 1, RGB(0,255,0)))
        {
            ASSERT(FALSE);
		    return;
        }

	    // Set the image list for the tree control.
	    SetImageList(&m_imageList, TVSIL_NORMAL);
        m_bInit = TRUE;
    }

    if (!strRootPath.IsEmpty())
    {
        CFileSearch::FormatPath(strRootPath);

        tree_structure_info& tree_item = m_xaTreeStruct.Add();
        x_strcpy(tree_item.cPath,strRootPath);
        x_strcpy(tree_item.cWildcard,strWildcard);
        x_strcpy(tree_item.cForcedExt,strForcedExt);

        Refresh();
    }
}

void CFileTreeCtrl::ClearTree()
{
    DeleteAllItems();
    m_xaTreeStruct.Clear();
}

void CFileTreeCtrl::Refresh()
{
    DeleteAllItems(); //clear gui

    xhandle hHandle;
    hHandle.Handle = HNULL;
    for( s32 i=0; i < m_xaTreeStruct.GetCount(); i++ )
    {
        tree_structure_info& tree_item = m_xaTreeStruct[i];
        hHandle = m_xaTreeStruct.GetHandleByIndex( i );

        RecursePath(hHandle, CString(tree_item.cPath), CString(tree_item.cWildcard), TVI_ROOT);
    }
}

void CFileTreeCtrl::RecursePath(xhandle hData, CString strPath, CString strWildcard, HTREEITEM hRoot)
{
    CFileSearch::FormatPath(strPath);
    CString strName;
    
    if (m_bUsePrev && (hRoot == TVI_ROOT)) //only for root directories
    {
        strName = strPath.Left(strPath.ReverseFind('\\'));
        strName = strName.Right(strName.GetLength() - strName.ReverseFind('\\') - 1);
    }
    else
    {
        strName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
    }
    
    HTREEITEM hItem = InsertItem( strName, 0, 1, hRoot, TVI_SORT);
    SetItemData(hItem, hData);

    CFileSearch fSearch;
    fSearch.GetDirs(strPath);
    CStringList &lstDirs = fSearch.Dirs();
    CString strNextDir = fSearch.GetNextDir(TRUE);
    while (!strNextDir.IsEmpty())
    {   
        //add subdirs
        CString strNewPath = strPath + "\\" + strNextDir;
        RecursePath(hData, strNewPath, strWildcard, hItem);
        strNextDir = fSearch.GetNextDir(TRUE);
    }

    //add files
    fSearch.ClearDirs();
    fSearch.AddDirPath(strPath);
    fSearch.GetFiles(strWildcard);
    CString strNextFile = fSearch.GetNextFile(TRUE);
    while (!strNextFile.IsEmpty())
    {   
        CString strFileName = strNextFile.Right(strNextFile.GetLength() - strNextFile.ReverseFind('\\') - 1);
        HTREEITEM hFileItem = InsertItem( strFileName, 2, 3, hItem, TVI_SORT);
        SetItemData(hFileItem, hData);

        strNextFile = fSearch.GetNextFile(TRUE);
    }
}

CString CFileTreeCtrl::GetSelectedPath()
{
    HTREEITEM hItem = GetSelectedItem();
    return ItemToPath(hItem);
}

CString CFileTreeCtrl::ItemToPath(HTREEITEM hItem)
{
    CString strPath;

    if (hItem)
    {
        //Create the full string of the tree item
        HTREEITEM hParent = hItem;
        while (hParent)
        {
            CString strItem = GetItemText(hParent);
            int nLength = strItem.GetLength();
            ASSERT(nLength);
            hParent = GetParentItem(hParent);

            if (hParent)
            {
                if (strItem.GetAt(nLength-1) == _T('\\'))
                    strPath = strItem + strPath;
                else
                {
                    if (strPath.GetLength())
                        strPath = strItem + _T('\\') + strPath;
                    else
                        strPath = strItem;
                }
            }
        }

        //Add the root folder if there is one
        xhandle hData = (xhandle)GetItemData(hItem);
        if (hData != HNULL)
        {
            s32 index = m_xaTreeStruct.GetIndexByHandle(hData);
            ASSERT(index<m_xaTreeStruct.GetCount());
            tree_structure_info& tree_item = m_xaTreeStruct[index];

            strPath = CString(tree_item.cPath) + _T('\\') + strPath;
        }

        CFileSearch::FormatPath(strPath);
    }

    return strPath;
}

BOOL CFileTreeCtrl::IsFolder(CString strPath)
{
    CFileStatus rStatus;
    if( CFile::GetStatus( strPath, rStatus ) )
    {
        if (!(rStatus.m_attribute & CFile::directory))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CFileTreeCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    //Work out the position of where the context menu should be
    CPoint p(GetCurrentMessage()->pt);
    CPoint pt(p);
    ScreenToClient(&pt);
    Select(HitTest(pt), TVGN_CARET);
    OnContextMenu(NULL, p);

    *pResult = 0;
}

void CFileTreeCtrl::OnContextMenu(CWnd*, CPoint point)
{
	CXTMenu menu;
    menu.CreatePopupMenu();
    
    UINT nFlags;
    if (IsFolder(GetSelectedPath()))
    {
        nFlags = MF_STRING|MF_GRAYED;
    }
    else
    {
        nFlags = MF_STRING|MF_ENABLED;
    }

    menu.AppendMenu( nFlags, IDM_FBC_FILE_RENAME,         "Rename"         );    
    menu.AppendMenu( nFlags, IDM_FBC_FILE_DELETE,         "Delete"         );

    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
}

void CFileTreeCtrl::OnFileRename() 
{
    EditLabel(GetSelectedItem());
}

void CFileTreeCtrl::OnFileDelete() 
{
    Delete(GetSelectedItem());
}

void CFileTreeCtrl::Delete(HTREEITEM hItem)
{
    CString strPreItem = ItemToPath(hItem);
    GetParent()->SendMessage(WM_USER_MSG_FILE_ITEM_PRECHANGE,(long)strPreItem.GetBuffer(strPreItem.GetLength()),0);
    strPreItem.ReleaseBuffer();

    CString strItem = ItemToPath(hItem);
    if (IsFolder(strItem))
    {
        ASSERT(FALSE);
        return;
    }

    if (DeleteFile(strItem) && DeleteItem( hItem ))
    {
        //send user message to the parent
        if (GetParent())
        {
            GetParent()->SendMessage(WM_USER_MSG_FILE_ITEM_POSTCHANGE,(long)strItem.GetBuffer(strItem.GetLength()),0);
            strItem.ReleaseBuffer();
        }
    }
}

void CFileTreeCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    TV_DISPINFO* pDispInfo = (TV_DISPINFO*)pNMHDR;
    if (pDispInfo->item.hItem == 0)
    {
        *pResult = TRUE;
    }
    else
    {
        CString strPath = ItemToPath(pDispInfo->item.hItem);
        if (IsFolder(strPath))
        {
            *pResult = TRUE;
        }
        else
        {
            *pResult = FALSE;
        }
    }
}

void CFileTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    TV_DISPINFO* pDispInfo = (TV_DISPINFO*)pNMHDR;
    if (pDispInfo->item.pszText)
    {
        CString strNewName = pDispInfo->item.pszText;

        if (strNewName.IsEmpty())
        {
            *pResult = FALSE;
            return;
        }

        xhandle hData = (xhandle)GetItemData(pDispInfo->item.hItem);
        if (hData != HNULL)
        {
            s32 index = m_xaTreeStruct.GetIndexByHandle(hData);
            ASSERT(index<m_xaTreeStruct.GetCount());
            tree_structure_info& tree_item = m_xaTreeStruct[index];
            
            CString strForcedExt(tree_item.cForcedExt);
            if (!strForcedExt.IsEmpty())
            {
                CString strExt;
                int nIndex = strNewName.ReverseFind('.');
                if (nIndex!=-1)
                {
                    strExt = strNewName.Right(strNewName.GetLength() - nIndex - 1);
                }
                else
                {
                    nIndex = strNewName.GetLength();
                }

                if (strExt.CompareNoCase(strForcedExt) != 0)
                {
                    //change ext
                    strNewName = strNewName.Left(nIndex) + "." + strForcedExt;
                }
            }
        }

        CString strOldPath = ItemToPath(pDispInfo->item.hItem);
        CString strNewPath = strOldPath.Left(strOldPath.ReverseFind('\\') + 1) + strNewName;

        if (CFileSearch::DoesFileExist(strNewPath))
        {
            //already exists
            *pResult = FALSE;
            return;
        }

        CString strPreOldPath = strOldPath;
        CString strPreNewPath = strNewPath;
        GetParent()->SendMessage(WM_USER_MSG_FILE_ITEM_PRECHANGE,
                    (long)strPreOldPath.GetBuffer(strPreOldPath.GetLength()),
                    (long)strPreNewPath.GetBuffer(strPreNewPath.GetLength()));
        strPreOldPath.ReleaseBuffer();
        strPreNewPath.ReleaseBuffer();

        if (::MoveFileEx(strOldPath,strNewPath,MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING) &&
            SetItemText( pDispInfo->item.hItem, strNewName ))
        {
            //send user message to the parent
            if (GetParent())
            {
                GetParent()->SendMessage(WM_USER_MSG_FILE_ITEM_POSTCHANGE,
                    (long)strOldPath.GetBuffer(strOldPath.GetLength()),
                    (long)strNewPath.GetBuffer(strNewPath.GetLength()));
                strOldPath.ReleaseBuffer();
                strNewPath.ReleaseBuffer();
            }
            *pResult = TRUE;
        }
    }
	
	*pResult = FALSE;
}
