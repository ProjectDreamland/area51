// PartEdDoc.cpp : implementation of the CPartEdDoc class
//

#include "stdafx.h"
#include "PartEd.h"

#include "ParticleView3D.h"
#include "PartEdDoc.h"
#include "ManipulatorMgr.h"
#include "ManipTranslate.h"
#include "ManipScale.h"
#include "x_bytestream.hpp"
#include "MainFrm.h"
#include "Auxiliary/fx_core/export.hpp"
#include "Auxiliary/fx_core/TextureMgr.hpp"
#include "ErrorDialog.h"
#include "Auxiliary/fx_core/igfmgr.hpp"
#include "AVIGenerator.h"
#include "AviOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMainFrame*      g_pMainFrame;
extern CProperties*     g_pProperties;

/////////////////////////////////////////////////////////////////////////////
// CPartEdDoc

IMPLEMENT_DYNCREATE(CPartEdDoc, CDocument)

BEGIN_MESSAGE_MAP(CPartEdDoc, CDocument)
	//{{AFX_MSG_MAP(CPartEdDoc)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_EXPORTVIDEOAVI, OnFileExportvideoavi)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPartEdDoc construction/destruction

CPartEdDoc::CPartEdDoc()
{
    m_Animate = FALSE;
}

CPartEdDoc::~CPartEdDoc()
{
}

BOOL CPartEdDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    m_Effect.Destroy();
    m_SelSet.RemoveAll();
    UpdateAllViews(NULL);

    PopulatePropertyControl();
    UpdateKeyBar();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Save
