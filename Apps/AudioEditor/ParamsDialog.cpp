//=========================================================================
// PARAMSDIALOG.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "Resource.h"
#include "ParamsDialog.h"
#include "AudioEditor.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================

CParamsDialog::CParamsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CParamsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CParamsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=========================================================================

void CParamsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CParamsDialog)
	DDX_Control(pDX, IDC_EDIT_VOLUME,       m_Volume        );
    DDX_Control(pDX, IDC_EDIT_VOLUME_VAR,   m_VolumeVar     );
    DDX_Control(pDX, IDC_EDIT_PITCH,        m_Pitch         );
    DDX_Control(pDX, IDC_EDIT_PITCH_VAR,    m_PitchVar      );
    DDX_Control(pDX, IDC_EDIT_PAN,          m_Pan           );
    DDX_Control(pDX, IDC_EDIT_PRIORITY,     m_Priority      );
    DDX_Control(pDX, IDC_EDIT_EFFECT_SEND,  m_EffectSend    );
    DDX_Control(pDX, IDC_EDIT_NEAR_FALLOFF, m_NearFalloff   );
    DDX_Control(pDX, IDC_EDIT_FAR_FALLOFF,  m_FarFalloff    );
	//}}AFX_DATA_MAP
}

//=========================================================================

BEGIN_MESSAGE_MAP(CParamsDialog, CDialog)
	//{{AFX_MSG_MAP(CParamsDialog)
        

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

BOOL CParamsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_Volume.SetWindowText      ( (const char*)xfs("%g", g_DescVolume       ) );
    m_VolumeVar.SetWindowText   ( (const char*)xfs("%g", g_DescVolumeVar    ) );
    m_Pitch.SetWindowText       ( (const char*)xfs("%g", g_DescPitch        ) );
    m_PitchVar.SetWindowText    ( (const char*)xfs("%g", g_DescPitchVar     ) );
    m_Pan.SetWindowText         ( (const char*)xfs("%g", g_DescPan          ) );
    m_Priority.SetWindowText    ( (const char*)xfs("%d", g_DescPriority     ) );
    m_EffectSend.SetWindowText  ( (const char*)xfs("%g", g_DescEffectSend   ) );
    m_NearFalloff.SetWindowText ( (const char*)xfs("%g", g_DescNearFalloff  ) );
    m_FarFalloff.SetWindowText  ( (const char*)xfs("%g", g_DescFarFalloff   ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CParamsDialog::OnOK( )
{
    x_try;

    CString String;
   
    m_Volume.GetWindowText( String );
    g_DescVolume = x_atof( (LPCTSTR)String );

    if( ((g_DescVolume < 0.0f) || (g_DescVolume > 1.0f)) && (g_DescVolume != -2.0f) )
    {
        g_DescVolume = -2.0f;
        x_throw( "Volume parameter out of range 0 -> 1." );
    }

    m_VolumeVar.GetWindowText( String );
    g_DescVolumeVar = x_atof( (LPCTSTR)String );

    if( ((g_DescVolumeVar < 0.0f) || (g_DescVolumeVar > 1.0f)) && (g_DescVolumeVar != -2.0f) )
    {
        g_DescVolumeVar = -2.0f;
        x_throw( "Volume Var parameter out of range 0 -> 1." );
    }

    m_Pitch.GetWindowText( String );
    g_DescPitch = x_atof( (LPCTSTR)String );

    if( ((g_DescPitch < 0.015625f) || (g_DescPitch > 4.0f)) && (g_DescPitch != -2.0f) )
    {
        g_DescPitch = -2.0f;
        x_throw( "Pitch parameter out of range 0.015625 -> 4." );
    }

    m_PitchVar.GetWindowText( String );
    g_DescPitchVar = x_atof( (LPCTSTR)String );

    if( ((g_DescPitchVar < 0.0f) || (g_DescPitchVar > 1.0f)) && (g_DescPitchVar != -2.0f) )
    {
        g_DescPitchVar = -2.0f;
        x_throw( "Pitch Var parameter out of range 0 -> 1." );
    }

    m_Pan.GetWindowText( String );
    g_DescPan = x_atof( (LPCTSTR)String );

    if( ((g_DescPan < -1.0f) || (g_DescPan > 1.0f)) && (g_DescPan != -2.0f) )
    {
        g_DescPan = -2.0f;
        x_throw( "Pan Var parameter out of range -1 -> 1." );
    }

    m_Priority.GetWindowText( String );
    g_DescPriority = x_atoi( (LPCTSTR)String );

    if( ((g_DescPriority < 0) || (g_DescPriority > 255)) && (g_DescPriority != -2) )
    {
        g_DescPriority = -2;
        x_throw( "Priority Var parameter out of range 0 -> 255." );
    }

    m_EffectSend.GetWindowText( String );
    g_DescEffectSend = x_atof( (LPCTSTR)String );

    if( ((g_DescEffectSend < 0.0f) || (g_DescEffectSend > 1.0f)) && (g_DescEffectSend != -2.0f) )
    {
        g_DescEffectSend = -2.0f;
        x_throw( "EffectSend Var parameter out of range 0 -> 1." );
    }

    m_NearFalloff.GetWindowText( String );
    g_DescNearFalloff = x_atof( (LPCTSTR)String );

    if( ((g_DescNearFalloff < 0.0f) || (g_DescNearFalloff > 2.0f)) && (g_DescNearFalloff != -2.0f) )
    {
        g_DescNearFalloff = -2.0f;
        x_throw( "NearFalloff Var parameter out of range 0 -> 1." );
    }

    m_FarFalloff.GetWindowText( String );
    g_DescFarFalloff = x_atof( (LPCTSTR)String );

    if( ((g_DescFarFalloff < 0.0f) || (g_DescFarFalloff > 2.0f)) && (g_DescFarFalloff != -2.0f) )
    {
        g_DescFarFalloff = -2.0f;
        x_throw( "FarFalloff Var parameter out of range 0 -> 1." );
    }
    
    x_catch_display;

	CDialog::OnOK();
}

//=========================================================================