#if !defined(AFX_VIRTUALLISTCTRL_H__8D7FCD7C_D1DA_44B5_BEAE_60C0648270E0__INCLUDED_)
#define AFX_VIRTUALLISTCTRL_H__8D7FCD7C_D1DA_44B5_BEAE_60C0648270E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlPlus.h : header file
//

#include "SelectionSet.h"

enum vlc_messages
{
    VLC_CHECK_CHANGED        = 1000,
    VLC_FOCUS_CHANGED,
    VLC_CONTEXT_MENU,
};


/////////////////////////////////////////////////////////////////////////////
// CVirtualListCtrl window

class CVirtualListCtrl : public CWnd
{
    struct column_info
    {
        BOOL    m_Visible;
        CString m_Name;
        int     m_Width;
        int     m_Format;
        DWORD   m_TextAlign;
        int     m_nSubItem;
        int     m_Index;
    };

typedef CArray<column_info,column_info&> column_array;

// Construction
public:
	CVirtualListCtrl();

// Attributes
public:
    // Configuration
    BOOL            m_LineMode;                     // TRUE = line mode scrolling, FALSE = pixel mode scrolling
    DWORD           m_ExtendedStyle;                // Extended styles for the control

    // State
    BOOL            m_IsFocusWnd;                   // Is the windows with focus
    BOOL            m_TrackingLeave;                // Track mouse leave events
    int             m_HotCheck;                     // Hot checkbox row

    // Font
    CFont           m_Font;                         // Font for header and list

    // Header
    CHeaderCtrl     m_Header;                       // Header control
    CRect           m_HeaderRect;                   // CRect for positioning the header control

    // Mouse
    CPoint          m_ClickedPoint;                 // Point of click
    int             m_ClickedRow;                   // Row clicked
    int             m_MouseCaptured;                // Mouse if captured

    // User area = Client area - header and scrollbars
    CRect           m_UserRect;

    // Scrollbars
    CScrollBar      m_SizeBox;                      // Size box
    CScrollBar      m_HorzScrollbar;                // Horizontal scrollbar
    CScrollBar      m_VertScrollbar;                // Vertical scrollbar
    BOOL            m_HorzSBVisible;                // Horizontal Bar Visible
    BOOL            m_VertSBVisible;                // Vertical Bar Visible
    int             m_ScrollX;                      // X scroll in pixels
    int             m_ScrollY;                      // Y scroll in pixels

    // Theme
    HTHEME          m_hTheme;                       // Handle to theme data
    CSize           m_CheckSize;                    // Size of the checkbox

    // Colors
    COLORREF        m_ColorSeperator;               // Color of seperator

    // Columns
    column_array    m_Columns;                      // Array of columns
    int             m_SortColumn;                   // -1 = none
    BOOL            m_SortAscending;                // true = Ascending, false = Descending

    // Rows
    int             m_CellHeight;                   // Pixel height of cell
    int             m_RowHeight;                    // Pixel height of row
    int             m_nItems;                       // Number of items
    int             m_FocusItem;                    // Focus item, -1 = none
    int             m_SelectionRoot;                // Root for ranged selections
    CSelectionSet   m_Selection;                    // Selection set for the list

// Operations
public:
        BOOL            Create              ( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
        DWORD           SetExtendedStyle    ( DWORD dwNewStyle );

        // Misc
        void            SetRedraw           ( BOOL bRedraw = TRUE ) {};

        // Font
        CFont*          GetFont             ( void );
        void            SetFont             ( CFont* pFont, BOOL bRedraw = TRUE );

        // Header and Columns
        CHeaderCtrl*    GetHeaderCtrl       ( void );
        int             InsertColumn        ( int nCol, const LVCOLUMN* pColumn );
        int             InsertColumn        ( int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem );
        BOOL            DeleteColumn        ( int nCol );
        void            SetSortColumn       ( int nCol, BOOL Ascending = TRUE );

        // Items
virtual int             GetItemCount        ( void );
virtual void            SetItemCount        ( int nItems );
virtual void            SetItemCountEx      ( int nItems, int Flags = 0 );
virtual bool            IsValidItem         ( int nItem );
virtual bool            IsVisible           ( int nItem );
virtual BOOL            EnsureVisible       ( int nItem, int PartialOk = FALSE );
virtual void            EnsureCentered      ( int nItem );
virtual BOOL            IsLastItemInFocus   ( void );
virtual void            SetFocusItem        ( int nItem );
        int             GetFocusItem        ( void );

        // Higher level marking functions
        void            ToggleMarkSelected  ( void );
        void            GotoNextMark        ( s32 Direction = 1 );

        // Selection, Marks and Checks
  const CSelectionSet&  GetSelectionSet     ( void );
virtual void            BuildSelectionSet   ( void );
        void            ClearSelection      ( void );
        bool            GetSelected         ( int iRow );
        void            SetSelected         ( int iRow, bool State = true );
        void            SetSelected         ( int iRowStart, int iRowEnd, bool State = true );
        void            ClearMarked         ( void );
        bool            GetMarked           ( int iRow );
        void            SetMarked           ( int iRow, bool State = true );
        void            ClearChecked        ( void );
        bool            GetChecked          ( int iRow );
        void            SetChecked          ( int iRow, bool State = true );

protected:
        // Colors
        COLORREF        DarkenColor         ( COLORREF Color, int r, int g, int b );
        COLORREF        DarkenColorPercent  ( COLORREF Color, int r, int g, int b );

        // Picking
        BOOL            GetCellRect         ( int iRow, int iCol, CRect* pRect );
        int             PointToRow          ( const CPoint& Point, BOOL Clip = FALSE );
        int             PointToCol          ( const CPoint& Point );

        // Header
        int             GetHeaderWidth      ( void );
        void            PositionHeader      ( void );

        // Scrollbars
        void            UpdateScrollbars    ( int cx, int cy );
        void            UpdateScrollbars    ( void );
        void            SetVertScroll       ( int Value );

        // Virtual overrides
virtual bool            OnGetSelected       ( int iRow );
virtual void            OnSetSelected       ( int iRow, bool State );
virtual bool            OnGetMarked         ( int iRow );
virtual void            OnSetMarked         ( int iRow, bool State );
virtual bool            OnGetChecked        ( int iRow );
virtual void            OnSetChecked        ( int iRow, bool State );
virtual BOOL            OnDrawCell          ( CDC* pDC, CRect& rCell, int iRow, int iCol );
virtual void            OnSort              ( void );
virtual int             OnGetColumnFitWidth ( int iCol );
virtual void            OnFind              ( void );
virtual void            OnFindNext          ( void );
virtual void            OnFindPrevious      ( void );
virtual void            OnFocusChanged      ( void );

virtual void            OnCopy              ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVirtualListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVirtualListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVirtualListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    afx_msg void OnHeaderBeginTrack     ( NMHDR* pNotifyStruct, LRESULT* pResult );
    afx_msg void OnHeaderItemChanged    ( NMHDR* pNotifyStruct, LRESULT* pResult );
    afx_msg void OnHeaderEndDrag        ( NMHDR* pNotifyStruct, LRESULT* pResult );
    afx_msg void OnHeaderItemClick      ( NMHDR* pNotifyStruct, LRESULT* pResult );
    afx_msg void OnHeaderDividerDblClick( NMHDR* pNotifyStruct, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIRTUALLISTCTRL_H__8D7FCD7C_D1DA_44B5_BEAE_60C0648270E0__INCLUDED_)
