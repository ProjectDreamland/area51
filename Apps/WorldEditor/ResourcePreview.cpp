// ResourcePreview.cpp : implementation file
//

#include "stdafx.h"
#include "ResourcePreview.h"
#include "..\WinControls\FileSearch.h"
#include "..\EDRscDesc\RSCDesc.hpp"
#include "..\MeshViewer\RigidDesc.hpp"
#include "..\MeshViewer\SkinDesc.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CResourcePreview
//=========================================================================

CResourcePreview::CResourcePreview()
{
    m_v3Mult = vector3(1.2f,1.2f,1.2f);
}

//=========================================================================

CResourcePreview::~CResourcePreview()
{
}

//=========================================================================

BEGIN_MESSAGE_MAP(CResourcePreview, CWnd)
	//{{AFX_MSG_MAP(CResourcePreview)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CResourcePreview message handlers
//=========================================================================

int CResourcePreview::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    // Initialize the view to something resanable
    m_View.SetXFOV( R_60 );
    m_View.SetPosition( vector3(100,100,200) );
    m_View.LookAtPoint( vector3(  0,  0,  0) );
    m_View.SetZLimits ( 1, 50000 );

	return 0;
}

//=========================================================================

void CResourcePreview::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    d3deng_UpdateDisplayWindow( GetSafeHwnd() );

    eng_MaximizeViewport( m_View );
    eng_SetView         ( m_View );

    if( eng_Begin("Preview") )
    {
        m_Grid.Render();
        m_Axis.Render();
        m_Viewer.SetBackFacets( TRUE );
        m_Viewer.Render();
        eng_End();
    }

    eng_PageFlip();

//    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
//                         0x00000000, 1.0f, 0L ); 
}

//=========================================================================

BOOL CResourcePreview::LoadGeom(CString strType, CString strPath, BOOL bAdditional, const vector3& Pos, const radian3& Rot ) 
{
    BOOL bReturn = FALSE;

    text_in GeomFile;
    GeomFile.OpenFile(strPath);
    prop_query propQuery;
    char cMatxString[MAX_PATH];
    propQuery.RQueryFileName( "ResDesc\\FileName", &cMatxString[0]);

    if (strType.CompareNoCase("RigidGeom")==0)
    {
        rigidgeom_rsc_desc RscDesc;
        RscDesc.OnLoad(GeomFile);
        if (RscDesc.OnProperty(propQuery)) bReturn = TRUE;
    }
    else if (strType.CompareNoCase("SkinGeom")==0)
    {
        skingeom_rsc_desc RscDesc;
        RscDesc.OnLoad(GeomFile);
        if (RscDesc.OnProperty(propQuery)) bReturn = TRUE;
    }

    if (bReturn)
    {
        if(bAdditional)
        {
            m_Viewer.LoadAdditional(cMatxString, Pos, Rot);
        }
        else
        {
            m_Viewer.Load(cMatxString);
        }
        bbox Bounds = m_Viewer.GetBBox();
        f32 fMax = Bounds.GetRadius();
        m_View.SetPosition( fMax*m_v3Mult );

        f32 fMaxY = Bounds.Max.GetY() * 0.6f;
        if (bAdditional)
        {
            m_LastMaxY = MAX(fMaxY, m_LastMaxY);
        }
        else
        {
            m_LastMaxY = fMaxY;
        }
    }

    GeomFile.CloseFile();

    return bReturn;
}

//=========================================================================

void CResourcePreview::ClearGeom() 
{
    m_LastMaxY = 0;
    OnStopTimer();
    m_Viewer.CleanUp();
    RedrawWindow();
}

//=========================================================================

void CResourcePreview::OnTimer(UINT nIDEvent) 
{
    //Rotate
    {
        radian      Pitch;
        radian      Yaw;
        vector3     Dir      = m_View.GetPosition() - vector3(0,0,0);
        f32         Distance = Dir.Length();

        Dir.GetPitchYaw( Pitch, Yaw );

        Pitch -= 0 * 0.005f;
        Yaw   -= 20 * 0.005f;

        if( Pitch >  R_89 ) Pitch =  R_89;
        if( Pitch < -R_89 ) Pitch = -R_89;

        m_View.OrbitPoint( vector3(0,0,0), Distance, Pitch, Yaw );
        m_View.LookAtPoint( vector3(  0,  m_LastMaxY,  0) );
    }

	RedrawWindow();
	
	CWnd::OnTimer(nIDEvent);
}

//=========================================================================

void CResourcePreview::OnStartTimer() 
{
    m_nTimer = SetTimer(1, 100, 0);
}

//=========================================================================

void CResourcePreview::OnStopTimer() 
{
    KillTimer(m_nTimer);   
}
