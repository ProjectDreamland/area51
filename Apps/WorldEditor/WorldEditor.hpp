#ifndef WORLD_EDITOR_HPP
#define WORLD_EDITOR_HPP

#include "x_files.hpp"
#include "Objects\Object.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"



class ptr_list;
class transaction_entry;
class transaction_data;
class rigid_inst;
class static_decal;


struct guid_map {
    guid NewGuid;
    guid OldGuid;
};
struct RigidGeomData {
    u16     uCount;
    xbool   bRegistered;
    xstring xstrName;
    s32     nVerts;
    s32     nPolys;
    s32     nMaterials;
    s32     nTextures;
    s32     nMeshes;
    s32     nSubMeshes;
};

//=========================================================================
//=========================================================================

class editor_handler
{
public:
    virtual xbool                   HandleExternal( xstring &xstrType, xstring &xstrValue ) = 0; 
    virtual void                    SetProgressRange( s32 Min, s32 Max ) = 0;                      
    virtual void                    SetProgress( s32 Pos ) = 0;       
   
    //non-required
    virtual void                    RefreshGlobals()                    {};
    virtual void                    RefreshTriggerView()                {};
    virtual void                    SetLayerDirty( const xstring &xstrLayer ) {};
    virtual void                    ForceLayerUpdate( )                 {};
    virtual void                    ShowWarning( xstring &xstrWarning ) {};
    virtual xbool                   IsFileReadOnly( const char* pFile ) { return TRUE; }
    virtual void                    AlterGridColor( xcolor Color )      {};
    virtual void                    ChangeSelectObjectsZone()           {};
    virtual xbool                   IsZoneLoaded( const xstring &xstrLayer, const xstring &xstrZone ) {return FALSE;}
};

//=========================================================================

class editor_item_descript
{
public:
    guid                            Guid;
    xstring                         Layer;
    xstring                         LayerPath;
    xbool                           IsInBlueprint;
};

//=========================================================================

class editor_blueprint_placement
{
public:
    xstring                         ThemeName;
    xstring                         RelativePath;
    xstring                         LayerPath;
    vector3                         Position;
    matrix4                         Transform;
    guid                            PrimaryGuid;
};

//=========================================================================

class editor_blueprint_ref
{
public:
    editor_blueprint_ref() { Transform.Identity(); LayerPath = "\\"; }

    guid                            Guid;
    xstring                         ThemeName;
    xstring                         RelativePath;
    xstring                         LayerPath;
    xarray<guid>                    ObjectsInBlueprint;
    matrix4                         Transform;

    guid                            Anchor;

    vector3     GetAnchorPosition   ( void );
    bbox        ComputeBBox         ( void );
};

//=========================================================================

class editor_object_ref
{
public:
    editor_object_ref() { LayerPath = "\\"; }

    guid                            Guid;
    xstring                         LayerPath;
};

//=========================================================================

class editor_zone_ref : public prop_interface
{
public:
    editor_zone_ref( void );

    u8                              Id;
    xstring                         Name;
    xstring                         Layer;
    f32                             SndAbsorption;
    s32                             MinPlayers;
    s32                             MaxPlayers;
    char                            EnvMap[256];
    char                            FogMap[256];
    xbool                           QuickFog;
    
    virtual void        OnEnumProp  ( prop_enum&    List );
    virtual xbool       OnProperty  ( prop_query&   I    );
};

//=========================================================================

class editor_layer
{
public:
    editor_layer();

    xstring                         Name;
    xarray<editor_object_ref>       Objects;
    xarray<editor_blueprint_ref>    Blueprints;
    xarray<xstring>                 Resources;
    xbool                           IsDirty;
    xbool                           IsLoaded;
    xbool                           IsEditable;
    xbool                           IsNull;         //for temp layer

public:
    xbool AddObject                 ( editor_object_ref Object ); 
    xbool RemoveObject              ( guid ObjectGuid ); 
    xbool AddBlueprint              ( const editor_blueprint_ref& Blueprint ); 
    xbool RemoveBlueprint           ( guid BlueprintGuid ); 

    xbool AddResource               ( xstring ResourceName ); 
    xbool RemoveResource            ( xstring ResourceName ); 
};

//=========================================================================

