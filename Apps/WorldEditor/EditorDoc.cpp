// EditorDoc.cpp : implementation of the CEditorDoc class
//

#include "StdAfx.h"


#include "EditorDoc.h"
#include "EditorView.h"
#include "EditorFrame.h"

#include "..\..\xCore\Parsing\TextIn.hpp"
#include "..\WinControls\ListBoxDlg.h"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\MainFrm.h"
#include "..\Editor\Project.hpp"
#include "..\WorldEditor\EditorView.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"

#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

#include "ai_editor.hpp"
#include "EditorLayerView.h"
#include "EditorGlobalView.h"
#include "EditorBlueprintView.h"
#include "ResourceBrowserDlg.h"
#include "AudioPkgDialog.h"
#include "AnimPkgDialog.h"
#include "GameLib\binlevel.hpp"
#include "CollisionMgr\PolyCache.hpp"

#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"
#include "..\EDRscDesc\RscDesc.hpp"

#include "aux_Bitmap.hpp"

#include "transaction_mgr.hpp"

//support
#include "ManagerRegistration.hpp"

#include "..\..\Support\ZoneMgr\ZoneMgr.hpp"
#include "..\..\Support\Obj_mgr\obj_mgr.hpp"
#include "..\..\Support\Objects\Portal.hpp"
#include "..\..\Support\Objects\ParticleEmiter.hpp"

#include "..\..\Support\AudioMgr\AudioMgr.hpp"

#include "..\..\Support\Decals\DecalMgr.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define CHECK_FOR_DUPLICATE_GUID


extern xbool g_game_running;
extern xbool g_first_person;
extern xbool g_bAutoBuild;

xbool g_EditorBreakpoint = FALSE;

user_settings    g_SaveTrackUserSettings;
user_settings    g_LoadUpdateUserSettings;



//X_FILE* g_PreLoadFP = NULL;

/*
//=========================================================================
// CGridSettings
//=========================================================================

void CGridSettings::Reset() 
{ 
    m_nGridSnap = 100; 
    m_nSpeed = 25;
    m_nRotateSnap = 90; 
    m_nImageScale = 100;
    m_nImageAlpha = 96;
    m_fGridLocation = vector3(0.0f, 0.0f, 0.0f);
    m_fFarZLimit = 50000.0f;
    m_bImageDraw = FALSE;
    m_bShowStats = FALSE;
    m_xcBackground = xcolor(0,0,0);
    m_strSchematic.Empty();
    m_fTimeDilation = 1.0f;
}

//=========================================================================

void CGridSettings::LoadImage()
{
    if (m_bImageLoaded)
    {
        UnloadImage();
    }

    if (CFileSearch::DoesFileExist(m_strSchematic))
    {
        if( auxbmp_LoadNative( m_xbmpImage, m_strSchematic ) )
        {
            BOOL bBadImage = FALSE;
            x_try;
            s32 nWidth = m_xbmpImage.GetWidth();
            while( true )
            {
                if (((nWidth%2) != 0) && (nWidth != 1) )
                {
                    bBadImage = TRUE;
                    x_throw( "Image width and height must be a power of 2" );
                }
                else if( nWidth == 1 )
                    break;

                nWidth /= 2;
            }

            s32 nHeight = m_xbmpImage.GetHeight();
            while( true )
            {
                if (((nHeight%2) != 0) && (nHeight != 1) )
                {
                    bBadImage = TRUE;
                    x_throw( "Image width and height must be a power of 2" );
                }
                else if( nHeight == 1 )
                    break;

                nHeight /= 2;
            }

            m_bImageLoaded = TRUE;
            vram_Register( m_xbmpImage  );

            x_catch_display;

            if (bBadImage)
            {
                 m_xbmpImage.Kill();
            }
        }
    }
}

//=========================================================================

void CGridSettings::UnloadImage()
{
    if (m_bImageLoaded)
    {
        m_bImageLoaded = FALSE;
        vram_Unregister( m_xbmpImage    );
        m_xbmpImage.Kill();
    }
}

//=========================================================================

void CGridSettings::OnEnumProp ( prop_enum&    List )
{
    List.AddString  (  "Grid", "Header which contains information about the current Grid settings", PROP_TYPE_HEADER );
    List.AddInt     (  "Grid\\Speed", "Speed in cm. Applies to W,A,S,D,R,F movement.");
    List.AddInt     (  "Grid\\Grid Snap", "Grid size, used for snapping to grid and raising and lowering grid.");
    List.AddInt     (  "Grid\\Rotate Snap", "Snap for keyboard rotation of objects.");
    List.AddVector3 (  "Grid\\Position", "Position of the grid.");
    List.AddColor   (  "Grid\\Background", "Background color for the world editor.");
    List.AddString  (  "Grid\\Schematic", "Schematic image to aid in laying out the levels.", PROP_TYPE_HEADER );
    List.AddBool    (  "Grid\\Schematic\\Draw", "Draw the overlay schematic." );
    List.AddFileName(  "Grid\\Schematic\\File", "Bitmaps (*.bmp)|*.bmp|Targas (*.tga)|*.tga|PNG (*.png)|*.png||", "File name for the schematic bitmap. Image Size must be a power of 2." );
    List.AddInt     (  "Grid\\Schematic\\Scale", "Scale of the schematic image.");
    List.AddInt     (  "Grid\\Schematic\\Alpha", "Transparency value for the schematic image.");
    List.AddString  (  "Engine", "Header which contains information about the current engine settings", PROP_TYPE_HEADER );
    List.AddBool    (  "Engine\\Show Stats", "Show engine stats in Debug window, when game is running." );
    List.AddFloat   (  "Engine\\Far Z Clip", "Limit for the viewable distance.  Make this number smaller to improve framerate on large levels." );
    List.AddFloat   (  "Engine\\Time Dilation", "Slow or Speed up time, 1.0 is normal, higher is faster time, lower is slower");
}

//=========================================================================

xbool CGridSettings::OnProperty ( prop_query&   I    )
{
    if( I.IsVar( "Grid\\Grid Snap" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nGridSnap );
        }
        else
        {
            m_nGridSnap = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Grid\\Speed" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nSpeed );
        }
        else
        {
            m_nSpeed = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Grid\\Rotate Snap" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nRotateSnap );
        }
        else
        {
            m_nRotateSnap = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Grid\\Position" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarVector3( m_fGridLocation );
        }
        else
        {
            m_fGridLocation = I.GetVarVector3();
        }
    }
    else if( I.IsVar( "Grid\\Background" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarColor( m_xcBackground );
        }
        else
        {
            m_xcBackground = I.GetVarColor();
        }
    }
    else if( I.IsVar( "Grid\\Schematic\\File" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_strSchematic, 256 );
        }
        else
        {
            m_strSchematic = CString(I.GetVarFileName());
            LoadImage();
        }
    }    
    else if( I.IsVar( "Grid\\Schematic\\Scale" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nImageScale );
        }
        else
        {
            m_nImageScale = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Grid\\Schematic\\Alpha" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nImageAlpha );
        }
        else
        {
            m_nImageAlpha = I.GetVarInt();
        }
    }
    else if( I.VarBool( "Grid\\Schematic\\Draw", m_bImageDraw ) )
    {
        //do nothing
    }
    else if( I.VarBool( "Engine\\Show Stats", m_bShowStats ) )
    {
        //do nothing
    }
    else if( I.IsVar( "Engine\\Far Z Clip" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_fFarZLimit );
        }
        else
        {
            m_fFarZLimit = I.GetVarFloat();
        }
    }
    else if ( I.IsVar( "Engine\\Time Dilation" ) ) 
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_fTimeDilation );
        }
        else
        {
            m_fTimeDilation = I.GetVarFloat();
            if (m_fTimeDilation < 0) //keep it positive
            {
                m_fTimeDilation = 0.0f;
            }
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}
*/

//=========================================================================
// THE CALLBACK HANDLER
//=========================================================================

CEditorHandler::CEditorHandler()
{
    m_pDoc = NULL;
}

//=========================================================================

void CEditorHandler::SetProgressRange( s32 Min, s32 Max )
{
    if (CMainFrame::s_pMainFrame->m_pwndProgCtrl2)
    {
        CMainFrame::s_pMainFrame->m_pwndProgCtrl2->SetRange32( Min, Max );
        CMainFrame::s_pMainFrame->m_pwndProgCtrl2->SetPos( Max );
    }
}

//=========================================================================

void CEditorHandler::SetProgress( s32 Pos )
{
    if (CMainFrame::s_pMainFrame->m_pwndProgCtrl2)
    {
        CMainFrame::s_pMainFrame->m_pwndProgCtrl2->SetPos( Pos );
    }
}

//=========================================================================

void CEditorHandler::PrintLog( const char* pMessage )
{
    /*
    CMainFrame::s_pMainFrame->m_wndOutputBar.m_LogMsgOutput.SetSel( -1, -1 );
    CMainFrame::s_pMainFrame->m_wndOutputBar.m_LogMsgOutput.ReplaceSel( pMessage, FALSE );
    CMainFrame::s_pMainFrame->m_wndOutputBar.m_LogMsgOutput.SendMessage(EM_SCROLLCARET, 0, 0);
    */
}

//=========================================================================

