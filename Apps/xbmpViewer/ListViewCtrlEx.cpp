/******************************************************************************

$Author: Holte $
  
$Modtime: 15.10.02 13:24 $
$Revision: 3 $

Description: Implementation of class "CListBase"
             (list control with sort icons and colored sort column)

$Log: /NUKO2/B0000009/Administration/ListViewCtrlEx.cpp $

3     15.10.02 13:32 Holte
Overloading of the member functions "InsertColumn", "HitTest", etc. has become
obsolete because the corresponding messages will be intercepted directly.
Now the extended styles LVS_EX_ONECLICKACTIVATE, LVS_EX_TWOCLICKACTIVATE, and
LVS_EX_UNDERLINEHOT are supported in owner drawn mode.

2     16.09.02 15:50 Holte
Changes of the class "CListBase":
Member function "ColorSortColumn" geдndert (device context will be freed after
usage),
member functions "DrawSubitemText" und "GetLabelWidth" changed (calling of
"MakeShortString" is noew obsolete),
member function "MakeShortString" deleted.

1     9.07.02 11:21 Holte

******************************************************************************/

#include "stdafx.h"
#include "ListViewCtrlEx.h"

/*** Defines not yet available in MS VC++ 6.0 ********************************/
#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

/*** Definition of class "CListCtrlEx" ***************************************/

IMPLEMENT_DYNCREATE(CListCtrlEx, CListCtrl)

/*** Public Member functions *************************************************/

/*** Protected member functions **********************************************/

/*** A column header has been clicked ****************************************/
BOOL CListCtrlEx::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  return CListBase::OnColumnclick(pNMHDR, pResult);
}

/*** A list view (sub)item will be drawn *************************************/
void CListCtrlEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CListBase::OnCustomDraw(pNMHDR, pResult);
}

/*** The background will be erased *******************************************/
BOOL CListCtrlEx::OnEraseBkgnd(CDC* pDC) 
{
  if (CListBase::OnEraseBkgnd(pDC))
    return TRUE;
  else
    return CListCtrl::OnEraseBkgnd(pDC);
}

/*** The extended list view style will be retrieved **************************/
LRESULT CListCtrlEx::OnGetExtendedStyle(WPARAM, LPARAM)
{
  return m_dwExtendedStyle;
}

/*** The list-view item at a specified position will be determined ***********/
LRESULT CListCtrlEx::OnHitTest(WPARAM, LPARAM lParam)
{
  Default();
  return CListBase::OnHitTest(reinterpret_cast<LPLVHITTESTINFO>(lParam));
}

/*** List control has been scrolled horizontally *****************************/
void CListCtrlEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CListCtrl::OnHScroll (nSBCode, nPos, pScrollBar);
  InvalidateNonItemArea();
}

/*** A new column in a list-view control will be inserted ********************/
LRESULT CListCtrlEx::OnInsertColumn(WPARAM, LPARAM lParam)
{
  LRESULT lResult = Default();

  if (lResult == 0)
    JustifyFirstColumn(reinterpret_cast<LPLVCOLUMN>(lParam)->fmt);

  return lResult;
}

/*** A key has been pressed  *************************************************/
void CListCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (!CListBase::OnKeyDown(nChar))
    CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

/*** A key has been released *************************************************/
void CListCtrlEx::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  CListBase::OnKeyUp(nChar);
  CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

/*** List control loses input focus ******************************************/
void CListCtrlEx::OnKillFocus(CWnd* pNewWnd) 
{
  CListCtrl::OnKillFocus(pNewWnd);
  CListBase::OnKillFocus();
}

/*** The user double-clicks the left mouse button ****************************/
void CListCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnLButtonDblClk(point))
    CListCtrl::OnLButtonDblClk(nFlags, point);
}

/*** The user presses the left mouse button **********************************/
void CListCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnLButtonDown(point))
    CListCtrl::OnLButtonDown(nFlags, point);
  CListBase::OnMouseMove(point);
}

/*** The user releases the left mouse button *********************************/
void CListCtrlEx::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CListBase::OnLButtonUp();
    CListCtrl::OnLButtonUp(nFlags, point);
}

/*** The user double-clicks the right mouse button ****************************/
void CListCtrlEx::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnRButtonDblClk(point))
    CListCtrl::OnRButtonDblClk(nFlags, point);
}

/*** The user presses the right mouse button **********************************/
void CListCtrlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
  if (!CListBase::OnRButtonDown(point))
    GetParent()->SendMessage(WM_RBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(point.x, point.y)); //Блять, я хз как ещё реализовывать эту поеботу.
    CListCtrl::OnRButtonDown(nFlags, point); //Оно не работает :(
  CListBase::OnMouseMove(point);
}

/*** The user releases the right mouse button *********************************/
void CListCtrlEx::OnRButtonUp(UINT nFlags, CPoint point) 
{
  CListBase::OnRButtonUp();
    CListCtrl::OnRButtonUp(nFlags, point);
}

/*** The mouse has been moved ************************************************/
void CListCtrlEx::OnMouseMove(UINT nFlags, CPoint point) 
{
    CListCtrl::OnMouseMove(nFlags, point);
  CListBase::OnMouseMove(point);
}

/*** Divider in header control has been dragged ******************************/
BOOL CListCtrlEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  BOOL bRet = CListCtrl::OnNotify(wParam, lParam, pResult);

  if (CListBase::OnNotify(lParam)) return TRUE; else return bRet;
}

/*** The extended list view style will be set ********************************/
LRESULT CListCtrlEx::OnSetExtendedStyle(WPARAM wParam, LPARAM lParam)
{
  if (m_bOnSetExtendedStyle)
    return Default();
  else
  {
    DWORD dwResult;

    if (CListBase::OnSetExtendedStyle(static_cast<DWORD>(wParam),
                                      static_cast<DWORD>(lParam), &dwResult))
      return dwResult;
    else
      return Default();
  }
}

/*** List control gains input focus ******************************************/
void CListCtrlEx::OnSetFocus(CWnd* pOldWnd) 
{
  CListCtrl::OnSetFocus(pOldWnd);
  CListBase::OnSetFocus();
}

/*** An image list will be assigned ******************************************/
LRESULT CListCtrlEx::OnSetImageList(WPARAM wParam, LPARAM lParam)
{
  LRESULT lResult = Default();

  CListBase::OnSetImageList(static_cast<int>(wParam),
                            reinterpret_cast<HIMAGELIST>(lParam));

  return lResult;
}

/*** System colors have been changed *****************************************/
void CListCtrlEx::OnSysColorChange()
{
  CListCtrl::OnSysColorChange();
  CListBase::OnSysColorChange();
}

/*** Table of message handlers ***********************************************/
BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
    //{{AFX_MSG_MAP(CListCtrlEx)
    ON_WM_SYSCOLORCHANGE()
    ON_WM_ERASEBKGND()
    ON_WM_HSCROLL()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
  ON_MESSAGE(LVM_GETEXTENDEDLISTVIEWSTYLE , OnGetExtendedStyle)
  ON_MESSAGE(LVM_HITTEST, OnHitTest)
  ON_MESSAGE(LVM_INSERTCOLUMN, OnInsertColumn)
  ON_MESSAGE(LVM_SETEXTENDEDLISTVIEWSTYLE , OnSetExtendedStyle)
  ON_MESSAGE(LVM_SETIMAGELIST, OnSetImageList)
    ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnclick)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


/*** Definition of class "CListViewEx" ***************************************/

IMPLEMENT_DYNCREATE(CListViewEx, CListView)

/*** Protected member functions **********************************************/

/*** A column header has been clicked ****************************************/
BOOL CListViewEx::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  return CListBase::OnColumnclick(pNMHDR, pResult);
}

/*** A list view (sub)item will be drawn *************************************/
void CListViewEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CListBase::OnCustomDraw(pNMHDR, pResult);
}

/*** The background will be erased *******************************************/
BOOL CListViewEx::OnEraseBkgnd(CDC* pDC) 
{
  if (CListBase::OnEraseBkgnd(pDC))
    return TRUE;
  else
    return CListView::OnEraseBkgnd(pDC);
}

