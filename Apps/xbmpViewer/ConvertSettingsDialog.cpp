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
    m_SelectedPlatform   = _T("PC");
    m_GenericCompression = 0;
    m_SelectedFormat     = _T("32_ARGB_8888");
    m_MipLevels          = 0;
}

void CConvertSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_PLATFORM_COMBO, m_SelectedPlatform);      //Platform definications
    DDX_Check(pDX, IDC_GENERIC_COMPRESSION, m_GenericCompression);  //Generic definications
    DDX_CBString(pDX, IDC_FORMAT_COMBO, m_SelectedFormat);          //Compression definications
    DDX_Text(pDX, IDC_MIP_EDIT, m_MipLevels);                       //Mip Levels definications
}

BEGIN_MESSAGE_MAP(CConvertSettingsDialog, CDialog)
    ON_BN_CLICKED(IDC_GENERIC_COMPRESSION, &CConvertSettingsDialog::OnUpdateDialog)
    ON_CBN_SELCHANGE(IDC_PLATFORM_COMBO, &CConvertSettingsDialog::OnPlatformChanged)
END_MESSAGE_MAP()

BOOL CConvertSettingsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Initialize platform combo box
    m_CtrlPlatformCombo.SubclassDlgItem(IDC_PLATFORM_COMBO, this);
    m_CtrlPlatformCombo.AddString(_T("PC"));
    m_CtrlPlatformCombo.AddString(_T("Xbox"));
    m_CtrlPlatformCombo.AddString(_T("PS2"));
    m_CtrlPlatformCombo.AddString(_T("GameCube"));
    m_CtrlPlatformCombo.AddString(_T("Native"));

    // Initialize format combo box
    m_CtrlFormatCombo.SubclassDlgItem(IDC_FORMAT_COMBO, this);
    
    // Initialize with "PC" formats by default
    UpdateFormatsByPlatform(_T("PC"), FALSE);
    
    // Initialize settings for UI.
    m_CtrlPlatformCombo.SetCurSel(0);  //"PC" by default.
    m_CtrlFormatCombo.SetCurSel(2);    //"32_ARGB_8888" by default, for PC.

    return TRUE;
}

void CConvertSettingsDialog::OnUpdateDialog()
{
    UpdateData(TRUE);
    
    // Update platform list
    m_CtrlPlatformCombo.ResetContent();
    
    if (m_GenericCompression)
    {
        m_CtrlPlatformCombo.AddString(_T("Native"));
    }
    else
    {
        m_CtrlPlatformCombo.AddString(_T("PC"));
        m_CtrlPlatformCombo.AddString(_T("Xbox"));
        m_CtrlPlatformCombo.AddString(_T("PS2"));
        m_CtrlPlatformCombo.AddString(_T("GameCube"));
        m_CtrlPlatformCombo.AddString(_T("Native"));
    }
    
    // Select first platform in list
    m_CtrlPlatformCombo.SetCurSel(0);
    
    // Update formats based on platform and compression mode
    if (m_GenericCompression)
    {
        UpdateFormatsByPlatform(_T("PC"), TRUE);
    }
    else
    {
        CString platform;
        m_CtrlPlatformCombo.GetLBText(m_CtrlPlatformCombo.GetCurSel(), platform);
        UpdateFormatsByPlatform(platform, FALSE);
    }
    
    UpdateData(FALSE);
}

void CConvertSettingsDialog::OnPlatformChanged()
{
    UpdateData(TRUE);
    
    // Get selected platform
    CString platform;
    m_CtrlPlatformCombo.GetLBText(m_CtrlPlatformCombo.GetCurSel(), platform);
    
    // Update formats based on platform
    UpdateFormatsByPlatform(platform, m_GenericCompression);
    
    UpdateData(FALSE);
}

