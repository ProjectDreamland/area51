/******************************************************************************

$Author: Holte $
  
$Modtime: 15.10.02 13:22 $
$Revision: 3 $

Description: Interfaces of the classes "CListCtrlEx" and "CListViewEx"
             (list control and list view with sort icons and
              colored sort column)

$Log: /NUKO2/B0000009/Administration/ListViewCtrlEx.h $

3     15.10.02 13:33 Holte
Overloading of the member functions "InsertColumn", "HitTest", etc. has become
obsolete because the corresponding messages will be intercepted directly.
Now the extended styles LVS_EX_ONECLICKACTIVATE, LVS_EX_TWOCLICKACTIVATE, and
LVS_EX_UNDERLINEHOT are supported in owner drawn mode.

2     16.09.02 15:53 Holte
Changes of the class "CListBase":
Member function "MakeShortString" deleted.

1     20.06.02 14:17 Holte

******************************************************************************/

#pragma once

/*** Defines not yet available in MS VC++ 6.0 ********************************/
#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT 26
#endif

/*** Declaration of "workhorse" class "CListBase" ****************************/
class CListBase
{
    public:
    CListBase(): m_nIconXOff          (4),
                 m_pListCtrl          (0),
                 m_bSortIconEnabled   (false),
                 m_bColorSortColumn   (false),
                 m_nSortColumn        (0),
                 m_bKeepLabelLeft     (false),
                 m_bLocked            (false),
                 m_bControl           (false),
                 m_bIconXOffCalculated(false),
                 m_dwExtendedStyle    (0),
                 m_nHotItem           (-1),
                 m_dwHotLite          ( _winver <= 0x0400 ? RGB(0, 0, 128) : GetSysColor(COLOR_HOTLIGHT)),
                 m_hcursorCustom      (0),
                 m_hcursorArrow       (0),
                 m_hcursorHand        (0),
                 m_bOnSetExtendedStyle(false) {}

    ~CListBase()
    {
        if( _winver <= 0x400 && m_hcursorHand )
            DestroyCursor( m_hcursorHand );
    }

    void            ColorSortColumn ( BOOL bEnable = TRUE, int nSortColumn = 0 );
    virtual void    DrawSmallIcon   ( CDC* pDC, LVITEM* pItem, LPRECT pRect );
    virtual void    DrawStateIcon   ( CDC* pDC, LVITEM* pItem, LPRECT pRect );
    virtual void    DrawSubItemText ( CDC* pDC, LVITEM* pItem, LVCOLUMN* pColumn, LPRECT pRect );
    void            EnableSortIcon  ( BOOL bEnable = true, int nSortColumn = 0 );
    int             IndexToOrder    ( int nIndex );
    bool            KeepLabelLeft   ( bool bKeepLeft = true );
    void            SetSortColumn   ( int nColumn );
    int             GetSortColumn   ( void );

private:
    friend class CListCtrlEx;
    friend class CListViewEx;

    void            CreateSortIcons      ( void );
    void            DrawItem             ( LPDRAWITEMSTRUCT lpDrawItemStruct );
    int             GetLabelWidth        ( CDC* pDC, LVITEM* pItem, int nMaxWidth ) const;
    LVITEM*         GetLVITEM            ( int nItem, int nSubItem = 0 ) const;
    bool            GetRealSubItemRect   ( int iItem, int iSubItem, int nArea, CRect& ref );
    bool            GetStateIconRect     ( int nItem, LPRECT pRect );
    void            InvalidateNonItemArea( void );
    void            JustifyFirstColumn   ( int nFormat );
    BOOL            OnColumnclick        ( NMHDR* pNMHDR, LRESULT* pResult );
    void            OnCustomDraw         ( NMHDR* pNMHDR, LRESULT* pResult );
    bool            OnEraseBkgnd         ( CDC* pDC );
    int             OnHitTest            ( LPLVHITTESTINFO pHitTestInfo );
    bool            OnKeyDown            ( UINT nChar );
    void            OnKeyUp              ( UINT nChar );
    void            OnKillFocus          ( void );
    bool            OnLButtonDblClk      ( CPoint point );
    bool            OnLButtonDown        ( CPoint point );
    void            OnLButtonUp          ( void );
    bool            OnRButtonDblClk      ( CPoint point );
    bool            OnRButtonDown        ( CPoint point );
    void            OnRButtonUp          ( void );
    void            OnMouseMove          ( CPoint point );
    bool            OnNotify             ( LPARAM lParam );
    bool            OnSetExtendedStyle   ( DWORD dwMask, DWORD dwExStyle, LPDWORD pdwResult );
    void            OnSetFocus           ( void );
    void            OnSetImageList       ( int nImageList, HIMAGELIST himl );
    void            OnSysColorChange     ( void );
    int             OrderToIndex         ( int nOrder );
    void            PrepareHotUnderlining( void );
    void            SetSortIcon          ( void );

