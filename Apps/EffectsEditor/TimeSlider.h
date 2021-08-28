#if !defined(AFX_TIMESLIDER_H__28494A6B_B1BF_4508_A2BE_85E18B1B2D87__INCLUDED_)
#define AFX_TIMESLIDER_H__28494A6B_B1BF_4508_A2BE_85E18B1B2D87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeSlider.h : header file
//

#include "KeySet.h"
#include "KeyFilter.h"

/////////////////////////////////////////////////////////////////////////////
// CTimeSlider window

#define WM_USER_MSG_TIMESLIDER_FRAMESTART_CHANGED       WM_USER + 200
#define WM_USER_MSG_TIMESLIDER_FRAMEEND_CHANGED         WM_USER + 201
#define WM_USER_MSG_TIMESLIDER_FRAMECURRENT_CHANGED     WM_USER + 202

#define WM_USER_MSG_TIMESLIDER_KEYS_SELECTION_CHANGED   WM_USER + 203
#define WM_USER_MSG_TIMESLIDER_KEYS_MOVED               WM_USER + 204
#define WM_USER_MSG_TIMESLIDER_KEYS_DELETED             WM_USER + 205
#define WM_USER_MSG_TIMESLIDER_KEYS_CHANGED             WM_USER + 206

#define WM_USER_MSG_TIMESLIDER_EDIT_BEGIN               WM_USER + 207

class CTimeSlider : public CWnd
{
// Construction
public:
	CTimeSlider();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeSlider)
	public:
	virtual BOOL Create(CWnd* pParentWnd, int posX, int posY, int nWidth, int nHeight, UINT nID);
	virtual void MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
	//}}AFX_VIRTUAL


    //***************************************************
    // SAFE COPY:  ClassWizard may delete these above!!!
    //***************************************************
    /*
	    virtual BOOL Create(CWnd* pParentWnd, int posX, int posY, int nWidth, int nHeight, UINT nID);
        virtual void MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
    */

// Implementation
public:
	virtual ~CTimeSlider();
        void            SetFrameStart       ( int frameStart );
        void            SetFrameEnd         ( int frameEnd );
        void            SetFrameCurrent     ( int frameCurrent );
        void            SetTime             ( int frameStart, int frameEnd, int frameCurrent );
        void            SetAnimateMode      ( bool AnimateModeOn );

        void            SetKeySetCount      ( int nKeySets );
        void            SetKeySet           ( int KeySetIdx,   KeySet*    pKeySet     );
        int             SetKeySets          ( int nKeySets,    KeySet*    pKeySets    );
        int             SetKeyFilters       ( int nKeyFilters, KeyFilter* pKeyFilters );
        bool            SetKeyFilterState   ( int iKeyFilter,  bool       FilterState );

    protected:
        // Draw Properties
        COLORREF        m_ColorBorder;
        COLORREF        m_ColorBackground;

        COLORREF        m_ColorFrameLines;
        COLORREF        m_ColorFrameText;

        COLORREF        m_ColorTimeFill;
        COLORREF        m_ColorTimeBorder;

        COLORREF        m_ColorKeyBorder;

        CFont           m_Font;

        int             m_Width;
        int             m_WidthFrames;
        int             m_Height;
        int             m_HeightFrames;

        bool            m_DrawSelectMarquee;
        bool            m_AnimateModeOn;


        // Time Properties
        int             m_FrameStart;
        int             m_FrameEnd;
        int             m_FrameCurrent;

        void            SetTimeChange       ( CPoint point );
        void            SetTimePan          ( CPoint point );

        // Key Properties
        int             m_nKeyFilters;
        KeyFilter*      m_pKeyFilters;

        int             m_nKeySets;
        KeySet*         m_pKeySets;

        // Key Editing Properties
        int             m_nEditKeySets;
        int*            m_piEditKeySets;

        KeySet*         m_pKeySets_Edit_InActive;
        KeySet*         m_pKeySets_Edit_Active;
        KeySet*         m_pKeySets_Edit_ActiveRef;

        // Key Functions
        bool            CheckFilter             ( KeySet& CheckKeySet ) const;
        COLORREF        GetFilterColor          ( KeySet& CheckKeySet ) const;
        int             GetNumSelectedKeys      ( void ) const;

        void            DeselectVisibleKeys     ( void );
        bool            IsMouseOverValidKey     ( CPoint point, bool CheckSelection ) const;

        void            SetKeyIsSelectedByPoint ( CPoint point, bool IsSelected );
        void            SetKeyIsSelectedByRange ( int posX1, int posX2, bool IsSelected );
        void            SetOverKeyCursor        ( CPoint point, bool IsSelected );

        // Key Editing Functions
        void            DeleteSelectedKeys      ( void );
        void            MoveSelectedKeys        ( CPoint point );

        int             GetEditableKeySets      ( int*& pResult_KeySetIndices );

        int             KeyEditSetup_Delete     ( void );
        int             KeyEditSetup_Move       ( void );
        int             KeyEditSetup_CopyMove   ( void );

        bool            CopyKeySets             ( KeySet*       pSource,
                                                  KeySet*&      pTarget,
                                                  int           nKeySets,
                                                  int*          pSourceIndices,
                                                  bool          Copy_Visible_Selected       = true,
                                                  bool          Copy_Visible_UnSelected     = true,
                                                  bool          Copy_Invisible_Selected     = true,
                                                  bool          Copy_Invisible_UnSelected   = true );


        // Navigation Properties
        enum NavType
        {
            NAVIGATE_NULL,

            NAVIGATE_TIME_CHANGE,
            NAVIGATE_TIME_PAN,
            NAVIGATE_KEY_SELECT,
            NAVIGATE_KEY_MOVE,

            NAVIGATE_PAD = 0xffffffff
        };

        NavType     m_NavigateType;
        CPoint      m_NavigatePoint;
        CPoint      m_NavigateDragPoint;

        int         m_NavigateTimeStart;
        int         m_NavigateTimeEnd;
        int         m_NavigateTimeCurrent;

        void        BeginEdit   ( void );



	// Generated message map functions
protected:
	//{{AFX_MSG(CTimeSlider)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMESLIDER_H__28494A6B_B1BF_4508_A2BE_85E18B1B2D87__INCLUDED_)