xbool CEditorHandler::HandleExternal( xstring &xstrType, xstring &xstrValue )
{
    int iSlash = xstrType.Find('\\');
    if( iSlash > 0)
    {
        //we found a slash
        CString strOperation( xstrType.Left(iSlash) );
        CString strVal( xstrType.Right(xstrType.GetLength() - iSlash - 1) );

        if ((strOperation.CompareNoCase("prop_mod") == 0) ||
            (strOperation.CompareNoCase("prop_chk") == 0))
        {
            //modify a property
            CListBoxDlg dlg;
            dlg.SetDisplayText("Select an object property to modify...");

            //create and enum this kind of object
            guid ObjGuid = g_ObjMgr.CreateObject(strVal);
            object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
            prop_enum List;
            pObj->OnEnumProp(List);
            for (int i=0; i < List.GetCount(); i++)
            {
                prop_enum::node& enData = List[i];

                if (strOperation.CompareNoCase("prop_mod") == 0)
                {
                    //we are modifying a property
                    if ( (enData.GetType() & PROP_TYPE_EXPOSE ) &&
                         (enData.GetType() & ~PROP_TYPE_READ_ONLY ) )
                    {
                        dlg.AddString( enData.GetName() );
                    }
                }
                else if (strOperation.CompareNoCase("prop_chk") == 0)
                {
                    //we are checking a property
                    if (enData.GetType() & PROP_TYPE_EXPOSE )
                    {
                        dlg.AddString( enData.GetName() );
                    }
                }
            }
            g_ObjMgr.DestroyObjectEx(ObjGuid, TRUE);

            if (dlg.DoModal() == IDOK)
            {
                xstrValue = dlg.GetSelection();
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else if (x_strcmp(strOperation,"attachpnt_picker") == 0)
        {
            CListBoxDlg dlg;
            dlg.SetDisplayText("Pick an bone to use...");

            object* pObject = g_ObjMgr.GetObjectByGuid(guid_FromString(strVal));
            if (pObject)
            {
                xstring List;

                pObject->EnumAttachPoints( List );
            
                char* pCur = &List[0];

                while (*pCur != 0)
                {
                    // Add the string
                    dlg.AddString( pCur );

                    // Advance to null terminator
                    while (*pCur != 0)
                        pCur++;

                    // Step beyond it
                    pCur++;
                }
            
                if (dlg.DoModal() == IDOK)
                {
                    xstrValue = dlg.GetSelection();
                    return TRUE;
                }
            }
            return FALSE;
        }   
        else if (x_strcmp(strOperation,"bone") == 0)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid(guid_FromString(strVal));
            if (pObject)
            {
                const anim_group* pAnimGroup = pObject->GetAnimGroupPtr();
                if( pAnimGroup )
                {
                    // Setup dialog
                    CListBoxDlg dlg;
                    dlg.SetDisplayText("Pick bone to use...");

                    // Add null bone
                    dlg.AddString( "<NONE>" );
                    
                    // Add bones to list
                    for( s32 i = 0; i < pAnimGroup->GetNBones(); i++ )
                        dlg.AddString( pAnimGroup->GetBone( i ).Name );

                    // Selected a bone?
                    if ( dlg.DoModal() == IDOK )
                    {
                        xstrValue = dlg.GetSelection();
                        return TRUE;
                    }
                }                
            }

            return FALSE;
        }
    }
    else if( (x_strcmp(xstrType, "animexternal") == 0) )
    { 
        CAnimPkgDialog dlg;
        int Result = dlg.DoModal();
        if( Result == IDOK )
        {
            if( dlg.m_DescLoaded )
                xstrValue = dlg.m_DescName;

            return TRUE;
        }
        else if( Result = IDCLEAR )
        {
            xstrValue = "<null>";
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if( (x_strcmp(xstrType, "soundemitter") == 0) || (x_strcmp(xstrType, "soundexternal") == 0) )
    { 
        CAudioPkgDialog dlg;
        int Result = dlg.DoModal();
        if( Result == IDOK )
        {
            if( dlg.m_DescLoaded )
                xstrValue = dlg.m_DescName;

            return TRUE;
        }
        else if( Result == IDCLEAR )
        {
            xstrValue = dlg.m_DescName;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if( x_strcmp(xstrType, "audiopkg") == 0 )
    { 
        CResourceBrowserDlg dlg;
        dlg.SetType(CString(xstrType));
        int Result = dlg.DoModal();
        if( Result == IDOK)
        {
            //set the objects property
            xstrValue = xstring(dlg.GetName());
            return TRUE;
        }
        else if( Result == IDCLEAR )
        {
            xstrValue = "<null>";
            return TRUE;
        }
        return FALSE;
    }
    else if( x_strcmp(xstrType, "Zoning") == 0 )
    {
        //portal
        CListBoxDlg dlg;
        dlg.SetDisplayText("Which Zone do you want to connect to?");

        xarray<xstring> List;
        g_WorldEditor.GetZoneList(List);
        for (int i = 0; i < List.GetCount(); i++)
        {
            dlg.AddString(CString(List.GetAt(i)));
        }

        if (dlg.DoModal() == IDOK)
        {
            xstrValue = dlg.GetSelection();
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if( x_strcmp(xstrType, "blueprint") == 0 )
    {
        //browse for a blueprint
        CString strInitialPath = g_Project.GetBlueprintPath();
	    CFileDialog	FileOpen( TRUE, _T("*.bpx"), "", OFN_FILEMUSTEXIST, "Blueprints|*.bpx||");
        FileOpen.m_ofn.lpstrInitialDir = strInitialPath;

        if( FileOpen.DoModal() == IDOK )
        {
            CString strPath = FileOpen.GetPathName();
            CString strName = FileOpen.GetFileTitle();
            if (!strPath.IsEmpty())
            {
//                if (g_TemplateMgr.EditorRegisterBlueprint(strName, strPath))
                {
                    xstrValue = strName;
                    return TRUE;
                }
            }
        }
        return FALSE;
    }
    else if( x_strcmp(xstrType, "particle") == 0 )
    {
        //browse for a fxd files..
        CString strInitialPath = g_Project.GetParticlePath();
	    CFileDialog	FileOpen( TRUE, _T("*.fxo"), "", OFN_FILEMUSTEXIST, "Particles|*.fxo||");
        FileOpen.m_ofn.lpstrInitialDir = strInitialPath;

        if( FileOpen.DoModal() == IDOK )
        {
            CString strPath = FileOpen.GetPathName();
            CString strName = FileOpen.GetFileTitle();
            if (!strPath.IsEmpty())
            {
                xstrValue = strName;
                return TRUE;
            }
        }
        return FALSE;
    }
    else if( x_strcmp(xstrType, "template") == 0 )
    {
        //browse for a blueprint
        CString strInitialPath = g_Project.GetBlueprintPath();
	    CFileDialog	FileOpen( TRUE, _T("*.bpx"), "", OFN_FILEMUSTEXIST, "Blueprints|*.bpx||");
        FileOpen.m_ofn.lpstrInitialDir = strInitialPath;
 
        if( FileOpen.DoModal() == IDOK )
        {
            CString strPath = FileOpen.GetPathName();
            CString strName = FileOpen.GetFileTitle();
            if (!strPath.IsEmpty())
            {
                xstrValue = strPath;
                return TRUE;
            }
        }
        return FALSE;
    }
    else if (( x_strcmp(xstrType, "global_guid") == 0 ) ||
             ( x_strcmp(xstrType, "global_all") == 0  ) )
    {
        BOOL bAllGlobals = ( x_strcmp(xstrType, "global_all") == 0 );
        //portal
        CListBoxDlg dlg;
        dlg.SetDisplayText("Which Global do you want to include?");

        xharray<var_mngr::global_def> hGlobalArray;
        g_VarMgr.GetGlobalsList(hGlobalArray);
        //we should now have all the current globals...
        for ( int i = 0; i < hGlobalArray.GetCount(); i++ )
        {
            var_mngr::global_def& Def = hGlobalArray[i];
            if ( bAllGlobals || (Def.Type == var_mngr::GLOBAL_GUID) )
            {
                CString strGlobal(Def.Name.Get());
                dlg.AddString(strGlobal);
            }
        }

        if (dlg.DoModal() == IDOK)
        {
            xstrValue = dlg.GetSelection();
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if (x_strcmp(xstrType,"object_picker") == 0)
    {
        CListBoxDlg dlg;
        dlg.SetDisplayText("Pick an object to use...");

        for( object_desc* pNode = object_desc::GetFirstType(); 
                          pNode; 
                          pNode = object_desc::GetNextType( pNode )  )
        {
            dlg.AddString( pNode->GetTypeName() );
        }

        if (dlg.DoModal() == IDOK)
        {
            xstrValue = dlg.GetSelection();
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if( x_strcmp(xstrType, "bintext") == 0 )
    {
        //browse for a bin files..
        CString strInitialPath = g_Project.GetParticlePath();
	    CFileDialog	FileOpen( TRUE, _T("*.bin"), "", OFN_FILEMUSTEXIST, "Binary Text|*.bin||");
        FileOpen.m_ofn.lpstrInitialDir = strInitialPath;

        if( FileOpen.DoModal() == IDOK )
        {
            CString strPath = FileOpen.GetPathName();
            CString strName = FileOpen.GetFileTitle();
            if (!strPath.IsEmpty())
            {
                xstrValue = strName;
                return TRUE;
            }
        }
        return FALSE;
    }

    // Show general resource dialog 
    CResourceBrowserDlg dlg;
    dlg.SetType( CString( xstrType ) );
    int Result = dlg.DoModal();
    if( Result == IDOK )
    {
        // Set the objects property
        xstrValue = xstring( dlg.GetName() );
        return TRUE;
    }
    else if( Result == IDCLEAR )
    {
        // Clear the property
        xstrValue = "<null>";
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void CEditorHandler::SetLayerDirty( const xstring &xstrLayer )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
        if (pView)
        {
            pView->SetLayerDirtyColor(CString(xstrLayer));
        }     
    }
}

//=========================================================================

xbool CEditorHandler::IsZoneLoaded( const xstring &xstrLayer, const xstring &xstrZone )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
        if (pView)
        {
            return pView->DoesZoneExist(CString(xstrLayer), CString(xstrZone));
        }     
    }

    return FALSE;
}

//=========================================================================

void CEditorHandler::ForceLayerUpdate( )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
        if (pView)
        {
            pView->LoadLayers();
        }     
    }
}

//=========================================================================

void CEditorHandler::RefreshGlobals( )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        CEditorGlobalView* pView = (CEditorGlobalView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorGlobalView));
        if (pView)
        {
            pView->RefreshView();
        }     
    }
}

//=========================================================================

void CEditorHandler::RefreshWatches( )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        if (pFrame)
        {
            pFrame->RefreshWatchWindowIfActive();
        }     
    }
}

//=========================================================================

void CEditorHandler::AlterGridColor( xcolor Color )
{
    if (m_pDoc && m_pDoc->GetView())
    {
        CEditorFrame *pFrame = m_pDoc->GetView()->GetFrame();
        pFrame->GetPropertyEditorDoc()->SetGridBackgroundColor(RGB(Color.R, Color.G, Color.B));
    }
}

//=========================================================================

void CEditorHandler::ShowWarning( xstring &xstrWarning )
{
    MessageBeep(MB_ICONASTERISK);
    x_DebugMsg(xstrWarning);
    if (m_pDoc && m_pDoc->GetView())
    {
        m_pDoc->GetView()->SetMessage(CString(xstrWarning));
    }
}

//=========================================================================

xbool CEditorHandler::IsFileReadOnly( const char* pFile ) 
{
    CFileStatus status;

    if( CFile::GetStatus( pFile, status ) )   // static function
    {
        return (status.m_attribute & CFile::readOnly);
    }

    return FALSE;
}

//=========================================================================

void CEditorHandler::ChangeSelectObjectsZone()
{
    CListBoxDlg dlg;
    dlg.SetDisplayText("Which Zone do you want to move the selected objects into (they will be put in the root of that zone)?");

    xarray<xstring> List;
    g_WorldEditor.GetZoneList(List);
    for (int i = 0; i < List.GetCount(); i++)
    {
        dlg.AddString(CString(List.GetAt(i)));
    }

    if (dlg.DoModal() == IDOK)
    {
        xstring xstrValue = dlg.GetSelection();
        const char* pLayer = g_WorldEditor.GetZoneLayer(xstrValue);
        
        if (x_strlen(pLayer) > 0)
        {       
            if (g_WorldEditor.IsLayerReadonly(pLayer))
            {
                ::AfxMessageBox("Selected zone's layer is readonly, and can not have objects added to it.");
            }
            else
            {
                const char* pOldLayer = g_WorldEditor.GetActiveLayer( );
                const char* pOldPath  = g_WorldEditor.GetActiveLayerPath( );           
                g_WorldEditor.SetActiveLayer(pLayer,xfs("\\%s\\",(const char*)xstrValue));
                m_pDoc->GetView()->GetFrame()->OnWetbMoveObjectsToActiveLayer();
                g_WorldEditor.SetActiveLayer(pOldLayer,pOldPath);
            }
        }
    }
}

//=========================================================================
// Themere Importer
//=========================================================================

void theme_importer::OnEnumProp( prop_enum&  List )
{
}

//=========================================================================

xbool theme_importer::OnProperty( prop_query& I    )
{
    if( I.IsVar( "Project\\Theme[]\\FileName" ) )
    {
        if( !I.IsRead() )
        {
            m_ThemePaths.Append( I.GetVarString() );
        }
    }
    return TRUE;
}

//=========================================================================
// CEditorDoc
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorDoc, CBaseDocument)

BEGIN_MESSAGE_MAP(CEditorDoc, CBaseDocument)
	//{{AFX_MSG_MAP(CEditorDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// CEditorDoc construction/destruction
//=========================================================================

CEditorDoc::CEditorDoc() :
m_pHandler(NULL),
m_MutexLock((CSyncObject*)&m_Mutex),
m_bWantToStop(FALSE)
{
    m_bGameRunning = FALSE;
    m_bGamePaused  = FALSE;
    m_bGameStep    = FALSE;

    m_bIsFPV = FALSE;
    m_pHandler = new CEditorHandler();
    if (m_pHandler)
    {
        m_pHandler->SetDoc(this);
        g_WorldEditor.SetExternalHandler(m_pHandler);
    }

    //register grid settings
    g_RegGameMgrs.AddManager("Grid",  &m_GridSettings);

    g_LoadUpdateUserSettings.CompileGCN = FALSE;
    g_LoadUpdateUserSettings.CompilePC = TRUE;
    g_LoadUpdateUserSettings.CompilePS2 = TRUE;
    g_LoadUpdateUserSettings.CompileXBOX = FALSE;


    //register decal settings
    // DECAL TODO: Replace with new decal manager
    //g_RegGameMgrs.AddManager("Decal", &m_DecalSettings);

     //register input settings
    g_RegGameMgrs.AddManager("Input", &m_InputSettings);
}

//=========================================================================

CEditorDoc::~CEditorDoc()
{

    if (m_pHandler) delete m_pHandler;
} 

//=========================================================================
// CEditorDoc serialization
//=========================================================================

void CEditorDoc::Serialize(CArchive& ar)
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
// CEditorDoc diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorDoc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

void CEditorDoc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorDoc commands
//=========================================================================

BOOL CEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return FALSE;
}