/*** The extended list view style will be retrieved **************************/
LRESULT CListViewEx::OnGetExtendedStyle(WPARAM, LPARAM)
{
  return m_dwExtendedStyle;
}

/*** The list-view item at a specified position will be determined ***********/
LRESULT CListViewEx::OnHitTest(WPARAM, LPARAM lParam)
{
  Default();
  return CListBase::OnHitTest(reinterpret_cast<LPLVHITTESTINFO>(lParam));
}

/*** List control has been scrolled horizontally *****************************/
void CListViewEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CListView::OnHScroll (nSBCode, nPos, pScrollBar);
  InvalidateNonItemArea();
}

/*** A new column in a list-view control will be inserted ********************/
LRESULT CListViewEx::OnInsertColumn(WPARAM, LPARAM lParam)
{
  LRESULT lResult = Default();

  if (lResult == 0)
    JustifyFirstColumn(reinterpret_cast<LPLVCOLUMN>(lParam)->fmt);

  return lResult;
}

/*** A key has been pressed  *************************************************/
void CListViewEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (!CListBase::OnKeyDown(nChar))
    CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}

/*** A key has been released *************************************************/
void CListViewEx::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  CListBase::OnKeyUp(nChar);
  CListView::OnKeyUp(nChar, nRepCnt, nFlags);
}

/*** List control loses input focus ******************************************/
void CListViewEx::OnKillFocus(CWnd* pNewWnd) 
{
  CListView::OnKillFocus(pNewWnd);
  CListBase::OnKillFocus();
}

/*** The user double-clicks the left mouse button ****************************/
void CListViewEx::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnLButtonDblClk(point))
    CListView::OnLButtonDblClk(nFlags, point);
}

/*** The user presses the left mouse button **********************************/
void CListViewEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnLButtonDown(point))
    CListView::OnLButtonDown(nFlags, point);
  CListBase::OnMouseMove(point);
}


/*** The user releases the left mouse button *********************************/
void CListViewEx::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CListBase::OnLButtonUp();
    CListView::OnLButtonUp(nFlags, point);
}

/*** The user double-clicks the right mouse button ****************************/
void CListViewEx::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnRButtonDblClk(point))
    CListView::OnRButtonDblClk(nFlags, point);
}

/*** The user presses the right mouse button **********************************/
void CListViewEx::OnRButtonDown(UINT nFlags, CPoint point) 
{
  if (!CListBase::OnRButtonDown(point))
    CListView::OnRButtonDown(nFlags, point);
  CListBase::OnMouseMove(point);
}


/*** The user releases the right mouse button *********************************/
void CListViewEx::OnRButtonUp(UINT nFlags, CPoint point) 
{
  CListBase::OnRButtonUp();
    CListView::OnRButtonUp(nFlags, point);
}

/*** The mouse has been moved ************************************************/
void CListViewEx::OnMouseMove(UINT nFlags, CPoint point) 
{
    CListView::OnMouseMove(nFlags, point);
  CListBase::OnMouseMove(point);
}

/*** Divider in header control has been dragged ******************************/
BOOL CListViewEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  BOOL bRet = CListView::OnNotify(wParam, lParam, pResult);

  if (CListBase::OnNotify(lParam)) return TRUE; else return bRet;
}

/*** The extended list view style will be set ********************************/
LRESULT CListViewEx::OnSetExtendedStyle(WPARAM wParam, LPARAM lParam)
{
  if (m_bOnSetExtendedStyle)
    return Default();
  else
  {
    DWORD dwResult;

    if (CListBase::OnSetExtendedStyle(static_cast<DWORD>(wParam),
                                      static_cast<DWORD>(lParam), &dwResult))
      return dwResult;
    else
      return Default();
  }
}

/*** List control gains input focus ******************************************/
void CListViewEx::OnSetFocus(CWnd* pOldWnd) 
{
  CListView::OnSetFocus(pOldWnd);
  CListBase::OnSetFocus();
}

/*** An image list will be assigned ******************************************/
LRESULT CListViewEx::OnSetImageList(WPARAM wParam, LPARAM lParam)
{
  LRESULT lResult = Default();

  CListBase::OnSetImageList(static_cast<DWORD>(wParam),
                            reinterpret_cast<HIMAGELIST>(lParam));

  return lResult;
}

/*** System colors have been changed *****************************************/
void CListViewEx::OnSysColorChange()
{
  CListView::OnSysColorChange();
  CListBase::OnSysColorChange();
}

/*** Table of message handlers ***********************************************/
BEGIN_MESSAGE_MAP(CListViewEx, CListView)
    //{{AFX_MSG_MAP(CListViewEx)
    ON_WM_SYSCOLORCHANGE()
    ON_WM_ERASEBKGND()
    ON_WM_HSCROLL()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
  ON_MESSAGE(LVM_GETEXTENDEDLISTVIEWSTYLE , OnGetExtendedStyle)
  ON_MESSAGE(LVM_HITTEST, OnHitTest)
  ON_MESSAGE(LVM_INSERTCOLUMN, OnInsertColumn)
  ON_MESSAGE(LVM_SETEXTENDEDLISTVIEWSTYLE , OnSetExtendedStyle)
  ON_MESSAGE(LVM_SETIMAGELIST, OnSetImageList)
    ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnclick)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


/*** Definition of "workhorse" class "CListBase" *****************************/

const int CListBase::m_nFirstColXOff = 2;   // x-offset of first column
const int CListBase::m_nNextColXOff  = 6;   // x-offset of other columns

/*** Public member functions *************************************************/

/*** Enable or disable coloring of sort column *******************************/
#if _MSC_VER == 1300
#pragma runtime_checks("c", off)  // due to a flaw in the definition of
#endif                            // Get[R|G|B]Value
void CListBase::ColorSortColumn(BOOL bEnable, int nSortColumn)
{
  ASSERT(m_pListCtrl);

  if (bEnable == m_bColorSortColumn &&
      (nSortColumn == 0 || nSortColumn == m_nSortColumn)) return;

  if (bEnable)
  {
    // 256 colors --> don't color sort column
    CDC* pDC    = m_pListCtrl->GetDC();
    bool bColor = pDC->GetDeviceCaps(BITSPIXEL) > 8;

    m_pListCtrl->ReleaseDC(pDC);
    if (!bColor) return;
  }

  if (bEnable != m_bColorSortColumn)
  {
    if (bEnable)
    {
      DWORD dwColNormalColor = m_pListCtrl->GetBkColor();

      // emulate sort column coloring of Windows XP explorer
      UINT nRed   = GetRValue(dwColNormalColor);
      UINT nGreen = GetGValue(dwColNormalColor);
      UINT nBlue  = GetBValue(dwColNormalColor);

      if (nRed > 240 && nGreen > 240 && nBlue > 240)
      {
        nRed   -= 8;
        nGreen -= 8;
        nBlue  -= 8;
      }
      else
      {
        if (nRed   < 232) nRed   += nRed   / 10; else nRed   = 255;
        if (nGreen < 232) nGreen += nGreen / 10; else nGreen = 255;
        if (nBlue  < 232) nBlue  += nBlue  / 10; else nBlue  = 255;
      }
      m_dwColSortColor = RGB(nRed, nGreen, nBlue);
    }
    m_bColorSortColumn = bEnable;
  }
  if (nSortColumn != 0) m_nSortColumn = nSortColumn;
  m_pListCtrl->Invalidate();
}
#if _MSC_VER == 1300
#pragma runtime_checks("c", restore)
#endif

/*** Draw the label of an item or subitem ************************************/
void CListBase::DrawSubItemText(CDC* pDC, LVITEM* pItem, LVCOLUMN* pColumn,
                                LPRECT pRect)
{
  if (*pItem->pszText)
    if (pRect->right - pRect->left > 0)
      pDC->DrawText(pItem->pszText, -1, pRect,
                    (pColumn->fmt & LVCFMT_CENTER ? DT_CENTER :
                     pColumn->fmt & LVCFMT_RIGHT  ? DT_RIGHT  : DT_LEFT) |
                     DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX |
                     DT_NOCLIP | DT_VCENTER);
}

