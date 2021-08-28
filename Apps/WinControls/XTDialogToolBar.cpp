// XTDialogToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "XTDialogToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int EXPANSION_BUTTON_WIDTH = 12;
static const int EXPANSION_BUTTON_INSET = 1;

/////////////////////////////////////////////////////////////////////////////
// CXTToolCmdUI
/////////////////////////////////////////////////////////////////////////////
class CXTToolCmdUI : public CCmdUI
{
public:
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

struct XT_CONTROLPOS
{
	int   nID;
	int   nIndex;
	CRect rectOldPos;
};

struct XT_OLDTOOLINFO
{
	UINT cbSize;
	UINT uFlags;
	HWND hwnd;
	UINT uId;
	RECT rect;
	HINSTANCE hinst;
	LPTSTR lpszText;
};
/////////////////////////////////////////////////////////////////////////////
// CXTDialogToolBar

CXTDialogToolBar::CXTDialogToolBar()
{
}

CXTDialogToolBar::~CXTDialogToolBar()
{
}


BEGIN_MESSAGE_MAP(CXTDialogToolBar, CXTToolBar)
	//{{AFX_MSG_MAP(CXTDialogToolBar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXTDialogToolBar message handlers

CSize CXTDialogToolBar::CalcLayout(DWORD dwMode, int nLength)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	if (dwMode & LM_HORZDOCK)
		ASSERT(dwMode & LM_HORZ);

	CSize sizeResult(0);
	bool bRedraw = false;

	// Load Buttons
	int nCount = SendMessage(TB_BUTTONCOUNT, 0, 0);
	if (nCount > 0)
	{	
		TBBUTTON* pData = (TBBUTTON*)_alloca(nCount * sizeof(TBBUTTON));
		for (int i = 0; i < nCount; i++)
		{
			GetButton(i, &pData[i]);
		}

//		if (!(m_dwStyle & CBRS_SIZE_FIXED))
		{
			BOOL bDynamic = TRUE;

			if (bDynamic && (dwMode & LM_MRUWIDTH))
				SizeToolBar(pData, nCount, m_nMRUWidth);
			else if (bDynamic && (dwMode & LM_HORZDOCK))
				SizeToolBar(pData, nCount, 32767);
			else if (bDynamic && (dwMode & LM_VERTDOCK))
				SizeToolBar(pData, nCount, 0);
			else if (bDynamic && (nLength != -1))
			{
				CRect rect; rect.SetRectEmpty();
				CalcInsideRect(rect, (dwMode & LM_HORZ));
				BOOL bVert = (dwMode & LM_LENGTHY);
				int nLen = nLength + (bVert ? rect.Height() : rect.Width());

				SizeToolBar(pData, nCount, nLen, bVert);
			}
			else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
				SizeToolBar(pData, nCount, m_nMRUWidth);
			else
				SizeToolBar(pData, nCount, (dwMode & LM_HORZ) ? 32767 : 0);
		}

		sizeResult = CalcSize(pData, nCount);

		if (dwMode & LM_COMMIT)
		{
			CArray<XT_CONTROLPOS, XT_CONTROLPOS&> controls;
			BOOL bIsDelayed = m_bDelayedButtonLayout;
			m_bDelayedButtonLayout = FALSE;

			for(int i = 0; i < nCount; i++)
			{
				if ((pData[i].fsStyle & TBSTYLE_SEP) &&
					(pData[i].idCommand != 0) &&
					!(pData[i].fsState & TBSTATE_HIDDEN))
				{
					XT_CONTROLPOS controlPos;
					controlPos.nIndex = i;
					controlPos.nID = pData[i].idCommand;

					CRect rect;
					GetItemRect(i, &rect);
					ClientToScreen(&rect);
					controlPos.rectOldPos = rect;

					controls.Add(controlPos);
				}
			}

			if ((m_dwStyle & CBRS_FLOATING) && (m_dwStyle & CBRS_SIZE_DYNAMIC))
				m_nMRUWidth = sizeResult.cx;
			for (i = 0; i < nCount; i++)
				SetButton(i, &pData[i]);

			if (controls.GetSize() > 0)
			{
				for (int i = 0; i < controls.GetSize(); i++)
				{
					CWnd* pWnd = GetDlgItem(controls[i].nID);
					if (pWnd != NULL)
					{
						CRect rect;
						pWnd->GetWindowRect(&rect);
						CPoint pt = rect.TopLeft() - controls[i].rectOldPos.TopLeft();
						GetItemRect(controls[i].nIndex, &rect);
						pt = rect.TopLeft() + pt;
						pWnd->SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
					}
				}
			}
/*
			// show/hide/reposition the expansion button
			if (CXTExpButton* button = (CXTExpButton*)GetDlgItem(XT_IDC_CUSTOMIZE))
			{
				bool bFloating = (m_dwStyle & CBRS_FLOATING) != 0;
				if (!bFloating && (IsExpansionCustomizes() || IsExpansionChevron()))
				{
					if (dwMode & LM_COMMIT)
					{
						int cx, cy;
						if (IsChevronHorizontal())
						{
							cx = EXPANSION_BUTTON_WIDTH;
							cy = sizeResult.cy - 2 * EXPANSION_BUTTON_INSET;
						}
						else
						{
							cx = sizeResult.cx - 2 * EXPANSION_BUTTON_INSET;
							cy = EXPANSION_BUTTON_WIDTH;
						}
						CRect rectButton;
						button->GetWindowRect(&rectButton);
						ScreenToClient(rectButton);
						if (rectButton.Width() != cx || rectButton.Height() != cy || !button->IsWindowVisible())
						{
							button->SetWindowPos(NULL, 0, 0, cx, cy,
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOMOVE );
							bRedraw = true;
						}
					}
				}
				else
				{
					if (dwMode & LM_COMMIT)
					{
						if (button->IsWindowVisible())
						{
							button->ShowWindow(SW_HIDE);
							bRedraw = true;
						}
					}
				}
			}
*/
			m_bDelayedButtonLayout = bIsDelayed;
		}
	}

	//BLOCK: Adjust Margins
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, (dwMode & LM_HORZ));
		sizeResult.cy -= rect.Height();
		sizeResult.cx -= rect.Width();

		CSize size = CControlBar::CalcFixedLayout((dwMode & LM_STRETCH), (dwMode & LM_HORZ));
		sizeResult.cx = __max(sizeResult.cx, size.cx);
		sizeResult.cy = __max(sizeResult.cy, size.cy);
	}

	if (bRedraw)
	{
		// force update of the UI state 
		PostMessage(WM_IDLEUPDATECMDUI, true);
		//RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	return sizeResult;
}