//=========================================================================

void CEditorDoc::OnProjectOpen()
{
    CONTEXT( "CEditorDoc::OnProjectOpen" );

    GetView()->GetFrame()->SetWindowText( g_Project.GetName() );
    GetView()->GetFrame()->SetProject(g_Project.GetName());
    ReloadLevel(CString(g_Project.GetWorkingPath()));
}

//=========================================================================

void CEditorDoc::OnProjectClose()
{
    OnProjectNew();
    GetView()->GetFrame()->SetProject("");
}

//=========================================================================

void CEditorDoc::OnProjectSave()
{    
    if (IsGameRunning())
    {
        ::AfxMessageBox("What are you thinking, can't save while the game is running!");
        return;
    }

    g_WorldEditor.ZoneSanityCheck();
    
    x_DebugMsg("CEditorDoc::Saving %d Dirty Layers\n",g_WorldEditor.GetDirtyLayerCount());

    //backup any files first
    CString strSave = CString(g_Project.GetWorkingPath());
    CString strBackupDir = strSave + "\\Backup\\";
    CreateDirectory(strBackupDir,NULL);

    CFileSearch fSearch;
    fSearch.AddDirPath(strSave);
    fSearch.GetFiles("*.layer");
    CString strNextFile = fSearch.GetNextFile(TRUE);
    while (!strNextFile.IsEmpty())
    {   
        CString strFileNameFrom = strSave + "\\";
        strFileNameFrom += strNextFile;
        CString strFileNameTo = strBackupDir + strNextFile;
        
        //copy all files for backup

        CFileStatus status;
        if( CFile::GetStatus( strFileNameTo, status ) )   // static function
        {
            if (status.m_attribute & CFile::readOnly)
            {
                status.m_attribute = status.m_attribute & ~CFile::readOnly;
                CFile::SetStatus(strFileNameTo, status);
            }
        }
        ::CopyFile(strFileNameFrom,strFileNameTo, FALSE);
        strNextFile = fSearch.GetNextFile(TRUE);
    }

    CWaitCursor wc;

    ai_editor::GetAIEditor()->CleanNavMap();

	if (!g_WorldEditor.SaveLayers(strSave))
    {
        ::AfxMessageBox("There are dirty layers that could not be saved. It is possible these may be marked readonly. (If there are dirty layers that you have not checked out, it is possible these had errors and were fixed by the editor)");
        LOG_WARNING("Editor", "Not all layers could be saved probably since some are marked readonly!");
    }

    //save Zoning file
    g_WorldEditor.SaveZoneFile();

    CFileStatus status;
    CString strInfoFile = xfs("%sLoadLayers.ProjectInfo", g_Project.GetWorkingPath());
    BOOL bSaveInfo = TRUE;
    if( CFile::GetStatus( strInfoFile, status ) )   // static function
    {
        if (status.m_attribute & CFile::readOnly)
        {
            bSaveInfo = FALSE;
            LOG_WARNING("Editor", "Could not save the LoadLayers.ProjectInfo file since it is marked readonly!");
        }
    }

    xarray<xstring> List;
    g_WorldEditor.GetLayerNames( List );

    //save the loaded layers
    if (bSaveInfo)
    {
        x_try;

        //Save which layers are open
        text_out OpenLayerFile;
        OpenLayerFile.OpenFile( strInfoFile );

        xarray<xstring> ListLoaded;
        for (int iLayers =0; iLayers< List.GetCount(); iLayers++)
        {
            CString strLayer(List.GetAt(iLayers));
            if ((strLayer.CompareNoCase(g_WorldEditor.GetDefaultLayer())!=0) &&
                (strLayer.CompareNoCase(g_WorldEditor.GetGlobalLayer())!=0))
            {
                //not default
                if (g_WorldEditor.IsLayerLoaded(strLayer))
                {
                    //first need to create a count
                    ListLoaded.Append(xstring(strLayer));
                }
            }
        }

        OpenLayerFile.AddHeader("LoadedLayers",ListLoaded.GetCount());
        for (int iLoaded =0; iLoaded< ListLoaded.GetCount(); iLoaded++)
        {
            //layer loaded, so save it
            OpenLayerFile.AddString("LayerName", ListLoaded.GetAt(iLoaded));
            OpenLayerFile.AddEndLine();
        }

        OpenLayerFile.CloseFile();

        x_catch_display;
    }

    //now delete any invalid layers
    fSearch.ClearFiles();
    fSearch.GetFiles("*.layer");
    strNextFile = fSearch.GetNextFile(TRUE);
    while (!strNextFile.IsEmpty())
    {   
        CString strFileNameDelete = strSave + "\\" + strNextFile;
        CString strLayerName = strNextFile.Left(strNextFile.ReverseFind('.'));
        BOOL bValid = FALSE;

        for (int i = 0; i < List.GetCount(); i++)
	    {
            if (strLayerName.CompareNoCase(List.GetAt(i)) == 0)
            {
                bValid = TRUE;
            }
        }

        if (!bValid)
        {
            //delete the invalid layer file
            VERIFY(::DeleteFile(strFileNameDelete));
        }

        strNextFile = fSearch.GetNextFile(TRUE);
    }

    //save ai
//    CString navSaveName = strSave + "NavMap.nmp";
//    ai_editor::GetAIEditor()->SaveNavMap(navSaveName);

    //save globals.
    CString GlobalsSaveName = strSave + "Globals.ProjectInfo";
    CFileStatus statusGlobals;
    BOOL bSaveGlobals = TRUE;
    if( CFile::GetStatus( GlobalsSaveName, statusGlobals ) )   // static function
    {
        if (statusGlobals.m_attribute & CFile::readOnly)
        {
            bSaveGlobals = FALSE;
            LOG_WARNING("Editor", "Could not save the Globals.glb file since it is marked readonly!");
        }
    }

    if (bSaveGlobals)
    {
        g_VarMgr.SaveGlobals( GlobalsSaveName );
    }
    
    //save particle paths
    //CString ParticlesSaveName = strSave+"Particles.ppd";
    ///GetFxDictionary().Save( ParticlesSaveName );

// new templates have full paths, no longer need this file    
//save templates
//    CString TemplatesSaveName = strSave+"Templates.ProjectInfo";
//    g_TemplateMgr.EditorSaveTemplatePaths( TemplatesSaveName );

/*    
    //save grid settings
    CString strGridSave = strSave + "Grid.settings";
    text_out TextOut;
    TextOut.OpenFile( strGridSave );
    m_GridSettings.OnSave(TextOut);
    TextOut.CloseFile();
*/

    //make sure settings directory exists
    strSave += "Config\\";
    CreateDirectory( strSave,     NULL );
    BOOL bFailureToSaveSettings = FALSE;

    //save all settings files
    for (int i=0; i < g_RegGameMgrs.GetCount(); i++)
    {
        CString strMgrName = g_RegGameMgrs.GetName(i);
        strMgrName.Replace( '\\', '_' );
        CString strFullPathToSave = strSave + strMgrName + ".settings";

        CFileStatus statusSettings;
        BOOL bSaveSettings = TRUE;
        if( CFile::GetStatus( strFullPathToSave, statusSettings ) )   // static function
        {
            if (statusSettings.m_attribute & CFile::readOnly)
            {
                bSaveSettings = FALSE;
                bFailureToSaveSettings = TRUE;
            }
        }

        if (bSaveSettings)
        {
            x_try;

            text_out TextOut;
            TextOut.OpenFile( strFullPathToSave );

            prop_interface* pMgr = g_RegGameMgrs.GetInterface( i );
            if (pMgr) pMgr->OnSave(TextOut);
            TextOut.CloseFile();

            x_catch_display;
        }
    }

    if (bFailureToSaveSettings)
    {
        LOG_WARNING("Editor", "Could not save some Config Settings files since some or all were marked readonly!");
    }

    //set all layers clean
    CEditorFrame *pFrame = GetView()->GetFrame();
    CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        pView->SetAllLayersClean();
    }    
    //Save userSettings - bjt
    SaveUserSettings();

    GetView()->GetFrame()->GetSettingsEditorDoc()->Refresh();
}

//=========================================================================

void CEditorDoc::RefreshPropertyView()
{
    GetView()->GetFrame()->GetPropertyEditorDoc()->Refresh();
}

//=========================================================================

void CEditorDoc::OnProjectNew()
{
    GetView()->CleanView();

    m_GridSettings.Reset();
    // DECAL TODO: Replace this once the new decal manager is finished
//    m_DecalSettings.Reset();

    g_WorldEditor.PreInitialize();
    g_WorldEditor.MarkLayerLoaded(g_WorldEditor.GetDefaultLayer(), TRUE);
    g_WorldEditor.MarkLayerLoaded(g_WorldEditor.GetGlobalLayer(), TRUE);

    //force save of the default layer
    g_WorldEditor.MarkLayerDirty(g_WorldEditor.GetDefaultLayer());
    g_WorldEditor.MarkLayerDirty(g_WorldEditor.GetGlobalLayer());
    CString strSave = CString(g_Project.GetWorkingPath());
	g_WorldEditor.SaveLayers(strSave);

    GetView()->GetFrame()->RefreshAll();
}

//=========================================================================

