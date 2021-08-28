#if !defined(AFX_AUDIOEDITORFRAME_H__F43BE948_D867_4D59_8D87_46951519672F__INCLUDED_)
#define AFX_AUDIOEDITORFRAME_H__F43BE948_D867_4D59_8D87_46951519672F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CAudioEditorFrame.h : header file
//

//==============================================================================
// INCLUDES
//==============================================================================
#include "..\Editor\BaseFrame.h"
#include "TabCtrlViewFix.h"

//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CPropertyEditorDoc;
class CSoundDoc;

//==============================================================================
// CAUDIOEDITORFRAME FRAME
//==============================================================================

class CAudioEditorFrame : public CBaseFrame
{

public:
    
    void OnNewAudioElement              ( void );
    void OnNewAudioPackage              ( void );
    void OnNewAudioDescriptor           ( void );
    void OnRefDescriptor                ( void );

    void OnSave                         ( void );
    void OnLoad                         ( void );
    void OnGetSourcePath                ( void );
    void OnAddFader                     ( void );   
    void OnChangeDefaultParams          ( void );
    void OnChangeDefaultElementParams   ( void );
    void OnSortDescending               ( void );

    void OnCopy                         ( void );
    void OnPaste                        ( void );

    void OnPlaySound                    ( void );
    void OnStopSound                    ( void );
 
    void OnSoundUnits                   ( void );

protected:

    CTabCtrlViewFix   m_wndProperty;
    CTabCtrlViewFix   m_wndWrkspBar;
    CTabCtrlViewFix   m_wndSample;


    s32                 m_SoundViewCount;

    CSoundDoc*          m_pDoc;
    CPropertyEditorDoc* m_pPropEditor;
    CImageList          m_imageList;
	CXTToolBar          m_wndToolBar;
    CXTToolBar          m_wndPackageToolBar;
    CXTFlatComboBox     m_wndComboUnits;    
    xbool               m_OnInit;
    CString             m_StatusSampleRate;
    CString             m_StatusNumSamples;
    CString             m_StatusSelStart;
    CString             m_StatusSelEnd;
    CString             m_StatusZoom;
    CString             m_CurrentPropertyName;
    xtimer              m_AudioDeltaTime;
    u32                 m_nTimer;
    f32                 m_LastTime;
    ial_hchannel        hChannel;

//==============================================================================
// MFC
//==============================================================================
protected:
	
	CAudioEditorFrame();           // protected constructor used by dynamic creation
	virtual ~CAudioEditorFrame();
    DECLARE_DYNCREATE(CAudioEditorFrame)

//==============================================================================
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioEditorFrame)
	virtual void ActivateFrame                  (int nCmdShow = -1);
	virtual BOOL PreCreateWindow                (CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CEventEditorFrame)
	afx_msg int     OnCreate                    (LPCREATESTRUCT lpCreateStruct);
    afx_msg void    OnDestroy                   ();
    afx_msg LRESULT OnOpenSoundFile             (WPARAM wParam, LPARAM lParam);  
    afx_msg LRESULT OnClearAllSoundFile         (WPARAM wParam, LPARAM lParam);  
    afx_msg LRESULT SetStatusText               (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT SetStatusSampleRate         (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT SetStatusNumSamples         (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT SetStatusSelectionStart     (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT SetStatusSelectionEnd       (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT SetStatusZoom               (WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT OnPropertyEditorSelChange   (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CAudioEditorFrame)
	afx_msg void OnTimer                        (UINT nIDEvent);
	afx_msg void OnClose                        ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_AUDIOEDITORFRAME_H__F43BE948_D867_4D59_8D87_46951519672F__INCLUDED_)
