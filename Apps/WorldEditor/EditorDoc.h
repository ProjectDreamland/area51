// EditorDoc.h : interface of the CEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITORDOC_H__82BA1788_655F_4269_BCDA_AF7C5F43E1D9__INCLUDED_)
#define AFX_EDITORDOC_H__82BA1788_655F_4269_BCDA_AF7C5F43E1D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EditorDocGridMngr.h"
#include "EditorDocInputMngr.h"

#include "..\Editor\BaseDocument.h"
#include "..\MiscUtils\SimpleUtils.hpp"

#include <afxmt.h>

//=========================================================================

class CEditorView;
class CEditorDoc;
class ptr_list;
class transaction_entry;
class view_favorite;

//=========================================================================
//=========================================================================

class CEditorHandler : public editor_handler
{
public:
    CEditorHandler();

    virtual xbool                   HandleExternal( xstring &xstrType, xstring &xstrValue ); 
    virtual void                    SetProgressRange( s32 Min, s32 Max );                      
    virtual void                    SetProgress( s32 Pos );           
            void                    PrintLog    ( const char* pMessage );
            void                    SetDoc(CEditorDoc* pDoc) { m_pDoc = pDoc; }
    
    //additional methods
    virtual void                    AlterGridColor( xcolor Color );
    virtual void                    RefreshGlobals(); 
    virtual void                    RefreshWatches();
    virtual void                    SetLayerDirty( const xstring &xstrLayer );
    virtual void                    ForceLayerUpdate( );
    virtual void                    ShowWarning( xstring &xstrWarning );
    virtual xbool                   IsFileReadOnly( const char* pFile );
    virtual void                    ChangeSelectObjectsZone();
    virtual xbool                   IsZoneLoaded( const xstring &xstrLayer, const xstring &xstrZone );

private:

    CEditorDoc*                     m_pDoc;
};

//=========================================================================
//=========================================================================

class string_match
{
public:
    CString String1;
    CString String2;
};

//=========================================================================
//=========================================================================

class theme_importer : public prop_interface
{
public:
                        theme_importer    ( void ) {};

    virtual void        OnEnumProp          ( prop_enum&  List );
    virtual xbool       OnProperty          ( prop_query& I    );

    xarray<xstring>     m_ThemePaths;
};
//bjt
struct user_settings{
    
    //camera
    xbool   UpdateCameraFlag;
    vector3 CameraPos;
    radian  Pitch;
    radian  Yaw;
  
    //layers
    xbool   UpdateLayers;
    xarray <xstring> UnloadedLayers;
    xarray <guid> HiddenObjects;
    xarray <guid> UnSelectAbleObjects;
    xarray <view_favorite> ViewFavorites;

    //Tree View
    xarray <xstring> TreeView;
    int ScrollBarPos;
    //compile options
    BOOL UpdateCompileButtonsFlag;
    BOOL CompilePC;
    BOOL CompilePS2;
    BOOL CompileXBOX;
    BOOL CompileGCN;
};


//=========================================================================

extern xbool g_first_person;

//=========================================================================

class CEditorDoc : public CBaseDocument
{
public:
    BOOL            SaveUserSettings(void);
    BOOL            LoadUserSettings(void);

protected: // create from serialization only
	CEditorDoc();
	DECLARE_DYNCREATE(CEditorDoc)
    CEditorView* GetView();

    void            ReloadLevel(CString strLevel);

    virtual void    OnProjectOpen();
    virtual void    OnProjectClose();
    virtual void    OnProjectSave();
    virtual void    OnProjectNew();
    virtual void    OnProjectImport();

    void            RunGame          ( void );
    void            StopGame         ( void );
    void            PauseGame        ( BOOL bPause );
    void            StepGame         ( void );
    void            SaveGame         ( void );
    void            LoadGame         ( void );
    BOOL            IsGameRunning    ( void );
    BOOL            IsGamePaused     ( void );
    void            AdvanceLogic     ( void );
    
    BOOL            IsFPV            ( void )       { return m_bIsFPV; }
    void            SetFPV           ( BOOL bFPV )  
    { 
        m_bIsFPV = bFPV; 

        if (m_bIsFPV)
            g_first_person = TRUE;
        else
            g_first_person = FALSE;
    }
    
    void            InitSettingsInterface();

