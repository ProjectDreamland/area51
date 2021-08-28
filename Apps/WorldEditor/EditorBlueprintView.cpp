// EditorBlueprintView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorBlueprintView.h"
#include "EditorPaletteDoc.h"
#include "WorldEditor.hpp"
#include "EditorFrame.h"
#include "EditorDoc.h"
#include "EditorView.h"
#include "resource.h"
#include "..\WinControls\FileSearch.h"
#include "..\WinControls\ListBoxDlg.h"
#include "..\Editor\Project.hpp"

#include "transaction_mgr.hpp"
#include "transaction_file_data.hpp"

#include "..\MeshViewer\RigidDesc.hpp"
#include "..\MeshViewer\SkinDesc.hpp"

#include "Objects\PlaySurface.hpp"
#include "Objects\PropSurface.hpp"
#include "Objects\AnimSurface.hpp"
#include "Objects\SkinPropSurface.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEditorBlueprintView

IMPLEMENT_DYNCREATE(CEditorBlueprintView, CPaletteView)

CEditorBlueprintView::CEditorBlueprintView() 
{
}

//=========================================================================

CEditorBlueprintView::~CEditorBlueprintView()
{
}

//=========================================================================


BEGIN_MESSAGE_MAP(CEditorBlueprintView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorBlueprintView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDR_OBJECT_LIST, OnSelchangeList)
	ON_COMMAND(ID_BPTB_CREATE_ANCHOR, OnBptbCreateAnchor)
	ON_COMMAND(ID_BPTB_CREATE_BLUEPRINT, OnBptbCreateBlueprint)
    ON_COMMAND(ID_BPTB_ADD_BLUEPRINT_AS_OBJECTS, OnBptbAddBlueprintAsObjects)
    ON_COMMAND(ID_BPTB_ADD_BLUEPRINT, OnBptbAddBlueprint)
    ON_COMMAND(ID_BPTB_SHATTER_BLUEPRINT, OnBptbShatterBlueprint)
    ON_COMMAND(ID_BPTB_SHATTER_FOR_EDIT, OnBptbShatterBlueprintForEdit)
    ON_COMMAND(ID_BPTB_REFRESH, OnBptbRefresh)
    ON_COMMAND(ID_BPTB_REPLACE_SELECTED_WITH_BLUEPRINT, OnBptbReplaceSelectedWithBlueprint)
    ON_COMMAND(ID_BPTB_RESYNC, OnBptbResync)
    ON_MESSAGE(WM_USER_MSG_FILE_ITEM_POSTCHANGE, OnFileItemChange)
    ON_MESSAGE(WM_USER_MSG_FILE_ITEM_PRECHANGE, OnFileItemPreChange)
    ON_UPDATE_COMMAND_UI(ID_BPTB_CREATE_BLUEPRINT, OnUpdateBptbCreateBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_CREATE_ANCHOR, OnUpdateBptbCreateAnchor)
    ON_UPDATE_COMMAND_UI(ID_BPTB_ADD_BLUEPRINT_AS_OBJECTS, OnUpdateBptbAddBlueprintAsObjects)
    ON_UPDATE_COMMAND_UI(ID_BPTB_ADD_BLUEPRINT, OnUpdateBptbAddBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_SHATTER_BLUEPRINT, OnUpdateBptbShatterBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_SHATTER_FOR_EDIT, OnUpdateBptbShatterBlueprintForEdit)   
    ON_UPDATE_COMMAND_UI(ID_BPTB_REFRESH, OnUpdateBptbRefresh)       
    ON_UPDATE_COMMAND_UI(ID_BPTB_RESYNC, OnUpdateBptbResync)       
    ON_UPDATE_COMMAND_UI(ID_BPTB_REPLACE_SELECTED_WITH_BLUEPRINT, OnUpdateBptbReplaceSelectedWithBlueprint)       
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditorBlueprintView drawing

void CEditorBlueprintView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CEditorBlueprintView diagnostics

#ifdef _DEBUG
void CEditorBlueprintView::AssertValid() const
{
	CPaletteView::AssertValid();
}

//=========================================================================

void CEditorBlueprintView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorBlueprintView message handlers