void CEditorDoc::OnProjectImport( void )
{
    if (IsGameRunning())
    {
        ::AfxMessageBox("You can not import while the game is running.");
        return;
    }
 
    if (! GetView()->IsStandardMode() ) 
    {
        ::AfxMessageBox("You must exit any special mode, such as move mode prior to importing.");
        return;
    }

    CFileDialog dlgFileOpen(TRUE, 
                    _T("project"), 
                    _T("Game.project"), 
                    0, 
                    "project files (*.project)|*.project||");

    CString strLevelPath(g_Settings.GetSourcePath());
    strLevelPath += "\\Levels";
    dlgFileOpen.m_ofn.lpstrInitialDir = strLevelPath;
    if( dlgFileOpen.DoModal() == IDOK )
    {
        CString strProjectPath = dlgFileOpen.GetPathName();
        char FullFileName[256];
        g_Project.GetFileName(FullFileName);
        if (strProjectPath.CompareNoCase(FullFileName) == 0)
        {
            ::AfxMessageBox("You can not import the currently open level!");
            return;
        }

        if (g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) )
        {
            ::AfxMessageBox("To complete a project import the Global Layer file must not be readonly. Import will not continue.");
            return;
        }

        CFileStatus status;
        CString strInfoFile = xfs("%s\\Globals.ProjectInfo", g_Project.GetWorkingPath());
        if( CFile::GetStatus( strInfoFile, status ) )   // static function
        {
            if (status.m_attribute & CFile::readOnly)
            {
                ::AfxMessageBox("To complete a project import the Globals file must not be readonly. Import will not continue.");
                return;
            }
        }

        strInfoFile = xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath());
        if( CFile::GetStatus( strInfoFile, status ) )   // static function
        {
            if (status.m_attribute & CFile::readOnly)
            {
                ::AfxMessageBox("To complete a project import the Zoning file must not be readonly. Import will not continue.");
                return;
            }
        }

        strInfoFile = xfs("%s\\Portals.ProjectInfo", g_Project.GetWorkingPath());
        if( CFile::GetStatus( strInfoFile, status ) )   // static function
        {
            if (status.m_attribute & CFile::readOnly)
            {
                ::AfxMessageBox("To complete a project import the Portals file must not be readonly. Import will not continue.");
                return;
            }
        }

        CString strSourcePath = strProjectPath.Left(strProjectPath.ReverseFind('\\'));

        //pre-load Zoning file
        xarray<editor_zone_ref> ZoneList;
        if (CFileSearch::DoesFileExist(xfs("%s\\Zoning.ProjectInfo", (const char*)strSourcePath)) )
        {
            if (!g_WorldEditor.LoadZoneListFromFile( xfs("%s\\Zoning.ProjectInfo", (const char*)strSourcePath), ZoneList ))
                ZoneList.Clear();
        }

        int i = 0;

        //make sure zones don't already exist
        for ( i = 0; i < ZoneList.GetCount(); i++)
        {
            editor_zone_ref& Zone = ZoneList.GetAt(i);
            if (g_WorldEditor.DoesZoneExist(Zone.Name))
            {
                ::AfxMessageBox(xfs("The project you are attempting to import has a zone already named \"%d\". Zone names must be unique prior to import. Import will not happen.", Zone.Name));
                return;
            }
        }

        //pre-load Layers
        CFileSearch fSearch;
        fSearch.AddDirPath(strSourcePath);
        fSearch.GetFiles("*.layer");
        CString strNextFile = fSearch.GetNextFile(TRUE);
        xarray<CString> LayerList;
        while (!strNextFile.IsEmpty())
        {   
            CString strLayerName;
            strLayerName = strNextFile.Left(strNextFile.ReverseFind('.'));
            if ( (strLayerName.CompareNoCase(g_WorldEditor.GetGlobalLayer()) != 0) && 
                 (strLayerName.CompareNoCase(g_WorldEditor.GetDefaultLayer()) != 0) )
            {
                //must make sure we don't have duplicate layers
                if (g_WorldEditor.DoesLayerExist(strLayerName))
                {
                    ::AfxMessageBox(xfs("The project you are attempting to import has the layer \"%s\", this layer conflicts with an already existing layer. Import will not happen.", (const char*)strLayerName));
                    return;
                }
                LayerList.Append(strLayerName);
            }
            strNextFile = fSearch.GetNextFile(TRUE);
        }

        if (::AfxMessageBox("Are you sure you wish to import this level. This action can not be undone and will clear you undo stack. Import will copy all layers (except default), zones, portals, and globals from the selected project. This will attempt to copy themes and blueprints, but not resources from the selected level. (note: Blueprints will overwrite blueprints of the same name.)", MB_YESNO) != IDYES)
        {
            return;
        }

        //--------------------import level-------------------------

        //first import the themes
        theme_importer ThemeImport;
        ThemeImport.OnLoad(strProjectPath);
        for ( i = 0; i < ThemeImport.m_ThemePaths.GetCount(); i++)
        {
            xbool bAlreadyExists = FALSE;
            for (int j = 0; j < g_Project.GetThemeCount(); j++)
            {
                char ThemePath[256];
                g_Project.GetThemePath(j, ThemePath);
                if (x_stricmp(ThemePath, ThemeImport.m_ThemePaths.GetAt(i)) == 0)
                {
                    bAlreadyExists = TRUE;
                    break;
                }
            }
            
            if (!bAlreadyExists)
                g_Project.InsertTheme(ThemeImport.m_ThemePaths.GetAt(i));
        }

        //import blueprints
        CString strCopyBlueprintFromPath(xfs("%s\\Blueprint", (const char*)strSourcePath));
        CString strCopyBlueprintToPath(xfs("%s",g_Project.GetBlueprintPath()));
        ::CreateDirectory(strCopyBlueprintToPath, NULL); //make sure blueprint directory exists       
        CopyPath(strCopyBlueprintFromPath, "*.bpx", strCopyBlueprintToPath);

        //next update the resources
        g_RescDescMGR.RefreshDesc();

        //import globals
        g_VarMgr.ImportGlobals( xfs("%s\\Globals.ProjectInfo", (const char*)strSourcePath) );

        //add layers
        for ( i = 0; i < LayerList.GetCount(); i++ )
        {
            g_WorldEditor.AddLayer(LayerList.GetAt(i), TRUE);
        }
        
        //add zones
        for ( i = 0; i < ZoneList.GetCount(); i++)
        {
            editor_zone_ref& Zone = ZoneList.GetAt(i);
            g_WorldEditor.CreateZone(Zone.Name, Zone.Layer);
        }

        //start guid logging
        g_WorldEditor.m_bLogGuids = TRUE;
        g_WorldEditor.m_GuidLog.Clear();

        //first create a list of all current portals
        xarray<guid> OldPortalList;
        g_WorldEditor.GetListOfPortals(OldPortalList);

        //load global layer and Portals file
        //always load global layer
        if (CFileSearch::DoesFileExist(xfs("%s\\%s.layer", (const char*)strSourcePath, g_WorldEditor.GetGlobalLayer())) )
        {
            //load Portals file
            if (CFileSearch::DoesFileExist(xfs("%s\\Portals.ProjectInfo", (const char*)strSourcePath)) )
            {
                g_WorldEditor.LoadPortals(xfs("%s\\Portals.ProjectInfo", (const char*)strSourcePath));
            }

            //now import the global layer
            LoadLayerFile(g_WorldEditor.GetGlobalLayer(), CString(xfs("%s\\%s.layer", (const char*)strSourcePath, g_WorldEditor.GetGlobalLayer())) );
            g_WorldEditor.MarkLayerReadonly(g_WorldEditor.GetGlobalLayer(), FALSE);
        }

        //now go through and resolve all new portals
        xarray<guid> NewPortalList;
        g_WorldEditor.GetListOfPortals(NewPortalList);

        //we need to update the zones
        for ( i = 0; i < NewPortalList.GetCount(); i++ )
        {
            BOOL bExists = FALSE;
            for ( int j=0; j < OldPortalList.GetCount(); j++)
            {
                if (NewPortalList.GetAt(i) == OldPortalList.GetAt(j))
                {
                    bExists = TRUE;
                    break;
                }
            }

            if (!bExists)
            {
                //this portal is not in the old list, must be new
                //so we need to update its zones
                object* pObjectBase = g_ObjMgr.GetObjectByGuid(NewPortalList.GetAt(i));
                if (pObjectBase && pObjectBase->IsKindOf(zone_portal::GetRTTI()))
                {
                    zone_portal* pPortal = (zone_portal*)pObjectBase;
                    
                    //find and update Zone1 
                    u8 Zone1ID = pPortal->GetZone1();
                    for ( int k = 0; k < ZoneList.GetCount(); k++)
                    {
                        editor_zone_ref& Zone = ZoneList.GetAt(k);
                        if (Zone.Id == Zone1ID)
                        {
                            u8 NewId = g_WorldEditor.GetZoneId(Zone.Name);
                            pPortal->SetZone1(NewId);
                            break;
                        }
                    }

                    //find and update Zone2 
                    u8 Zone2ID = pPortal->GetZone2();
                    for ( k = 0; k < ZoneList.GetCount(); k++)
                    {
                        editor_zone_ref& Zone = ZoneList.GetAt(k);
                        if (Zone.Id == Zone2ID)
                        {
                            u8 NewId = g_WorldEditor.GetZoneId(Zone.Name);
                            pPortal->SetZone2(NewId);
                            break;
                        }
                    }
                }
            }
        }

        //copy and load all layers
        for ( i = 0; i < LayerList.GetCount(); i++ )
        {
            CString strOriginal(xfs("%s\\%s.layer", (const char*)strSourcePath, (const char*)LayerList.GetAt(i)));
            CString strCopy(xfs("%s\\%s.layer", g_Project.GetWorkingPath(), (const char*)LayerList.GetAt(i)));
            ::CopyFile(strOriginal,strCopy, FALSE);

            CFileStatus statusCopy;
            if( CFile::GetStatus( strCopy, statusCopy ) )   // static function
            {
                if (statusCopy.m_attribute & CFile::readOnly)
                {
                    statusCopy.m_attribute = statusCopy.m_attribute & ~CFile::readOnly;
                    //mark this as writeable
                    CFile::SetStatus( strCopy, statusCopy );
                }
            }
            LoadLayerFile(LayerList.GetAt(i));
        }

        //stop guid logging
        g_WorldEditor.m_bLogGuids = FALSE;

        g_WorldEditor.ClearSelectedObjectList();
        g_WorldEditor.ZoneSanityCheck();

        //now select all added objects
        for (i=0; i <g_WorldEditor.m_GuidLog.GetCount(); i++)
        {
            g_WorldEditor.SelectObject(g_WorldEditor.m_GuidLog.GetAt(i),FALSE);
        }

        g_WorldEditor.ClearUndoList();

        //refresh the project
        GetView()->GetFrame()->RefreshAll();

        //save the project
        OnProjectSave();
        g_Project.Save();

        //enter move mode
        ::AfxMessageBox(xfs("Import Complete. %d objects imported and entered into move mode for easy placement.", g_WorldEditor.m_GuidLog.GetCount() ));
        g_WorldEditor.m_GuidLog.Clear();
        GetView()->GetFrame()->OnWetbMoveMode();
    }
}

//=========================================================================

void CEditorDoc::CopyPath(CString strFromPath, CString strWildcard, CString strToPath)
{
    CFileSearch::FormatPath(strFromPath);
    CString strName = strFromPath.Right(strFromPath.GetLength() - strFromPath.ReverseFind('\\') - 1);
    
    CFileSearch fSearch;
    fSearch.GetDirs(strFromPath);
    CStringList &lstDirs = fSearch.Dirs();
    CString strNextDir = fSearch.GetNextDir(TRUE);
    while (!strNextDir.IsEmpty())
    {   
        //add subdirs
        CString strNewFromPath = strFromPath + "\\" + strNextDir;
        CString strNewToPath = strToPath + "\\" + strNextDir;
        ::CreateDirectory(strNewToPath, NULL);
        CopyPath(strNewFromPath, strWildcard, strNewToPath);
        strNextDir = fSearch.GetNextDir(TRUE);
    }

    //add files
    fSearch.ClearDirs();
    fSearch.AddDirPath(strFromPath);
    fSearch.GetFiles(strWildcard);
    CString strNextFile = fSearch.GetNextFile(TRUE);
    while (!strNextFile.IsEmpty())
    {   
        CString strFileName = strNextFile.Right(strNextFile.GetLength() - strNextFile.ReverseFind('\\') - 1);
        CString strSource(strFromPath + "\\" + strFileName);
        CString strDestination(strToPath + "\\" + strFileName);
        ::CopyFile(strSource, strDestination, FALSE);
        strNextFile = fSearch.GetNextFile(TRUE);
    }
}

//=========================================================================