class editor_state_ref
{
public:
    editor_state_ref()              {};

    const char*                     pObjectType;
    guid                            ObjGuid;
    xarray<prop_container>          ObjProps;
};

//=========================================================================

class editor_dif_prop_list
{
public:
    editor_dif_prop_list()          {};
    void                            OnSave      ( text_out& TextOut );
    void                            OnLoad      ( text_in& TextIn );

    guid                            m_ObjGuid;
    xarray<prop_container>          m_ObjProps;
};

//=========================================================================

struct property_error
{
    guid    m_ObjectGuid;   // Guid of object
    xstring m_ErrorMsg;     // Error message
};

//=========================================================================
class world_editor : public prop_interface
{
public:
    world_editor();
    ~world_editor();

    virtual void        OnEnumProp                      ( prop_enum&    List );
    virtual xbool       OnProperty                      ( prop_query&   I    );
    virtual xbool       ExternalProperties              ( prop_query&   I, prop_interface& PropInterface );

    f32                 GetVersionNumber                ( void );
    s32                 GetTotalObjectCount             ( void );
    void                SetExternalHandler              ( editor_handler* pHandler );
    void                AdvanceLogic                    ( f32 DeltaTime );
    s32                 GetObjectCount                  ( void );

    //globals
    xbool               IsGlobalInUse                   (const char* pGlobal);

    //undo redo
    void                ClearUndoList                   ( void );
    void                ClearUndoEntry                  ( void );
    void                UndoBeginSelObjectsPropsChange  ( void );
    void                UndoCommitSelObjectsProps       ( void );
    void                SetCurrentUndoEntry             ( transaction_entry* pEntry );
    void                AddStepToCurrentUndoEntry       ( transaction_data* pData );
    void                CommitCurrentUndoEntry          ( void );
    xbool               InTransaction                   ( void );

    //creating, loading, and saving of levels
    xbool               PreInitialize                   ( void );
    xbool               PostInitialize                  ( void );
    void                ClearLayers                     ( void );
    xbool               LoadLayer                       ( const char* pFileName, const char* pName, xarray<editor_blueprint_placement>& BPFiles, xarray<editor_dif_prop_list>& BPDiffs );
    xbool               UnLoadLayer                     ( const char* pName );
    xbool               SaveLayers                      ( const char* pPath );
    void                MarkLayerLoaded                 ( const char* pName, xbool bLoaded );
    xbool               IsLayerLoaded                   ( const char* pName );
    void                BuildZoneList                   ( void );
    void                ComputeSoundPropagation         ( void );

    //exporting data
    xbool               ExportToLevel                   ( const char* pName );
    void                ExportToLevelTo3dMax            ( const char* pName );
    void                CollectGuidsToExport            ( xarray<guid>& lstGuidsToExport );
    void                CollectPlaySurfacesToExport     ( xarray<guid>& lstPlaySurfaces );
    void                CollectDecalsToExport           ( xarray<guid>& lstDecals );
    xbool               ExportSanityCheck               ( void );
    void                RefreshResources                ( void );
    void                ForceRscRefresh                 ( void ) { m_bRefreshRsc = TRUE; }
    xbool               ShouldRscRefresh                ( void ) { return m_bRefreshRsc; }

    //dfs file specific
    void                CreateDFSFile                   ( const char* pFilename );
    xbool               BuildDFS                        ( const char* pFilename );
    void                MakeRegisteredRigidList         ( xarray<RigidGeomData> &RigidGeoms);
    xbool               BuildDFSFile                    ( const char* pExportName,
                                                          const char* pReleasePath,
                                                          const char* pPlatformString,
                                                          platform     PlatformType
                                                         );

    //initial state options
    void                StoreState                      ( xbool bDynamicOnly );
    void                ResetState                      ( xbool bDynamicOnly );

    //geom info
    rigid_inst*         GetRigidInstForObject           ( object* pObject );