int CEditorBlueprintView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_BLUEPRINT_BAR;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_fbcBlueprints.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                           TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDR_OBJECT_LIST))
    {
        return -1;	      
    }
    m_fbcBlueprints.UsePreviousPathAsDisplay(TRUE);

    if (!m_wndPreview.Create(_T("STATIC"),"PreviewWnd", WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_STATIC))
    {
        ASSERT(FALSE);
        return -1;
    }

    if (!m_stPlaceholder.Create("",WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_ICON,
                         CRect(0,0,0,0),this,IDC_STATIC))
    {
        ASSERT(FALSE);
		return -1;      // fail to create
    }
    
    BuildTreeFromProject();

	return 0;
}

//=========================================================================

void CEditorBlueprintView::BuildTreeFromProject()
{
    m_fbcBlueprints.ClearTree();
    char cPath[MAX_PATH];
    for( s32 i = g_Project.GetFirstBlueprintDir( cPath ); i != -1; i = g_Project.GetNextBlueprintDir( i, cPath ) )
    {
        m_fbcBlueprints.BuildTreeFromPath(cPath, "*.bpx", "bpx");
    }
}

//=========================================================================

void CEditorBlueprintView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorBlueprintView::OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
    CString strPath = m_fbcBlueprints.GetSelectedPath();

    if (!m_fbcBlueprints.IsFolder(strPath))
    {
        //its a blueprint
        CWaitCursor wc;
        if (RenderBlueprint(strPath))
        {
            m_wndPreview.OnStartTimer();
        }
        else
        {
            m_wndPreview.ClearGeom();
        }
    }
    else
    {
        m_wndPreview.ClearGeom();
    }

	*pResult = 0;
}

//=========================================================================

BOOL CEditorBlueprintView::RenderBlueprint(CString strPath)
{
    m_wndPreview.ClearGeom();

    x_try;
    text_in Blueprint;
    Blueprint.OpenFile( strPath );

    vector3 anchorPos;
    while (Blueprint.ReadHeader())
    {
        if( x_stricmp( Blueprint.GetHeaderName(), "Anchor" ) == 0 )
        {
            Blueprint.ReadFields();
            Blueprint.GetVector3("Position",anchorPos);
        }
        else if( x_stricmp( Blueprint.GetHeaderName(), "Object" ) == 0 )
        {
            Blueprint.ReadFields();
            s32 nType = -1;
            char cType[MAX_PATH];
            if (Blueprint.GetString("SType", cType)) //New Way
            {
                guid ObjGuid = g_ObjMgr.CreateObject(cType);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
                if (pObject && pObject->GetTypeDesc().QuickResourceName())
                {
                    const char* pQuickResourceName = pObject->GetTypeDesc().QuickResourceName();

                    if ( pQuickResourceName )
                    {
                        if ( x_stricmp(pQuickResourceName, "RigidGeom") == 0)
                        {
                            AddObjectToRender(Blueprint, strPath, anchorPos, TRUE);
                        }
                        else if ( x_stricmp(pQuickResourceName, "SkinGeom") == 0)
                        {
                            AddObjectToRender(Blueprint, strPath, anchorPos, FALSE);
                        }
                    }
                }
                g_ObjMgr.DestroyObjectEx(ObjGuid, TRUE);
            }
        }
    }

    Blueprint.CloseFile();
    return TRUE;

    x_catch_display;

    return FALSE;
}

//=========================================================================

