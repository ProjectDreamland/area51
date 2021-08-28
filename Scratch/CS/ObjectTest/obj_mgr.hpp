//==============================================================================
//
//  ObjectMgr.hpp
//
//==============================================================================

#ifndef OBJ_MGR_HPP
#define OBJ_MRG_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Object.hpp"
#include "Guid.hpp"
#include "x_time.hpp"
#include "x_stdio.hpp"
#include "x_array.hpp"
#include "x_string.hpp"
#include "SpatialDBase\SpatialDBase.hpp"

//==============================================================================
//  TYPES
//==============================================================================
class   object;
//struct  obj_type_info;
typedef s16     obj_ref;    
typedef s16     slot_ID;

//==============================================================================
// OBJ_CELL
//==============================================================================

struct obj_cell_link
{
    spatial_cell_id     CellID;
    slot_ID             Next;
    slot_ID             Prev;
};


//==============================================================================
//  CLASS OBJ_MGR
//==============================================================================

class obj_mgr
{
//------------------------------------------------------------------------------
//  Public Values and Types   
//------------------------------------------------------------------------------

public:
    enum 
    {
        INVALID_OBJECT          =   -1,
        MAX_OBJECTS             =   4096,           //  Max number of objects.  62 bytes each
        MAX_REF_NODES           =   MAX_OBJECTS
    };

    enum query_types
    {
        QUERY_NULL  =   0,
        QUERY_VIS,
        QUERY_BBOX,
        QUERY_RAY,
        QUERY_VOLUME,
        QUERY_ATTRIBUTE
    
    };

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------


                    obj_mgr         ( void );
                   ~obj_mgr         ( void );

        void        Init            ( void );
        void        Kill            ( void );
        void        SanityCheck     ( void );

        void        Clear           ( void );
        void        DisplayStats    ( void );

        guid        CreateObject    ( object::type ObjectType );
		guid		CreateObject	( char* pObjectTypeName );
        void        DestroyObject   ( guid ObjectGuid );

        void        AdvanceAllLogic ( f32 DeltaTime );

//        void        BroadcastMessage( message Message, xhandle *ThisSet = NULL );   //  Send message to the results of a query or all objects
//        void        BroadcastMessage( message Message, type ThisType);              //  Send message to all objects of this type

		void		 GetAllTypes		( xarray<xwstring> &Names, xarray<object::type> &Types);  //Get all object mgr types
        object::type GetTypeFromName( const char* pName );
        const char*  GetNameFromType( object::type Type );

        s32          GetNumInstances ( object::type Type );

        const object* GetObjectBySlot( slot_ID SlotID );         //  Returns a pointer to the object in this slot
        const object* GetObjectByGuid( guid    Guid);

        slot_ID      GetFirst        ( object::type Type );      //  Gets the first object of this type
        slot_ID      GetNext         ( slot_ID SlotID );         //  Gets the next object of the same type

        void         RemoveFromSpatialDBase(slot_ID SlotID);
        void         AddToSpatialDBase(slot_ID SlotID);
        void         RemoveFromSpatialDBase(guid Guid);
        void         AddToSpatialDBase(guid Guid);




        obj_cell_link*  GetCellLinks  ( void ) { return m_ObjectLink; }

//------------------------------------------------------------------------------
//  Pain
//------------------------------------------------------------------------------
/*
        xbool       ApplyPain       (   guid            Guid, 
                                        const vector3&  Pos, 
                                        f32             Radius, 
                                        f32             Pain, 
                                        f32             Force,
                                        u32             Attributes,
                                        s32             Type = object::PAIN_UNKNOWN );
*/

//------------------------------------------------------------------------------
//  Visibility
//------------------------------------------------------------------------------

        void        SolveVisibleObjects( const view& View, xbool DoCulling=TRUE );
//      s32         GetNVisible        ( object::type Type );       //  Not needed

    
//------------------------------------------------------------------------------
//  Queries
//------------------------------------------------------------------------------

        

        // Selects all objects whose bbox intersects the bbox provided
        slot_ID     SelectBBox      ( slot_ID SrcSet, u32 Attribute, const bbox&    BBox, object::type Type = object::type::TYPE_NULL );
         

        // Selects all objects whose bbox is intersected by the line segment provided
        //   If Unsorted is FALSE, returns them in sorted order (closest to start first)
        //   Also, if Unsorted is FALSE, objects which contain the start point of the vector are *ignored* (otherwise this is not the case)
        slot_ID     SelectRay       ( slot_ID SrcSet, u32 Attribute, const vector3& Start, const vector3& End, xbool Unsorted = TRUE, object::type Type = object::type::TYPE_NULL );
        