BOOL CPartEdDoc::OnSaveDocument( LPCTSTR lpszPathName )
{

    igfmgr      Igf;
    
    //SetTitle( lpszPathName );
    //g_pMainFrame->SetTitle( lpszPathName );
    //SetPathName( lpszPathName );
    g_pMainFrame->SetWindowText( lpszPathName );

    // fill out the header
    Igf.SetHeaderInfo( "FX Editor Document", "Tools", 1.0f );

    Igf.AddComment( "Effects Data File" );

    hfield StatsGrp = Igf.AddGroup( "Stats", "Document-specific data" );
    Igf.EnterGroup( StatsGrp );
    {   
        s32 MinT, MaxT;
        g_pMainFrame->GetMinMaxTime( MinT, MaxT );
        Igf.AddS32( "Min_T", MinT, "Time range minimum" );
        Igf.AddS32( "Max_T", MaxT, "Time range maximum" );
        Igf.ExitGroup();
    }

    // Write the viewport configuration
    s32 ViewCount = 0;

    POSITION Pos = GetFirstViewPosition();
    while ( Pos )
    {
        ViewCount++;
        GetNextView( Pos );
    }

    // Save the viewport configuration
    hfield NewGrp = Igf.AddGroup( "Viewports", "Viewport settings" );
    Igf.EnterGroup( NewGrp );
    {
        // write the number of viewports
        Igf.AddS32( "Count", ViewCount, "Number of viewports" );

        Pos = GetFirstViewPosition();
        while ( Pos )
        {
            vector3     CamPos;
            quaternion  Q;
            hfield      CamGrp;

            CamGrp = Igf.AddGroup( "Camera", "Individual camera settings" );
            Igf.EnterGroup( CamGrp );
            {
                // Get the camera info for this view
                CParticleView3D* pView = (CParticleView3D*)GetNextView( Pos );
                pView->GetCameraInfo( CamPos, Q );

                Igf.AddF32( "X", CamPos.GetX() );
                Igf.AddF32( "Y", CamPos.GetY() );
                Igf.AddF32( "Z", CamPos.GetZ() );
                Igf.AddF32( "Q1", Q.X );
                Igf.AddF32( "Q2", Q.Y );
                Igf.AddF32( "Q3", Q.Z );
                Igf.AddF32( "Q4", Q.W );

                Igf.ExitGroup();
            }
        }

        Igf.ExitGroup();
    }

    // now save the rest
    m_Effect.Save( Igf );

    // save text mode version
    Igf.Save( lpszPathName );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Load
BOOL CPartEdDoc::OnOpenDocument( LPCTSTR lpszPathName )
{
    igfmgr      Igf;

    m_Effect.Destroy();
    m_SelSet.RemoveAll();
    
    // load it all
    Igf.Load( lpszPathName );

    // now dissect it

    hfield Stats = Igf.GetGroup( "Stats" );

    // Get Time Values
    s32 TimeMin, TimeMax;

    if ( Stats )
    {
        Igf.EnterGroup( Stats );
        {
            TimeMin = Igf.GetS32( "Min_T" );
            TimeMax = Igf.GetS32( "Max_T" );
            Igf.ExitGroup();
        }
    }
    else
    {
        TimeMin = 0;
        TimeMax = 60;
    }

    // Set Time Values
    g_pMainFrame->SetMinMaxTime( TimeMin, TimeMax );
    g_pMainFrame->SetGlobalTime( TimeMin );

    // Read the viewport configuration
    hfield NewGrp = Igf.GetGroup( "Viewports" );
    Igf.EnterGroup( NewGrp );
    {
        vector3     CamPos[4];
        quaternion  Q[4];
        s32         i;
        s32         ViewCount = Igf.GetS32( "Count" );
        hfield      CamGrp;

        CamGrp =    Igf.Find( "Camera" );

        POSITION Pos = GetFirstViewPosition();

        for ( i = 0; i < ViewCount; i++ )
        {
            Igf.EnterGroup( CamGrp );
            {
                // Get the camera info for this view
                //CParticleView3D* pView = (CParticleView3D*)GetNextView( Pos );
                //pView->GetCameraInfo( CamPos, Q );

                CamPos[i].GetX() = Igf.GetF32( "X"  );
                CamPos[i].GetY() = Igf.GetF32( "Y"  );
                CamPos[i].GetZ() = Igf.GetF32( "Z"  );
                Q[i].X = Igf.GetF32( "Q1" );
                Q[i].Y = Igf.GetF32( "Q2" );
                Q[i].Z = Igf.GetF32( "Q3" );
                Q[i].W = Igf.GetF32( "Q4" );

                CParticleView3D* pView = (CParticleView3D*)GetNextView( Pos );        
                pView->SetCameraInfo( CamPos[i], Q[i] );

                Igf.ExitGroup();
            }
            CamGrp = Igf.Next();
        }

        Igf.ExitGroup();
    }

    // now load the rest of the stuff
    m_Effect.Load( Igf );

    // Update the UI Controls
    PopulatePropertyControl();
    UpdateKeyBar();

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CPartEdDoc::PopulatePropertyControl( void )
{
    xstring             Name, Value;
    xcolor              UIColor;
    CString             CName, CValue;
    fx_core::base::prop_type     Type;
    s32                 Idx;
    char                ID[64];
    xstring             Str;
    CString             CStr;                        
    xbool               Done = FALSE;
    xbool               IsDisabled;
    
    CProperties*        pProps = g_pProperties;
    
    // Exit if property grid isn't created yet
    if( !pProps->m_PropertyGrid )
        return;

    pProps->m_PropertyGrid.SetRedraw( FALSE );

    // reset the list
    pProps->EraseAll();

    // bunch of duplicated code...don't mind for now because the property control
    // is going to get the AXE!

    pProps->AddGridDataElement( "EFFECT", "", "The Effect", CGridItemInfo::GCT_STRING_EDIT, RGB(192,192,192), 0, FALSE, FALSE, TRUE );

    if( m_SelSet.GetCount() == 0 )
    {
        Idx = 0;
        do 
        {
            if ( m_Effect.GetProperty( Idx, (s32)g_pMainFrame->GetGlobalTime(), Name, Value, IsDisabled, Type ) == TRUE )
            {
                fx_core::effect* pEffect = &m_Effect;

                CName.Format( "EFFECT\\%s", Name );
                CValue.Format( "%s", Value );

                switch ( Type )
                {
                    case fx_core::element::PROP_BOOL:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_BOOL, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_INT:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_NUMERIC_EDIT, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_FLOAT:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_FLOAT_EDIT, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_STRING:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_STRING_EDIT, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_V3:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_3D_COORDINATE, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_R3:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_ROTATION_EDIT, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_COLOR:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COLOR_BUTTON, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_FILENAME:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_DIR_BUTTON, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_LOOPMODE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                            if( pItem )
                            {
                                CStringList List;
                                List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::CLAMP     ) );
                                List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::LOOP      ) );
                                List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::PINGPONG  ) );
                                pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    case fx_core::element::PROP_CONTROLLERTYPE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                            if( pItem )
                            {
                                CStringList List;
                                List.AddTail( fx_core::base::ControllerType_ToString( fx_core::base::CONTROLLERTYPE_LINEAR ) );
                                List.AddTail( fx_core::base::ControllerType_ToString( fx_core::base::CONTROLLERTYPE_SMOOTH ) );
                                pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    case fx_core::element::PROP_COMBINEMODE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, RGB(192,192,192), (int)pEffect, FALSE, FALSE, FALSE );
                            if( pItem )
                            {
                                CStringList List;
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_ALPHA       ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_ADDITIVE    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_SUBTRACTIVE ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_ALPHA  ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_ADD    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_SUB    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_DISTORT     ) );
                                pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    default:
                        ASSERT( FALSE );
                        break;
                }
            
                Idx++;
            }
            else
                Done = TRUE;
        
        } while(!Done);
    }

    // reset
    Done = FALSE;

    pProps->AddGridDataElement( "EFFECT\\Elements", "", "Effect Elements", CGridItemInfo::GCT_STRING_EDIT, RGB(119, 128, 144), 0, FALSE, FALSE, TRUE );

    POSITION Pos = m_SelSet.GetHeadPosition();
    while( Pos )
    {
        fx_core::element*    pElement = m_SelSet.GetAt( Pos );
        
        pElement->GetElementID( ID );

        Str.Format( "EFFECT\\Elements\\%s", ID );
        
        CStr.Format( "%s", Str );
        
        pProps->AddGridDataElement( CStr, pElement->GetType(), "", CGridItemInfo::GCT_NULL_ENTRY, RGB(144,160,176), (int)this, FALSE, FALSE, FALSE );

        // reset Idx
        Idx = 0;

        do
        {
            if ( pElement->GetProperty( Idx, (s32)g_pMainFrame->GetGlobalTime(), UIColor, Name, Value, IsDisabled, Type ) == TRUE )
            {
                CName.Format( "%s\\%s", (const char*)Str, Name );
                CValue.Format( "%s", Value );

                COLORREF    uiFieldColor    = RGB( UIColor.R, UIColor.G, UIColor.B );
                CStringList     DropList;

                switch ( Type )
                {
                    case fx_core::element::PROP_BOOL:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_BOOL, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_INT:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_NUMERIC_EDIT, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_FLOAT:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_FLOAT_EDIT, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_STRING:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_STRING_EDIT, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_V3:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_3D_COORDINATE, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_R3:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_ROTATION_EDIT, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_COLOR:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COLOR_BUTTON, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_FILENAME:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_DIR_BUTTON, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_HEADER:
                        pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_NULL_ENTRY, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                        break;
                    case fx_core::element::PROP_LOOPMODE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                            if( pItem )
                            {
                            CStringList List;
                            List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::CLAMP     ) );
                            List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::LOOP      ) );
                            List.AddTail( fx_core::controller::OutOfRangeType_ToString( fx_core::controller::PINGPONG  ) );
                            pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    case fx_core::element::PROP_CONTROLLERTYPE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                            if( pItem )
                            {
                                CStringList List;
                                List.AddTail( fx_core::base::ControllerType_ToString( fx_core::base::CONTROLLERTYPE_LINEAR ) );
                                List.AddTail( fx_core::base::ControllerType_ToString( fx_core::base::CONTROLLERTYPE_SMOOTH ) );
                                pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    case fx_core::element::PROP_COMBINEMODE:
                        {
                            CGridItemInfo* pItem = pProps->AddGridDataElement( CName, CValue, "", CGridItemInfo::GCT_COMBOBOX, uiFieldColor, (int)pElement, IsDisabled, FALSE, FALSE );
                            if( pItem )
                            {
                                CStringList List;
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_ALPHA       ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_ADDITIVE    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_SUBTRACTIVE ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_ALPHA  ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_ADD    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_GLOW_SUB    ) );
                                List.AddTail( fx_core::base::CombineMode_ToString( fx_core::base::COMBINEMODE_DISTORT     ) );
                                pItem->SetListData( 0, &List );
                            }
                        }
                        break;
                    default:
                        ASSERT( FALSE );
                        break;
                }
                
                Idx++;
            }
            else
                Done = TRUE;
        
        } while (!Done);

        // pElement->ShowProperties( g_pMainFrame->GetGlobalTime(), g_pProperties );
        m_SelSet.GetNext( Pos );
    }

    g_pProperties->ExpandAll();

    pProps->m_PropertyGrid.SetRedraw( TRUE );
}