    //layer controls
    xbool               SetActiveLayer                  ( const char* pLayer, const char* pLayerPath );
    const char*         GetActiveLayer                  ( void );
    const char*         GetActiveLayerPath              ( void );
    const char*         GetDefaultLayer                 ( void );
    const char*         GetGlobalLayer                  ( void );
    xbool               DoesLayerExist                  ( const char* pLayer );
    xbool               DoesLayerContainZones           ( const char* pLayer );
    xbool               AddLayer                        ( const char* pLayer, xbool bIsDirty );
    xbool               RemoveLayer                     ( const char* pLayer );
    xbool               RenameLayer                     ( const char* pOldName, const char* pNewName );
    void                GetLayerNames                   ( xarray<xstring>& List );
    const char*         GetLayerContainingBlueprint     ( guid BlueprintGuid );
    const char*         GetLayerContainingObject        ( guid ObjectGuid );
    void                SelectAllItemsInLayer           ( const char* pLayer );        
    xbool               IsLayerDirty                    ( const char* pLayer );   
    xbool               IsLayerReadonly                 ( const char* pLayer );   
    void                MarkLayerReadonly               ( const char* pLayer, xbool bReadonly );   
    s32                 GetDirtyLayerCount              ( void );
    void                DisableDirtyTracking            ( void );
    void                EnableDirtyTracking             ( void );
    void                MarkLayerDirty                  ( const char* pName );
    xbool               SetCurrentObjectsLayerAsActive  ( void );

    //lighting controls
    void                ComputeLightLayer               ( const char* pLayer, s32 iType );

    //resources
    xbool               IsResourceInLayer               ( const char* pRes, const char* pLayer );
    xbool               AddResourceToLayer              ( const char* pRes, const char* pLayer, xbool bIsDirty );
    xbool               RemoveResourceFromLayer         ( const char* pRes, const char* pLayer );
    xbool               GetResourcesInLayer             ( const char* pLayer, xarray<xstring>& List );

    //layer and objects
    s32                 ValidateObjectProperties        ( xarray<property_error>& Errors, const char* pLogChannel );
    xbool               RemoveObjectFromLayer           ( guid ObjectGuid , const char* pLayer );
    xbool               AddObjectToLayer                ( editor_object_ref ObjRef , const char* pLayer, xbool bIsDirty );
    xbool               RemoveBlueprintFromLayer        ( guid BlueprintGuid , const char* pLayer, xbool bIsDirty );
    xbool               AddBlueprintToLayer             ( const editor_blueprint_ref& Blueprint, const char* pLayer, xbool bIsDirty );
    xbool               AddObjectToActiveLayer          ( guid ObjectGuid );
    xbool               AddObjectToActiveLayer          ( editor_object_ref ObjRef );
    xbool               AddBlueprintToActiveLayer       ( const editor_blueprint_ref& Blueprint, xbool bIsDirty );
    xbool               GetObjectsInLayer               ( const char* pLayer, xarray<guid>& List );
    xbool               GetObjectsInLayer               ( const char* pLayer, xarray<editor_object_ref>& List );
    xbool               GetBlueprintsInLayer            ( const char* pLayer, xarray<editor_blueprint_ref>& List );
    void                SetObjectsLayerAsDirty          ( guid ObjectGuid );
    void                SetObjectLayerPath              ( guid ObjectGuid, const char* pLayer, const char* pLayerPath );
    void                SetBlueprintLayerPath           ( guid BPGuid, const char* pLayer, const char* pLayerPath );
    
    //zoning - note: id 0 is undefined zone (valid zones are 1 - 255)
    void                SelectZone                      ( const char* pZone );
    void                UnSelectZone                    ( void );
    const char*         GetZoneLayer                    ( const char* pZone );
    u8                  GetZoneId                       ( const char* pZone );
    u16                 GetZoneIdForPortal              ( const char* pPortalName );
    u8                  GetZoneCount                    ( void );
    const char*         GetZoneForId                    ( u8 uId ); 
    xbool               RenameZone                      ( const char* pOldName, const char* pNewName );
    xbool               DoesZoneExist                   ( const char* pZone );
    s32                 GetZoneIndex                    ( u8 uId ) const;
    xbool               DoesZoneExist                   ( u8 uId );
    xbool               CreateZone                      ( const char* pZone, const char* pLayer );
    xbool               DeleteZone                      ( const char* pZone );
    xbool               DeleteZone                      ( u8 uId );
    xbool               SaveZoneFile                    ( void );
    xbool               LoadZoneFile                    ( void );
    xbool               LoadZoneListFromFile            ( const char* pZoneFile, xarray<editor_zone_ref>& ZoneList );
    void                ClearZoneList                   ( void );
    xbool               SetObjectsZone                  ( guid ObjGuid, u8 Zone1, u8 Zone2 = 0 );
    xbool               SetBlueprintObjectsZone         ( guid BPGuid, u8 Zone1, u8 Zone2 = 0 );
    void                GetZoneList                     ( xarray<xstring>& List );
    void                GetZoneListForLayer             ( const char* pLayer, xarray<xstring>& List );
    void                ZoneSanityCheck                 ( void );
    void                GetListOfPortals                ( xarray<guid>& List );
    xbool               IsZoneFileEditable              ( void );

