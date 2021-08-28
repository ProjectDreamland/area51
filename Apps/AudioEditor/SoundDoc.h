// SoundDoc.h : interface of the CSoundDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDDOC_H__DA55E35B_65F9_4D24_BBCE_684F4997E82A__INCLUDED_)
#define AFX_SOUNDDOC_H__DA55E35B_65F9_4D24_BBCE_684F4997E82A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//==============================================================================
// INCLUDE
//==============================================================================

#include "sound_file.hpp"
#include "AudioEditor.hpp"
#include "..\Editor\BaseDocument.h"
#include "AudioDefines.h"

//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CPropertyEditorDoc;


//==============================================================================
// SOUND DOCUMENT
//==============================================================================

class CSoundDoc : public CBaseDocument
{


//==============================================================================
protected:
    xarray<sound_file>  m_pSoundFile;
    

public:

    audio_editor        m_AudioEditor;
    CPropertyEditorDoc* m_pPropEditor;
        
    xarray<f32> m_pOffset;

    s32         m_SelectionStart;
    s32         m_SelectionEnd;
    s32         m_SoundFileCount;
    f32         m_MaxTime;
    s32         m_MaxTimeIndex;
    CWnd*		m_pCommandHandler;
    CString     m_ElementPathName;
    DWORD       m_SelectedItemData;
    CString     m_AudioSourcePath;
    s32         m_SampleSelected;
    s32         m_SoundUnits;
    CString     m_RecentAudioSampleName;

    enum{ SAMPLES, SECONDS };
//==============================================================================
public:

    xbool           GetIsLoaded             ( s32 Index );
    s32             GetSampleRate           ( s32 Index );
    s32             GetNumChannels          ( s32 Index );
    s32             GetNumSamples           ( s32 Index );
    s32             GetBreakPoints          ( xarray<aiff_file::breakpoint>& BreakPoints, s32 Index );
    s16*            GetChannelData          ( s32 iChannel, s32 Index );
    xbool           LoadSample              ( LPCTSTR lpszPathName, f32 Offset = 0.0f );
    void            UnLoadSample            ( s32 Index );
    void            ClearAllSamples         ( void );

    s32             GetSelectionStart       ( void );
    s32             GetSelectionEnd         ( void );

    void            OnNewAudioPackage       ( void );
    void            OnNewAudioDescriptor    ( void );
    void            OnNewAudioElement       ( CString PathName );
    void            OnAddAudioElement       ( void );
    void            RefrenceDescriptor      ( void );

    DWORD           GetSelectedItemData     ( void );
    void            SampleToDescriptor      ( void );
    void            CreateDescFromSample    ( CString PathName );
    
    CString         GetAudioSourcePath      ( void );
    void            SetAudioSourcePath      ( CString& rPath );
    void            SetRecentAudioSampleName( CString SampleName );
    CString         GetRecentAudioSampleName( void );
    void            OnLoadFaderList         ( void );
    void            OnSortDescending        ( void );
    void            OnCopy                  ( void );
    void            OnPaste                 ( void );

    void            ElementSelectionChange  ( s32   SampleSelected );
    void            RefreshSampleView       ( void );
    void            RefreshIntensityView    ( void );

    virtual void    OnProjectOpen           ( void );
    virtual void    OnProjectClose          ( void );
    virtual void    OnProjectSave           ( void );
    virtual void    OnProjectNew            ( void );

    
    void            SetCommandHandler(CWnd *pWnd) { m_pCommandHandler = pWnd; }


//==============================================================================
// MFC
//==============================================================================
public:
	CSoundDoc();
	DECLARE_DYNCREATE(CSoundDoc)
	virtual ~CSoundDoc();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_SOUNDDOC_H__DA55E35B_65F9_4D24_BBCE_684F4997E82A__INCLUDED_)