        // Selects all objects whose bboxes are within the volume specified by the front 
        // sides of the planes provided. The planes must describe a convex volume.
        // You can use up to 16 planes to describe your volume.
        // When using the fast option it's possible to get false positives. 
        slot_ID     SelectVolume    ( slot_ID SrcSet, u32 Attribute, const plane* pPlane, s32 NPlanes, xbool Fast = TRUE, object::type Type = object::type::TYPE_NULL );
        

        //  Selects only objects with specific attributes but without geometry constraints
        slot_ID     SelectByAttribute(slot_ID SrcSec, u32 Attribute, object::type Type = object::type::TYPE_NULL );

        slot_ID     GetFirstResult       ( void );     //  Functions for walking the list of all queries
        slot_ID     GetNextResult        ( slot_ID SlotID );
        query_types GetLastQueryType     ( void );
        void        ResetSearchResult    ( void );
        void        SetNextSearchResult  ( slot_ID SlotID );

        slot_ID     GetFirstVis          ( object::type Type );
        slot_ID     GetNextVis           ( slot_ID SlotID );
        
        

//------------------------------------------------------------------------------
//  Protected Types
//------------------------------------------------------------------------------

protected:
    

    //  For keeping stats on each object type.  
    //  Current object size is 40 bytes
    struct obj_type_node
    {
        s32         InstanceCount;      //  How many of these do we have
        xtimer      LogicTime;          //  For keeping track of performance info
        slot_ID     FirstType;          //  First of this type in the obj_slot list
        slot_ID     FirstVis;           //  First fisible of this type
        s32         nVis;               //  Number visible of this type
        char*       pTypeName;          //  Stores the string name for objects of this type
    };

    //  This is used to store a list of all the objects currently in use in
    //  the game.  As objects are created an obj_slot is filled with info about
    //  the objects.
    //  Current object size is 14 bytes
    struct obj_slot
    {
        object*         pObject;        //  The only place that actually holds a pointer to the object
        slot_ID         Next;           //  Points to index of next object of this type
        slot_ID         Prev;           //  Points to index of previous object of this type
        slot_ID         NextVis;        //  For render order results
        slot_ID         NextSearch;     //  Points to next object in a search result
        s16             PauseCount;     //  Stored here for speed reasons since object manager will call
                                        //  update on objects.  It is a counted pause value.  Pause twice
                                        //  and you must unpause twice.  Zero is not paused
    };

protected:

        //  To pause objects
        void        PauseObject      ( slot_ID slot );
        void        ResumeObject     ( slot_ID slot );

//------------------------------------------------------------------------------
//  Protected Data
//------------------------------------------------------------------------------


        guid_lookup     m_GuidLookup;                           //  Guild lookup object                 Size = 28 bytess
        obj_type_node   m_ObjectType    [ object::TYPE_END_OF_LIST ];// Info about each type of object  Size = 40*numOfObjects
        obj_slot        m_ObjectSlot    [ MAX_OBJECTS   ];      //  The array that holds all pointers   Size = 14*4096
        obj_cell_link   m_ObjectLink    [ MAX_OBJECTS*8 ];      //  for the spatial structure           Size = 48*4096  
        slot_ID         m_FirstFreeSlot;                        //  index to fist free slot                     2
        slot_ID         m_FirstSearchResult;
        xbool           m_InVisLoop;                            //                                              4

//------------------------------------------------------------------------------
//  Protected Functions
//------------------------------------------------------------------------------

        s32         GetSlotFromGuid     ( guid Guid );
        void        RemoveFromTypeLoops ( s32 Slot );
//------------------------------------------------------------------------------



        
        
        
///////////////////////////////////////////////////////////////////////////////
//
//  Functions from Meridian editor left in tact till work on the editor begins
//  and we can figure out if these work for us
//
///////////////////////////////////////////////////////////////////////////////

#ifdef EDITOR

protected:
        xbool           m_Unselectable  [ object::TYPE_END_OF_LIST ];
        xbool           m_Hidden        [ object::TYPE_END_OF_LIST ];

        xarray<xbool>   m_LayerUnselectable;
        xarray<xbool>   m_LayerHidden;

        xarray<xwstring> m_LayerFileNames;
        xarray<xwstring> m_LayerTitles;
        xarray<guid>     m_LayerGuids;
        xbool           m_LayerFull;
        s32             m_LayerCursor;
public:
        // A value of 0 (FALSE) or 1 (TRUE) indicates what it should indicate.
        // A value of -1 (INDETERMINATE) indicates that some objects of this type
        //    have one condition, and others have another.
    