/////////////////////////////////////////////////////////////////////////////

void CPartEdDoc::UpdateKeyBar( void )
{
    POSITION            Pos     = GetFirstViewPosition();
    CParticleView3D*    pView   = (CParticleView3D*)GetNextView( Pos );

    pView->UpdateKeyBar();
}

/////////////////////////////////////////////////////////////////////////////
// CPartEdDoc serialization

void CPartEdDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPartEdDoc diagnostics

#ifdef _DEBUG
void CPartEdDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPartEdDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPartEdDoc commands

void CPartEdDoc::OnCloseDocument() 
{
	CDocument::OnCloseDocument();
}

void CPartEdDoc::OnFileExport() 
{
/*
    char Ext[] = "FX Object Files (*.fxo)|*.fxo|All Files (*.*)|*.*||";

	CFileDialog Dlg( FALSE, ".FXO", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Ext );    

    if ( Dlg.DoModal() == IDOK )
    {
        export Export;

        Export.ConstructData( &m_Effect );
        Export.SaveData( Dlg.GetPathName() );
    }
*/

    // Clear the log of errors
    g_ErrorLog.Clear();

    // Export to PC
    if( !m_Effect.m_ExportPC.IsEmpty() )
    {
        fx_core::export Export;
        Export.ConstructData( &m_Effect, fx_core::EXPORT_TARGET_PC );
        Export.SaveData( (const char*)m_Effect.m_ExportPC, fx_core::EXPORT_TARGET_PC );
    }

    // Export to PS2
    if( !m_Effect.m_ExportPS2.IsEmpty() )
    {
        fx_core::export Export;
        Export.ConstructData( &m_Effect, fx_core::EXPORT_TARGET_PS2 );
        Export.SaveData( (const char*)m_Effect.m_ExportPS2, fx_core::EXPORT_TARGET_PS2 );
    }

    // Export to GCN
    if( !m_Effect.m_ExportGCN.IsEmpty() )
    {
        fx_core::export Export;
        Export.ConstructData( &m_Effect, fx_core::EXPORT_TARGET_GCN );
        Export.SaveData( (const char*)m_Effect.m_ExportGCN, fx_core::EXPORT_TARGET_GCN );
    }

    // Export to XBOX
    if( !m_Effect.m_ExportXBOX.IsEmpty() )
    {
        fx_core::export Export;
        Export.ConstructData( &m_Effect, fx_core::EXPORT_TARGET_XBOX );
        Export.SaveData( (const char*)m_Effect.m_ExportXBOX, fx_core::EXPORT_TARGET_XBOX );
    } 

    // Check for errors
    if( g_ErrorLog.GetCount() > 0 )
    {
        CErrorDialog Dialog;
        Dialog.SetErrorLog( &g_ErrorLog );

        Dialog.DoModal();
    }
}
 