void CEditorDoc::ReloadLevel(CString strLevel)
{
    GetView()->CleanView();

    m_xaBlueprintPathUpdates.Clear();
	g_WorldEditor.PreInitialize();
    RefreshPropertyView();
    GetView()->GetFrame()->GetSettingsEditorDoc()->Refresh();

    xtimer LoadTimer;
    LoadTimer.Start();
    
//    CString navLoadName = strLevel + "NavMap.nmp";
//    ai_editor::GetAIEditor()->LoadNavMap( navLoadName );
    
    CString GlobalsLoadName = strLevel + "Globals.ProjectInfo";
    g_VarMgr.LoadGlobals( GlobalsLoadName );

    LoadUserSettings();  //bjt

    //CString ParticlesLoadName = strLevel+"Particles.ppd";
    //GetFxDictionary().Load( ParticlesLoadName );

    //particle_emitter::InitAllPresetParticles ();
    //particle_emitter::InitAllDynamicParticles();
 
// new templates have full paths, no longer need this file    
//    CString TemplatesLoadName = strLevel+"Templates.ProjectInfo";
//    if ( CFileSearch::DoesFileExist(TemplatesLoadName) )
//    {
//        g_TemplateMgr.EditorLoadTemplatePaths( TemplatesLoadName );
//    }

    x_DebugMsg("--TIMECHECK-- EditDoc::ReloadLevel(%s) AI:%g sec\n",strLevel, LoadTimer.TripSec());

    CFileSearch fSearch;
    fSearch.AddDirPath(strLevel);
    fSearch.GetFiles("*.layer");
    CString strNextFile = fSearch.GetNextFile(TRUE);
    //LOAD LAYERS
    while (!strNextFile.IsEmpty())
    {   
        CString strLayerName;
        strLayerName = strNextFile.Left(strNextFile.ReverseFind('.'));
        //CString strLayerPath = strLevel + strNextFile;
        g_WorldEditor.AddLayer(strLayerName, FALSE);

        strNextFile = fSearch.GetNextFile(TRUE);
    }

    //always load default layer
    LoadLayerFile(g_WorldEditor.GetDefaultLayer());

    //load Zoning file
    if (CFileSearch::DoesFileExist(xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath())) )
    {
        g_WorldEditor.LoadZoneFile();
    }
    
    //always load global layer
    if (CFileSearch::DoesFileExist(xfs("%s\\%s.layer", g_Project.GetWorkingPath(), g_WorldEditor.GetGlobalLayer())) )
    {
        //load Portals file
        if (CFileSearch::DoesFileExist(xfs("%s\\Portals.ProjectInfo", g_Project.GetWorkingPath())) )
        {
            g_WorldEditor.LoadPortals();
        }
        LoadLayerFile(g_WorldEditor.GetGlobalLayer());
    }
    else
    {
        g_WorldEditor.MarkLayerLoaded(g_WorldEditor.GetGlobalLayer(), TRUE);
        g_WorldEditor.MarkLayerDirty(g_WorldEditor.GetGlobalLayer());    
    }


    text_in OpenLayerFile;
    CString strInfoFile = xfs("%sLoadLayers.ProjectInfo", g_Project.GetWorkingPath());
    if (CFileSearch::DoesFileExist(strInfoFile))
    {
        OpenLayerFile.OpenFile( xfs("%sLoadLayers.ProjectInfo", g_Project.GetWorkingPath()) );
        while (OpenLayerFile.ReadHeader())
        {
            if( x_stricmp( OpenLayerFile.GetHeaderName(), "LoadedLayers" ) == 0 )
            {
                s32 nLoadedCount = OpenLayerFile.GetHeaderCount();
                for (s32 iCurrent =0; iCurrent < nLoadedCount; iCurrent++)
                {
                    OpenLayerFile.ReadFields();
                    char cName[MAX_PATH];
                    if (OpenLayerFile.GetString("LayerName",cName))
                    {
                        if(g_LoadUpdateUserSettings.UnloadedLayers.Find(cName) == -1)
                            LoadLayerFile(cName);
                        else
                            g_SaveTrackUserSettings.UnloadedLayers.Append(cName);
                    }   
                    else
                    {
                        if (!g_bAutoBuild)
                        {
                            ::AfxMessageBox(xfs("Error loading layer %s",cName));
                        }
                        else
                        {
                            LOG_ERROR("AutoBuild","Error loading layer %s",cName);
                        }
                    }
                }
            }
            else
            {
                ASSERT(FALSE);
            }
        }
        OpenLayerFile.CloseFile();
    }

    x_DebugMsg("LOADED %d objects into Obj_Mgr\n", g_WorldEditor.GetTotalObjectCount() );
#ifdef CHECK_FOR_DUPLICATE_GUID
    CheckForDuplicateGuidsInAllLayers();
#endif

	g_WorldEditor.PostInitialize();

    //make sure zones are right
    g_WorldEditor.ZoneSanityCheck();

    //bjt update all hidden and unselectable objects
    UpdateHiddenObjects();
    UpdateUnSelectAbleObjects();
    UpdateViewFavorites();
 

    GetView()->GetFrame()->RefreshAll();

    //KEEP FOR BACKWARDS COMPATIBILITY
    //load grid settings
    CString strGridSave = strLevel + "Grid.settings";
    if (CFileSearch::DoesFileExist(strGridSave))
    {
        text_in GridIn;
        GridIn.OpenFile( strGridSave );
        m_GridSettings.OnLoad(GridIn);
        GridIn.CloseFile();
    }
    else
    {
        m_GridSettings.Reset();
    }

    CString strSettings = strLevel + "Config\\";   
    //load all settings files
    for (int i=0; i < g_RegGameMgrs.GetCount(); i++)
    {
        CString strMgrName = g_RegGameMgrs.GetName(i);
        strMgrName.Replace( '\\', '_' );
        CString strFullPathToLoad = strSettings + strMgrName + ".settings";

        if (CFileSearch::DoesFileExist(strFullPathToLoad))
        {
            text_in TextIn;
            TextIn.OpenFile( strFullPathToLoad );

            prop_interface* pMgr = g_RegGameMgrs.GetInterface( i );
            if (pMgr) pMgr->OnLoad(TextIn);
            TextIn.CloseFile();
        }
    }


//    UpdateTreeView();
 
    GetView()->GetFrame()->GetSettingsEditorDoc()->Refresh();

    GetView()->SetViewDirty();
    UpdateTreeView();

}

//=========================================================================

BOOL CEditorDoc::UnloadLayerFile(CString strLayer)
{
    CWaitCursor wc;
    return g_WorldEditor.UnLoadLayer(strLayer);
}

//=========================================================================

BOOL CEditorDoc::LoadLayerFile(CString strLayer)
{
    return LoadLayerFile(strLayer, CString(xfs("%s\\%s.layer", g_Project.GetWorkingPath(), (const char*)strLayer)));
}

//=========================================================================

BOOL CEditorDoc::LoadLayerFile(CString strLayer, CString strFullPath)
{
    CWaitCursor wc;

    BOOL bReturn = TRUE;
    BOOL bBPForceDirty = FALSE;

    f32 fBPPlacementTime = 0.0f;


    xarray<editor_blueprint_placement> BPFiles;
    xarray<editor_dif_prop_list> BPDiffs;


    if (g_WorldEditor.LoadLayer(strFullPath, strLayer, BPFiles, BPDiffs))
    {
        if (m_pHandler) m_pHandler->SetProgressRange(0,BPFiles.GetCount());
        
        for (int i =0; i < BPFiles.GetCount(); i++)
        {
            if (m_pHandler) m_pHandler->SetProgress(i);

            //get the BP placement
            editor_blueprint_placement BPPlacement = BPFiles.GetAt(i);
            
            CString strBluePrintDir = g_Project.GetBlueprintPath();
            CString strTheme(BPPlacement.ThemeName);
            CString strRelativePath(BPPlacement.RelativePath);
            if (strTheme.IsEmpty())
            {
                //for backwards compatibility, try to build theme from relative path
                xstring xstrTheme, xstrRelativePath;
                if (g_WorldEditor.GetThemeInfoFromPath(BPPlacement.RelativePath, xstrTheme, xstrRelativePath))
                {
                    //got the info
                    strTheme = xstrTheme;
                    strRelativePath = xstrRelativePath;
                }
                else
                {
                    //couldn't find it, use project as default
                    strTheme = g_Project.GetName();
                    int iSlash = strRelativePath.ReverseFind('\\');
                    if (iSlash != -1)
                    {
                        strRelativePath = strRelativePath.Right(strRelativePath.GetLength() - iSlash);
                    }
                    else
                    {
                        strRelativePath = '\\' + strRelativePath;
                    }
                }

                // auto-set layer dirty unless autobuild is running
                if( !g_bAutoBuild )
                {
                    bBPForceDirty = TRUE;
                }
            }

            CString strFileName;
            CString strBPPath(g_Project.GetBlueprintDirForTheme(strTheme));
            if (strBPPath.IsEmpty())
            {
                strTheme = g_Project.GetName();
                strFileName = strBluePrintDir + strRelativePath;
            }
            else
            {
                strFileName = strBPPath + strRelativePath;
            }  

            //resolve the BP
            BOOL bFileReady = TRUE;
            if (!CFileSearch::DoesFileExist( strFileName ))
            {
                TRACE(xfs("Blueprint File Does Not Exist :: %s\n", (const char*)strFileName));
                bFileReady = FALSE;

                // auto-set layer dirty unless autobuild is running
                if( !g_bAutoBuild )
                {
                    bBPForceDirty = TRUE;
                }

                //first check files we've already identified!
                BOOL bFoundInList = FALSE;
                for (int j=0 ; j< m_xaBlueprintPathUpdates.GetCount(); j++)
                {
                    string_match Match = m_xaBlueprintPathUpdates.GetAt(j);

                    if (strFileName.CompareNoCase(Match.String1)==0)
                    {
                        //found it
                        bFoundInList = TRUE;
                        bFileReady = TRUE;
                        x_DebugMsg("(%s) Auto Updating Blueprint Path :: %s to %s\n",strLayer, strFileName, Match.String2);
                        strFileName = Match.String2;
                    }
                }

                if (!bFoundInList)
                {
                    CFileSearch fSearch;
                    fSearch.Recurse(TRUE);
                    fSearch.GetDirs(strBluePrintDir);
                    fSearch.GetFiles("*.bpx");
/*
                    if (fSearch.GetFileCount()==1)
                    {
                        //use the file we found
                        CString strNewFileName = fSearch.GetNextFile(FALSE);

                        string_match NewMatch;                 
                        NewMatch.String1 = strFileName;
                        NewMatch.String2 = strNewFileName;
                        m_xaBlueprintPathUpdates.Append(NewMatch);

                        x_DebugMsg("(%s) Code Updating Blueprint Path :: %s to %s\n",strFileName, strNewFileName);

                        strFileName = strNewFileName;
                        bFileReady = TRUE;
                    }
*/
                    if (fSearch.GetFileCount()>0)
                    {
                        if (!g_bAutoBuild)
                        {
                            //we found more than 1 file with that name
                            CListBoxDlg dlg;
                            CString strText;
                            strText.Format("Blueprint file %s could not be found for Layer %s, please select a replacement from the list below. Canceling will allow for a manual path search.",strFileName, strLayer);
                            dlg.SetDisplayText(strText);

                            CString strNextFile = fSearch.GetNextFile(FALSE);
                            while (!strNextFile.IsEmpty())
                            {   
                                dlg.AddString(strNextFile);
                                strNextFile = fSearch.GetNextFile(FALSE);
                            }

                            if (dlg.DoModal() == IDOK)
                            {
                                CString strNewFileName = dlg.GetSelection();
                                if (!strNewFileName.IsEmpty())
                                {
                                    string_match NewMatch;                 
                                    NewMatch.String1 = strFileName; 
                                    NewMatch.String2 = strNewFileName;
                                    m_xaBlueprintPathUpdates.Append(NewMatch);

                                    x_DebugMsg("(%s) User Updating Blueprint Path :: %s to %s\n",strLayer,strFileName, strNewFileName);

                                    strFileName = strNewFileName;
                                    bFileReady = TRUE;
                                }
                            }
                        }
                        else
                        {
                            LOG_ERROR("AutoBuild","Failed to load blueprint %s in Layer %s.",strFileName,strLayer);
                        }
                    }
                    
                    if (!bFileReady)
                    {
                        if (!g_bAutoBuild)
                        {
                            //file did not exist
                            ::AfxMessageBox(xfs("Could not find blueprint file %s for Layer %s\n\nPlease browse for a replacement file.", (const char*)strFileName, (const char*)strLayer));

	                        CFileDialog		FileOpen(	TRUE, 
								                        _T("*.bpx"), 
								                        strBluePrintDir, 
								                        OFN_FILEMUSTEXIST, 
								                        "Blueprints|*.bpx||");

                            FileOpen.m_ofn.lpstrInitialDir = g_Project.GetBlueprintPath();

                            if( FileOpen.DoModal() == IDOK )
                            {
                                CString strNewFileName = FileOpen.GetPathName();
                                if (!strNewFileName.IsEmpty())
                                {
                                    string_match NewMatch;                 
                                    NewMatch.String1 = strFileName;
                                    NewMatch.String2 = strNewFileName;
                                    m_xaBlueprintPathUpdates.Append(NewMatch);

                                    x_DebugMsg("(%s) User Updating Blueprint Path :: %s to %s\n",strLayer,strFileName, strNewFileName);

                                    strFileName = strNewFileName;
                                    bFileReady = TRUE;
                                }
                            }
                        }
                        else
                        {
                            LOG_ERROR("AutoBuild","Failed to load blueprint %s in Layer %s.",strFileName,strLayer);
                        }
                    }
                }
            }

            //ensure we have a valid theme and relative path
            if (bFileReady)
            {
                xstring xstrTheme, xstrRelativePath;
                if (g_WorldEditor.GetThemeInfoFromPath(strFileName, xstrTheme, xstrRelativePath))
                {
                    //got the info
                    strTheme = xstrTheme;
                    strRelativePath = xstrRelativePath;
                }
                else
                {
                    bFileReady = FALSE;
                    x_DebugMsg("ERROR! Blueprint(%s) not within a theme",strFileName);
                }                           
            }

            //add the BP
            if (bFileReady)
            {
                xtimer LoadTimer;
                LoadTimer.Start();
                
                editor_blueprint_ref BPRef;
                g_WorldEditor.AddBlueprintToSpecificLayer(strTheme, strRelativePath, 
                                                          strLayer, BPPlacement.LayerPath,
                                                          BPRef, TRUE, bBPForceDirty,
                                                          BPPlacement.PrimaryGuid);
                g_WorldEditor.RotateSelectedObjects( BPPlacement.Transform );
                g_WorldEditor.MoveBlueprintObjectsToAnchor( BPPlacement.Position );
                g_WorldEditor.ClearSelectedObjectList();

                fBPPlacementTime += LoadTimer.TripSec();
            }
            else
            {
                LOG_ERROR("Editor", "(%s) Error loading Blueprint Path %s",strLayer,strFileName);
            }
        }

        //ok, now update all blueprint over write modifications
        for ( int j = 0; j < BPDiffs.GetCount(); j++)
        {
            editor_dif_prop_list& DiffInfo = BPDiffs.GetAt(j);
            object* pObjToMod = g_ObjMgr.GetObjectByGuid(DiffInfo.m_ObjGuid);
            if (pObjToMod)
            {
                pObjToMod->OnPaste(DiffInfo.m_ObjProps);
            }
            else
            {
                LOG_ERROR( "Editor", "Failed to modify properties in blueprinted object %08X:%08X", DiffInfo.m_ObjGuid.GetHigh(), DiffInfo.m_ObjGuid.GetLow() );
            }
        }
    }

    x_DebugMsg("--TIMECHECK-- EditDoc::LoadLayerFile(%s) BPPT:%g\n",strLayer,fBPPlacementTime);
    
    if (m_pHandler) m_pHandler->SetProgress(0);
    
    return bReturn;
}