/*** Draw small icon *********************************************************/
void CListBase::DrawSmallIcon(CDC* pDC, LVITEM* pItem, LPRECT pRect)
{
  ASSERT(m_pListCtrl);

  if (pItem->iImage > 0)
  {
    CImageList* pimglst = m_pListCtrl->GetImageList(LVSIL_SMALL);

    if (pimglst)
    {
      IMAGEINFO imgInfo;

      if (pimglst->GetImageInfo(pItem->iImage, &imgInfo))
      {
        pimglst->DrawIndirect(
          pDC, pItem->iImage, CPoint(pRect->left, pRect->top),
          CSize(__min(pRect->right - pRect->left + 1,
                      imgInfo.rcImage.right - imgInfo.rcImage.left),
                __min(pRect->bottom - pRect->top + 1,
                      imgInfo.rcImage.bottom - imgInfo.rcImage.top)),
          CPoint(0, 0),
          pItem->state & LVIS_SELECTED &&
          m_pListCtrl->GetFocus() == m_pListCtrl ? ILD_SELECTED : ILD_NORMAL,
          SRCCOPY, CLR_NONE);
      }
    }
  }
}

/*** Draw state icon *********************************************************/
void CListBase::DrawStateIcon(CDC* pDC, LVITEM* pItem, LPRECT pRect)
{
  ASSERT(m_pListCtrl);

  int nImage = (pItem->state & LVIS_STATEIMAGEMASK) >> 12;

  if (nImage > 0)
  {
    CImageList* pimglst = m_pListCtrl->GetImageList(LVSIL_STATE);

    if (pimglst)
    {
      IMAGEINFO imgInfo;
      // image indices are zero-based
      if (pimglst->GetImageInfo(--nImage, &imgInfo))
      {
        pimglst->DrawIndirect(
        pDC, nImage, CPoint(pRect->left, pRect->top),
        CSize(__min(pRect->right - pRect->left + 1,
                    imgInfo.rcImage.right - imgInfo.rcImage.left),
              __min(pRect->bottom - pRect->top + 1,
                    imgInfo.rcImage.bottom - imgInfo.rcImage.top)),
        CPoint(0, 0),
        pItem->state & LVIS_SELECTED &&
        m_pListCtrl->GetFocus() == m_pListCtrl ? ILD_SELECTED : ILD_NORMAL,
        SRCCOPY, CLR_NONE);
      }
    }
  }
}

/*** Enable or disable sort icon *********************************************/
void CListBase::EnableSortIcon(BOOL bEnable, int nSortColumn)
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  if (bEnable) CreateSortIcons();
  m_bSortIconEnabled = bEnable;
  if (nSortColumn != 0) m_nSortColumn = nSortColumn;
  m_pListCtrl->GetHeaderCtrl()->SetImageList(&m_imglstSortIcons);
  SetSortIcon                               ();  // display or hide sort icon
}

/*** Calculate width of item or subitem label ********************************/
int CListBase::GetLabelWidth(CDC* pDC, LVITEM* pItem, int nMaxWidth) const
{
  if (nMaxWidth > 0 && *pItem->pszText)
    return
      __min(pDC->GetTextExtent(pItem->pszText,
                               static_cast<int>(_tcslen(pItem->pszText))).cx,
                               nMaxWidth);
  else
    return 0;
}

/*** Small icon always should be kept left ***********************************/
bool CListBase::KeepLabelLeft(bool bKeepLeft)
{
  bool bSuccess = true;

  if (bKeepLeft)
    if ((m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED) == 0)
    {
      bKeepLeft = false;
      bSuccess  = false;
    }

  m_bKeepLabelLeft = bKeepLeft;
  return bSuccess;
}

/*** Set column to be sorted *************************************************/
void CListBase::SetSortColumn(int nColumn)
{
  ASSERT(nColumn != 0);
  ASSERT(m_pListCtrl);

  if (nColumn != m_nSortColumn)
  {
    m_nSortColumn = nColumn;
    SetSortIcon();
    if (abs(nColumn) != abs(m_nSortColumn)) m_pListCtrl->Invalidate();
  }
}

int CListBase::GetSortColumn( void )
{
    return m_nSortColumn;
}

/*** Private member functions ************************************************/

/*** Create image list with sort icons ***************************************/
void CListBase::CreateSortIcons()
{
  if (!m_imglstSortIcons.m_hImageList)
  {
    COLORMAP cm = {RGB(0, 0, 0), GetSysColor(COLOR_GRAYTEXT)};

    m_imglstSortIcons.Create   (9, 5, ILC_COLOR24 | ILC_MASK, 2, 0);
    m_bmpUpArrow.LoadMappedBitmap(IDB_HDRUP, 0, &cm, 1);
    m_nUpArrow = m_imglstSortIcons.Add(&m_bmpUpArrow, RGB(255, 255, 255));
    m_bmpDownArrow.LoadMappedBitmap(IDB_HDRDOWN, 0, &cm, 1);
    m_nDownArrow = m_imglstSortIcons.Add(&m_bmpDownArrow, RGB(255, 255, 255));
  }
}

