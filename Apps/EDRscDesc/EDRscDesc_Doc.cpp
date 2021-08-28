// EDRscDesc_Doc.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "EDRscDesc_Doc.h"
#include "ReadPipe.h"
#include "CompErrorDisplayCtrl.h"
#include "EDRscDesc_Frame.h"
#include "EDRscDesc_View.h"
#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "MainFrm.h"

#include "../Editor/Project.hpp"
#include "../MeshViewer/RigidDesc.hpp"
#include "../MeshViewer/SkinDesc.hpp"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_Doc

IMPLEMENT_DYNCREATE(EDRscDesc_Doc, CBaseDocument)

BEGIN_MESSAGE_MAP(EDRscDesc_Doc, CBaseDocument)
	//{{AFX_MSG_MAP(EDRscDesc_Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VARS
/////////////////////////////////////////////////////////////////////////////

// Create the editor description
REG_EDITOR( s_RegRscEditor, "Resource Editor", "", IDR_RSC_DESC, EDRscDesc_Doc, EDRscDesc_Frame, EDRscDesc_View );
void LinkResourceEditor(void){}

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////


//=========================================================================

EDRscDesc_Doc::EDRscDesc_Doc() 
{
    m_bCompileNintendo = FALSE;
    m_bCompilePS2      = FALSE;     
    m_bCompileXBox     = FALSE;    
    m_bCompilePC       = FALSE;      
    m_bVerboseMode     = FALSE;
    m_bColorMipsMode   = FALSE;
}

//=========================================================================

BOOL EDRscDesc_Doc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

//=========================================================================
void EDRscDesc_Doc::OnProjectOpen( void )
{
    CONTEXT( "EDRscDesc_Doc::OnProjectOpen" );

    Refresh();
}

//=========================================================================
void EDRscDesc_Doc::OnProjectClose( void )
{
    Refresh();
}


//=========================================================================

EDRscDesc_Doc::~EDRscDesc_Doc()
{
}

//=========================================================================

#ifdef _DEBUG
void EDRscDesc_Doc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void EDRscDesc_Doc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void EDRscDesc_Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

//=========================================================================