void CEditorBlueprintView::AddObjectToRender( text_in& TextIn, CString strPath, const vector3& Offset, BOOL bIsRigid )
{
    TextIn.ReadHeader();
    CString RescFile;
    vector3 Position;
    radian3 Rotation;

    if( x_strcmp( TextIn.GetHeaderName(), "Properties" ) != 0 )
        x_throw( "Unable to load the properties in the file.\nCould not find the right header" );

    // Okay now lets go for it
    s32 Count = TextIn.GetHeaderCount();
    for( s32 i=0; i<Count; i++ )
    {
        TextIn.ReadFields();

        char Name [256];
        char Type [256];
        char Value[256];
        TextIn.GetField( "Name:s",  Name   );
        TextIn.GetField( "Type:s",  Type   );
        TextIn.GetField( "Value:s", Value  );

        if (x_stricmp( Name, "Base\\Position") == 0)
        {
            if( x_strcmp( Type, "VECTOR3") == 0 )
            {
                sscanf( Value, "%f %f %f", &Position.GetX(), &Position.GetY(), &Position.GetZ() );
            }
        }
        else if (x_stricmp( Name, "Base\\Rotation") == 0)
        {
            if( x_strcmp( Type, "ROTATION") == 0 )
            {
                sscanf( Value, "%f %f %f", &Rotation.Roll, &Rotation.Pitch, &Rotation.Yaw );

                Rotation.Roll  = DEG_TO_RAD(Rotation.Roll);
                Rotation.Pitch = DEG_TO_RAD(Rotation.Pitch);
                Rotation.Yaw   = DEG_TO_RAD(Rotation.Yaw);
            }
        }
        else if (x_stricmp( Name, "RenderInst\\File") == 0)
        {
            if( x_strcmp( Type, "EXTERNAL") == 0 )
            {                
                char cThemePath[MAX_PATH];
                CString strThemePath;
                CFileSearch fSearch;
                fSearch.Recurse(TRUE);
                //get all resource paths included to search through
                for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
                {
                    fSearch.AddDirPath(cThemePath);
                    fSearch.GetDirs(cThemePath);
                }

                if (bIsRigid)
                {
                    fSearch.GetFiles("*.RigidGeom");
                }
                else
                {
                    fSearch.GetFiles("*.SkinGeom");
                }
                
                CString strNextFile = fSearch.GetNextFile();
                while (!strNextFile.IsEmpty())
                {   
                    CString strName = strNextFile.Right(strNextFile.GetLength() - strNextFile.ReverseFind('\\') - 1);
                    if (strName.CompareNoCase(Value)==0)
                    {
                        RescFile = strNextFile;
                        strNextFile.Empty();
                    }
                    else
                    {
                        strNextFile = fSearch.GetNextFile();
                    }
                }

/*
                xstring xstrTheme, xstrRelativePath;
                if (g_WorldEditor.GetThemeInfoFromPath( strPath, xstrTheme, xstrRelativePath ))
                {
                    CString strResPath = g_Project.GetResourceDirForTheme(xstrTheme);
                    CString strFile = strResPath + "\\" + CString(Value);
                    if (CFileSearch::DoesFileExist(strFile))
                    {
                        RescFile = strFile;
                    }
                    else
                    {
                        CFileSearch fSearch;
                        fSearch.Recurse(TRUE);
                        fSearch.GetDirs(strResPath);
                        CString strNextDir = fSearch.GetNextDir();
                        while (!strNextDir.IsEmpty())
                        {   
                            strFile = strNextDir + "\\" + CString(Value);
                            if (CFileSearch::DoesFileExist(strFile))
                            {
                                RescFile = strFile;
                                strNextDir.Empty();
                            }
                            else
                            {
                                strNextDir = fSearch.GetNextDir();
                            }
                        }
                    }
                }
*/
            }
        }
    }

    //now insert Object into render
    if (!RescFile.IsEmpty() && CFileSearch::DoesFileExist(RescFile))
    {
        if (bIsRigid)
        {
            m_wndPreview.LoadGeom("RigidGeom",RescFile,TRUE,(Position-Offset),Rotation);
        }
        else
        {
            m_wndPreview.LoadGeom("SkinGeom",RescFile,TRUE,(Position-Offset),Rotation);
        }
    }
}

//=========================================================================

void CEditorBlueprintView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    int nHt = cx/2;
	CSize size = m_wndToolBar.CalcLayout(LM_HORZ| LM_COMMIT,nHt);
	m_wndToolBar.MoveWindow(0,0,size.cx,size.cy);

    if (nHt > (cy/2)) nHt = cy/2;

    m_wndPreview.MoveWindow( size.cx, 0, cx - size.cx, nHt);

    if (size.cy > nHt) 
    {
        m_stPlaceholder.MoveWindow( size.cx, nHt, cx - size.cx, size.cy - nHt );
        nHt = size.cy;
    }
    else
    {
        m_stPlaceholder.MoveWindow( 0, size.cy, size.cx, nHt - size.cy );
    }

    m_fbcBlueprints.MoveWindow(0,nHt,cx,cy - nHt);
}

//=========================================================================

