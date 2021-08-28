#if !defined(AFX_INTENSITYDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
#define AFX_INTENSITYDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntensityDialog.h : header file
//

#include "AudioEditor.hpp"


//==============================================================================
// CIntensityDialog
//==============================================================================

class CIntensityDialog : public CDialog
{
//==============================================================================
public:
	CIntensityDialog(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_INTENSITY_DIALOG };
    CListBox	    m_DescListBox;
    xarray<xstring> m_Desc;
    
    void        InsertDescpritors( audio_editor& AudioEditor );

//==============================================================================
protected:

	// Generated message map functions
	//{{AFX_MSG(CIntensityDialog)
	virtual BOOL OnInitDialog();
    virtual void OnOK( );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern xarray< xstring > g_DescAdded;

#endif
