// XBMPSettings.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpviewer.h"
#include "ConvertSettingsDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CConvertSettingsDialog dialog

CConvertSettingsDialog::CConvertSettingsDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CConvertSettingsDialog::IDD, pParent)
{
    //Default settings INI.
    m_MipLevels        = 0;
    m_SelectedPlatform = _T("PC");
    m_SelectedFormat   = _T("32_ARGB_8888");
}

void CConvertSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_MIP_EDIT, m_MipLevels);                  //Mip Levels definications
    DDX_CBString(pDX, IDC_PLATFORM_COMBO, m_SelectedPlatform); //Platform definications
    DDX_CBString(pDX, IDC_FORMAT_COMBO, m_SelectedFormat);     //Compression definications
}

BEGIN_MESSAGE_MAP(CConvertSettingsDialog, CDialog)
END_MESSAGE_MAP()

BOOL CConvertSettingsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_CtrlPlatformCombo.SubclassDlgItem(IDC_PLATFORM_COMBO, this);
    m_CtrlPlatformCombo.AddString(_T("PC"));
    m_CtrlPlatformCombo.AddString(_T("Xbox"));
    m_CtrlPlatformCombo.AddString(_T("PS2"));
    m_CtrlPlatformCombo.AddString(_T("GameCube"));
    m_CtrlPlatformCombo.AddString(_T("Native"));

    m_CtrlFormatCombo.SubclassDlgItem(IDC_FORMAT_COMBO, this);
    m_CtrlFormatCombo.AddString(_T("32_RGBA_8888"));
    m_CtrlFormatCombo.AddString(_T("32_RGBU_8888"));
    m_CtrlFormatCombo.AddString(_T("32_ARGB_8888"));
    m_CtrlFormatCombo.AddString(_T("32_URGB_8888"));
    m_CtrlFormatCombo.AddString(_T("24_RGB_888"));
    m_CtrlFormatCombo.AddString(_T("16_RGBA_4444"));
    m_CtrlFormatCombo.AddString(_T("16_ARGB_4444"));
    m_CtrlFormatCombo.AddString(_T("16_RGBA_5551"));
    m_CtrlFormatCombo.AddString(_T("16_RGBU_5551"));
    m_CtrlFormatCombo.AddString(_T("16_ARGB_1555"));
    m_CtrlFormatCombo.AddString(_T("16_URGB_1555"));
    m_CtrlFormatCombo.AddString(_T("16_RGB_565"));
    m_CtrlFormatCombo.AddString(_T("NoComp"));

    //Default settings for UI.
    m_CtrlPlatformCombo.SetCurSel(0);  //"PC" by default.
    m_CtrlFormatCombo.SetCurSel(2);    //"32_ARGB_8888" by default, for PC.

    return TRUE;
}