LRESULT CEditorBlueprintView::OnFileItemPreChange(WPARAM wParam, LPARAM lParam)
{
    CString strFrom((char*)wParam);
    CString strTo((char*)lParam);

    //just the names please
    CString strToShort = strTo.Right(strTo.GetLength() - strTo.ReverseFind('\\') - 1);
    CString strFromShort = strFrom.Right(strFrom.GetLength() - strFrom.ReverseFind('\\') - 1);

    //setup UNDO
    if (strFrom.IsEmpty())
    {
        //unknown
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry("Unknown Blueprint file Action"));
    }
    else if (strTo.IsEmpty())
    {
        //delete
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Deleting Blueprint File(%s)", (const char*)strFromShort)));
        transaction_file_data* pFileData = new transaction_file_data(
                                            transaction_file_data::TFILE_DELETE,
                                            xstring(strFrom), xstring(strTo));
        pFileData->StoreFileData(transaction_data::TRANSACTION_OLD_STATE);
        g_WorldEditor.AddStepToCurrentUndoEntry(pFileData);
    }
    else
    {
        //rename
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Rename Blueprint File(%s to %s)", (const char*)strFromShort, (const char*)strToShort)));
        transaction_file_data* pFileData = new transaction_file_data(
                                            transaction_file_data::TFILE_RENAME,
                                            xstring(strFrom), xstring(strTo));
        g_WorldEditor.AddStepToCurrentUndoEntry(pFileData);
    }
    return 1;
}

//=========================================================================

LRESULT CEditorBlueprintView::OnFileItemChange(WPARAM wParam, LPARAM lParam)
{
    CString strFrom((char*)wParam);
    CString strTo((char*)lParam);

    //ensure this was a file change
    CString strExt = strFrom.Right(4);
    if (strExt.CompareNoCase(".bpx") != 0)
    {
        //not a blueprint file, ignore it
        g_WorldEditor.ClearUndoEntry();
        return 0;
    }

    if (!strFrom.IsEmpty() &&
        (strFrom.CompareNoCase(strTo) != 0) &&
        GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        if (strTo.IsEmpty())
        {
            //hmmm deleting strFrom
            if (::AfxMessageBox("Do you also want to delete any objects that reference this blueprint?",MB_YESNO) == IDYES)
            {
                xstring xstrTheme, xstrRelativePath;
                if (g_WorldEditor.GetThemeInfoFromPath( strFrom, xstrTheme, xstrRelativePath ))
                {
                    xarray<editor_item_descript> lstItems;
                    xarray<editor_blueprint_placement> lstPlacement;
                    g_WorldEditor.DeleteBlueprintsWithFile(xstrTheme, xstrRelativePath, lstItems, lstPlacement);

                    for (int i=0; i < lstItems.GetCount(); i++)
                    {
                        editor_item_descript Item = lstItems.GetAt(i);
                        GetDocument()->GetFramePointer()->RemoveBlueprintFromLayerView(CString(Item.Layer),
                            CString(Item.LayerPath),Item.Guid);
                    }
                }
                else
                {
                    ::AfxMessageBox("Error::CEditorBlueprintView::OnFileItemChange::DELETE\n\nThis blueprint doesn't appear to exist within a theme!");       
                }
            }
        }
        else
        {
            //determine if this was a move
            CString strExtTo = strTo.Right(4);
            if (strExtTo.CompareNoCase(".bpx") != 0)
            {
                //must be a move
                CString strFileName = strFrom.Right(strFrom.GetLength() - strFrom.ReverseFind('\\') - 1);
                CFileSearch::FormatPath(strTo);
                strTo += "\\" + strFileName;
            }

            xstring xstrTheme, xstrRelativePath;
            xstring xstrNewTheme, xstrNewRelativePath;
            if (g_WorldEditor.GetThemeInfoFromPath( strFrom, xstrTheme, xstrRelativePath ) && 
                g_WorldEditor.GetThemeInfoFromPath( strTo, xstrNewTheme, xstrNewRelativePath ))
            {
                //object changing
                xarray<editor_item_descript> lstItems;
                g_WorldEditor.UpdateBlueprintsWithFile(xstrTheme, xstrRelativePath, xstrNewTheme, xstrNewRelativePath, lstItems);

                for (int i=0; i < lstItems.GetCount(); i++)
                {
                    editor_item_descript Item = lstItems.GetAt(i);
                    GetDocument()->GetFramePointer()->RemoveBlueprintFromLayerView(CString(Item.Layer),
                        CString(Item.LayerPath), Item.Guid);
                    GetDocument()->GetFramePointer()->AddBlueprintToLayerView(CString(Item.Layer),
                        CString(Item.LayerPath), Item.Guid);
                }
            }
            else
            {
                ::AfxMessageBox("Error::CEditorBlueprintView::OnFileItemChange::RENAME\n\nThis blueprint doesn't appear to exist within a theme!");       
            }
        }

        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        
        g_WorldEditor.CommitCurrentUndoEntry();
    }
    else
    {
        g_WorldEditor.ClearUndoEntry();
    }

    return 1;
}