    s32             GetGridSnap() { return m_GridSettings.m_nGridSnap; }
    void            SetGridSnap(s32 Size) { m_GridSettings.m_nGridSnap = Size; }
    s32             GetRotateSnap() { return m_GridSettings.m_nRotateSnap; }
    BOOL            IsSchematicLoaded() { return m_GridSettings.m_bImageLoaded; }
    vector3         GetGridPos() { return m_GridSettings.m_fGridLocation; }
    void            SetGridPos( const vector3& v3 );
    f32             GetFarZLimit() { return m_GridSettings.m_fFarZLimit; }
    void            SetFarZLimit( f32 fLimit ) { m_GridSettings.m_fFarZLimit = fLimit; }
    s32             GetMovementSpeed() { return m_GridSettings.m_nSpeed; }
    f32             GetTimeDilation() { return m_GridSettings.m_fTimeDilation; }
    void            SetGridPreset(s32 iIndex);
    void            SetRotatePreset(s32 iIndex);
    void            IncrementGridSnap(void);
    void            DecrementGridSnap(void);
    void            IncrementRotateSnap(void);
    void            DecrementRotateSnap(void);
    xbool           IsBaselineGridVisible(void) { return m_GridSettings.m_bShowBaselineGrid; }

    xcolor          GetBackgroundColor() { return m_GridSettings.m_xcBackground; }
    xbitmap&        GetSchematic() { return m_GridSettings.m_xbmpImage; }
    s32             GetSchematicScale() { return m_GridSettings.m_nImageScale; }
    s32             GetSchematicAlpha() { return m_GridSettings.m_nImageAlpha; }
    BOOL            DrawSchematic() { return m_GridSettings.m_bImageDraw; }
    BOOL            ShowEngineStats() { return m_GridSettings.m_bShowStats; }

    BOOL            LoadLayerFile(CString strLayer);
    BOOL            LoadLayerFile(CString strLayer, CString strFullPath);
    BOOL            UnloadLayerFile(CString strLayer);
    void            GetUserErrorPath(CString *strPathFileName, BOOL CheckDirectoryFlag);

    BOOL            CheckForDuplicateGuidsInLayer(int LayerIndex, int StartGuidIndex, guid CheckGuid);
    BOOL            CheckForDuplicateGuidsInAllLayers(void);


//    void            CreateStaticDecal   ( vector3 Begin, vector3 End );

    void            RefreshPropertyView();
    //Load and save user settings functions
    BOOL            LoadUserSettingsCamera( CString FileName);
    BOOL            LoadUserSettingsUnloadedLayers( CString FileName );
    BOOL            LoadUserSettingsHiddenObjects( CString FileName );
    BOOL            LoadUserSettingsUnSelectAbleObjects( CString FileName );
    int             BuildListOfHiddenObjects( void );
    int             BuildListOfUnSelectableObjects( void );
    void            UpdateHiddenObjects(void);
    void            UpdateUnSelectAbleObjects(void);
    BOOL            HasUserSettingsChanged(void);
    int             UpdateViewFavorites( void );
    int             BuildListOfViewFavorites( void );
    int             LoadUserViewFavorites( CString FileName);
    void            GetUserSettingsPath(CString *FileName, BOOL CheckDirectoryFlag); 
    int             BuildTreeViewList( void );
    BOOL            LoadUserOpenTree( CString FileName);
    int             UpdateTreeView( void );
    int             BuildCompileOptions( void );
    int             UpdateCompileOptions( void );
    int             LoadUserCompileOptions( CString FileName );







// Attributes
public:
    BOOL                m_bGameRunning;
    BOOL                m_bGamePaused;
    BOOL                m_bGameStep;
    BOOL                m_bWantToStop;
    f32                 m_TimeRunning;
    BOOL                m_bIsFPV;
    xtimer              m_Timer;

    CMutex              m_Mutex;
    CSingleLock         m_MutexLock;

// Operations
public:
    
    void            PrintLog            ( const char* pMessage );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CEditorDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void CopyPath(CString strFromPath, CString strWildcard, CString strToPath);

private:
    xarray<string_match> m_xaBlueprintPathUpdates;
    CEditorHandler*      m_pHandler;

    CGridSettings        m_GridSettings;
    CInputSettings       m_InputSettings;
};

inline CEditorView* CEditorDoc::GetView()
{ 
	POSITION pos = GetFirstViewPosition();
	return (CEditorView*)GetNextView (pos); 
}

extern user_settings            g_SaveTrackUserSettings;
extern user_settings            g_LoadUpdateUserSettings;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORDOC_H__82BA1788_655F_4269_BCDA_AF7C5F43E1D9__INCLUDED_)