static s32 s_Run_Times = 0;
//=========================================================================

void CEditorDoc::RunGame()
{
    ai_editor::GetAIEditor()->ClearTestPath();

    if (!m_bGameRunning)
    {
        if (!m_MutexLock.IsLocked())
        {
            if (m_MutexLock.Lock())
            {                    
                g_EditorBreakpoint = FALSE;
                g_BinLevelMgr.SetDesireToSave(FALSE);
                g_BinLevelMgr.SetDesireToLoad(FALSE);
                
                s_Run_Times ++;
                ASSERT( s_Run_Times == 1 );

                //make sure zones are right
                g_WorldEditor.ZoneSanityCheck();

                g_game_running = TRUE;
                m_TimeRunning = 0.0f;

                g_WorldEditor.ClearSelectedObjectList();

                VERIFY(g_BinLevelMgr.SaveRuntimeDynamicData());
                VERIFY(g_VarMgr.SaveRuntimeData());

                g_WorldEditor.StoreState(TRUE);
                g_VarMgr.StoreState();

                g_AudioManager.ClearReceiverQueue();
                g_AudioManager.ClearAlertReceiverQueue();

                g_WorldEditor.BuildZoneList();

                //create template info
                g_TemplateMgr.EditorCreateGameData();
                ai_editor::GetAIEditor()->CreateNavMap();

                // invalidate polycache
                g_PolyCache.InvalidateAllCells();

                m_Timer.Start();

                RefreshPropertyView();
                m_bGameRunning = TRUE;

                m_MutexLock.Unlock();
                m_bWantToStop = FALSE;

                if (GetView())
                {
                    GetView()->UpdateWatchTimer(TRUE);
                }
            }
        }
    }
}

//=========================================================================

void CEditorDoc::StopGame()
{
    if (m_bGameRunning && m_TimeRunning > 0.1f)
    {
        if (!m_MutexLock.IsLocked())
        {
            if (m_MutexLock.Lock())
            {
                m_bGameRunning = FALSE;
                g_BinLevelMgr.SetDesireToSave(FALSE);
                g_BinLevelMgr.SetDesireToLoad(FALSE);
        
                s_Run_Times--;
                ASSERT( s_Run_Times == 0 );
        
                g_game_running = FALSE;
                g_first_person = FALSE;

                g_WorldEditor.ClearSelectedObjectList();
    
                g_ZoneMgr.Reset();
                SetFPV(FALSE);
                g_WorldEditor.ResetState(TRUE);
                g_VarMgr.ResetState();

                g_PolyCache.InvalidateAllCells();

                m_Timer.Stop();

                RefreshPropertyView();
                g_AudioMgr.ReleaseAll();

                m_TimeRunning = 0.0f;
                m_MutexLock.Unlock();
                m_bWantToStop = FALSE;

                g_DecalMgr.ResetDynamicDecals();

                if (GetView())
                {
                    GetView()->GetFrame()->RefreshWatchWindowIfActive();
                    GetView()->UpdateWatchTimer(FALSE);
                    GetView()->SetViewDirty();
                }
            }
        }
        else 
        {
            m_bWantToStop = TRUE;
        }
    }
}

//=========================================================================

void CEditorDoc::SaveGame()
{
    g_BinLevelMgr.SetDesireToSave(TRUE);
}

//=========================================================================

void CEditorDoc::LoadGame()
{
    g_BinLevelMgr.SetDesireToLoad(TRUE);
}

//=========================================================================

void CEditorDoc::PauseGame(BOOL bPause)
{
    m_bGamePaused = bPause;

    if( bPause )
        g_AudioMgr.PauseAll();
    else
        g_AudioMgr.ResumeAll();
}

//=========================================================================

void CEditorDoc::StepGame( void )
{
    m_bGameStep = TRUE ;
}

//=========================================================================

BOOL CEditorDoc::IsGamePaused()
{
    return m_bGamePaused;
}

//=========================================================================

BOOL CEditorDoc::IsGameRunning()
{
    return m_bGameRunning;
}

//=========================================================================

void CEditorDoc::AdvanceLogic(void)
{
    f32 Time = m_Timer.TripSec() * GetTimeDilation();

    if (g_EditorBreakpoint)
    {
        g_EditorBreakpoint = FALSE;
        m_bGamePaused = TRUE;
    }

    if( m_bGameRunning)
    {
        if ( g_BinLevelMgr.WantsToSave() )
        {
            //VERIFY(g_BinLevelMgr.SaveRuntimeDynamicData());
            //VERIFY(g_VarMgr.SaveRuntimeData());
#ifdef OLD_SAVE
            g_SaveMgr.SaveGame();
#endif
            g_BinLevelMgr.SetDesireToSave( FALSE );
        }
        else if ( g_BinLevelMgr.WantsToLoad() )
        {
            //VERIFY(g_BinLevelMgr.LoadRuntimeDynamicData());
            //VERIFY(g_VarMgr.LoadRuntimeData());
            g_WorldEditor.ResetState( TRUE );
#ifdef OLD_SAVE
            g_SaveMgr.RestoreGame( TRUE );
#endif
            g_BinLevelMgr.SetDesireToLoad( FALSE );
        }
        
        if ((!m_bGamePaused) || (m_bGameStep))
        {
            if (m_bWantToStop)
            {
                StopGame();
            }
            else if (!m_MutexLock.IsLocked())
            {
                if (m_MutexLock.Lock())
                {
                    // Force frame rate to 30 FPS if stepping
                    if (m_bGameStep)
                    {
                        m_bGameStep = FALSE ;
                        Time        = GetTimeDilation() / 30.0f ;

                        // Let audio run for step time so lip sync anims work
                        xtimer AudioTimer;
                        g_AudioMgr.ResumeAll();
                        AudioTimer.Start();
                        while( AudioTimer.ReadSec() < Time )
                        {
                            // Do nothing...
                        }
                        g_AudioMgr.PauseAll();
                    }

                    g_WorldEditor.AdvanceLogic(Time);
                    m_TimeRunning += Time;
                    m_MutexLock.Unlock();
                }
            }
        }
    }
}

//=========================================================================

void CEditorDoc::InitSettingsInterface()
{
    prop_interface* pProps = g_RegGameMgrs.GetManagerInterface("Grid");
    if (pProps)
    {
        GetView()->GetFrame()->GetSettingsEditorDoc()->SetInterface(*pProps);
    }
    else
    {
        GetView()->GetFrame()->GetSettingsEditorDoc()->ClearGrid();
    }
}

//=========================================================================

void CEditorDoc::SetGridPreset(s32 iIndex)
{
    iIndex = MINMAX(0, iIndex, 4);
    m_GridSettings.m_nGridSnap = m_GridSettings.m_GridPresets[iIndex];
}

//=========================================================================

void CEditorDoc::SetRotatePreset(s32 iIndex)
{
    iIndex = MINMAX(0, iIndex, 4);
    m_GridSettings.m_nRotateSnap = m_GridSettings.m_RotatePresets[iIndex];
}

//=========================================================================

void CEditorDoc::IncrementGridSnap(void)
{
    m_GridSettings.m_nGridSnap += m_GridSettings.m_nGridIncrement;
    s32 tempVal = m_GridSettings.m_nGridSnap % m_GridSettings.m_nGridIncrement;
    m_GridSettings.m_nGridSnap -= tempVal;
    m_GridSettings.m_nGridSnap = MAX(1,m_GridSettings.m_nGridSnap);
}

//=========================================================================

void CEditorDoc::DecrementGridSnap(void)
{
    m_GridSettings.m_nGridSnap -= m_GridSettings.m_nGridIncrement;
    s32 tempVal = m_GridSettings.m_nGridSnap % m_GridSettings.m_nGridIncrement;
    m_GridSettings.m_nGridSnap -= tempVal;
    m_GridSettings.m_nGridSnap = MAX(1,m_GridSettings.m_nGridSnap);
}

//=========================================================================

void CEditorDoc::IncrementRotateSnap(void)
{
    m_GridSettings.m_nRotateSnap += m_GridSettings.m_nRotateIncrement;
    s32 tempVal = m_GridSettings.m_nRotateSnap % m_GridSettings.m_nRotateIncrement;
    m_GridSettings.m_nRotateSnap -= tempVal;
    m_GridSettings.m_nRotateSnap = MAX(1,m_GridSettings.m_nRotateSnap);
}

//=========================================================================

void CEditorDoc::DecrementRotateSnap(void)
{
    m_GridSettings.m_nRotateSnap -= m_GridSettings.m_nRotateIncrement;
    s32 tempVal = m_GridSettings.m_nRotateSnap % m_GridSettings.m_nRotateIncrement;
    m_GridSettings.m_nRotateSnap -= tempVal;
    m_GridSettings.m_nRotateSnap = MAX(1,m_GridSettings.m_nRotateSnap);
}

//=========================================================================

void CEditorDoc::SetGridPos( const vector3& v3 ) 
{
    m_GridSettings.m_fGridLocation = v3;
    GetView()->GetFrame()->GetSettingsEditorDoc()->Refresh();
}

//=========================================================================