    //portals
    xbool               SavePortals                     ( xarray<editor_object_ref>& PortalList );
    xbool               CanAddRemovePortals             ( void );
    void                LoadPortals                     ( void );
    void                LoadPortals                     ( const char* pFileName );
    void                UpdateAllChildrenOfPortal       ( guid PortalGuid );
    void                RepathPortalChildren            ( const char* pOldName, const char* pNewName );
    xbool               RenamePortal                    ( guid ObjGuid, const char* pName );
    void                UpdatePlayerZone                ( void );

    //generic object functions
    bbox                GetObjectsBoundingBox           ( guid ObjectGuid );
    bbox                GetBlueprintsBoundingBox        ( editor_blueprint_ref& Blueprint );
    bbox                GetBlueprintsBoundingBox        ( guid BPGuid );

    //get munge data
    void                GetDisplayNameForObject         ( guid ObjectGuid, xstring& xstrName );
    void                GetDisplayNameForBlueprint      ( const editor_blueprint_ref& Blueprint, xstring& xstrName );
    void                GetDisplayNameForBlueprint      ( guid BlueprintGuid, xstring& xstrName );

    //handle temporary objects for placement
    xbool               CreateTemporaryObject           ( const char* pObjectType );
    xbool               CreateTemporaryObjectFromSel    ( void );
    xbool               CreateTemporaryBlueprintObjects ( const char* pTheme, const char* pRelPath );
    void                MoveTemporaryObjects            ( vector3& pos );
    void                ClearTemporaryObjects           ( void );
    void                RotateTemporaryObjects          ( const radian3& r );
    guid                PlaceObjectsFromTemporary       ( void );
    guid                GetGuidOfLastPlacedTemp         ( void );
    void                SetTempObjectExternal           ( const char* pProperty, const char* pValue );
    void                SetTempObjectInt                ( const char* pProperty, s32 Data );
    vector3             GetMinPositionForTempObjects    ( void );
    bbox                GetTemporaryObjectsBounds       ( void );

    xbool               IsGlobalObject                  ( guid ObjGuid );

    // decal objects
    static_decal*       GetTempDecal                    ( void );
    void                StartNewDecal                   ( void );
    xbool               CalcDecalPlacement              ( vector3& RayStart, vector3& RayEnd );
    guid                PlaceDecalFromTemporary         ( void );
    void                ExportDecals                    ( const char* FileName, xarray<guid>& lstGuids, platform PlatformType );

    //handle objects given a guid
    guid                CreateObject                    ( const char* pName, const char* pObjectType, const vector3& Pos, const char* pLayer, const char* pPath );
    xbool               DeleteObject                    ( guid ObjectGuid );
    guid                CloneObject                     ( guid ObjectGuid );
    xbool               SelectObject                    ( guid ObjectGuid, xbool bClearSelectionList = TRUE, xbool bSelectAllSubParts = FALSE );

    //select the first object dictated by the ray, sets selected flag and returns guid
    guid                SelectObjectWithRay             ( const vector3& Start, const vector3& End, xbool bIncludeIcons  );
    xbool               GetCollisionPointIgnoreTemp     ( const vector3& Start, const vector3& End, vector3& CollisionPt );
    xbool               GetCollisionPointIgnoreSel      ( const vector3& Start, const vector3& End, vector3& CollisionPt );