/*** Draw the entire item (called if window has style LVS_OWNERDRAWFIXED) ****/
void CListBase::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  ASSERT(m_pListCtrl);

  LVITEM* pItem = GetLVITEM(lpDrawItemStruct->itemID);

  bool bAlwaysSelected  = pItem->state & LVIS_SELECTED &&
                          (m_pListCtrl->GetStyle() & LVS_SHOWSELALWAYS) != 0;
  bool bLVHasFocus      = m_pListCtrl->GetFocus() == m_pListCtrl;
  bool bItemHasFocus    = pItem->state & LVIS_FOCUSED  && bLVHasFocus;
  bool bSelected        = (pItem->state & LVIS_SELECTED) != 0;
  bool bReallySelected  = bSelected && bLVHasFocus;
  bool bFullRowSelected = (m_dwExtendedStyle & LVS_EX_FULLROWSELECT) != 0;

  CRect rcItem;                   // rectangle bounding complete item
  m_pListCtrl->GetItemRect(pItem->iItem, rcItem, LVIR_BOUNDS);

  CRect rcSubItem0;               // rectangle bounding subitem 0
  GetRealSubItemRect(pItem->iItem, 0, LVIR_BOUNDS, rcSubItem0);

  CRect rcLabel;                  // rectangle bounding item label
  GetRealSubItemRect(pItem->iItem, 0, LVIR_LABEL, rcLabel);

    CRect rcSelection;              // rectangle bounding selection
  if (bFullRowSelected)
  {
      rcSelection = rcItem;
    if (IndexToOrder(0) == 0 || m_bKeepLabelLeft)
    {
      rcSelection.left = rcLabel.left;
      int nWidthOfCol0 = m_pListCtrl->GetColumnWidth(OrderToIndex(0));
      if (rcSelection.left > nWidthOfCol0) rcSelection.left = nWidthOfCol0;
    }
  }

    CDC*   pDC          = CDC::FromHandle(lpDrawItemStruct->hDC);
  CBrush brushHiLite;
  DWORD  dwNormalText = GetSysColor(COLOR_WINDOWTEXT);
  DWORD  dwHiLiteBk   = 0;

    if (bReallySelected)
    {
    dwHiLiteBk =
    pItem->iItem == m_nHotItem ? m_dwHotLite : GetSysColor(COLOR_HIGHLIGHT);
    brushHiLite.CreateSolidBrush(dwHiLiteBk);
    }
    else if (bAlwaysSelected)
  {
    dwHiLiteBk = GetSysColor(COLOR_3DFACE);
    brushHiLite.CreateSolidBrush(dwHiLiteBk);
  }

  CFont* pfontPrev          = 0;
  CFont* pfontHotUnderlined = 0;
  bool   bHotUnderlining    = false;
  if (m_dwExtendedStyle & LVS_EX_TWOCLICKACTIVATE                           &&
      (bSelected || bAlwaysSelected)                                        ||
      (m_dwExtendedStyle & (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE)) ==
      (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE)                       &&
      pItem->iItem == m_nHotItem)
  {
    CFont*  pFont = m_pListCtrl->GetFont();
    LOGFONT logFont;

    if (pFont->GetLogFont(&logFont))
    {
      logFont.lfUnderline = TRUE;
      pfontHotUnderlined  = new CFont;
      pfontHotUnderlined->CreateFontIndirect(&logFont);
      pfontPrev           = pDC->SelectObject(pfontHotUnderlined);
      bHotUnderlining     = true;
    }
  }

  LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH;

  // display all subitems
  int nIndex;
    for (int nColumn = 0;
       m_pListCtrl->GetColumn(nIndex = OrderToIndex(nColumn), &lvc);
       nColumn++)
  {
    LVITEM* pSubItem  = nIndex > 0 ? GetLVITEM(pItem->iItem, nIndex) : pItem;
    CRect   rcSubItem;                  // rectangle bounding subitem
    CRect   rcText;                     // output rectangle
    DWORD   dwBkColor =                 // background color of curremt column
    m_bColorSortColumn && nIndex+1 == abs(m_nSortColumn) ?
    m_dwColSortColor : m_pListCtrl->GetBkColor();
    CBrush  brushBk(dwBkColor);

    // consider column margins
    if (nColumn > 0)
    {
      // move output rectangle over next column
      rcSubItem.left   = rcSubItem.right;
      rcSubItem.right += lvc.cx;
    }
    else
    {
      rcSubItem       = rcItem;
      rcSubItem.right = rcSubItem.left + lvc.cx;
    }

    if (nIndex == 0 && !m_bKeepLabelLeft || nColumn == 0 && m_bKeepLabelLeft)
    {
      rcText        = rcLabel;
      rcText.left  += m_nFirstColXOff;
      rcText.right -= nIndex > 0 ? m_nNextColXOff : m_nFirstColXOff;
    }
    else
    {
      rcText        = rcSubItem;
      rcText.left  += m_nNextColXOff;
      rcText.right -= m_nNextColXOff;
    }

    int  nLabelWidth = GetLabelWidth(pDC, pSubItem, rcText.Width());
    bool bHiLite     = false;

    if (bReallySelected ||
        bAlwaysSelected ||
        bItemHasFocus && !bFullRowSelected)
      if (nIndex == 0 && nColumn != 0 && !m_bKeepLabelLeft && !bFullRowSelected
          ||
          nColumn == 0 && (m_bKeepLabelLeft || nIndex == 0))
      {
        // calculate selection area
        CRect rcSubItemSelection(rcLabel);

        if (!bFullRowSelected)
        {
          int nFormat = nIndex == 0 && nColumn == 0 || !m_bKeepLabelLeft ?
                        m_nFormatOfSubItem0 : lvc.fmt & LVCFMT_JUSTIFYMASK;

          switch (nFormat)
          {
            case LVCFMT_LEFT:
              rcSubItemSelection.right =
                rcSubItemSelection.left +
                  m_nFirstColXOff + nLabelWidth + m_nNextColXOff;
              break;

            case LVCFMT_RIGHT:
              rcSubItemSelection.left =
                rcSubItemSelection.right -
                  m_nFirstColXOff - nLabelWidth - m_nNextColXOff;
              break;

            case LVCFMT_CENTER:
            {
              int nSelectionWidth =
                m_nNextColXOff + nLabelWidth + m_nNextColXOff;
              rcSubItemSelection.left  =
                rcLabel.left + (rcLabel.Width() - nSelectionWidth) / 2;
              rcSubItemSelection.right =
                rcSubItemSelection.left + nSelectionWidth;
              break;
            }

            default:
              ASSERT(false);
              break;
          }
          if (rcSubItemSelection.left < rcLabel.left)
            rcSubItemSelection.left = rcLabel.left;
          if (rcSubItemSelection.right > rcLabel.right)
            rcSubItemSelection.right = rcLabel.right;

          rcSelection = rcSubItemSelection;
        }

        if (bReallySelected || bAlwaysSelected)
        {
          if (rcSubItemSelection.left > rcSubItem.left)
          {
            // fill area left from selection with background color
            CRect rc(rcSubItem);
            rc.right = rcSubItemSelection.left-1;
            CBrush brush(dwBkColor);
            pDC->FillRect(rc, &brushBk);
          }

          // fill selection area with highlight color
          pDC->FillRect(rcSubItemSelection, &brushHiLite);

          // fill area right from selection with background color
          if (rcSubItemSelection.right < rcSubItem.right)
          {
            CRect rc(rcSubItem);
            rc.left = rcSubItemSelection.right+1;
            CBrush brush(dwBkColor);
            pDC->FillRect(rc, &brushBk);
          }

          pDC->SetBkColor  (dwHiLiteBk);
          pDC->SetTextColor(
            bReallySelected ? GetSysColor(COLOR_HIGHLIGHTTEXT) :
            bHotUnderlining ? m_dwHotLite                        :
            dwNormalText);
          bHiLite = true;
        }
      }
      else
        if (bFullRowSelected)
        {
          pDC->FillRect    (rcSubItem, &brushHiLite);
          pDC->SetBkColor  (dwHiLiteBk);
          pDC->SetTextColor(
            bReallySelected ? GetSysColor(COLOR_HIGHLIGHTTEXT) :
            bHotUnderlining ? m_dwHotLite                        :
            dwNormalText);
          bHiLite = true;
        }

    if (!bHiLite)
    {
      pDC->FillRect    (rcSubItem, &brushBk);
      pDC->SetBkColor  (dwBkColor);
      pDC->SetTextColor(bHotUnderlining ? m_dwHotLite : dwNormalText);
    }

    if (nIndex == 0 && !m_bKeepLabelLeft || nColumn == 0 && m_bKeepLabelLeft)
    {
      CRect rcIcon;

      if (GetStateIconRect(pItem->iItem, rcIcon))
        DrawStateIcon(pDC, pItem, rcIcon);

      if (GetRealSubItemRect(pItem->iItem, 0, LVIR_ICON, rcIcon))
        DrawSmallIcon(pDC, pItem, rcIcon);
    }

    DrawSubItemText(pDC, pSubItem, &lvc, rcText);

    if (nIndex > 0)
    {
      delete[] pSubItem->pszText;
      delete pSubItem;
    }
  }

  if (bHotUnderlining)
  {
    pDC->SelectObject(pfontPrev);
    delete pfontHotUnderlined;
  }
  delete[] pItem->pszText;
  delete pItem;

  // If item has focus draw focus rectangle
    if (bItemHasFocus)
  {
    pDC->SetTextColor (dwNormalText);
    pDC->DrawFocusRect(rcSelection);
  }
}

/*** Get all attributes of a given item or subitem ***************************/
LVITEM* CListBase::GetLVITEM(int nItem, int nSubItem) const
{
  ASSERT(m_pListCtrl);

  LVITEM* pItem = new LVITEM;

  pItem->mask      = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
  pItem->iItem     = nItem;
  pItem->iSubItem  = nSubItem;
  pItem->stateMask = ~0U;

  // enlarge text buffer gradually until it's large enough
  for (int nLen = 128; nLen += nLen;)
  {
    pItem->cchTextMax = nLen;
    pItem->pszText    = new TCHAR[nLen];
    if (!m_pListCtrl->GetItem(pItem))
    {
      delete[] pItem->pszText;
      delete pItem;
      return 0;
    }
    if (static_cast<int>(_tcslen(pItem->pszText)) < nLen-3) break;
    //                         ^
    // reserve extra space for two additional dots
    // to be added by "MakeShortString"
    delete[] pItem->pszText;
  }

  return pItem;
}