void CConvertSettingsDialog::UpdateFormatsByPlatform(const CString& platform, BOOL genericCompression)
{
    m_CtrlFormatCombo.ResetContent();
    
    // Add basic formats available on all platforms
    if (platform == _T("PC") || platform == _T("Xbox"))
    {
        // PC/Xbox specific formats
        m_CtrlFormatCombo.AddString(_T("32_ARGB_8888"));
        m_CtrlFormatCombo.AddString(_T("32_URGB_8888"));
        m_CtrlFormatCombo.AddString(_T("24_RGB_888"));
        m_CtrlFormatCombo.AddString(_T("16_ARGB_1555"));
        m_CtrlFormatCombo.AddString(_T("16_URGB_1555"));
        m_CtrlFormatCombo.AddString(_T("16_RGB_565"));        
    }
    else if (platform == _T("PS2"))
    {
        // PS2 specific formats - BGR ordering
        m_CtrlFormatCombo.AddString(_T("32_ABGR_8888"));
        m_CtrlFormatCombo.AddString(_T("32_UBGR_8888"));
        m_CtrlFormatCombo.AddString(_T("24_BGR_888"));
        m_CtrlFormatCombo.AddString(_T("16_ABGR_1555"));
        m_CtrlFormatCombo.AddString(_T("16_UBGR_1555"));
        
        // PS2 palette-based formats
        m_CtrlFormatCombo.AddString(_T("P8_ABGR_8888"));
        m_CtrlFormatCombo.AddString(_T("P8_UBGR_8888"));
        m_CtrlFormatCombo.AddString(_T("P4_ABGR_8888"));
        m_CtrlFormatCombo.AddString(_T("P4_UBGR_8888"));
    }
    else if (platform == _T("GameCube"))
    {
        // GameCube specific formats - RGBA ordering
        m_CtrlFormatCombo.AddString(_T("32_RGBA_8888"));
        m_CtrlFormatCombo.AddString(_T("32_RGBU_8888"));
        m_CtrlFormatCombo.AddString(_T("16_RGBA_4444"));
        m_CtrlFormatCombo.AddString(_T("16_RGBA_5551"));
        m_CtrlFormatCombo.AddString(_T("16_RGBU_5551"));
        m_CtrlFormatCombo.AddString(_T("16_RGB_565"));
        
        // GameCube palette-based formats
        m_CtrlFormatCombo.AddString(_T("P8_RGBA_8888"));
        m_CtrlFormatCombo.AddString(_T("P8_RGBU_8888"));
        m_CtrlFormatCombo.AddString(_T("P4_RGBA_8888"));
        m_CtrlFormatCombo.AddString(_T("P4_RGBU_8888"));
    }
    else if (platform == _T("Native"))
    {
        // For Native, include all formats from all platforms
        // PC/Xbox formats
        m_CtrlFormatCombo.AddString(_T("32_ARGB_8888"));
        m_CtrlFormatCombo.AddString(_T("32_URGB_8888"));
        m_CtrlFormatCombo.AddString(_T("24_RGB_888"));
        m_CtrlFormatCombo.AddString(_T("16_ARGB_1555"));
        m_CtrlFormatCombo.AddString(_T("16_URGB_1555"));
        m_CtrlFormatCombo.AddString(_T("16_RGB_565"));
        
        // PS2 formats
        m_CtrlFormatCombo.AddString(_T("32_ABGR_8888"));
        m_CtrlFormatCombo.AddString(_T("32_UBGR_8888"));
        m_CtrlFormatCombo.AddString(_T("24_BGR_888"));
        m_CtrlFormatCombo.AddString(_T("16_ABGR_1555"));
        m_CtrlFormatCombo.AddString(_T("16_UBGR_1555"));
        
        // GameCube formats
        m_CtrlFormatCombo.AddString(_T("32_RGBA_8888"));
        m_CtrlFormatCombo.AddString(_T("32_RGBU_8888"));
        m_CtrlFormatCombo.AddString(_T("16_RGBA_4444"));
        m_CtrlFormatCombo.AddString(_T("16_RGBA_5551"));
        m_CtrlFormatCombo.AddString(_T("16_RGBU_5551"));
    }
    
    // Add all additional formats if generic compression is enabled.
    if (genericCompression)
    {
        // Additional RGB formats
        m_CtrlFormatCombo.AddString(_T("16_RGBA_4444"));
        m_CtrlFormatCombo.AddString(_T("16_ARGB_4444"));
        m_CtrlFormatCombo.AddString(_T("16_RGBA_5551"));
        m_CtrlFormatCombo.AddString(_T("16_RGBU_5551"));
        
        // Additional BGR formats
        m_CtrlFormatCombo.AddString(_T("16_BGRA_4444"));
        m_CtrlFormatCombo.AddString(_T("16_ABGR_4444"));
        m_CtrlFormatCombo.AddString(_T("16_BGRA_5551"));
        m_CtrlFormatCombo.AddString(_T("16_BGRU_5551"));
        
        // Additional palette formats
        m_CtrlFormatCombo.AddString(_T("P8_RGB_888"));
        m_CtrlFormatCombo.AddString(_T("P8_RGBA_4444"));
        m_CtrlFormatCombo.AddString(_T("P8_ARGB_4444"));
        m_CtrlFormatCombo.AddString(_T("P8_ARGB_1555"));
        m_CtrlFormatCombo.AddString(_T("P8_URGB_1555"));
        m_CtrlFormatCombo.AddString(_T("P8_RGB_565"));
        
        m_CtrlFormatCombo.AddString(_T("P4_RGB_888"));
        m_CtrlFormatCombo.AddString(_T("P4_RGBA_4444"));
        m_CtrlFormatCombo.AddString(_T("P4_ARGB_4444"));
        m_CtrlFormatCombo.AddString(_T("P4_ARGB_1555"));
        m_CtrlFormatCombo.AddString(_T("P4_URGB_1555"));
        m_CtrlFormatCombo.AddString(_T("P4_RGB_565"));
    
        // Only PC and Xbox support DXT compression. Probably deprecated for A51.
        m_CtrlFormatCombo.AddString(_T("DXT1"));
        m_CtrlFormatCombo.AddString(_T("DXT2"));
        m_CtrlFormatCombo.AddString(_T("DXT3"));
        m_CtrlFormatCombo.AddString(_T("DXT4"));
        m_CtrlFormatCombo.AddString(_T("DXT5"));
        m_CtrlFormatCombo.AddString(_T("A8"));
    }
    
    // Set default selection
    if (platform == _T("PC") || platform == _T("Xbox") || platform == _T("Native"))
        m_CtrlFormatCombo.SelectString(-1, _T("32_ARGB_8888"));
    else if (platform == _T("PS2"))
        m_CtrlFormatCombo.SelectString(-1, _T("32_ABGR_8888"));
    else if (platform == _T("GameCube"))
        m_CtrlFormatCombo.SelectString(-1, _T("32_RGBA_8888"));
}