    //handle selected objects
    u32                 GetSelectedCount                ( void );
    guid                GetSelectedObjectsByIndex       ( s32 index );
    void                ClearSelectedObjectList         ( void );
    void                DeleteSelectedObjects           ( xarray<editor_item_descript>& lstItems );
    void                RotateSelectedObjects           ( const radian3& r );
    void                MoveSelectedObjects             ( vector3& pos );
    void                MoveSelectedObjectsToCenterPt   ( vector3& pos );
    bbox                GetSelectedObjectsBounds        ( void );
    void                RotateSelectedObjects           ( const matrix4& Transform );
    vector3             GetMinPositionForSelected       ( void );
    void                CopySelectedObjects             ( xarray<guid>& lstObjects, xarray<guid>& lstBPRefs, xbool bKeepCurrentSelection = FALSE );
    xbool               MoveSelectedObjectsToLayer      ( const char* pLayer, const char* pLayerPath, xarray<editor_item_descript>& lstItems );
    xbool               MoveSelectedObjectsToActiveLayer( xarray<editor_item_descript>& lstItems );
    void                GetSelectedList                 ( xarray<guid>& lstObjects );
    xbool               CheckSelectedForDuplicates      ( void );
    xbool               HideUnselectedObjects           ( void );
    xbool               UnHideObjects                   ( void );
    void                SelectLastSelected              ( void );
    void                SelectObjectsByAnimation        ( guid CheckGuid );
    void                SelectObjectsByRidgedGeom       ( guid CheckGuid );
    void                SelectObjectsByType             ( guid CheckGuid );
    void                SelectObjectsByLayer            ( const char * pCheckLayer );
    void                SelectObjectsByFolder           ( xarray <guid> &ObjList );
    void                SelectObjectsAll                ( void );

    void                MakeSelectUnSelectObjectsByFolder   (xarray <guid> &ObjList, xbool HiddenUnHidden);
    void                MakeHiddenUnHiddenObjectsByFolder   (xarray <guid> &ObjList, xbool HiddenUnHidden);
    void                MakeHiddenUnHiddenAllObjects        (xbool HiddenUnHidden);
    void                MakeHiddenUnHiddenObjectsByLayer    (const char * pCheckLayer, xbool HiddenUnHidden);
    void                MakeSelectUnSelectAllObjects        (xbool SelectUnSelect);
    void                MakeSelectUnSelectObjectsByLayer    (const char * pCheckLayer, xbool SelectUnSelect);
    xbool               IsSelectedObjectsEditable           ( void );

    void                ShowHideObjectsByRidgedGeom     (guid CheckGuid, xbool ShowHide);
    void                ShowHideObjectsByAnimation      (guid CheckGuid, xbool ShowHide);
    void                ThawFreezeObjectsByRidgedGeom   (guid CheckGuid, xbool ShowHide);
    void                ThawFreezeObjectsByAnimation    (guid CheckGuid, xbool ShowHide);
    xbool               GetAllObjectsInLayer            ( const char* pLayer, xarray<guid> &lstGuids );
    xbool               GetAllObjectsInAllBluePrints    ( xarray<guid> &lstGuids );
    xbool               OverRideReadOnly                (void);



    //anchors
    void                MoveBlueprintObjectsToAnchor    ( const vector3& Pos );

