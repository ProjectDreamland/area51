#if !defined(AFX_PARAMSDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
#define AFX_PARAMSDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParamsDialog.h : header file
//

//==============================================================================
// CPARAMSDIALOG DIALOG
//==============================================================================

class CParamsDialog : public CDialog
{
//==============================================================================
public:
	CParamsDialog(CWnd* pParent = NULL);   // standard constructor

    CEdit           m_Volume;
    CEdit           m_VolumeVar;
    CEdit           m_Pitch;
    CEdit           m_PitchVar;
    CEdit           m_Pan;
    CEdit           m_Priority;
    CEdit           m_EffectSend;
    CEdit           m_NearFalloff;
    CEdit           m_FarFalloff;

    CCheckListBox   m_VolumeCheck;
    CCheckListBox   m_VolumeVarCheck;
    CCheckListBox   m_PitchCheck;
    CCheckListBox   m_PitchVarCheck;
    CCheckListBox   m_PanCheck;
    CCheckListBox   m_PriorityCheck;
    CCheckListBox   m_EffectSendCheck;
    CCheckListBox   m_NearFalloffCheck;
    CCheckListBox   m_FarFalloffCheck;

	enum { IDD = IDD_PARAM_DIALOG };

//==============================================================================
protected:

	// Generated message map functions
	//{{AFX_MSG(CParamsDialog)
	virtual BOOL OnInitDialog();
    virtual void OnOK( );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_PARAMSDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