/*** Calculates the bounding rectangle of a subitem.                       ***/
/*** Difference to GetSubItemRect:                                         ***/
/*** If the bounding rectangle of subitem 0 is requested, the function     ***/
/*** returns the bounding rectangle of the label including the icons only, ***/
/*** not the bounding rectangle of the whole item.                         ***/
bool CListBase::GetRealSubItemRect(int iItem, int iSubItem, int nArea,
                                       CRect& ref)
{
  ASSERT(m_pListCtrl);
  ASSERT(iSubItem >= 0);

  switch (nArea)
  {
    case LVIR_BOUNDS:
      if (m_pListCtrl->GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, ref))
        if (iSubItem == 0)
        {
          CRect rcLabel;

          if (m_pListCtrl->GetSubItemRect(iItem, 0, LVIR_LABEL, rcLabel))
          {
            ref.right = rcLabel.right;

            int nOrder = IndexToOrder(0);
            if (nOrder > 0)
            {
              // The left edge of subitem 0 is identical with the right edge of
              // the subitem left of subitem 0.
              CRect rcSubItem;

              if (m_pListCtrl->GetSubItemRect(iItem, OrderToIndex(nOrder - 1),
                                              LVIR_BOUNDS, rcSubItem))
              {
                ref.left = rcSubItem.right;
                return true;
              }
            }
            else
              return true;
          }
        }
        else
          return true;
      break;

    case LVIR_ICON:
    {
      CRect rcIcon;           // rectangle bounding small icon of subitem 0  

      if (m_pListCtrl->GetSubItemRect(iItem, 0, LVIR_ICON, rcIcon))
      {
        CRect rcSubItem0;     // rectangle bounding subitem 0

        if (GetRealSubItemRect(iItem, 0, LVIR_BOUNDS, rcSubItem0))
          if (IndexToOrder(0) > 0 && m_bKeepLabelLeft)
          {
            int nIndex = OrderToIndex(0);

            if (GetRealSubItemRect(iItem, nIndex, LVIR_BOUNDS, ref))
            {
              int nSmallIconXOff = rcIcon.left - rcSubItem0.left;

              ref.left  += nSmallIconXOff;
              ref.right  = ref.left + rcIcon.Width();

              // clip rectangle at right edge if necessary
              int nWidth = m_pListCtrl->GetColumnWidth(nIndex);
              if (nSmallIconXOff + ref.Width() >= nWidth)
                ref.right = ref.left - nSmallIconXOff + nWidth - 1;
              return true;
            }
          }
          else
          {
            // clip rectangle at right edge if necessary
            if (rcIcon.right > rcSubItem0.right)
              rcIcon.right = rcSubItem0.right;
            ref = rcIcon;
            return true;
          }
      }
      break;
    }

    case LVIR_LABEL:
    {
      CRect rcLabel;          // rectangle bounding label of subitem 0

      if (m_pListCtrl->GetSubItemRect(iItem, 0, LVIR_LABEL, rcLabel))
      {
        CRect rcSubItem0;     // rectangle bounding subitem 0

        if (GetRealSubItemRect(iItem, 0, LVIR_BOUNDS, rcSubItem0))
          if (IndexToOrder(0) > 0 && m_bKeepLabelLeft)
          {
            if (GetRealSubItemRect(iItem, OrderToIndex(0), LVIR_BOUNDS, ref))
            {
              ref.left += rcLabel.left - rcSubItem0.left;
              return true;
            }
          }
          else
          {
            ref = rcLabel;
            return true;
          }
      }
      break;
    }

    default:
      ASSERT(false);
      break;
  }

  return false;
}

/*** Retrieves the bounding rectangle for the state icon of an item **********/
bool CListBase::GetStateIconRect(int nItem, LPRECT pRect)
{
  CRect rcSubItem;

  if (GetRealSubItemRect(nItem, m_bKeepLabelLeft ? OrderToIndex(0) : 0,
                         LVIR_BOUNDS, rcSubItem))
  {
    CRect rcSmallIcon;

    if (GetRealSubItemRect(nItem, 0, LVIR_ICON, rcSmallIcon))
    {
      *pRect       = rcSubItem;
      pRect->right = rcSmallIcon.left;
      if (rcSmallIcon.right > rcSmallIcon.left) pRect->right--;

      // calculate x-offset of state icon
      if (!m_bIconXOffCalculated)
      {
        ASSERT(m_pListCtrl);
        CImageList* pImageList = m_pListCtrl->GetImageList(LVSIL_STATE);

        if (pImageList)
        {
          // retrieve width of state icon
          IMAGEINFO ii = {0, 0, 0, 0};

          if (pImageList->GetImageInfo(0, &ii))
          {
            int nXOff = pRect->right - (ii.rcImage.right - ii.rcImage.left) -
                        pRect->left;

            if (nXOff < 0)
              m_nIconXOff = 0;
            else if (nXOff < 4)
              m_nIconXOff = nXOff;
            else
              m_nIconXOff = 4;
          }
          else
            m_nIconXOff = 4;
        }
        else
          m_nIconXOff = 4;

        m_bIconXOffCalculated = true;
      }

      pRect->left += m_nIconXOff;

      // clip at right column border
      int nWidth = rcSubItem.Width();
      if (pRect->right >= rcSubItem.left + nWidth)
        pRect->right = pRect->left - m_nIconXOff + nWidth - 1;

      return true;
    }
  }

  return false;
}

/*** Return the order in the header control of a subitem, based on its index */
int CListBase::IndexToOrder(int nIndex)
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  HDITEM headerItem = {HDI_ORDER};
  return m_pListCtrl->GetHeaderCtrl()->GetItem(nIndex, &headerItem) ?
         headerItem.iOrder : -1;

}

/*** Invalidate client area not covered by list control items ****************/
void CListBase::InvalidateNonItemArea()
{
  ASSERT(m_pListCtrl);

  int nTopIndex = m_pListCtrl->GetTopIndex();

  if (nTopIndex >= 0)
  {
    // update coloring of sort column
    CRect rectHdrItem;
    ASSERT(m_pListCtrl->GetHeaderCtrl());
    if (m_pListCtrl->GetHeaderCtrl()->GetItemRect(
          abs(m_nSortColumn) - 1, rectHdrItem))
    {
      // erase area above top item
      CRect rectThis;
      m_pListCtrl->GetClientRect(rectThis);

      CRect rectItem;
      m_pListCtrl->GetItemRect(nTopIndex, rectItem, LVIR_BOUNDS);

      CRect rectToBeErased(rectThis.left, rectHdrItem.bottom,
                           rectThis.right, rectItem.top);
      m_pListCtrl->InvalidateRect(rectToBeErased);

      // erase area below bottom item
      m_pListCtrl->GetItemRect(m_pListCtrl->GetItemCount() - 1, rectItem,
                               LVIR_BOUNDS);
      if (rectItem.bottom < rectThis.bottom)
      {
        rectToBeErased.top    = rectItem.bottom;
        rectToBeErased.bottom = rectThis.bottom;
      }
      m_pListCtrl->InvalidateRect(rectToBeErased);
    }
  }
}

/*** Rejustify first column of listview control to enable a right- ***********/
/*** justified or centerd first column                             ***********/
void CListBase::JustifyFirstColumn(int nFormat)
{
  ASSERT(m_pListCtrl);

  m_nFormatOfSubItem0 = nFormat;

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED ||
      m_dwExtendedStyle       & LVS_EX_FULLROWSELECT)
  {
    CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
    ASSERT(pHeaderCtrl);
    HDITEM hdrItem;

    hdrItem.mask = HDI_FORMAT;
    if (pHeaderCtrl->GetItem(0, &hdrItem))
    {
      hdrItem.fmt =
        hdrItem.fmt & ~HDF_JUSTIFYMASK | nFormat & HDF_JUSTIFYMASK;
      pHeaderCtrl->SetItem(0, &hdrItem);
    }
  }
}

/*** A column header has been clicked ****************************************/
BOOL CListBase::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);

    int          nSortColumn = pNMListView->iSubItem + 1;

    if( nSortColumn == abs(m_nSortColumn) )
    {
        m_nSortColumn = -m_nSortColumn;
    }
    else
    {
        m_nSortColumn = nSortColumn;
    }
  
    SetSortIcon();

    *pResult = 0;
    return FALSE;
}

