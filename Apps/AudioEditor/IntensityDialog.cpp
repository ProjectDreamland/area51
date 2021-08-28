// IntensityDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "IntensityDialog.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\AudioEditor\AudioEditor.hpp"
#include "..\Editor\Project.hpp"
#include "SoundDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

xarray< xstring > g_DescAdded;

//=========================================================================

CIntensityDialog::CIntensityDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CIntensityDialog::IDD, pParent)
{
    m_Desc.Clear();
    m_Desc.SetGrowAmount( 16 );
    //{{AFX_DATA_INIT(CIntensityDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=========================================================================

void CIntensityDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CIntensityDialog)

    DDX_Control(pDX, IDC_INTENSITY_LIST,     m_DescListBox);
	//}}AFX_DATA_MAP
}

//=========================================================================

BEGIN_MESSAGE_MAP(CIntensityDialog, CDialog)
	//{{AFX_MSG_MAP(CIntensityDialog)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

BOOL CIntensityDialog::OnInitDialog() 
{
    x_try;

	CDialog::OnInitDialog();
    
    for( s32 i = 0; i < m_Desc.GetCount(); i++ )
        m_DescListBox.AddString( (const char *)m_Desc[i] );

    x_catch_display;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CIntensityDialog::InsertDescpritors( audio_editor& AudioEditor )
{
    x_try;

    xhandle Package     = AudioEditor.m_PackageSelected;
    s32 IntensitySel    = AudioEditor.m_pDesc( Package ).m_IntensitySelected;
    
    
    if( IntensitySel == -1 )
        x_throw( "Please select an intensity item." );

    editor_intensity& Intensity  = AudioEditor.m_pDesc( Package ).m_pIntensity[ IntensitySel ];
    s32 i = 0;

    for( i = 0; i < AudioEditor.m_pDesc( Package ).m_pDescriptorList.GetCount(); i++ )
    {
        editor_descriptor& rDesc =  AudioEditor.m_pDesc( Package ).m_pDescriptorList[i];

        for( s32 j = 0; j < Intensity.m_pDescriptors.GetCount(); j++ )
        {
            if( !x_strcmp( Intensity.m_pDescriptors[j], rDesc.m_Label ) )
                break;
        }
        
        if( j == Intensity.m_pDescriptors.GetCount() )
        {
            xstring String( rDesc.m_Label );
            m_Desc.Append( String );
        }       
    }
    
    x_catch_display;
}

//=========================================================================

void CIntensityDialog::OnOK( )
{
    
    s32 i = 0;

    // Get the indexes of all the selected items.
    int nCount = m_DescListBox.GetSelCount();
    CArray<int,int> aryListBoxSel;

    aryListBoxSel.SetSize(nCount);
    m_DescListBox.GetSelItems(nCount, aryListBoxSel.GetData());

    g_DescAdded.Clear();
            
    for( i = 0; i < aryListBoxSel.GetSize(); i++ )
    {
        char DescriptorName[128];
        m_DescListBox.GetText( aryListBoxSel[i], DescriptorName );
        xstring String( DescriptorName );
        g_DescAdded.Append( String );
    }

    CDialog::OnOK();
}

//=========================================================================