//=========================================================================

void CEditorBlueprintView::OnBptbRefresh()
{
/*
    if (::AfxMessageBox("Are you sure you want to reload all objects?\n(current objects will not be saved)",MB_YESNO) == IDYES)
    {
        GetDocument()->GetFramePointer()->ReloadCurrentLevel();
        BuildTreeFromProject();
    }
*/
    BuildTreeFromProject();
    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
}

//=========================================================================

void CEditorBlueprintView::OnBptbReplaceSelectedWithBlueprint()
{
    s32 nCount = g_WorldEditor.GetSelectedCount();
    if (nCount > 0)
    {
        CString strPath = m_fbcBlueprints.GetSelectedPath();
        xstring xstrTheme, xstrRelativePath;
        if (g_WorldEditor.GetThemeInfoFromPath( strPath, xstrTheme, xstrRelativePath ))
        {
            if (::AfxMessageBox(xfs("Are you sure you want to replace %d objects with Blueprint (%s)?",nCount, (const char*)strPath),MB_YESNO) == IDYES)
            {
                g_WorldEditor.UpdateSelectedObjsWithBlueprint(xstrTheme, xstrRelativePath);
            }
        }    
    }
}

//=========================================================================

void CEditorBlueprintView::OnBptbCreateAnchor() 
{
    if (GetDocument()->GetFramePointer())
    {
        //setup generic undo entry for moving of objects
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry("Creating Blueprint Anchor"));

        guid Guid = g_WorldEditor.CreateBlueprintAnchor();

        g_WorldEditor.SelectObject(Guid);
        GetDocument()->GetFramePointer()->GetEditorView()->EnterMovementMode();
        //for UNDO, must be done after entering movement mode, force UNDO commit after moving
        GetDocument()->GetFramePointer()->GetEditorView()->ForceMovementUndoRecording();
    }
}

//=========================================================================