/*** A list view (sub)item will be drawn *************************************/
void CListBase::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  NMLVCUSTOMDRAW* pNMLVCustomDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

  switch (pNMLVCustomDraw->nmcd.dwDrawStage)
  {
    case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      *pResult = CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      if (m_bColorSortColumn)
      {
        pNMLVCustomDraw->clrTextBk =
          pNMLVCustomDraw->iSubItem + 1 == abs(m_nSortColumn) ?
          m_dwColSortColor : m_pListCtrl->GetBkColor();
        *pResult = CDRF_NEWFONT;
      }
      else
        *pResult = CDRF_DODEFAULT;
      if (pNMLVCustomDraw->iSubItem == 0) *pResult |= CDRF_NOTIFYPOSTPAINT;
      break;

    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    {
      // special treatment for first column:
      // fill empty area left of text label
      ASSERT(pNMLVCustomDraw->iSubItem == 0);
      CRect rect;
      m_pListCtrl->GetHeaderCtrl()->GetItemRect(0, rect);
      int  nColumnWidth = rect.Width();
      int  nLeftX       = rect.left - m_pListCtrl->GetScrollPos(SB_HORZ);
      bool bFirstColumn = rect.left == 0;
      bool bHasFocus    = m_pListCtrl->GetFocus() == m_pListCtrl;

      m_pListCtrl->GetItemRect(static_cast<int>(
        pNMLVCustomDraw->nmcd.dwItemSpec), rect, LVIR_LABEL);

      CDC*   pDC = CDC::FromHandle(pNMLVCustomDraw->nmcd.hdc);
      CBrush brushColColor;
      int    nSortColumn    = abs(m_nSortColumn);
      bool   bFullRowSelect = (m_dwExtendedStyle & LVS_EX_FULLROWSELECT) != 0;

      LVITEM item;
      item.mask      = LVIF_IMAGE | LVIF_STATE;
      item.iItem     = static_cast<int>(pNMLVCustomDraw->nmcd.dwItemSpec);
      item.iSubItem  = 0;
      item.stateMask = LVIS_SELECTED | LVIS_FOCUSED | LVIS_STATEIMAGEMASK;
      m_pListCtrl->GetItem(&item);
      
      if (!bFirstColumn              &&
          bFullRowSelect             &&
          item.state & LVIS_SELECTED &&
          bHasFocus                  &&
          item.iItem == m_pListCtrl->GetHotItem())
        // create brush with hot-tracked color
        brushColColor.CreateSolidBrush(m_dwHotLite);
      else if (!bFirstColumn              &&
               bFullRowSelect             &&
               item.state & LVIS_SELECTED &&
               bHasFocus)
        // create brush with highlight color
        brushColColor.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
      else if (!bFirstColumn              &&
               bFullRowSelect             &&
               item.state & LVIS_SELECTED &&
               m_pListCtrl->GetStyle() & LVS_SHOWSELALWAYS)
        // create brush with highlight (nonfocus) color
        brushColColor.CreateSolidBrush(GetSysColor(COLOR_3DFACE));
      else if (m_bColorSortColumn && nSortColumn == 1)
        // create brush with sort color
        brushColColor.CreateSolidBrush(m_dwColSortColor);
      else
        // create brush with normal background color
        brushColColor.CreateSolidBrush(m_pListCtrl->GetBkColor());

      // select new brush and save previous brush
      CBrush* pbrushPrev = pDC->SelectObject(&brushColColor);

      // color area left of text label
      pDC->PatBlt(nLeftX, rect.top, __min(rect.left, rect.right) - nLeftX,
                  rect.Height(), PATCOPY);

      if (nColumnWidth > m_nIconXOff)
      {
        // draw state icon
        CImageList* pimglst = m_pListCtrl->GetImageList(LVSIL_STATE);
        if (pimglst)
        {
          int nImage = (item.state & LVIS_STATEIMAGEMASK) >> 12;
          if (nImage > 0)
          {
            IMAGEINFO imgInfo;
            // image indices are zero-based
            if (pimglst->GetImageInfo(--nImage, &imgInfo) &&
                GetStateIconRect(item.iItem, rect))
              pimglst->DrawIndirect(
                pDC, nImage, rect.TopLeft(),
                CSize(__min(rect.Width(),
                            imgInfo.rcImage.right - imgInfo.rcImage.left),
                      __min(rect.Height(),
                            imgInfo.rcImage.bottom - imgInfo.rcImage.top)),
                CPoint(0, 0),
                item.state & LVIS_SELECTED && bHasFocus ?
                ILD_SELECTED : ILD_NORMAL,
                SRCCOPY, CLR_NONE);
          }
        }

        // draw small icon
        pimglst = m_pListCtrl->GetImageList(LVSIL_SMALL);
        if (pimglst)
          if (item.iImage > 0)
          {
            IMAGEINFO imgInfo;
            if (pimglst->GetImageInfo(item.iImage, &imgInfo) &&
                m_pListCtrl->GetItemRect(item.iItem, rect, LVIR_ICON))
            {
              int nIconOffset = rect.left - nLeftX;
              if (nColumnWidth > nIconOffset)
                pimglst->DrawIndirect(
                  pDC, item.iImage, rect.TopLeft(),
                  CSize(__min(nColumnWidth - nIconOffset,
                              imgInfo.rcImage.right - imgInfo.rcImage.left),
                        __min(rect.Height(),
                              imgInfo.rcImage.bottom - imgInfo.rcImage.top)),
                  CPoint(0, 0),
                  item.state & LVIS_SELECTED && bHasFocus ?
                  ILD_SELECTED : ILD_NORMAL,
                  SRCCOPY, CLR_NONE);
            }
          }
      }

      // restore previous brush
      pDC->SelectObject(pbrushPrev);

      *pResult = CDRF_DODEFAULT;
      break;
    }

    default:
      *pResult = CDRF_DODEFAULT;
      break;
  }
}

/*** The background will be erased *******************************************/
bool CListBase::OnEraseBkgnd(CDC* pDC) 
{
  ASSERT(m_pListCtrl);

  if (m_bColorSortColumn)
  {
    CRect rect;
    if (!m_pListCtrl->GetHeaderCtrl()->GetItemRect(
           abs(m_nSortColumn) - 1, rect)) return false;

    int nXScrlOff = m_pListCtrl->GetScrollPos(SB_HORZ);
    int nLeftX    = rect.left  - nXScrlOff;
    int nRightX   = rect.right - nXScrlOff;

    pDC->GetClipBox(&rect);     // get area to be erased
    if (nLeftX < rect.right && nRightX > rect.left)
    {
      CBrush  brushNormalColor(m_pListCtrl->GetBkColor());
      CBrush  brushSortColor(m_dwColSortColor);
      CBrush* pbrushPrev = 0;

      if (nLeftX > rect.left)
      {
        // select brush with normal background color and save default brush
        pbrushPrev = pDC->SelectObject(&brushNormalColor);

        // erase area left from sort column with normal background color
        pDC->PatBlt(rect.left, rect.top, nLeftX - rect.left, rect.Height(),
                    PATCOPY);
      }
    
      // select brush with sort color and save default brush
      if (pbrushPrev)
        pDC->SelectObject(&brushSortColor);
      else
        pbrushPrev = pDC->SelectObject(&brushSortColor);

      // erase area inside sort column with sort color
      if (nLeftX < rect.left)
        if (nRightX < rect.right)
          pDC->PatBlt(rect.left, rect.top, nRightX - rect.left,
                      rect.Height(), PATCOPY);
        else
          pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(),
                      PATCOPY);
      else
        if (nRightX < rect.right)
          pDC->PatBlt(nLeftX, rect.top, nRightX - nLeftX, rect.Height(),
                      PATCOPY);
        else
          pDC->PatBlt(nLeftX, rect.top, rect.right - nLeftX, rect.Height(),
                      PATCOPY);

      if (nRightX < rect.right)
      {
        // select brush with normal background color
        pDC->SelectObject(&brushNormalColor);

        // color area right from sort column
        pDC->PatBlt(nRightX, rect.top, rect.right - nRightX, rect.Height(),
                    PATCOPY);
      }

      // restore previous brush
      pDC->SelectObject(pbrushPrev);

      return true;
    }
    else
      return false;
  }
  else
    return false;
}

