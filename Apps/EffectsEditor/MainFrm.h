// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include "KeyBar.h"
#include "KeySet.h"
#include "KeyFilter.h"
#include "Properties.h"
#include "resource.h"
#include "ParametricSplitter.h"

#include "XTFrameWnd.h"
#include "XTToolBar.h"

#if !defined(AFX_MAINFRM_H__5CA82DBB_1F0F_4E94_A6EA_4B2C82B88937__INCLUDED_)
#define AFX_MAINFRM_H__5CA82DBB_1F0F_4E94_A6EA_4B2C82B88937__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPartEdDoc;

class CMainFrame : public CXTFrameWnd
{
    // Operations
    public:
        enum manipulate_mode
        {
            MANIPULATE_SELECT,
            MANIPULATE_MOVE,
            MANIPULATE_ROTATE,
            MANIPULATE_SCALE
        };

        manipulate_mode     GetManipulateMode( void )               { return m_ManipulateMode; }

        f32                 GetGlobalTime( void )                   { return m_KeyBar.GetTime(); }
        void                SetGlobalTime( s32 T )                  { m_KeyBar.SetTime( T );     }
        void                GetMinMaxTime( s32& MinT, s32& MaxT )   { MinT = m_KeyBar.GetTimeRangeStart(); MaxT = m_KeyBar.GetTimeRangeEnd(); }
        void                SetMinMaxTime( s32 MinT, s32 MaxT )     { MinT = m_KeyBar.SetTimeRange( MinT, MaxT ); }

        void    PopulatePropertyControl( void );

    // Attributes
    public:
        CKeyBar             m_KeyBar;

    protected:
	    CXTToolBar          m_wndToolBar;
        CProperties         m_wndProperties;
        CParametricSplitter m_Splitter;

        CPartEdDoc*         m_pDoc;
        KeySet*             m_pKeySets;
        KeyFilter*          m_pKeyFilters;

        manipulate_mode     m_ManipulateMode;

    protected:
        bool    InitKeyBar( void );

    protected: // create from serialization only
	    CMainFrame();
	    DECLARE_DYNCREATE(CMainFrame)

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CMainFrame)
	    public:
	    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	    //}}AFX_VIRTUAL

    // Implementation
    public:
	    virtual ~CMainFrame();
    #ifdef _DEBUG
	    virtual void AssertValid() const;
	    virtual void Dump(CDumpContext& dc) const;
    #endif


    // Generated message map functions
    protected:
	    //{{AFX_MSG(CMainFrame)
	    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	    afx_msg void OnDestroy();
	    afx_msg void OnClose();
        afx_msg LRESULT OnTimeChange(WPARAM, LPARAM);
        afx_msg LRESULT OnKeysChange( WPARAM UserData, LPARAM NewData );
        afx_msg LRESULT OnAnimModeChange( WPARAM UserData, LPARAM NewData );
	    afx_msg void OnMainToolbarUndo();
	    afx_msg void OnUpdateMainToolbarUndo(CCmdUI* pCmdUI);
	    afx_msg void OnMainToolbarRedo();
	    afx_msg void OnUpdateMainToolbarRedo(CCmdUI* pCmdUI);
	    afx_msg void OnMainToolbarSelect();
	    afx_msg void OnMainToolbarMove();
	    afx_msg void OnMainToolbarRotate();
	    afx_msg void OnMainToolbarScale();
	    afx_msg void OnMainToolbarAxisX();
	    afx_msg void OnMainToolbarAxisY();
	    afx_msg void OnMainToolbarAxisZ();
	    afx_msg void OnMainToolbarAlign();
	    afx_msg void OnUpdateMainToolbarSelect(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarMove(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarRotate(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarScale(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarAxisX(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarAxisY(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarAxisZ(CCmdUI* pCmdUI);
	    afx_msg void OnUpdateMainToolbarAlign(CCmdUI* pCmdUI);
	    afx_msg void OnEditSelectAll();
	    afx_msg void OnEditSelectNone();
	    afx_msg void OnEditSelectInvert();
	    afx_msg void OnViewViewportMaximize();
	    afx_msg void OnUpdateViewViewportMaximize(CCmdUI* pCmdUI);
	    //}}AFX_MSG
	    DECLARE_MESSAGE_MAP()

	    afx_msg void    OnTimeGotoFirstFrame            ( void );
	    afx_msg void    OnTimeGotoLastFrame             ( void );
	    afx_msg void    OnTimeGotoPreviousFrame         ( void );
	    afx_msg void    OnTimeGotoNextFrame             ( void );
	    afx_msg void    OnTimePlay                      ( void );
	    afx_msg void    OnTimeZoomTimeExtents           ( void );

        afx_msg LRESULT OnProperties_Property_Changed   ( WPARAM ControlID, LPARAM lParam );
};

extern CMainFrame*      g_pMainFrame;
extern CProperties*     g_pProperties;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__5CA82DBB_1F0F_4E94_A6EA_4B2C82B88937__INCLUDED_)