        //If AdjustClasses is left at the default value of TRUE, calling SetXX will cause the
        //   object manager to adjust the status of the XX Flag in the corresponding object type.
        //   This should always be the desired behavior, *except* (for reasons of recursion) inside
        //   the corresponding calls at the class level.
        void        SetUnselectable     ( object::type ObjType, s32 Flag, xbool AdjustObjects = TRUE );
        s32         GetUnselectable     ( object::type ObjType );
        void        SetUnselectable     ( const xwstring& LayerFileName, s32 Flag, xbool AdjustObjects = TRUE );
        s32         GetUnselectable     ( const xwstring& LayerFileName );
        void        SetHidden           ( object::type ObjType, s32 Flag, xbool AdjustObjects = TRUE );
        s32         GetHidden           ( object::type ObjType );
        void        SetHidden           ( const xwstring& LayerFileName, s32 Flag, xbool AdjustObjects = TRUE );
        s32         GetHidden           ( const xwstring& LayerFileName );
        void        SetAllUnselectable  ( s32 Flag );
        s32         GetAllUnselectable  ( void );
        void        SetAllHidden        ( s32 Flag );
        s32         GetAllHidden        ( void );

        void        AdjustLayer         (xwstring LayerName);

        //Operate very similarly to the XXType functions shown above, but operate on layers, or
        //  logical, file-based groupings of objects.  Note that you can only have one "Layer Loop"
        //  active at a time - attempting to open a second one will assert.
        void        AddLayer            ( const xwstring& LayerFileName, const xwstring& LayerTitle );
        void        RemoveLayer         ( const xwstring& LayerFileName );
        //void        RenameLayer         ( xwstring OldLayerName, xwstring NewLayerName );
        void        ClearLayers         ( void );
        void		GetAllLayers		( xarray<xwstring> &LayerFileNames, xarray<xwstring> &LayerTitles);  //Get all object mgr layers
        void        StartLayerLoop      ( const xwstring& LayerFileName );
        void        SetLayerTitle       ( const xwstring& LayerFileName, const xwstring& LayerTitle );
        xwstring    GetLayerTitle       ( const xwstring& LayerFileName );
        guid        GetNextInLayer      ( void );
        void        EndLayerLoop        ( void );

#endif //EDITOR

};
 
//==============================================================================
//==============================================================================
//==============================================================================
// OBJ_TYPE_INFO
//==============================================================================
//==============================================================================
//==============================================================================



//==============================================================================

extern obj_mgr g_ObjMgr;

//==============================================================================
#endif // OBJECTMGR_HPP
//==============================================================================







/*
	Code removed from the Meridian base file.  Moved down her for archiving
	to keep the code looking cleaner

//------------------------------------------------------------------------------
//  Stats
//------------------------------------------------------------------------------

    struct
    {
        s32         numLockedObjects;
        s32         numLockedObjectsPerType[object::TYPE_END_OF_LIST];
        s32         numTotalLocks;
        s32         numPhonyUnlocks;
        s32         maxLocksPerObject;
        s32         maxNumberLockedObjects;
    } Stats;        

    void        AssertNoLockedObjects(void);
    void        PrintStats(void);

//      object*     LockObject      ( guid ObjectGuid );
//      void        UnlockObject    ( guid ObjectGuid );



//
//  Another optimizing structure
//      reduced to storage class 
struct obj_type_info
{
public:

//    char*               pTypeName;
    obj_create_fn*      pCreateFn;              //  Function for creation.  storage prevents switch statement

//  obj_type_info       ( object::type   Type, 
//                        char*          pTypeName, 
//                        obj_create_fn* pCreateFn
//                        u32            DefaultAttrBits,
//                        xbool          DoRunLogic = TRUE);

//  object::type        Type;           
//  u32                 DefaultAttrBits;        //  Starting attributes for an object.  Should be set by object Init()
//  xbool               bRunLogic;




*/
/*
xhandle Select          ( xhandle SrcSet, u32 Attribute );
xhandle SelectSphere    ( xhandle SrcSet, u32 Attribute, const vector3& Center, f32 Radius );
xhandle SelectBBox      ( xhandle SrcSet, u32 Attribute, const bbox&    BBox );
xhandle SelectCone      ( xhandle SrcSet, u32 Attribute, const vector3& Start, const vector3& End, radian Angle???? );
xhandle SelectView      ( xhandle SrcSet, u32 Attribute, const view& View );
*/