void CEditorBlueprintView::OnBptbCreateBlueprint() 
{
    if (g_WorldEditor.CanMakeBlueprintFromSelected())
    {
        if (GetDocument()->GetFramePointer() &&
            GetDocument()->GetFramePointer()->GetEditorDoc())
        {
            CListBoxDlg dlg;
            CString strTheme;
            dlg.SetDisplayText("Into which of the following themes do you wish to add the blueprint?");
    
            dlg.AddString(g_Project.GetName());
            for (int i = 0; i < g_Project.GetThemeCount(); i++)
            {
                dlg.AddString(g_Project.GetThemeName(i));
            }

            if (dlg.DoModal() == IDOK)
            {
                strTheme = dlg.GetSelection();
            }

            if (strTheme.IsEmpty())
            {
                //cancel add
                return; 
            }

	        CFileDialog		dlgBrowse(	FALSE, 
								        _T("bpx"), 
								        xfs("%d_Objects.bpx",g_WorldEditor.GetSelectedCount()), 
								        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
								        (_T("Blueprint Files (*.bpx)|*.bpx||")));
            CString strBPPath = g_Project.GetBlueprintDirForTheme(strTheme);
            dlgBrowse.m_ofn.lpstrInitialDir = strBPPath;
	        if (dlgBrowse.DoModal() == IDOK)
	        {
                //setup generic undo entry for moving of objects
                g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
                    xfs("Creating Blueprint(%s) from %d Objects",
                    dlgBrowse.GetFileName(),
                    g_WorldEditor.GetSelectedCount())));

		        CString strPath = dlgBrowse.GetPathName( );
                xstring xstrTheme, xstrRelativePath;
                if (g_WorldEditor.GetThemeInfoFromPath( strPath, xstrTheme, xstrRelativePath ))
                {
                    g_WorldEditor.SaveSelectedObjectsAsBlueprint(xstrTheme, xstrRelativePath);

        //BEGIN REF CHANGE SECTION
                    // this code will delete the objects and replace them with a blueprint... 
                    xarray<editor_item_descript> lstItems;

                    //delete the objects
                    g_WorldEditor.DeleteSelectedObjects(lstItems);
                    for (int i=0; i<lstItems.GetCount(); i++)
                    {
                        editor_item_descript Description = lstItems.GetAt(i);
                        CString strName;
                        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
                        if (Description.IsInBlueprint)
                        {
                            GetDocument()->GetFramePointer()->RemoveBlueprintFromLayerView(strName,
                                CString(Description.LayerPath), Description.Guid);
                        }
                        else
                        {
                            GetDocument()->GetFramePointer()->RemoveObjectFromLayerView(strName,
                                CString(Description.LayerPath), Description.Guid);
                        }
                    }

                    //add objects as blueprint
                    editor_blueprint_ref BlueprintReference;
	                int nAdded = g_WorldEditor.AddBlueprint(xstrTheme, xstrRelativePath, BlueprintReference, TRUE, TRUE);
                    if (nAdded>0)
                    {
                        GetDocument()->GetFramePointer()->AddBlueprintToActiveLayerView(BlueprintReference.Guid);
                    }
        //END REF CHANGE SECTION

                    //now just to make sure, lets update all blueprints within the level
                    xarray<editor_item_descript> lstNewBlueprints;
                    xarray<editor_blueprint_placement> lstPlacements;
                    g_WorldEditor.DeleteBlueprintsWithFile(xstrTheme, xstrRelativePath, lstNewBlueprints, lstPlacements);

                    for (s32 j=0; j < lstNewBlueprints.GetCount(); j++)
                    {
                        editor_item_descript& NewBpItem = lstNewBlueprints.GetAt(j);
                        editor_blueprint_placement& NewBpPlacement = lstPlacements.GetAt(j);
                        editor_blueprint_ref BPNewRef;
                        g_WorldEditor.AddBlueprintToSpecificLayer(xstrTheme, xstrRelativePath, 
                                                                  NewBpItem.Layer, NewBpPlacement.LayerPath,
                                                                  BPNewRef, TRUE, TRUE,
                                                                  NewBpPlacement.PrimaryGuid);
                        g_WorldEditor.RotateSelectedObjects( NewBpPlacement.Transform );
                        g_WorldEditor.MoveBlueprintObjectsToAnchor( NewBpPlacement.Position );
                    }

                    //Commit the Undo
                    g_WorldEditor.CommitCurrentUndoEntry();           

                    //reselect original BP
                    g_WorldEditor.SelectBlueprintObjects(BlueprintReference,FALSE);

                    m_fbcBlueprints.Refresh();
	            }	
            }
        }
        else
        {
            ASSERT(FALSE);
        }
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    }
    else
    {
        ::AfxMessageBox("In order to create a blueprint you must have 1 and only 1 blueprint anchor also selected. The blueprint anchor will be used as the origin point for the new blueprint.\n\nAlso you must not have any Global Only objects such as Nav Nodes or Nav Connections.");
    }
}

//=========================================================================