BOOL CEditorDoc::CheckForDuplicateGuidsInAllLayers(void)
{
    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    s32 result = 0;
    xarray< xstring >DuplicateGuild;
    DuplicateGuild.Clear();
    CMapStringToPtr Map;

    for (int nLayer=0; nLayer < ListLayers.GetCount(); nLayer++)
    {
        if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(nLayer), ObjList ))
        {
            for (int nGuid=0; nGuid < ObjList.GetCount(); nGuid++)
            {
                // Build a string key from the guid
                guid& ObjGuid = ObjList.GetAt(nGuid);
                CString s;
                s.Format( "%08X:%08X", ObjGuid.GetHigh(), ObjGuid.GetLow() );

                // Check the map for this key
                void* nLayerFound;
                if( Map.Lookup( s, nLayerFound ) )
                {
                    // This key already exists in the map
                    ::AfxMessageBox(xfs("Found Duplicated Guid %s in Layers %s and %s",
                        (const char*)guid_ToString(ObjGuid), (const char*)ListLayers.GetAt(nLayer), (const char*)ListLayers.GetAt((int)nLayerFound)));
                    result++;
                    //Add to string log
                    DuplicateGuild.Append(ListLayers.GetAt(nLayer));
                    DuplicateGuild.Append(ListLayers.GetAt((int)nLayerFound));
                    DuplicateGuild.Append(guid_ToString(ObjGuid));
                    log_LOCK();
                    log_ERROR( "Duplicate Guid", guid_ToString(ObjGuid));
                    LOG_FLUSH();

                }
                else
                {
                    // Add this key into the map
                    Map[s] = (void*)nLayer;
                }

            }
        }
    }
    if(result)
    {
        ::AfxMessageBox(xfs("A Total of %d duplicated Guids where found.\n Error Log was Dump in your usersettings directory", result));
        //Dump to a text file
        CString strPathFileName;
        GetUserErrorPath(&strPathFileName, TRUE);
        FILE* Fp = fopen( strPathFileName, "w" );
        fclose( Fp );

        x_try;
        text_out ErrorLog;
        ErrorLog.OpenFile(strPathFileName);
        ErrorLog.AddHeader(g_Project.GetName());
        ErrorLog.AddHeader("Duplicate Guids", DuplicateGuild.GetCount() / 3);
        for (int i = 0; i < DuplicateGuild.GetCount(); i+=3)
        {
            ErrorLog.AddString("FirstLayer", DuplicateGuild.GetAt(i));
            ErrorLog.AddString("SecondLayer", DuplicateGuild.GetAt(i + 1));
            ErrorLog.AddString("Guid", DuplicateGuild.GetAt(i + 2));
            ErrorLog.AddEndLine();
        }
        //Done. Save out user data
        ErrorLog.CloseFile();

        x_catch_display;
        return FALSE;
    }
    else
        return TRUE;
}




//=========================================================================

//void CEditorDoc::CreateStaticDecal( vector3 Begin, vector3 End )
//{
    // DECAL TODO: Replace this once the new decal manager is finished
    //m_DecalSettings.CreateStaticDecal(Begin,End);
//}

//=========================================================================

void CEditorDoc::PrintLog( const char* pMessage )
{
    m_pHandler->PrintLog( pMessage );
}

//=========================================================================

BOOL CEditorDoc::SaveUserSettings( void )
{
    if( g_bAutoBuild )
        return TRUE;

    //Set up path 
    CString strPathFileName, UserLevelName;

    GetUserSettingsPath(&strPathFileName, TRUE);
    UserLevelName = g_Project.GetName();
    if(UserLevelName.IsEmpty())
        return TRUE;

    FILE* Fp = fopen( strPathFileName, "r" );
    if(!Fp)
    {
        //No File Create One
        Fp = fopen( strPathFileName, "w" );
    }
    fclose( Fp );

    x_try;
    text_out UserSettings;
    UserSettings.OpenFile(strPathFileName);

    UserLevelName = g_Project.GetName();
    UserLevelName += " - UserSettings";
    UserSettings.AddHeader(UserLevelName);
 
   //Do Camera Postion and Rotation

    vector3 CameraPos   = g_SaveTrackUserSettings.CameraPos;
    radian3 Rot(g_SaveTrackUserSettings.Pitch, g_SaveTrackUserSettings.Yaw, 0);

    UserSettings.AddHeader("Camera");
    UserSettings.AddVector3("CameraPosition", CameraPos);
    UserSettings.AddRadian3("CameraRotation", Rot);
    UserSettings.AddEndLine();
  
    //Save which layers have been unloaded

    if(g_SaveTrackUserSettings.UnloadedLayers.GetCount())
    {
        UserSettings.AddHeader("UnLoadedLayers",g_SaveTrackUserSettings.UnloadedLayers.GetCount());
        for (int iLoaded =0; iLoaded< g_SaveTrackUserSettings.UnloadedLayers.GetCount(); iLoaded++)
        {
            //layer is unload loaded, so save it
            UserSettings.AddString("LayerName", g_SaveTrackUserSettings.UnloadedLayers.GetAt(iLoaded));
            UserSettings.AddEndLine();
        }
    }
    //Save out all Hidden objects
    int HiddenCount = BuildListOfHiddenObjects();
    if(HiddenCount)
    {
        UserSettings.AddHeader("HidddenObjects",g_SaveTrackUserSettings.HiddenObjects.GetCount());
        for (int i = 0; i < HiddenCount; i++)
        {
            guid& ObjGuid = g_SaveTrackUserSettings.HiddenObjects.GetAt(i);
            object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
            if( pObject )
            {
                //if the object exists, save it out
                UserSettings.AddGuid("HiddenObjectGuid", ObjGuid);
                UserSettings.AddEndLine();
            }
        }
    }

    //Save Out All unSelectAble Objects

    int UnSelectAbleCount = BuildListOfUnSelectableObjects();
    if(UnSelectAbleCount)
    {
        UserSettings.AddHeader("UnSelectAbleObjects",g_SaveTrackUserSettings.UnSelectAbleObjects.GetCount());
        for (int i = 0; i < UnSelectAbleCount; i++)
        {
            guid& ObjGuid = g_SaveTrackUserSettings.UnSelectAbleObjects.GetAt(i);
            object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
            if( pObject )
            {
                //if the object exists, save it out
                UserSettings.AddGuid("UnSelectAbleObjectGuid", ObjGuid);
                UserSettings.AddEndLine();
            }
        }
    }
    //Save View Favorites

    int ViewFavoritesCount = BuildListOfViewFavorites();
    if(ViewFavoritesCount)
    {
        UserSettings.AddHeader("ViewFavorites",g_SaveTrackUserSettings.ViewFavorites.GetCount());
        for (int i = 0; i < ViewFavoritesCount; i++)
        {
            view_favorite& FView = g_SaveTrackUserSettings.ViewFavorites.GetAt(i);
            radian3 CamRot;
            vector3 CamPos;

            FView.m_View.GetPitchYaw(CamRot.Pitch, CamRot.Yaw);
            CamPos = FView.m_View.GetPosition();

            UserSettings.AddString("FavoriteName", FView.m_Name);
            UserSettings.AddRadian3("FavoriteCameraRotation", CamRot);
            UserSettings.AddVector3("FavoriteCameraPosition", CamPos);
            UserSettings.AddVector3("FavoriteFocusPosition",FView.m_FocusPos);
            UserSettings.AddEndLine();
        }
    }
    int TreeViewCount = BuildTreeViewList();
    //Save All Open Folder Paths
    if(TreeViewCount)
    {
        UserSettings.AddHeader("OpenTreeFolders",g_SaveTrackUserSettings.TreeView.GetCount());
        for (int iLoaded =0; iLoaded< g_SaveTrackUserSettings.TreeView.GetCount(); iLoaded++)
        {
            UserSettings.AddString("FolderPath", g_SaveTrackUserSettings.TreeView.GetAt(iLoaded));
            UserSettings.AddEndLine();
        }
        UserSettings.AddHeader("ScrollBarPosition",1);
        UserSettings.AddS32("Position", g_SaveTrackUserSettings.ScrollBarPos);
        UserSettings.AddEndLine();
    }

    //Add Save Options

    UserSettings.AddHeader("CompileOptions");
    UserSettings.AddBool("CompilePC", g_SaveTrackUserSettings.CompilePC);
    UserSettings.AddBool("CompilePS2", g_SaveTrackUserSettings.CompilePS2);
    UserSettings.AddBool("CompileXBOX", g_SaveTrackUserSettings.CompileXBOX);
    UserSettings.AddBool("CompileGCN", g_SaveTrackUserSettings.CompileGCN);
    UserSettings.AddEndLine();

    //Done. Save out user data
    UserSettings.CloseFile();

    x_catch_display;
    return TRUE;

}

//=========================================================================

BOOL CEditorDoc::LoadUserSettings( void )
{

    //Set up path 
    CString strPathFileName;

    GetUserSettingsPath(&strPathFileName, FALSE);
    g_LoadUpdateUserSettings.UnloadedLayers.Clear();
    g_SaveTrackUserSettings.UnloadedLayers.Clear();

    g_LoadUpdateUserSettings.HiddenObjects.Clear();
    g_SaveTrackUserSettings.HiddenObjects.Clear();

    g_SaveTrackUserSettings.UnSelectAbleObjects.Clear();
    g_LoadUpdateUserSettings.UnSelectAbleObjects.Clear();

    g_SaveTrackUserSettings.UnSelectAbleObjects.Clear();
    g_LoadUpdateUserSettings.UnSelectAbleObjects.Clear();
    g_SaveTrackUserSettings.ViewFavorites.Clear();
    g_LoadUpdateUserSettings.ViewFavorites.Clear();
    g_SaveTrackUserSettings.TreeView.Clear();
    g_LoadUpdateUserSettings.TreeView.Clear();


    g_LoadUpdateUserSettings.UpdateCameraFlag = FALSE;
    g_LoadUpdateUserSettings.UpdateCompileButtonsFlag = FALSE;
 
    // check to see if file exist.
    FILE* Fp = fopen( strPathFileName, "r" );
    if(!Fp)
    {
        return FALSE;
    }
    fclose( Fp );

    //Do Camera Here

    LoadUserSettingsCamera(strPathFileName);

    //Do layers 
   
    LoadUserSettingsUnloadedLayers(strPathFileName);

    // Do Hidden Objects

    LoadUserSettingsHiddenObjects(strPathFileName);

    // Do UnSelectable Objects

    LoadUserSettingsUnSelectAbleObjects(strPathFileName);

    // Do favorite camara Positions

    LoadUserViewFavorites(strPathFileName);

    //Load tree view

    LoadUserOpenTree(strPathFileName);

    //Compile Button Selections

    LoadUserCompileOptions(strPathFileName);


    return TRUE;

}

//=========================================================================
 

void CEditorDoc::GetUserSettingsPath(CString *strPathFileName, BOOL CheckDirectoryFlag)
{
    //Set up path 
    //if directory doesn't exist, create it
 
    char strPathName[MAX_PATH];
    strcpy(strPathName,g_Settings.GetReleasePath());
    
    for(int Count = strlen(strPathName); Count > 0; Count--)
    {
        if(strPathName[Count] == '\\')
        {
            strPathName[Count] = 0;
            break;
        }
    }
 
    CString strFileName = g_Project.GetName();
    CString strPlatform = "\\UserSettings\\";
    CString strSubDirectory = "\\UserSettings";
    CString strPath;

    strFileName += "_usersettings.cfg";

    *strPathFileName = strPathName + strPlatform + strFileName;
    strPath = strPathName + strSubDirectory;

    if(CheckDirectoryFlag)
    {
        x_try;
            // Try to create the directory.
            ::CreateDirectory(strPath, NULL);
        x_catch_display;
    }
    return;
}

//=========================================================================
 