void EDRscDesc_Doc::StopBuild( void )
{
    x_try;

    g_RescDescMGR.StopBuild();

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::ScanResources( void )
{
    g_RescDescMGR.ScanResources();
}

//=========================================================================

void EDRscDesc_Doc::Build( void )
{
    x_try;

    // Clean the temp folder
    g_Settings.CleanTemp();

    //
    // Force to save everything
    //
    for( s32 i=0; i<g_RescDescMGR.GetRscDescCount(); i++ )
    {
        rsc_desc_mgr::node& Node = g_RescDescMGR.GetRscDescIndex(i);
        ASSERT( Node.pDesc );
        x_try;
        if( Node.pDesc->IsChanged() )
        {
            SaveNode( *Node.pDesc );
        }
        x_catch_append( xfs("Resource name [%s]", Node.pDesc->GetName() ) );
    }
    
    // Get the output control
    CRichEditCtrl& RitchCtrl = CompErrorDisplayCtrl::GetDisplay();

    // Make sure that the pipes are ready to work.
    CReadPipe::GlobalInit();

    //
    // Prepare the output window for the compilation.
    //
    {
        CTime t = CTime::GetCurrentTime();
        CString s = t.Format( "%H:%M:%S %A, %B %d, %Y" );
        RitchCtrl.SetWindowText( xfs("Begin compilation: [%s],\n", (const char*)s) );
        RitchCtrl.SetSel( -1, -1 );
    }
    
    //
    // Build the platform flag
    //
    u32 Platform = 0;
    if( m_bCompileNintendo ) Platform |= PLATFORM_GCN;
    if( m_bCompilePS2      ) Platform |= PLATFORM_PS2;
    if( m_bCompileXBox     ) Platform |= PLATFORM_XBOX;
    if( m_bCompilePC       ) Platform |= PLATFORM_PC;

    //
    // Set verbose mode
    //
    rsc_desc::SetVerbose( m_bVerboseMode );

    //
    // toggle the mips building
    //
    geom_rsc_desc::SetColoredMips(m_bColorMipsMode);

    //
    // Do we have anything to compile?
    //
    x_try;

    if( g_RescDescMGR.BeginCompiling( Platform ) == FALSE )
    {
        RitchCtrl.SetSel( -1, -1 );
        RitchCtrl.ReplaceSel( "There is nothing to compile all is upto date.\nEnd of compilation.\n", FALSE );
        RitchCtrl.SendMessage(EM_SCROLLCARET, 0, 0);
        return;
    }

    x_catch_begin;

    RitchCtrl.ReplaceSel( "End of compilation.\n", FALSE );

    x_catch_end_ret;

    //
    // Create compiling season
    //
    SYSTEM_INFO SystemInfo;
    x_memset( &SystemInfo, 0, sizeof(SystemInfo) );
    GetSystemInfo( &SystemInfo );

    // For Autobuilds we don't want the threaded compile, it causes potential threading problems where
    // data is not accessed in a thread safe manner. TODO: CJ: This code needs to be revisited and a
    // real solution found for the threaded compiles.
    extern xbool g_bAutoBuild;
    if( g_bAutoBuild )
    {
        CReadPipe* pPipe = new CReadPipe( RitchCtrl, 
            *CMainFrame::s_pMainFrame->m_pwndProgCtrl, 
            *CMainFrame::s_pMainFrame->m_pwndProgCtrl2, m_bVerboseMode, g_Settings.GetTempPath() );
        pPipe->RunDirectly();
        delete pPipe;
    }
    else
    {
        if( SystemInfo.dwNumberOfProcessors < 2 )
        {
            CReadPipe* pPipe = new CReadPipe( RitchCtrl, 
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl, 
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl2, m_bVerboseMode, g_Settings.GetTempPath() );
            pPipe->CreateThread(0, 0, NULL);
        }
        else
        {
            CReadPipe* pPipe1 = new CReadPipe( RitchCtrl, 
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl,
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl2, m_bVerboseMode, g_Settings.GetTempPath() );
            pPipe1->CreateThread(0, 0, NULL);

            CReadPipe* pPipe2 = new CReadPipe( RitchCtrl, 
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl, 
                *CMainFrame::s_pMainFrame->m_pwndProgCtrl2, m_bVerboseMode, g_Settings.GetTempPath() );
            pPipe2->CreateThread(0, 0, NULL);
        }
    }

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::GetTypeList( xarray<xstring>& Types )
{
    const rsc_desc_type* pType = g_RescDescMGR.GetFirstType();
    for( s32 i=0; pType; i++, pType = g_RescDescMGR.GetNextType( pType ) )
    {
        if( x_stricmp( "???", pType->GetName() ) == 0 )
            continue;

        Types.Append() = pType->GetName();
    }    
}

//=========================================================================

void EDRscDesc_Doc::RefreshViews( xbool bAllViews )
{
    CONTEXT( "EDRscDesc_Doc::RefreshViews" );

    // Update all the views
    POSITION Pos = s_RegRscEditor.m_pTemplate->GetFirstDocPosition() ;
    if( Pos )
    {
        while( Pos )
        {
            EDRscDesc_Doc* pDoc = (EDRscDesc_Doc*) s_RegRscEditor.m_pTemplate->GetNextDoc( Pos );
            pDoc->UpdateAllViews( (CView*)0, bAllViews );
        }
    }
}

//=========================================================================

void EDRscDesc_Doc::AddRscDesc( const char* pType )
{
    x_try;

    const rsc_desc_type&    Type        = g_RescDescMGR.GetType( pType );

    //
    // First check whether we need to save the current rsc_desc
    //
    if( m_pPropEditor->GetInterface() )
    {
        rsc_desc& RscDesc = rsc_desc::GetSafeType( *((rsc_desc*)m_pPropEditor->GetInterface()) );

        if( RscDesc.IsChanged() )
        {
            long ID = MessageBox( NULL, "Do you want to save the current RscDesc desc?", "Warning",MB_ICONWARNING|MB_YESNOCANCEL );
            if( IDYES == ID )
            {
                rsc_desc& RscDesc = GetActive();                
                SaveNode( RscDesc );
            }
            else if( IDCANCEL == ID )
            {
                return;
            }    
            else
            {
                // TODO:
                // The user must have press no. So we must reload the resource desc.
            }
        }
    }

    //
    // Okay green light to create a new rsc description
    //
	CFileDialog		FileOpen(	TRUE, 
								_T(""), 
								_T(""), 
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
								(xfs("%s|All Files (*.*)|*.*||", Type.GetFileExtensions() )));

    if( FileOpen.DoModal() == IDOK )
    {
        // Also make the resource interface point at this rsc_desc        
        rsc_desc& RscDesc = g_RescDescMGR.CreateRscDesc( FileOpen.GetPathName() );
        m_pPropEditor->SetInterface( RscDesc );
        StartStopEdit();
        RefreshViews();
    }

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::SaveNode( rsc_desc& RscDesc )
{
    RscDesc.OnCheckIntegrity();

    text_out TextOut;
    char FullName[256];
    RscDesc.GetFullName( FullName );
    TextOut.OpenFile( FullName );
    RscDesc.OnSave( TextOut );
    TextOut.CloseFile();
}

//=========================================================================

void EDRscDesc_Doc::SaveActive( void )
{
    x_try;
    rsc_desc& RscDesc = GetActive();
    SaveNode( RscDesc );
    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::StartStopEdit( void )
{
    x_try;

    rsc_desc& RscDesc = GetActive();
    if( IsSelectedBeenEditedLocal() )
    {
        if( RscDesc.IsChanged() )
        {
            long ID = MessageBox( NULL, "Do you want to save the current RscDesc desc?", "Warning",MB_ICONWARNING|MB_YESNOCANCEL );
            if( IDYES == ID )
            {
                rsc_desc& RscDesc = GetActive();
                SaveNode( RscDesc );
            }
            else if( IDCANCEL == ID )
            {
                return;
            }    
            else
            {
                // TODO:
                // The user must have press no. So we must reload the resource desc.
                // DONO WHAT TO DO HERE. MAY BE RELOAD RESOURCE?
                return;
            }
        }

        for( s32 i=0; i<m_LocalEdited.GetCount(); i++ )
        {
            if( m_LocalEdited[i] == &RscDesc )
            {
                RscDesc.SetBeingEdited( FALSE );
                m_LocalEdited.Delete( i );
            }
        }

        ASSERT( RscDesc.IsBeingEdited() == FALSE );
        m_pPropEditor->SetInterface( RscDesc, TRUE );
    }
    else if( RscDesc.IsBeingEdited() )
    {
        x_throw( "This resource is already being edited" );
    }
    else
    {
        RscDesc.OnStartEdit();
        m_pPropEditor->SetInterface( RscDesc );
        RscDesc.SetBeingEdited( TRUE );
        m_LocalEdited.Append() = &RscDesc;
    }

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::SetActiveDesc( const char* pDescName )
{
    rsc_desc& Desc = g_RescDescMGR.GetRscDescByString( pDescName );

    for( s32 i=0; i<m_LocalEdited.GetCount(); i++ )
    {
        if( &Desc == m_LocalEdited[i] )
            break;
    }

    if( i == m_LocalEdited.GetCount() )
    {
        m_pPropEditor->SetInterface( Desc, TRUE  );
    }
    else
    {
        m_pPropEditor->SetInterface( Desc, FALSE  );
    }
}

//=========================================================================

rsc_desc& EDRscDesc_Doc::GetActive( void )
{
    if( m_pPropEditor->GetInterface() == NULL )
        x_throw( "There are not a selected resource");

    return rsc_desc::GetSafeType(*((rsc_desc*)m_pPropEditor->GetInterface()));
}

//=========================================================================

xbool EDRscDesc_Doc::IsSelectedBeenEditedLocal( void )
{       
    if( m_pPropEditor->GetInterface() == NULL )
        return FALSE;

    rsc_desc& Desc = GetActive();
    for( s32 i=0; i<m_LocalEdited.GetCount(); i++ )
    {
        if( &Desc == m_LocalEdited[i] )
        {
            ASSERT( GetActive().IsBeingEdited() == TRUE );
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

xbool EDRscDesc_Doc::IsCompileNintendo       ( void ){ return m_bCompileNintendo; }
xbool EDRscDesc_Doc::IsCompilePS2            ( void ){ return m_bCompilePS2;      }
xbool EDRscDesc_Doc::IsCompileXBox           ( void ){ return m_bCompileXBox;     }
xbool EDRscDesc_Doc::IsCompilePC             ( void ){ return m_bCompilePC;       }
xbool EDRscDesc_Doc::IsVerboseMode           ( void ){ return m_bVerboseMode;     }
xbool EDRscDesc_Doc::IsColorMipsMode         ( void ){ return m_bColorMipsMode;   }

void  EDRscDesc_Doc::ToggleCompileNintendo   ( void ){ m_bCompileNintendo = !m_bCompileNintendo; }
void  EDRscDesc_Doc::ToggleCompilePS2        ( void ){ m_bCompilePS2      = !m_bCompilePS2;}
void  EDRscDesc_Doc::ToggleCompileXBox       ( void ){ m_bCompileXBox     = !m_bCompileXBox;}
void  EDRscDesc_Doc::ToggleCompilePC         ( void ){ m_bCompilePC       = !m_bCompilePC;}
void  EDRscDesc_Doc::ToggleVerboseMode       ( void ){ m_bVerboseMode     = !m_bVerboseMode;}
void  EDRscDesc_Doc::ToggleColorMipsMode     ( void ){ m_bColorMipsMode   = !m_bColorMipsMode;}

void  EDRscDesc_Doc::ForceCompileNintendo    ( void ){ m_bCompileNintendo = TRUE;}
void  EDRscDesc_Doc::ForceCompilePS2         ( void ){ m_bCompilePS2      = TRUE;}
void  EDRscDesc_Doc::ForceCompileXBox        ( void ){ m_bCompileXBox     = TRUE;}
void  EDRscDesc_Doc::ForceCompilePC          ( void ){ m_bCompilePC       = TRUE;}

void  EDRscDesc_Doc::ForceCompileNintendoOff    ( void ){ m_bCompileNintendo = FALSE;}
void  EDRscDesc_Doc::ForceCompilePS2Off         ( void ){ m_bCompilePS2      = FALSE;}
void  EDRscDesc_Doc::ForceCompileXBoxOff        ( void ){ m_bCompileXBox     = FALSE;}
void  EDRscDesc_Doc::ForceCompilePCOff          ( void ){ m_bCompilePC       = FALSE;}
  
//=========================================================================

void EDRscDesc_Doc::CleanSelected( void )
{
    x_try;

    rsc_desc& Desc = GetActive();
    g_RescDescMGR.CleanResource( Desc );

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::CheckOutSelected( void )
{
    x_try;

    rsc_desc& Desc = GetActive();
    char FullName[256];
    Desc.GetFullName(FullName);

    if (x_strlen(FullName) > 0)
    {
        CFileStatus status;
        if( CFile::GetStatus( FullName, status ) )   // static function
        {
            if (!(status.m_attribute & CFile::readOnly))
            {
                if (::AfxMessageBox("This file is already writeable, checking out this file could overwrite existing changes. Are you sure you want to continue?", MB_YESNO) == IDNO )
                {
                    return;
                }
            }
            
            //sync to perforce
            xstring P4Sync = xfs("p4 sync \"%s\"",FullName );
            const char* strP4Sync = P4Sync;            
            system( strP4Sync );

            //open file for editting
            xstring P4Edit = xfs("p4 edit \"%s\"",FullName );
            const char* strP4Edit = P4Edit;            
            system( strP4Edit );

            // If this is an audiopkg then open the output file too
            if( x_stristr( FullName, ".audiopkg" ) )
            {
/*
                if( m_pDoc->IsCompilePS2() )
                {
                }

                if( m_pDoc->IsCompileXBox() )
                {
                }
*/

                // Sync and check out
                xstring Path = xfs( "%s\\PC\\%s", g_Settings.GetReleasePath(), Desc.GetName() );
                P4Sync = xfs("p4 sync \"%s\"", Path );
                system( (const char*)P4Sync );
                P4Edit = xfs("p4 edit \"%s\"", Path );
                system( (const char*)P4Edit );

                Path = xfs( "%s\\PS2\\%s", g_Settings.GetReleasePath(), Desc.GetName() );
                P4Sync = xfs("p4 sync \"%s\"", Path );
                system( (const char*)P4Sync );
                P4Edit = xfs("p4 edit \"%s\"", Path );
                system( (const char*)P4Edit );

                Path = xfs( "%s\\Xbox\\%s", g_Settings.GetReleasePath(), Desc.GetName() );
                P4Sync = xfs("p4 sync \"%s\"", Path );
                system( (const char*)P4Sync );
                P4Edit = xfs("p4 edit \"%s\"", Path );
                system( (const char*)P4Edit );
            }
        }

    }

    RefreshViews(FALSE);

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::DeleteSelectedResource( void )
{
    x_try;

    rsc_desc& Desc = GetActive();

    char Path[256];
    Desc.GetFullName( Path );

    long ID = MessageBox( NULL, xfs("Are you sure you want to delete from your drive:\n[%s]??", Path ), 
                          "Warning",MB_ICONEXCLAMATION|MB_YESNO );
    if( IDYES != ID )
    {
        return;
    }

    m_pPropEditor->ClearInterface();
    g_RescDescMGR.DeleteRscDescFromDrive( Desc.GetName() );

    RefreshViews();

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::FrameInit( CPropertyEditorDoc* pRopEditor )
{
    ASSERT( pRopEditor );
    m_pPropEditor = pRopEditor;
}

//=========================================================================

void EDRscDesc_Doc::Refresh( void )
{
    x_try;

    m_pPropEditor->ClearInterface();
    g_RescDescMGR.RefreshDesc();
    RefreshViews();
    x_catch_display;
}

//=========================================================================

void EDRscDesc_Doc::OnProjectRefresh( void )
{
    Refresh();
}

//=========================================================================

void EDRscDesc_Doc::RebuildAll( void )
{
    x_try;

    // First lets clear all the resources
    for( s32 i=0; i<g_RescDescMGR.GetRscDescCount(); i++ )
    {
        x_try;
        g_RescDescMGR.CleanResource( *g_RescDescMGR.GetRscDescIndex(i).pDesc );
        x_catch_display;
    }

    // Then lets build all
    Build();
 
    x_catch_display;
}