    static const int    m_nFirstColXOff;
    static const int    m_nNextColXOff;
    int                 m_nIconXOff;                // offset of icon may vary of unknown reason
    CListCtrlEx*        m_pListCtrl;
    BOOL                m_bSortIconEnabled;
    BOOL                m_bColorSortColumn;
    CImageList          m_imglstSortIcons;
    CBitmap             m_bmpUpArrow;
    CBitmap             m_bmpDownArrow;
    int                 m_nUpArrow;
    int                 m_nDownArrow;
    DWORD               m_dwColSortColor;
    int                 m_nSortColumn;
    int                 m_nFormatOfSubItem0;
    bool                m_bKeepLabelLeft;
    bool                m_bLocked;
    bool                m_bControl;
    bool                m_bIconXOffCalculated;
    DWORD               m_dwExtendedStyle;
    int                 m_nHotItem;
    COLORREF            m_dwHotLite;
    HCURSOR             m_hcursorCustom;
    HCURSOR             m_hcursorArrow;
    HCURSOR             m_hcursorHand;
    bool                m_bOnSetExtendedStyle;      // execution flag
};

/*** Declaration of class "CListCtrlEx" **************************************/

class CListCtrlEx: public CListCtrl, public CListBase
{
    DECLARE_DYNCREATE(CListCtrlEx);

public:
    CListCtrlEx () { m_pListCtrl = this; }

    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
    {
        CListBase::DrawItem( lpDrawItemStruct );
    }

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CListViewCtrl)
    protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

    // Generated message map functions
protected:
    //{{AFX_MSG(CListCtrlEx)
    afx_msg void OnSysColorChange();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg BOOL    OnColumnclick     (NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void    OnCustomDraw      (NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnGetExtendedStyle(WPARAM, LPARAM);
    afx_msg LRESULT OnHitTest         (WPARAM, LPARAM lParam);
    afx_msg LRESULT OnInsertColumn    (WPARAM, LPARAM lParam);
    afx_msg LRESULT OnSetExtendedStyle(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetImageList    (WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};


/*** Declaration of class "CListViewEx" **************************************/
class CListViewEx: public CListView, public CListBase
{
    DECLARE_DYNCREATE(CListViewEx);

public:
    CListViewEx( ) { m_pListCtrl = static_cast<CListCtrlEx*>(&GetListCtrl()); }

    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
    {
        CListBase::DrawItem( lpDrawItemStruct );
    }

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CListViewCtrl)
    protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

    // Generated message map functions
protected:
    //{{AFX_MSG(CListViewEx)
    afx_msg void OnSysColorChange();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg BOOL    OnColumnclick     (NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void    OnCustomDraw      (NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnGetExtendedStyle(WPARAM, LPARAM);
    afx_msg LRESULT OnHitTest         (WPARAM, LPARAM lParam);
    afx_msg LRESULT OnInsertColumn    (WPARAM, LPARAM lParam);
    afx_msg LRESULT OnSetExtendedStyle(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetImageList    (WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};