/*** Determines which list view item, if any, is at a specified position *****/
int CListBase::OnHitTest(LPLVHITTESTINFO pHitTestInfo)
{
  ASSERT(m_pListCtrl);

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED &&
      pHitTestInfo->iItem    >= 0)
    if (pHitTestInfo->flags == LVHT_ONITEM)
    {
      CRect rc;
      if (GetRealSubItemRect(pHitTestInfo->iItem, 0, LVIR_LABEL, rc) &&
          rc.PtInRect(pHitTestInfo->pt))
        pHitTestInfo->flags = LVHT_ONITEMLABEL;
      else if (GetRealSubItemRect(pHitTestInfo->iItem, 0, LVIR_ICON, rc) &&
               rc.PtInRect(pHitTestInfo->pt))
        pHitTestInfo->flags = LVHT_ONITEMICON;
      else if (GetStateIconRect(pHitTestInfo->iItem, rc) &&
               rc.PtInRect(pHitTestInfo->pt))
        pHitTestInfo->flags = LVHT_ONITEMSTATEICON;
      else if (m_pListCtrl->GetItemRect(pHitTestInfo->iItem, rc, LVIR_BOUNDS)
               &&
               (!rc.PtInRect(pHitTestInfo->pt)    ||
                // The small area left from state icon is LVHT_NOWHERE!!!
                rc.left            > -m_nIconXOff &&
                pHitTestInfo->pt.x < __min(m_nIconXOff+rc.left, m_nIconXOff)))
        pHitTestInfo->flags = LVHT_NOWHERE;
    }

  return pHitTestInfo->iItem;
}

/*** A key has been pressed  *************************************************/
bool CListBase::OnKeyDown(UINT nChar) 
{
  ASSERT(m_pListCtrl);

  switch (nChar)
  {
    // Numpad-Plus
    case VK_ADD:
      // Ctrl-Numpad-Plus --> set optimum width for all columns
      if (m_bControl && m_bKeepLabelLeft && IndexToOrder(0) > 0)
      {
        LVCOLUMN lvc;
          lvc.mask = LVCF_FMT;

        int nIndex;
          for (int nColumn = 0;
             m_pListCtrl->GetColumn(nIndex = OrderToIndex(nColumn), &lvc);
             nColumn++)
        {
          int nOptWidth;

          if (nIndex == 0 || nColumn == 0)
          {
            // calculate needed column width
            nOptWidth = 0;
            for (int nItem = m_pListCtrl->GetItemCount(); --nItem >= 0;)
            {
              int nWidth =
                m_pListCtrl->GetStringWidth(
                  m_pListCtrl->GetItemText(nItem, nIndex));
              if (nWidth > nOptWidth) nOptWidth = nWidth;
            }

            if (nIndex > 0)
            {
              // add space for state icon and small icon
              CRect rcSubItem;
              if (GetRealSubItemRect(0, nIndex, LVIR_BOUNDS, rcSubItem))
              {
                CRect rcLabel;
                if (GetRealSubItemRect(0, nIndex, LVIR_LABEL, rcLabel))
                  nOptWidth += rcLabel.left - rcSubItem.left;
              }

              // add left offset
              nOptWidth += m_nFirstColXOff;
            }
            else
              // add left offset
              nOptWidth += m_nNextColXOff;

            // add right offset
            nOptWidth += m_nNextColXOff;
          }
          else
            nOptWidth = LVSCW_AUTOSIZE;

          m_pListCtrl->SetColumnWidth(nIndex, nOptWidth);
        }
        return true;
      }
      break;

    // Ctrl
    case VK_CONTROL:
      if (m_bKeepLabelLeft && IndexToOrder(0) > 0) return m_bControl = true;
      break;

    // All other keys
    default:
      break;
  }

    return false;
}

/*** A key has been released *************************************************/
void CListBase::OnKeyUp(UINT nChar) 
{
    if (nChar == VK_CONTROL) m_bControl = false;
}

/*** List control loses input focus ******************************************/
void CListBase::OnKillFocus() 
{
  ASSERT(m_pListCtrl);

    // manually remove focus state so that custom drawing will function properly
  int nItem = m_pListCtrl->GetNextItem(-1, LVNI_SELECTED);
  if (nItem >= 0) m_pListCtrl->SetItemState(nItem, 0, LVIS_FOCUSED);
}