void CEditorBlueprintView::OnBptbShatterBlueprint()
{
    xarray<editor_item_descript> lstItemsAdded;
    xarray<editor_item_descript> lstItemsRemoved;

    g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Shatter selected blueprints")));
	g_WorldEditor.ShatterSelectedBlueprints(lstItemsAdded, lstItemsRemoved);
    g_WorldEditor.CommitCurrentUndoEntry();

    //remove blueprints from layer view
    for (int i=0; i<lstItemsRemoved.GetCount(); i++)
    {
        editor_item_descript Description = lstItemsRemoved.GetAt(i);
        CString strName;
        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
        GetDocument()->GetFramePointer()->RemoveBlueprintFromLayerView(strName,
            CString(Description.LayerPath), Description.Guid);
    }

    //add new objects to layer view
    for (i=0; i<lstItemsAdded.GetCount(); i++)
    {
        editor_item_descript Description = lstItemsAdded.GetAt(i);
        CString strName;
        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
        GetDocument()->GetFramePointer()->AddObjectToLayerView(strName,
            CString(Description.LayerPath), Description.Guid);
    }
    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorBlueprintView::OnBptbShatterBlueprintForEdit()
{
    xarray<editor_item_descript> lstItemsAdded;
    xarray<editor_item_descript> lstItemsRemoved;


    editor_blueprint_ref BPRef;
    if (g_WorldEditor.IsOneBlueprintSelected(BPRef))
    {
        xstring xstrName;
        g_WorldEditor.GetDisplayNameForBlueprint(BPRef,xstrName);
        CString strName(xstrName);
        if (strName.ReverseFind('\\') != -1)
        {
            strName = strName.Right(strName.GetLength() - strName.ReverseFind('\\') - 1);
        }
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Shatter blueprint(%s) for Edit", (const char*)strName)));
    }
    else
    {
        return;
    }
    
    matrix4 m4Invert = BPRef.Transform;
    m4Invert.Invert();

    g_WorldEditor.RotateSelectedObjects(m4Invert);
	g_WorldEditor.ShatterSelectedBlueprints(lstItemsAdded, lstItemsRemoved);
    g_WorldEditor.CommitCurrentUndoEntry();

    //remove blueprints from layer view
    for (int i=0; i<lstItemsRemoved.GetCount(); i++)
    {
        editor_item_descript Description = lstItemsRemoved.GetAt(i);
        CString strName;
        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
        GetDocument()->GetFramePointer()->RemoveBlueprintFromLayerView(strName,
            CString(Description.LayerPath), Description.Guid);
    }

    //add new objects to layer view
    for (i=0; i<lstItemsAdded.GetCount(); i++)
    {
        editor_item_descript Description = lstItemsAdded.GetAt(i);
        CString strName;
        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
        GetDocument()->GetFramePointer()->AddObjectToLayerView(strName,
            CString(Description.LayerPath), Description.Guid);
    }
    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorBlueprintView::OnBptbResync()
{
    editor_blueprint_ref BPRef;
    if (g_WorldEditor.IsOneBlueprintSelected(BPRef))
    {
        xstring xstrName;
        g_WorldEditor.GetDisplayNameForBlueprint(BPRef,xstrName);
        CString strName(xstrName);
        if (strName.ReverseFind('\\') != -1)
        {
            strName = strName.Right(strName.GetLength() - strName.ReverseFind('\\') - 1);
        }

        xstring xstrLayer = g_WorldEditor.GetLayerContainingBlueprint(BPRef.Guid);
        if (g_WorldEditor.IsLayerReadonly(xstrLayer))
        {
            ::AfxMessageBox("Blueprints layer is NON-EDITABLE.");
            return;
        }

        object *pObjectAnchor = g_ObjMgr.GetObjectByGuid( BPRef.Anchor );
        vector3 Pos(0.0f, 0.0f, 0.0f);
        if (pObjectAnchor)
        {
            Pos = pObjectAnchor->GetPosition();
        }

        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Resync blueprint(%s)", (const char*)strName)));

        xarray<editor_item_descript> lstItems;
        g_WorldEditor.DeleteSelectedObjects(lstItems);

        //all of the following functions handle there respective undos
        editor_blueprint_ref BPNew;
        g_WorldEditor.AddBlueprintToSpecificLayer(
            BPRef.ThemeName,
            BPRef.RelativePath,
            xstrLayer,
            BPRef.LayerPath,
            BPNew,
            TRUE,
            TRUE,
            BPRef.Guid);

        g_WorldEditor.RotateSelectedObjects( BPRef.Transform.GetRotation() );
        g_WorldEditor.MoveBlueprintObjectsToAnchor( Pos );

        //last part of undo
        g_WorldEditor.CommitCurrentUndoEntry();  
    }

    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
}

//=========================================================================

void CEditorBlueprintView::OnBptbAddBlueprintAsObjects() 
{
    CString strPath = m_fbcBlueprints.GetSelectedPath();

    xstring xstrTheme, xstrRelativePath;
    if (g_WorldEditor.GetThemeInfoFromPath( strPath, xstrTheme, xstrRelativePath ))
    {
        xarray<guid> lstGuids;

        //setup generic undo entry for adding of objects
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Adding Objects From Blueprint(%s)", (const char*)strPath)));
        s32 iAdded = g_WorldEditor.AddBlueprintAsObjects(xstrTheme, xstrRelativePath, lstGuids);
        if (iAdded>0)
        {
            for (int i=0; i<lstGuids.GetCount();i++)
            {
                GetDocument()->GetFramePointer()->AddObjectToActiveLayerView(lstGuids.GetAt(i));
            }
            GetDocument()->GetFramePointer()->GetEditorView()->EnterMovementMode();
            //for UNDO, must be done after entering movement mode, force UNDO commit after moving
            GetDocument()->GetFramePointer()->GetEditorView()->ForceMovementUndoRecording();
        }
    }
    else
    {
        ::AfxMessageBox("Error::CEditorBlueprintView::OnBptbAddBlueprintAsObjects\n\nThis blueprint doesn't appear to exist within a theme!");       
    }

    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorBlueprintView::OnBptbAddBlueprint() 
{
    CString strPath = m_fbcBlueprints.GetSelectedPath();

    if (GetDocument()->GetFramePointer()->GetEditorView()->IsBlueprintMode())
    {
        GetDocument()->GetFramePointer()->GetEditorView()->CancelBlueprintMode();
    }
    else
    {
        xstring xstrTheme, xstrRelativePath;
        if (g_WorldEditor.GetThemeInfoFromPath( strPath, xstrTheme, xstrRelativePath ))
        {
            //add to null layer so this is just temporary
	        int nAdded = g_WorldEditor.CreateTemporaryBlueprintObjects(xstrTheme, xstrRelativePath);
            if (nAdded>0)
            {
                GetDocument()->GetFramePointer()->GetEditorView()->EnterBlueprintMode();
            }
        }
        else
        {
            ::AfxMessageBox("Error::CEditorBlueprintView::OnBptbAddBlueprint\n\nThis blueprint doesn't appear to exist within a theme!");       
        }
    }
    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus(); //SetFocus to 3D View
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

BOOL CEditorBlueprintView::IsBlueprintAddable(CEditorFrame *pFrame) 
{
    if (g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()))
    {
        return FALSE;
    }

    if (pFrame && pFrame->GetEditorView())
    {
        if (pFrame->GetEditorView()->IsStandardMode())
        {
            CString strPath = m_fbcBlueprints.GetSelectedPath();
            //not empty, not folder
            if (!strPath.IsEmpty() && !m_fbcBlueprints.IsFolder(strPath))
            {
                int iIndex = strPath.ReverseFind('.');
                CString strExtention = strPath.Right(strPath.GetLength()-iIndex-1);
                if (strExtention.CompareNoCase("bpx")==0)
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//=========================================================================

void CEditorBlueprintView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Blueprints");
        BuildTreeFromProject();
    }
    else
    {
        m_wndPreview.OnStopTimer();
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbCreateBlueprint(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbCreateBlueprint(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbCreateAnchor(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbCreateAnchor(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbAddBlueprintAsObjects(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbAddBlueprintAsObjects(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbShatterBlueprint(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbShatterBlueprint(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbRefresh(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbRefresh(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbShatterBlueprintForEdit(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbShatterBlueprintForEdit(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbAddBlueprint(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateBptbAddBlueprint(pCmdUI);
    }
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbReplaceSelectedWithBlueprint(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;
    if ( g_Project.IsProjectOpen() && 
        (g_WorldEditor.GetSelectedCount() > 0) &&
        GetDocument() && GetDocument()->GetFramePointer() && 
        GetDocument()->GetFramePointer()->GetEditorView()->IsStandardMode() &&
        !GetDocument()->GetFramePointer()->GetEditorDoc()->IsGameRunning())
    {
        CString strPath = m_fbcBlueprints.GetSelectedPath();
        //not empty, not folder
        if (!strPath.IsEmpty() && !m_fbcBlueprints.IsFolder(strPath))
        {
            bEnable = TRUE;
        }
    }
    else 
    {
    }
    pCmdUI->Enable(bEnable);	
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorBlueprintView::OnUpdateBptbResync(CCmdUI* pCmdUI) 
{
    editor_blueprint_ref BPRef;
    if (GetDocument() && GetDocument()->GetFramePointer() && 
        GetDocument()->GetFramePointer()->GetEditorDoc() &&
        GetDocument()->GetFramePointer()->GetEditorView()->IsStandardMode() &&
        !GetDocument()->GetFramePointer()->GetEditorDoc()->IsGameRunning() &&
        g_Project.IsProjectOpen() && 
        g_WorldEditor.IsOneBlueprintSelected(BPRef))
    {
        pCmdUI->Enable(TRUE);	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}