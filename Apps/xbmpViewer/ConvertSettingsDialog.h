#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// XBMPSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConvertSettingsDialog dialog

class CConvertSettingsDialog : public CDialog
{
public:
    CConvertSettingsDialog(CWnd* pParent = NULL);

    enum { IDD = IDD_CONVERT_XBMP_DIALOG };

public:
    int m_MipLevels;
    CString m_SelectedPlatform;
    CString m_SelectedFormat;
    int m_GenericCompression;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:

    CComboBox m_CtrlPlatformCombo;
    CComboBox m_CtrlFormatCombo;
    CEdit m_CtrlMipLevelsEdit;

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnUpdateDialog();
    virtual void OnPlatformChanged();
    void UpdateFormatsByPlatform(const CString& platform, BOOL genericCompression);
};