/*** The user double-clicks the left mouse button ****************************/
bool CListBase::OnLButtonDblClk(CPoint point) 
{
  ASSERT(m_pListCtrl);

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
  {
    UINT flags;
    int  nItem = m_pListCtrl->HitTest(point, &flags);

    if (nItem            >= 0                 &&
        m_dwExtendedStyle & LVS_EX_CHECKBOXES &&
        flags            == LVHT_ONITEMSTATEICON)
    {
      m_pListCtrl->SetCheck(nItem, !m_pListCtrl->GetCheck(nItem));
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

/*** The user presses the left mouse button **********************************/
bool CListBase::OnLButtonDown(CPoint point) 
{
  ASSERT(m_pListCtrl);

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
  {
    if (m_hcursorCustom) SetCursor(m_hcursorCustom);

    UINT flags;
    int  nItem = m_pListCtrl->HitTest(point, &flags);
    if (nItem            >= 0                 &&
        m_dwExtendedStyle & LVS_EX_CHECKBOXES &&
        flags            == LVHT_ONITEMSTATEICON)
    {
      m_pListCtrl->SetCheck(nItem, !m_pListCtrl->GetCheck(nItem));
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

/*** The user releases the left mouse button *********************************/
void CListBase::OnLButtonUp()
{
  ASSERT(m_pListCtrl);

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
    if (m_hcursorCustom) SetCursor(m_hcursorCustom);
}

/*** The user double-clicks the right mouse button ****************************/
bool CListBase::OnRButtonDblClk(CPoint point)
{
    return true;
}

/*** The user presses the right mouse button **********************************/
bool CListBase::OnRButtonDown(CPoint point)
{
    ASSERT(m_pListCtrl);

    if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
    {
        if (m_hcursorCustom)
            SetCursor(m_hcursorCustom);

        UINT flags;
        int nItem = m_pListCtrl->HitTest(point, &flags);

        if (nItem            >= 0                 &&
            m_dwExtendedStyle & LVS_EX_CHECKBOXES &&
            flags            == LVHT_ONITEMSTATEICON)
        {
            m_pListCtrl->SetCheck(nItem, !m_pListCtrl->GetCheck(nItem));
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

/*** The user releases the right mouse button *********************************/
void CListBase::OnRButtonUp()
{

}

/*** The mouse has been moved ************************************************/
void CListBase::OnMouseMove(CPoint point)
{
  ASSERT(m_pListCtrl);

  if (m_dwExtendedStyle & (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE |
                           LVS_EX_TWOCLICKACTIVATE))
  {
    // determine item under mouse cursor (hot item) and
    // the appropriate mouse cursor
    UINT flags;
    int  nItem       = m_pListCtrl->HitTest(point, &flags);
    int  nOldHotItem = m_nHotItem;

    m_nHotItem =
      nItem >= 0                                                        &&
      flags & LVHT_ONITEM & ~LVHT_ONITEMSTATEICON                       &&
      (m_dwExtendedStyle & LVS_EX_TWOCLICKACTIVATE                      &&
       m_pListCtrl->GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED ||
       m_dwExtendedStyle & LVS_EX_ONECLICKACTIVATE) ? nItem : -1;

    if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
    {
      m_hcursorCustom = m_nHotItem != -1 ? m_hcursorHand : m_hcursorArrow;

      if (m_nHotItem != nOldHotItem)
      {
        CRect rc;

        if (nOldHotItem != -1)
        {
          m_pListCtrl->GetItemRect   (nOldHotItem, rc, LVIR_BOUNDS);
          m_pListCtrl->InvalidateRect(rc, FALSE);
        }
        if (m_nHotItem != -1)
        {
          m_pListCtrl->GetItemRect   (m_nHotItem, rc, LVIR_BOUNDS);
          m_pListCtrl->InvalidateRect(rc, FALSE);
        }
      }

      SetCursor(m_hcursorCustom);
    }
  }
}

/*** Divider in header control has been dragged ******************************/
bool CListBase::OnNotify(LPARAM lParam) 
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  NMHEADER* pNMHdr = reinterpret_cast<NMHEADER*>(lParam);

  if (pNMHdr->hdr.hwndFrom == m_pListCtrl->GetHeaderCtrl()->m_hWnd)
    switch (pNMHdr->hdr.code)
    {
      case HDN_ENDTRACKW:
      case HDN_ENDTRACKA:
      case HDN_ITEMCHANGEDW:
      case HDN_ITEMCHANGEDA:
        if (m_bColorSortColumn) InvalidateNonItemArea();
        break;

      case HDN_DIVIDERDBLCLICKW:
      case HDN_DIVIDERDBLCLICKA:
        if (m_bKeepLabelLeft &&
            (pNMHdr->iItem  > 0 && IndexToOrder(pNMHdr->iItem) == 0 ||
             pNMHdr->iItem == 0 && IndexToOrder(pNMHdr->iItem)  > 0))
        {
          // calculate needed column width
          int nOptWidth = 0;
          for (int nItem = m_pListCtrl->GetItemCount(); --nItem >= 0;)
          {
            int nWidth =
              m_pListCtrl->GetStringWidth(
                m_pListCtrl->GetItemText(nItem, pNMHdr->iItem));
            if (nWidth > nOptWidth) nOptWidth = nWidth;
          }

          if (pNMHdr->iItem > 0)
          {
            // add space for state icon and small icon
            CRect rcSubItem;
            if (GetRealSubItemRect(0, pNMHdr->iItem, LVIR_BOUNDS, rcSubItem))
            {
              CRect rcLabel;
              if (GetRealSubItemRect(0, pNMHdr->iItem, LVIR_LABEL, rcLabel))
                nOptWidth += rcLabel.left - rcSubItem.left;
            }

            // add left offset
            nOptWidth += m_nFirstColXOff;
          }
          else
            // add left offset
            nOptWidth += m_nNextColXOff;

          // add right offset
          nOptWidth += m_nNextColXOff;

          m_pListCtrl->SetColumnWidth(pNMHdr->iItem, nOptWidth);
          return true;
        }
        break;

      default:
        break;
    }

  return false;
}

/*** The extended list view style will be set ********************************/
bool CListBase::OnSetExtendedStyle(DWORD dwMask, DWORD dwExStyle,
                                   LPDWORD pdwResult)
{
  m_bOnSetExtendedStyle = true;

  ASSERT(m_pListCtrl);

  *pdwResult = m_dwExtendedStyle;
  if (!dwMask) dwMask = ~0U;
  m_dwExtendedStyle = dwMask & dwExStyle;

  if (m_pListCtrl->GetStyle() & LVS_OWNERDRAWFIXED)
    // Hide the bits LVS_EX_UNDERLINEHOT, LVS_EX_ONECLICKACTIVATE, and
    // LVS_EX_TWOCLICKACTIVATE from Windows
    if (dwMask & (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE |
                  LVS_EX_TWOCLICKACTIVATE))
    {
      if (dwExStyle & (LVS_EX_UNDERLINEHOT | LVS_EX_ONECLICKACTIVATE |
                       LVS_EX_TWOCLICKACTIVATE)) PrepareHotUnderlining();

      m_pListCtrl->SetExtendedStyle(m_dwExtendedStyle         &
                                    ~(LVS_EX_UNDERLINEHOT     |
                                      LVS_EX_ONECLICKACTIVATE |
                                      LVS_EX_TWOCLICKACTIVATE));
      m_bOnSetExtendedStyle = false;
      return true;
    }
    else
      m_hcursorCustom = 0;

  m_bOnSetExtendedStyle = false;
  return false;
}

/*** List control gains input focus ******************************************/
void CListBase::OnSetFocus() 
{
  ASSERT(m_pListCtrl);

    // manually set focus state so that custom drawing will function properly
  int nItem = m_pListCtrl->GetNextItem(-1, LVNI_SELECTED);
  if (nItem >= 0) m_pListCtrl->SetItemState(nItem, LVIS_FOCUSED, LVIS_FOCUSED);
}

/*** Assigns an image list to a list control *********************************/
void CListBase::OnSetImageList(int nImageList, HIMAGELIST himl)
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());
  ASSERT(nImageList != LVSIL_NORMAL);

  if (nImageList == LVSIL_SMALL && himl)
  {
    // restore image list with sort icons because default behavior is that the
    // header control shares its image list with the small icon list of the
    // list control
    m_pListCtrl->GetHeaderCtrl()->SetImageList(&m_imglstSortIcons);
  }
}

/*** System colors have been changed *****************************************/
void CListBase::OnSysColorChange() 
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  // update color of sort icons
  if (m_imglstSortIcons.m_hImageList)
  {
    m_imglstSortIcons.DeleteImageList         ();
    m_bmpUpArrow.DeleteObject                 ();
    m_bmpDownArrow.DeleteObject               ();
    CreateSortIcons                           ();
    m_pListCtrl->GetHeaderCtrl()->SetImageList(&m_imglstSortIcons);
    SetSortIcon                               ();
  }

  // force update of column colors
  m_bColorSortColumn = !m_bColorSortColumn;
  ColorSortColumn(!m_bColorSortColumn);
}

/*** Return the index of a subitem, based in its order in the header control */
int CListBase::OrderToIndex(int nOrder)
{
  ASSERT(m_pListCtrl);
  ASSERT(m_pListCtrl->GetHeaderCtrl());

  return m_pListCtrl->GetHeaderCtrl()->OrderToIndex(nOrder);
}

/*** Prepare resources for hot underlining ***********************************/
void CListBase::PrepareHotUnderlining()
{
  ASSERT(m_pListCtrl);

  if (!m_hcursorArrow) m_hcursorArrow = LoadCursor(0, IDC_ARROW);
  if (!m_hcursorHand)
    if (_winver <= 0x0400)
      // Under Windows 95/NT we must create our own cursor
      // to indicate hot items
      m_hcursorHand = LoadCursor(AfxGetResourceHandle(), _T("IDC_HAND"));
    else
      m_hcursorHand = LoadCursor(0, IDC_HAND);
}

/*** Display or hide sort icon on column to be sorted ************************/
void CListBase::SetSortIcon()
{
  ASSERT(m_pListCtrl);

  CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
  ASSERT(pHeaderCtrl);
  int nCols = pHeaderCtrl->GetItemCount();

  for (int col = 0; col < nCols; col++)
  {
    HDITEM hdrItem;

    hdrItem.mask = HDI_FORMAT | HDI_IMAGE;
    pHeaderCtrl->GetItem(col, &hdrItem);
    if (m_bSortIconEnabled && m_nSortColumn - 1 == col)
    {
      hdrItem.iImage = m_nUpArrow;
      hdrItem.fmt    = hdrItem.fmt & HDF_JUSTIFYMASK |
                       HDF_IMAGE | HDF_STRING | HDF_BITMAP_ON_RIGHT;
    }
    else if (m_bSortIconEnabled && -m_nSortColumn - 1 == col)
    {
      hdrItem.iImage = m_nDownArrow;
      hdrItem.fmt    = hdrItem.fmt & HDF_JUSTIFYMASK |
                       HDF_IMAGE | HDF_STRING | HDF_BITMAP_ON_RIGHT;
    }
    else
      hdrItem.fmt = hdrItem.fmt & HDF_JUSTIFYMASK | HDF_STRING;

    pHeaderCtrl->SetItem(col, &hdrItem);
  }
}