//---------------------------------------------------------------------------

void CPartEdDoc::SelectAll( void )
{
    m_SelSet.RemoveAll();

    for( s32 i=0 ; i<m_Effect.GetNumElements() ; i++ )
    {
        fx_core::element* pElement = m_Effect.GetElement( i );
        m_SelSet.AddTail( pElement );
    }

    UpdateAllViews(NULL);
    PopulatePropertyControl();
    UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CPartEdDoc::SelectNone( void )
{
    m_SelSet.RemoveAll();
    UpdateAllViews(NULL);
    PopulatePropertyControl();
    UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CPartEdDoc::SelectInvert( void )
{
    CList<fx_core::element*, fx_core::element*>   OldSel;
    OldSel.AddTail( &m_SelSet );
    m_SelSet.RemoveAll();

    for( s32 i=0 ; i<m_Effect.GetNumElements() ; i++ )
    {
        fx_core::element* pElement = m_Effect.GetElement( i );
        if( !OldSel.Find( pElement ) )
            m_SelSet.AddTail( pElement );
    }

    UpdateAllViews(NULL);
    PopulatePropertyControl();
    UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CPartEdDoc::DeleteContents() 
{
    m_Effect.Destroy();
    
	CDocument::DeleteContents();
}

//---------------------------------------------------------------------------

void d3deng_BuildXBitmapFromScreen( xbitmap& Dst, s32 XRes, s32 YRes );

//---------------------------------------------------------------------------

void CPartEdDoc::OnFileExportvideoavi() 
{
    CAviOptions Options;

    Options.SetRange( g_pMainFrame->m_KeyBar.GetTimeRangeStart(), g_pMainFrame->m_KeyBar.GetTimeRangeEnd() );

    // If we don't cancel
    if ( Options.DoModal() == IDOK )
    {
	    // TODO: Add your command handler code here
        POSITION Pos = GetFirstViewPosition();

        CView*  pView;
        CView*  pViewTmp;
        s32     Idx = 0;
        s32     MaxArea = 0;
    
        // Find the view they have selected
        // MAXIMIZED?
        if ( Options.GetSelectedView() == CAviOptions::VIEW_MAX )
        {
            while ( pViewTmp = GetNextView( Pos ) )
            {
                RECT Rect;
                pViewTmp->GetClientRect( &Rect );

                s32 Area = Rect.right * Rect.bottom;

                if ( Area > MaxArea )
                {
                    MaxArea = Area;
                    pView = pViewTmp;
                }
            }    
        }
        else
        {
            s32 Count = (s32)Options.GetSelectedView();

            while ( pViewTmp = GetNextView( Pos ) )
            {
                if ( (Idx++) == Count )
                {
                    pView = pViewTmp;
                    break;
                }
            }
        }

        rect Rect;
        CRect dims;

        pView->GetClientRect( &dims );
        Rect.Set( 0.0f, 0.0f, (f32)dims.right, (f32)dims.bottom );

        // Create and fill out a bitmap info header
        BITMAPINFOHEADER bminfo;
        bminfo.biBitCount = 32;
        bminfo.biClrImportant = 0;
        bminfo.biClrUsed = 0;
        bminfo.biCompression = BI_RGB;

        switch( Options.GetSelectedRes() )
        {
            case CAviOptions::RES_160:
                bminfo.biWidth = 160;
                bminfo.biHeight = 120;
                break;

            case CAviOptions::RES_320:
                bminfo.biWidth = 320;
                bminfo.biHeight = 240;
                break;
            
            case CAviOptions::RES_640:
                bminfo.biWidth = 640;
                bminfo.biHeight = 480;
                break;

            default:
                ASSERT( FALSE );
        }

        bminfo.biPlanes = 1;
        bminfo.biSize = sizeof(BITMAPINFOHEADER);
        bminfo.biSizeImage = 0;
        bminfo.biXPelsPerMeter = 1;
        bminfo.biYPelsPerMeter = 1;

        CAVIGenerator Avi( Options.GetFileName(), &bminfo, 30 );

        // temporarily muck with the render resolution
        d3deng_SetResolution( bminfo.biWidth, bminfo.biHeight );


        if ( Avi.InitEngine() != AVIERR_OK )
        {
            MessageBox( (HWND)NULL, "Unable to render the movie!", "Error!", MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL );
        }
        else
        {
            xbitmap Dst;

            s32 Start, End;

            Options.GetRange( Start, End );

            // Render out each frame
            for ( s32 i = Start; i <= End; i++ )
            {
                g_pMainFrame->SetGlobalTime(i);
                pView->RedrawWindow();
                d3deng_BuildXBitmapFromScreen( Dst, bminfo.biWidth, bminfo.biHeight );
                // Dst.Resize( bminfo.biWidth, bminfo.biHeight );
                //Dst.FlipVertically();
                Avi.AddFrame( (u8*)Dst.GetPixelData() );
            }
        }

        // fix back the view
        RECT Window;
        ::GetWindowRect( ::GetDesktopWindow(), &Window );
        d3deng_SetResolution( Window.right - Window.left, Window.bottom - Window.top );
        pView->RedrawWindow();

        Avi.ReleaseEngine();
    }
}


void d3deng_BuildXBitmapFromScreen( xbitmap& Dst, s32 XRes, s32 YRes )
{

    IDirect3DSurface9*      pBackBuffer = NULL;
    IDirect3DTexture9*      pBlitTarget = NULL;
    IDirect3DSurface9*      pDestSurf   = NULL;

    g_pd3dDevice->GetRenderTarget( 0, &pBackBuffer );

    g_pd3dDevice->CreateTexture( XRes, YRes, 1,
                                 0,
                                 D3DFMT_X8R8G8B8,
                                 D3DPOOL_MANAGED,
                                 &pBlitTarget,
                                 NULL );

    pBlitTarget->GetSurfaceLevel( 0, &pDestSurf );

    g_pd3dDevice->GetRenderTargetData( pBackBuffer, pDestSurf );

    pBackBuffer->Release();
    pDestSurf->Release();

    D3DLOCKED_RECT      LockedInfo;
    
    HRESULT Result = pBlitTarget->LockRect( 0, &LockedInfo, NULL, 0 );

    byte* pData = new byte[XRes*YRes*4];
    ASSERT(pData);

    x_memcpy(pData, LockedInfo.pBits, XRes*YRes*4 );

    pBlitTarget->UnlockRect( 0 );

    Dst.Setup( xbitmap::FMT_32_ARGB_8888, XRes, YRes, TRUE, pData );
    //Dst.SaveTGA( "ResizedScreenie.tga" );

}