    //handle blueprints
    void                SaveSelectedObjectsAsBlueprint  ( const char* pTheme, const char* pRelPath );
    s32                 AddBlueprintAsObjects           ( const char* pTheme, const char* pRelPath, xarray<guid>& lstGuids );
    s32                 AddBlueprint                    ( const char* pTheme, const char* pRelPath, editor_blueprint_ref& BlueprintReference, xbool bAndSelect, xbool bIsDirty );
    s32                 AddBlueprintToSpecificLayer     ( const char* pTheme, const char* pRelPath, 
                                                          const char* pLayer, const char* pLayerPath, 
                                                          editor_blueprint_ref& BlueprintReference, 
                                                          xbool bAndSelect, xbool bIsDirty,
                                                          guid PrimaryGuid );
    void                DeleteBlueprintsWithFile        ( const char* pTheme, const char* pRelPath, 
                                                          xarray<editor_item_descript>& lstItems, 
                                                          xarray<editor_blueprint_placement>& lstPlacement );
    void                UpdateBlueprintsWithFile        ( const char* pTheme, const char* pRelPath, 
                                                          const char* pNewTheme, const char* pNewRelPath, 
                                                          xarray<editor_item_descript>& lstItems );
    guid                GetBlueprintGuidContainingObject( guid ObjectGuid );
    xbool               GetBlueprintRefContainingObject ( guid ObjectGuid, editor_blueprint_ref& BlueprintReference );
    xbool               GetBlueprintRefContainingObject2( guid ObjectGuid, editor_blueprint_ref** ppBlueprintReference );
    void                SelectBlueprintObjects          ( editor_blueprint_ref& BlueprintReference, xbool bAddSelection);
    xbool               ShatterSelectedBlueprints       ( xarray<editor_item_descript>& lstItemsAdded, xarray<editor_item_descript>& lstItemsRemoved );
    xbool               IsBlueprintSelected             ( void );
    xbool               IsOneBlueprintSelected          ( editor_blueprint_ref& BPRef );
    guid                CreateBlueprintAnchor           ( void );
    xbool               CanMakeBlueprintFromSelected    ( void );
    xbool               GetBlueprintByGuid              ( guid BPGuid, editor_blueprint_ref& BlueprintReference );
    xbool               GetBlueprintRefContainingAnchor ( guid AnchorGuid, editor_blueprint_ref& BlueprintReference );
    xbool               GetBlueprintPath                ( const char* pTheme, const char* pRelPath, xstring &xstrFullPath );
    xbool               GetThemeInfoFromPath            ( const char* pPath, xstring &xstrTheme, xstring &xstrRelativePath );
    xbool               HasSingleBlueprintChanged       ( const editor_blueprint_ref& BPRef, xarray<prop_container>& DifferentProperties );
    void				SelectAllMatchingBlueprints		( const editor_blueprint_ref& BPRef );

    //Render the world
    void                RenderSelectedObjectCollision   ( void );
    void                RenderObjects                   ( xbool bDoPortalWalk, const view& PortalView, u8 StartZone );
    void                RenderSpacialDBase              ( void );

    //external creation
    guid                CreateNewObjectAtPosition       ( const char* pObjectType, vector3& pos, xbool bSelect );
    const char*         GetObjectTypeName               ( guid ObjGuid );
    
    //search options
    editor_layer&       FindObjectsLayer                ( guid ObjectGuid, xbool bIncludeBlueprints );
    editor_layer&       FindBlueprintsLayer             ( guid BlueprintGuid );
    editor_layer&       GetLayerInfo                    ( const char* pLayer );
    const char*         FindLayerPathForObject          ( guid ObjectGuid, editor_layer& Layer );
    const char*         FindLayerPathForBlueprint       ( guid BlueprintGuid, editor_layer& Layer );

    //updater options 
    void                GetListOfMatchingGeoms          ( guid TemplateGuid, xarray<guid>& lstGuids );  
    void                UpdateListOfGeoms               ( guid TemplateGuid, xarray<guid>& lstGuids );  
    void                UpdateRigidInstances            ( xarray<guid>& lstGuids, const char* pNewRsc );  
    s32                 SelectMatchingGeomObjects       ( guid TemplateGuid );
    void                UpdateSelectedObjsWithBlueprint ( const char* pTheme, const char* pRelPath );

    //guid storage
    xbool               CaptureGuid                     ( void );
    guid                GetCapturedGuid                 ( void );

    //for hover selection
    xbool               IsInHoverSelectMode             ( void );
    void                SetGuidSelectMode               ( const char* pProperty, xbool bActivate, xbool bProperty = TRUE );
    guid                GetHoverSelectionGuid           ( void );
    xbool               IsHoverSelectionABlueprint      ( void );
    void                ClearHoverSelection             ( void );
    void                DoHoverSelect                   ( const vector3& Start, const vector3& End, xbool bIncludeIcons );
    void                DoGuidSelect                    ( void );
    xbool                IsGuidSelectMode                ( void );
    void                TreeDoGuidSelect                ( guid SelectedGuid );


    //reporting options
    void                PrintSelectedRigidInstanceInfo  ( void );
    s32                 GetRigidGeomUseCount            ( const char* pRsc );
    void                WriteRigidGeomReport            ( const char* pFile, xarray<xstring> &lstRigidGeoms );

    void                CreateResourceLoadList          ( const xarray<guid>& lstGuidsToExport, 
                                                          const xarray<guid>& lstPlaySurfaces, 
                                                          const xarray<guid>& lstDecals,
                                                          const char* fileName,
                                                                s32   iPlatform);