void CEditorDoc::GetUserErrorPath(CString *strPathFileName, BOOL CheckDirectoryFlag)
{
    //Set up path 
    //if directory doesn't exist, create it
 
    char strPathName[MAX_PATH];
    strcpy(strPathName,g_Settings.GetReleasePath());
    
    for(int Count = strlen(strPathName); Count > 0; Count--)
    {
        if(strPathName[Count] == '\\')
        {
            strPathName[Count] = 0;
            break;
        }
    }
 
    CString strFileName = g_Project.GetName();
    CString strPlatform = "\\UserSettings\\";
    CString strSubDirectory = "\\UserSettings";
    CString strPath;

    strFileName += "_error.log";

    *strPathFileName = strPathName + strPlatform + strFileName;
    strPath = strPathName + strSubDirectory;

    if(CheckDirectoryFlag)
    {
        x_try;
            // Try to create the directory.
            ::CreateDirectory(strPath, NULL);
        x_catch_display;
    }
    return;
}


//=========================================================================

BOOL CEditorDoc::LoadUserSettingsCamera( CString FileName)
{
    text_in UserSettings;
    int Result = FALSE;

    UserSettings.OpenFile(FileName);
    while (UserSettings.ReadHeader())
    {
        if(x_stricmp( UserSettings.GetHeaderName(), "Camera") == 0)
        {
            UserSettings.ReadFields();
            vector3 TempPos;
            radian3 TempRot;
            if (UserSettings.GetVector3("CameraPosition",TempPos) == TRUE)
            {
                if (UserSettings.GetRadian3("CameraRotation",TempRot) == TRUE)
                {
                    //Get camera corrdinates
                        
                    g_LoadUpdateUserSettings.CameraPos = TempPos;
                    g_LoadUpdateUserSettings.Pitch = TempRot.Pitch;
                    g_LoadUpdateUserSettings.Yaw = TempRot.Yaw;
                    g_LoadUpdateUserSettings.UpdateCameraFlag = TRUE;
                    Result = TRUE;
                }
            }
        }
    }
    UserSettings.CloseFile();
    return TRUE;
}

//=========================================================================

BOOL CEditorDoc::LoadUserSettingsUnloadedLayers( CString FileName )
{
    text_in UserSettings;
    UserSettings.OpenFile(FileName);

    //Do layers 

    while (UserSettings.ReadHeader())
    {
        if( x_stricmp( UserSettings.GetHeaderName(), "UnLoadedLayers" ) == 0 )
        {
            s32 nLoadedCount = UserSettings.GetHeaderCount();
            for (s32 iCurrent =0; iCurrent < nLoadedCount; iCurrent++)
            {

                UserSettings.ReadFields(); 
                char cName[MAX_PATH];
                if (UserSettings.GetString("LayerName",cName))
                {
                    g_LoadUpdateUserSettings.UnloadedLayers.Append(cName);
                }
            }
        }
    }
    UserSettings.CloseFile();
    return TRUE;
}

//=========================================================================

BOOL CEditorDoc::LoadUserSettingsHiddenObjects( CString FileName )
{
    text_in UserSettings;
    UserSettings.OpenFile(FileName);

    //Do Hidden object 

    while (UserSettings.ReadHeader())
    {
        if( x_stricmp( UserSettings.GetHeaderName(), "HidddenObjects" ) == 0 )
        {
            s32 nLoadedCount = UserSettings.GetHeaderCount();
            for (s32 iCurrent =0; iCurrent < nLoadedCount; iCurrent++)
            {
                UserSettings.ReadFields(); 
                guid ObjGuid;
                if (UserSettings.GetGuid("HiddenObjectGuid",ObjGuid))
                {
                    g_LoadUpdateUserSettings.HiddenObjects.Append(ObjGuid);
                }
            }
        }
    }
    UserSettings.CloseFile();
    return TRUE;
}

//=========================================================================

BOOL CEditorDoc::LoadUserSettingsUnSelectAbleObjects( CString FileName )
{
    text_in UserSettings;
    UserSettings.OpenFile(FileName);

    //Do UnSelectAble object 

    while (UserSettings.ReadHeader())
    {
        if( x_stricmp( UserSettings.GetHeaderName(), "UnSelectAbleObjects" ) == 0 )
        {
            s32 nLoadedCount = UserSettings.GetHeaderCount();
            for (s32 iCurrent =0; iCurrent < nLoadedCount; iCurrent++)
            {
                UserSettings.ReadFields(); 
                guid ObjGuid;
                if (UserSettings.GetGuid("UnSelectAbleObjectGuid",ObjGuid))
                {
                    g_LoadUpdateUserSettings.UnSelectAbleObjects.Append(ObjGuid);
                }
            }
        }
    }
    UserSettings.CloseFile();
    return TRUE;
}

   
//====================================================================

BOOL CEditorDoc::LoadUserViewFavorites( CString FileName)
{
    text_in UserSettings;
    int Result = FALSE;

    UserSettings.OpenFile(FileName);
    while (UserSettings.ReadHeader())
    {
        if(x_stricmp( UserSettings.GetHeaderName(), "ViewFavorites") == 0)
        {
            s32 nViewCount = UserSettings.GetHeaderCount();
            for (s32 iCurrent =0; iCurrent < nViewCount; iCurrent++)
            {
                UserSettings.ReadFields();
                view_favorite FView;
                radian3 CamRot;
                vector3 CamPos;
                char StrTemp[MAX_PATH];

                if (!UserSettings.GetString("FavoriteName", StrTemp))
                    continue;
                if (!UserSettings.GetRadian3("FavoriteCameraRotation",CamRot))
                    continue;
                if (!UserSettings.GetVector3("FavoriteCameraPosition",CamPos))
                    continue;
                if (!UserSettings.GetVector3("FavoriteFocusPosition",FView.m_FocusPos))
                    continue;

                //set View
                FView.m_Name = StrTemp;
                FView.m_View.SetRotation( radian3( CamRot.Pitch, CamRot.Yaw, 0 ) );
                FView.m_View.SetPosition(CamPos);
                g_LoadUpdateUserSettings.ViewFavorites.Append(FView);
                Result = TRUE;
            }
        }
    }
    UserSettings.CloseFile();
    return Result;
}

//=========================================================================

int CEditorDoc::LoadUserCompileOptions( CString FileName)
{
    text_in UserSettings;
    int Result = FALSE;

    UserSettings.OpenFile(FileName);
    while (UserSettings.ReadHeader())
    {
        if(x_stricmp( UserSettings.GetHeaderName(), "CompileOptions") == 0)
        {
            UserSettings.ReadFields();

            if (!UserSettings.GetBool("CompilePC", g_LoadUpdateUserSettings.CompilePC))
                break;
            if (!UserSettings.GetBool("CompilePS2", g_LoadUpdateUserSettings.CompilePS2))
                break;
            if (!UserSettings.GetBool("CompileXBOX", g_LoadUpdateUserSettings.CompileXBOX))
                break;
            if (!UserSettings.GetBool("CompileGCN", g_LoadUpdateUserSettings.CompileGCN))
                break;
        }
    }
    return TRUE;

}


//=========================================================================

BOOL CEditorDoc::LoadUserOpenTree( CString FileName)
{
    text_in UserSettings;
    UserSettings.OpenFile(FileName); 
    int Result = FALSE;

    while (UserSettings.ReadHeader())
    {
        if( x_stricmp( UserSettings.GetHeaderName(), "OpenTreeFolders" ) == 0 )
        {
            s32 nLoadedCount = UserSettings.GetHeaderCount();
            for (s32 iCurrent =0; iCurrent < nLoadedCount; iCurrent++)
            {
                UserSettings.ReadFields(); 
                char cName[1024];
                if (UserSettings.GetString("FolderPath",cName))
                {
                    g_LoadUpdateUserSettings.TreeView.Append(cName);
                }
            }
        }
    }
    UserSettings.CloseFile();
    UserSettings.OpenFile(FileName); 

    while (UserSettings.ReadHeader())
    {                                                 
        if( x_stricmp( UserSettings.GetHeaderName(), "ScrollBarPosition" ) == 0 )
        {
            UserSettings.ReadFields(); 
            int ScrollPos;
            if(UserSettings.GetS32("Position",ScrollPos))
            {
                g_LoadUpdateUserSettings.ScrollBarPos = ScrollPos;
            }
            else
                g_LoadUpdateUserSettings.ScrollBarPos = 0;
        }
    }

    UserSettings.CloseFile();
    return TRUE;
}
//=========================================================================

int CEditorDoc::BuildListOfHiddenObjects(void)
{
    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    g_SaveTrackUserSettings.HiddenObjects.Clear();

    for (int j=0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i=0; i<ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                if( pObject && pObject->IsHidden()==TRUE)
                {
                    g_SaveTrackUserSettings.HiddenObjects.Append(ObjGuid);
                }
            }
        }
    }
    return g_SaveTrackUserSettings.HiddenObjects.GetCount();
}

//=========================================================================

int CEditorDoc::BuildListOfUnSelectableObjects( void )
{
    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    g_SaveTrackUserSettings.UnSelectAbleObjects.Clear();
   for (int j=0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i=0; i<ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                if( pObject && pObject->IsSelectable()==FALSE)
                {
                    g_SaveTrackUserSettings.UnSelectAbleObjects.Append(ObjGuid);
                }
            }
        }
    }
    return g_SaveTrackUserSettings.UnSelectAbleObjects.GetCount();
}

//=========================================================================

int CEditorDoc::BuildListOfViewFavorites( void )
{
   // Get data
         
    xarray<view_favorite>&  ViewFavorite = GetView()->GetViewFavorites();
    int Count = ViewFavorite.GetCount();
 
    g_SaveTrackUserSettings.ViewFavorites.Clear();
 
    if(Count)
    {
        for (int i = 0; i < Count; i++)
        {
            g_SaveTrackUserSettings.ViewFavorites.Append(ViewFavorite.GetAt(i));
        }
    }
    return g_SaveTrackUserSettings.ViewFavorites.GetCount();
}

//=========================================================================

int CEditorDoc::BuildTreeViewList( void )
{
   //set all layers clean
    CEditorFrame *pFrame = GetView()->GetFrame();
    CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    g_SaveTrackUserSettings.TreeView.Clear();
    if (pView)
    {
        pView->TrackOpenTreeFolders();
    }    
    return g_SaveTrackUserSettings.TreeView.GetCount();
}

//=========================================================================

void CEditorDoc::UpdateHiddenObjects(void)
{
    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    for (int j=0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i=0; i<ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                if(pObject && g_LoadUpdateUserSettings.HiddenObjects.Find(ObjGuid)!= -1)
                    pObject->SetHidden(TRUE);
            }
        }
    }
}

//=========================================================================

void CEditorDoc::UpdateUnSelectAbleObjects(void)
{
    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    for (int j=0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i=0; i<ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                if(pObject && g_LoadUpdateUserSettings.UnSelectAbleObjects.Find(ObjGuid)!=-1)
                    pObject->SetSelectable(FALSE);
            }
        }
    }
}

//=========================================================================

int CEditorDoc::UpdateViewFavorites( void )
{
    xarray<view_favorite>&  ViewFavorite = GetView()->GetViewFavorites();
    ViewFavorite.Clear();
    int Count = g_LoadUpdateUserSettings.ViewFavorites.GetCount();

    if(Count)
    {
        for (int i = 0; i < Count; i++)
        {
            ViewFavorite.Append(g_LoadUpdateUserSettings.ViewFavorites.GetAt(i));
        }
    }
    return g_LoadUpdateUserSettings.ViewFavorites.GetCount();
}

//=========================================================================

int CEditorDoc::UpdateTreeView( void )
{
    CEditorFrame *pFrame = GetView()->GetFrame();
    CEditorLayerView* pView = (CEditorLayerView*)pFrame->FindViewFromTab( pFrame->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    int Count = g_LoadUpdateUserSettings.TreeView.GetCount();

    if(Count)
    {
        if (pView)
        {
            pView->UpdateTreeFolders();
        }    
    }
    return g_LoadUpdateUserSettings.TreeView.GetCount();
}



