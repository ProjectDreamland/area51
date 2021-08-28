#if !defined(AFX_KEYBAR_H__04031721_76EC_41D3_B8CA_3D4E04054959__INCLUDED_)
#define AFX_KEYBAR_H__04031721_76EC_41D3_B8CA_3D4E04054959__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyBar.h : header file
//

#include "PushButtonBmp.h"
#include "TimeSlider.h"
#include "EditInt.h"
#include "StatusBox.h"

#include "KeySet.h"
#include "KeyFilter.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyBar window

#define WM_USER_MSG_KEYBAR_ANIMMODECHANGE           WM_USER + 1000
#define WM_USER_MSG_KEYBAR_TIMECHANGE               WM_USER + 1001
#define WM_USER_MSG_KEYBAR_TIMERANGECHANGE          WM_USER + 1002
#define WM_USER_MSG_KEYBAR_PLAYBACKMODECHANGE       WM_USER + 1003

#define WM_USER_MSG_KEYBAR_KEYS_SELECTION_CHANGED   WM_USER + 1004
#define WM_USER_MSG_KEYBAR_KEYS_MOVED               WM_USER + 1005
#define WM_USER_MSG_KEYBAR_KEYS_DELETED             WM_USER + 1006
#define WM_USER_MSG_KEYBAR_KEYS_CHANGED             WM_USER + 1007


class CKeyBar : public CControlBar
{
    // Construction
    public:
	    CKeyBar();

    // Attributes
    public:

    // Operations
    public:

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CKeyBar)
	public:
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

    // Implementation
    public:
	    virtual ~CKeyBar();

        f32     GetTime             ( void ) const;
        int     GetTimeRangeStart   ( void ) const;
        int     GetTimeRangeEnd     ( void ) const;

        int     SetTime             ( int Frame );
        int     SetTimeRange        ( int FrameStart, int FrameEnd );
        void    SetKeySetCount      ( int nKeySets );
        void    SetKeySet           ( int KeySetIdx, KeySet* pKeySet );
        int     SetKeySets          ( int nKeySets, KeySet* pKeySets );
        int     SetKeyFilters       ( int nKeyFilters, KeyFilter* pKeyFilters );

        bool    IsPlayOn            ( void );
        void    StartPlayback       ( void );
        void    StopPlayback        ( void );

    protected:
        // Controls
        CPushButtonBmp  m_VCR_Filter;
        CPushButtonBmp  m_VCR_Animate;

        CPushButtonBmp  m_VCR_Home;
        CPushButtonBmp  m_VCR_StepBack;
        CPushButtonBmp  m_VCR_Play;
        CPushButtonBmp  m_VCR_StepForward;
        CPushButtonBmp  m_VCR_End;
        CPushButtonBmp  m_VCR_Repeat;
        CPushButtonBmp  m_VCR_KeyStep;

        CTimeSlider     m_TimeSlider;

        CEditInt        m_Edit_FrameStart;
        CEditInt        m_Edit_FrameEnd;
        CEditInt        m_Edit_FrameCurrent;

        CStatusBox      m_StatusLeft;
        CStatusBox      m_StatusRight;

        CMenu           m_FilterMenu;

        // Properties
        f32             m_FrameStart;
        f32             m_FrameEnd;
        f32             m_FrameCurrent;

        int             m_nKeyFilters;
        KeyFilter*      m_pKeyFilters;

        UINT            m_PlayTimer;
        xtimer          m_Timer;

        bool            m_IsAnimateOn;
        bool            m_IsPlayOn;

        bool            m_DoLoopPlayback;
        bool            m_DoKeyStep;

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CKeyBar)
	    afx_msg void    OnPaint();
	    afx_msg void    OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()

        afx_msg void    OnChange_FrameStart();
        afx_msg void    OnChange_FrameEnd();
        afx_msg void    OnChange_FrameCurrent();

        afx_msg LRESULT OnEdit_Entered                      ( WPARAM wParam, LPARAM lParam );

        afx_msg LRESULT OnTimeSlider_Edit_Begin             ( WPARAM wParam, LPARAM lParam );

        afx_msg LRESULT OnTimeSlider_FrameStart_Changed     ( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnTimeSlider_FrameEnd_Changed       ( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnTimeSlider_FrameCurrent_Changed   ( WPARAM wParam, LPARAM lParam );

        afx_msg LRESULT OnTimeSlider_Keys_Selection_Changed ( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnTimeSlider_Keys_Moved             ( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnTimeSlider_Keys_Deleted           ( WPARAM wParam, LPARAM lParam );
        afx_msg LRESULT OnTimeSlider_Keys_Changed           ( WPARAM wParam, LPARAM lParam );

        afx_msg LRESULT OnPushButton_Clicked                ( WPARAM wParam, LPARAM lParam );

        afx_msg void    OnUpdateCmdUI                       ( CFrameWnd* pTarget, BOOL bDisableIfNoHndler );
        afx_msg CSize   CalcFixedLayout                     ( BOOL bStretch, BOOL bHorz );


        //***************************************************
        // SAFE COPY:  ClassWizard may delete these above!!!
        //***************************************************
        /*
            void    OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler );
            CSize   CalcFixedLayout( BOOL bStretch, BOOL bHorz );
        */
        //*****************************************************
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYBAR_H__04031721_76EC_41D3_B8CA_3D4E04054959__INCLUDED_)
