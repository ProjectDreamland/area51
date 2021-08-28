//=========================================================================
// ELEMENTPARAMSDIALOG.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "Resource.h"
#include "ElementParamsDialog.h"
#include "AudioEditor.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================

CElementParamsDialog::CElementParamsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CElementParamsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CElementParamsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=========================================================================

void CElementParamsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CElementParamsDialog)
	DDX_Control(pDX, IDC_ELT_EDIT_VOLUME,       m_Volume        );
    DDX_Control(pDX, IDC_ELT_EDIT_VOLUME_VAR,   m_VolumeVar     );
    DDX_Control(pDX, IDC_ELT_EDIT_PITCH,        m_Pitch         );
    DDX_Control(pDX, IDC_ELT_EDIT_PITCH_VAR,    m_PitchVar      );
    DDX_Control(pDX, IDC_ELT_EDIT_PAN,          m_Pan           );
    DDX_Control(pDX, IDC_ELT_EDIT_PRIORITY,     m_Priority      );
    DDX_Control(pDX, IDC_ELT_EDIT_EFFECT_SEND,  m_EffectSend    );
    DDX_Control(pDX, IDC_ELT_EDIT_NEAR_FALLOFF, m_NearFalloff   );
    DDX_Control(pDX, IDC_ELT_EDIT_FAR_FALLOFF,  m_FarFalloff    );
    DDX_Control(pDX, IDC_TEMPERATURE_LIST,      m_Temperature   );
	//}}AFX_DATA_MAP
}

//=========================================================================

BEGIN_MESSAGE_MAP(CElementParamsDialog, CDialog)
	//{{AFX_MSG_MAP(CElementParamsDialog)
        

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

BOOL CElementParamsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_Volume.SetWindowText      ( (const char*)xfs("%g", g_ElementVolume       ) );
    m_VolumeVar.SetWindowText   ( (const char*)xfs("%g", g_ElementVolumeVar    ) );
    m_Pitch.SetWindowText       ( (const char*)xfs("%g", g_ElementPitch        ) );
    m_PitchVar.SetWindowText    ( (const char*)xfs("%g", g_ElementPitchVar     ) );
    m_Pan.SetWindowText         ( (const char*)xfs("%g", g_ElementPan          ) );
    m_Priority.SetWindowText    ( (const char*)xfs("%d", g_ElementPriority     ) );
    m_EffectSend.SetWindowText  ( (const char*)xfs("%g", g_ElementEffectSend   ) );
    m_NearFalloff.SetWindowText ( (const char*)xfs("%g", g_ElementNearFalloff  ) );
    m_FarFalloff.SetWindowText  ( (const char*)xfs("%g", g_ElementFarFalloff   ) );

    s32 HotIndex = m_Temperature.AddString( "HOT" );
    m_Temperature.SetItemData( HotIndex, (DWORD)editor_element::HOT );

    s32 WarmIndex = m_Temperature.AddString( "WARM" );
    m_Temperature.SetItemData( WarmIndex, (DWORD)editor_element::WARM );

    s32 ColdIndex = m_Temperature.AddString( "COLD" );
    m_Temperature.SetItemData( ColdIndex, (DWORD)editor_element::COLD );

    switch( g_ElementTemperature )
    {
        case editor_element::HOT:  m_Temperature.SetCurSel( HotIndex  ); break;
        case editor_element::WARM: m_Temperature.SetCurSel( WarmIndex ); break;
        case editor_element::COLD: m_Temperature.SetCurSel( ColdIndex ); break;
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CElementParamsDialog::OnOK( )
{
    x_try;

    CString String;
   
    m_Volume.GetWindowText( String );
    g_ElementVolume = x_atof( (LPCTSTR)String );

    if( ((g_ElementVolume < 0.0f) || (g_ElementVolume > 1.0f)) && (g_ElementVolume != -2.0f) )
    {
        g_ElementVolume = -2.0f;
        x_throw( "Volume parameter out of range 0 -> 1." );
    }

    m_VolumeVar.GetWindowText( String );
    g_ElementVolumeVar = x_atof( (LPCTSTR)String );

    if( ((g_ElementVolumeVar < 0.0f) || (g_ElementVolumeVar > 1.0f)) && (g_ElementVolumeVar != -2.0f) )
    {
        g_ElementVolumeVar = -2.0f;
        x_throw( "Volume Var parameter out of range 0 -> 1." );
    }

    m_Pitch.GetWindowText( String );
    g_ElementPitch = x_atof( (LPCTSTR)String );

    if( ((g_ElementPitch < 0.015625f) || (g_ElementPitch > 4.0f)) && (g_ElementPitch != -2.0f) )
    {
        g_ElementPitch = -2.0f;
        x_throw( "Pitch parameter out of range 0.015625 -> 4." );
    }

    m_PitchVar.GetWindowText( String );
    g_ElementPitchVar = x_atof( (LPCTSTR)String );

    if( ((g_ElementPitchVar < 0.0f) || (g_ElementPitchVar > 1.0f)) && (g_ElementPitchVar != -2.0f) )
    {
        g_ElementPitchVar = -2.0f;
        x_throw( "Pitch Var parameter out of range 0 -> 1." );
    }

    m_Pan.GetWindowText( String );
    g_ElementPan = x_atof( (LPCTSTR)String );

    if( ((g_ElementPan < -1.0f) || (g_ElementPan > 1.0f)) && (g_ElementPan != -2.0f) )
    {
        g_ElementPan = -2.0f;
        x_throw( "Pan Var parameter out of range -1 -> 1." );
    }

    m_Priority.GetWindowText( String );
    g_ElementPriority = x_atoi( (LPCTSTR)String );

    if( ((g_ElementPriority < 0) || (g_ElementPriority > 255)) && (g_ElementPriority != -2) )
    {
        g_ElementPriority = -2;
        x_throw( "Priority Var parameter out of range 0 -> 255." );
    }

    m_EffectSend.GetWindowText( String );
    g_ElementEffectSend = x_atof( (LPCTSTR)String );

    if( ((g_ElementEffectSend < 0.0f) || (g_ElementEffectSend > 1.0f)) && (g_ElementEffectSend != -2.0f) )
    {
        g_ElementEffectSend = -2.0f;
        x_throw( "EffectSend Var parameter out of range 0 -> 1." );
    }

    m_NearFalloff.GetWindowText( String );
    g_ElementNearFalloff = x_atof( (LPCTSTR)String );

    if( ((g_ElementNearFalloff < 0.0f) || (g_ElementNearFalloff > 2.0f)) && (g_ElementNearFalloff != -2.0f) )
    {
        g_ElementNearFalloff = -2.0f;
        x_throw( "NearFalloff Var parameter out of range 0 -> 1." );
    }

    m_FarFalloff.GetWindowText( String );
    g_ElementFarFalloff = x_atof( (LPCTSTR)String );

    if( ((g_ElementFarFalloff < 0.0f) || (g_ElementFarFalloff > 2.0f)) && (g_ElementFarFalloff != -2.0f) )
    {
        g_ElementFarFalloff = -2.0f;
        x_throw( "FarFalloff Var parameter out of range 0 -> 1." );
    }
    
    s32 Index = m_Temperature.GetCurSel();
    g_ElementTemperature = m_Temperature.GetItemData( Index );

    if( (g_ElementTemperature < editor_element::HOT) && (g_ElementTemperature > editor_element::COLD) )
    {
        g_ElementTemperature = editor_element::HOT;
        x_throw( "Wrong data returned from the Temperature listbox" );
    }
   
    x_catch_display;
	
    CDialog::OnOK();
}

//=========================================================================