    xbool               CanPerformReverseLookup         ( void );
    void                ReverseGuidLookup               ( void );

    void                ReverseGlobalLookup             ( const char* pGlobal );

    void                ClearGuidLookupList             ( void );
    xarray<guid>&       GetGuidLookupList               ( void );
    guid                GetLookupGuid                   ( void );

    //debug
    void                SetAutomatedBuildParams         ( const char* pLighting, const char* pExportName, xbool bAutoQuit, xbool bOldBreakVal );
    xbool               IsPerformingAutomatedBuild      ( void );
    xbool               UpdateAutomatedBuild            ( void );
    void                SelectAllObjectsOfTypeSelected  ( void );

    //maintenance
    void                DestroyObjectEx                 ( guid Guid, xbool bRemoveNow );
    void                CleanupZones                    ( void );

    editor_handler*     m_pHandler;
    xbool               m_bIgnoreReadOnlyChecksForDebug;

public:

    xarray<guid>        m_GuidLog;
    xbool               m_bLogGuids;

protected:

    xbool               InternalSelectObject            ( guid ObjectGuid , xbool bClearSelectionList, xbool bSelectAllSubParts );
    xbool               InternalObjectDestroy           ( guid Guid, xbool bImmediate );
    guid                InternalObjectCopy              ( guid ObjectGuid );
    xbool               CheckForDuplicateObjects        ( object* pObject );
    void                DeleteAllObjectsOfType          ( const char* pName );
    xbool               IsPartOfAZone                   ( const char* pPath, const char* pLayer, xstring &Zone );
    xbool               IsPartOfAPortal                 ( const char* pPath, const char* pLayer, u8 &Zone1, u8 &Zone2 );
    void                UpdatePointerGuids              ( xarray<guid_map>& GuidMapList );
    
    void                DetermineExternalType           ( prop_interface* pPropInterface, const char* pName, xstring& xstrType, xstring& xstrEnumName, prop_query* pq = NULL );

private:
   
    xarray<guid>                m_guidsSelectedObjs;
    xarray<guid>                m_RayCastOverlapGuids;  // List of ray cast overlapping objects
    s32                         m_iRayCastOverlapGuid;  // Current ray cast overlapping object to use

    xarray<editor_layer>        m_listLayers;
    xarray<editor_zone_ref>     m_listZones;
    xstring                     m_xstrActiveLayer;
    xstring                     m_xstrActiveLayerPath;

    ptr_list*                   m_pUndoSelectData;
    transaction_entry*          m_pCurrentUndoEntry;

    xarray<guid>                m_guidLstTempObject;
    guid                        m_guidLastTempPlaced;
    editor_blueprint_ref        m_bpRefTemp;            //temp reference used for blueprint placement;
    editor_layer                m_NullLayer;            //for non-associated objects

    xarray<editor_state_ref> 	m_ObjectsState;         //used for initial state
    u8                          m_SelectedZone;
    xbool                       m_bObjectGroupChanges;
    
    guid                        m_CapturedGuid;         //guid captured from selected object
    xbool                       m_bInHoverSelectMode;
    xbool                       m_bHoverSelectForProperty;
    guid                        m_HoverSelectionGuid;
    xbool                       m_bHoverSelectionIsBlueprint;
    xstring                     m_xstrHoverProperty;
    
    guid                        m_TempPropGuid;         //for reverse lookup checks
    guid                        m_ReverseLookupGuid;    //for reverse lookup checks
    xarray<guid>                m_GuidLookupList;
    
    xarray<guid>                m_HiddenSet;
    
    guid                        m_LastSelectedGuidA;
    guid                        m_LastSelectedGuidB;

    s32                         m_DisableDirtyTracking;     // Stack for disabling dirty tracking

    //autobuild parameters
    xbool                       m_bAutoBuilding;
    xbool                       m_bForceShutdown;
    xstring                     m_ExportName;
    xstring                     m_LightingType;
    xbool                       m_bOldBreakVal;
    xbool                       m_bRefreshRsc;
};

extern world_editor             g_WorldEditor;


//=========================================================================
// END
//=========================================================================
#endif
