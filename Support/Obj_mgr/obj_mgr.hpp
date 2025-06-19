//==============================================================================
//
//  ObjectMgr.hpp
//
//==============================================================================

#ifndef OBJ_MGR_HPP
#define OBJ_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\Object.hpp"
#include "PainMgr\Pain.hpp"
#include "TweakMgr\TweakMgr.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "Objects\ProxyPlaySurface.hpp"
#include "miscutils\Guid.hpp"
#include "x_time.hpp"
#include "x_stdio.hpp"
#include "x_array.hpp"
#include "x_string.hpp"
#include "SpatialDBase\SpatialDBase.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"

//==============================================================================
//  FLAGS
//==============================================================================
extern xbool g_GameLogicDebug;

//==============================================================================
//  TYPES
//==============================================================================
class   object_desc;
class   object;

typedef u16     obj_ref;
typedef u16     slot_id;
#ifdef TARGET_PC
typedef s32     link_id;
#else
typedef s16     link_id;
#endif

#define SLOT_NULL ((u16)0xffff)
#define LINK_NULL (-1)

struct obj_cell_link
{
    u16                 SpacialCellID;
    link_id             Next;           // Next link in the list. (link_id/8) == slot_id
    link_id             Prev;           // Prev link in the list. (link_id/8) == slot_id
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
#ifndef TARGET_PC
        MAX_OBJECTS             =   2000,       //  Max number of objects.  62 bytes each (PS2/Xbox)
#else 
        MAX_OBJECTS             =   60000,      //  Max number of objects.  62 bytes each (PC)
#endif
        MAX_REF_NODES           =   MAX_OBJECTS
    };

    enum
    {
#ifndef TARGET_PC
        MAX_VISIBLE_SHADOW_PROJECTORS   =    4,
#else
        MAX_VISIBLE_SHADOW_PROJECTORS   =  256,
#endif
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

                        obj_mgr                 ( void );
                       ~obj_mgr                 ( void );

        void            Init                    ( void );
        void            Kill                    ( void );
#if !defined(X_RETAIL)
        void            SanityCheck             ( void );
#endif
        xtick           GetGameTime             ( void ) const;
        f32             GetGameDeltaTime        ( xtick LastTime ) const;

        void            Render                  ( xbool DoPortalWalk, const view& PortalView, u8 StartZone );

#ifdef CONFIG_VIEWER
        void            RenderArtistViewer      ( const view& View );
#endif
        
        void            Clear                   ( void );
        void            DisplayStats            ( void );

        void            ReserveGuid             ( guid Guid );
        guid            CreateObject            ( const char* pObjectTypeName );
        void            CreateObject            ( const char* pObjectTypeName, guid Guid );
        guid            CreateObject            ( const object_desc& Desc );
        void            CreateObject            ( const object_desc& Desc, guid Guid );

        void            DestroyObject           ( guid ObjectGuid );
        
        void            ChangeObjectGuid        ( guid CurrentGuid, guid NewGuid );

        void            SetProxyPlaySurface     ( object* pObject );

        void            AdvanceAllLogic         ( f32 DeltaTime );

        object::type    GetTypeFromName         ( const char* pName );
        const char*     GetNameFromType         ( object::type Type );
        s32             GetNumInstances         ( object::type Type );

              object*   GetObjectBySlot         ( slot_id SlotID );         //  Returns a pointer to the object in this slot
              object*   GetObjectByGuid         ( guid Guid      );
#ifdef USE_OBJECT_NAMES
              object*   GetObjectByName         ( const char* pName );
#endif

        slot_id         GetFirst                ( object::type Type );      //  Gets the first object of this type
        slot_id         GetNext                 ( slot_id SlotID    );      //  Gets the next object of the same type
                        
        

        void            RemoveFromSpatialDBase  ( slot_id SlotID );
        void            AddToSpatialDBase       ( slot_id SlotID );
        void            RemoveFromSpatialDBase  ( guid Guid );
        void            AddToSpatialDBase       ( guid Guid );
        void            UpdateSpatialDBase      ( slot_id SlotID, const bbox& OldBBox );

        void            AddSlotToZone           ( slot_id SlotID );
        void            AddSlotToRenderable     ( slot_id SlotID );
        void            RemoveSlotFromZone      ( slot_id SlotID );
        void            RemoveSlotFromRenderable( slot_id SlotID );

        slot_id         GetSlotFromGuid         ( guid Guid );
        
        
        s32             GetNLogicLoops          ( void ) { return m_nLogicLoops;}
        
        const object_desc*  GetDescFromName     ( const char* pObjectTypeName );
        const object_desc*  GetTypeDesc         ( object::type Type )   { return m_ObjectType[ Type ].pDesc; }
        const bbox&     GetSafeBBox             ( void ) { return m_SafeBBox; }
        void            SetSafeBBox             ( const bbox& SafeBBox ) { m_SafeBBox = SafeBBox; }
        void            InflateSafeBBox         ( f32 Inflate ) { m_SafeBBox.Inflate( Inflate, Inflate, Inflate ); }

//------------------------------------------------------------------------------
//  Queries
//------------------------------------------------------------------------------
    
        //check LOS from point to point (ignores permeable and living)
        xbool       HasLOS          ( guid Object0, const vector3& P0, guid Object1, const vector3& P1 );

        // Selects all objects whose bbox intersects the bbox provided
        void        SelectBBox      ( u32 Attribute, const bbox& BBox, object::type Type = object::TYPE_ALL_TYPES, u32 NotTheseAttributes = 0x00000000);
         

        // Selects all objects whose bbox is intersected by the line segment provided
        void        SelectRay       ( u32 Attribute, const vector3& RayStart, const vector3& RayEnd, object::type Type = object::TYPE_ALL_TYPES, u32 NotTheseAttributes = 0x00000000);
        

        // Selects all objects whose bboxes are within the volume specified by the front 
        // sides of the planes provided. The planes must describe a convex volume.
        // You can use up to 16 planes to describe your volume.
        // When using the fast option it's possible to get false positives. 
        void        SelectVolume    ( u32 Attribute, const plane* pPlane, s32 NPlanes, xbool Fast = TRUE, object::type Type = object::TYPE_NULL );
        

        //  Selects only objects with specific attributes but without geometry constraints
        void        SelectByAttribute( u32 Attribute, object::type Type = object::TYPE_NULL );

        xbool       IsLoopActive         ( void ) { return m_InLoop; }
        slot_id     StartLoop            ( void );     //  Functions for walking the list of all queries
        slot_id     GetNextResult        ( slot_id SlotID );
        void        EndLoop              ( void );
        void        ResetSearchResult    ( void );

        query_types GetLastQueryType     ( void );
        
        void        SetNextSearchResult  ( slot_id SlotID );

        slot_id     GetFirstVis          ( object::type Type );
        slot_id     GetNextVis           ( slot_id SlotID );
        obj_cell_link*  GetCellLinks            ( void ) { return m_ObjectLink; }
        s32             IsBoxInView      ( const bbox&         BBox,
                                           u32                 CheckPlaneMask ) const;
        void        SetTimeDilation      ( f32 newTimeDilation ) { m_TimeDilation = newTimeDilation; }


//------------------------------------------------------------------------------
//  Visibility and rendering
//------------------------------------------------------------------------------
        
public:
        void            Render3dObjects             ( xbool DoPortalWalk, const view& PortalView, u8 StartZone );
        void            RenderIcons                 ( void );

#ifdef TARGET_XBOX
        void            RenderClothObjects          ( object::type type );
#endif

protected:        
        void            DoVisibilityTests           ( const view& View );
        void            CollectShadowCasters        ( void );
        void            CompleteVisAndShadowTests   ( void );
        void            CreateShadowMap             ( void );
        void            Render3dPrep                ( xbool DoPortalWalk, const view& PortalView, u8 StartZone );
        void            RenderNormalObjects         ( void );
        void            RenderPlaySurfaces          ( void );
        void            Render2dObjects             ( void );
        void            RenderSpecialObjects        ( void );

        #if !defined(X_RETAIL)
        void            SlotSanityCheck             ( void );
        #endif

public:
        #if !defined(X_RETAIL)
        void            DumpStatsToFile             ( const char* pFileName );
        #endif

//------------------------------------------------------------------------------
//  Protected Types
//------------------------------------------------------------------------------

protected:

    
    // For keeping stats on each object type.  
    // Current object size is 40 bytes
    struct obj_type_node
    {
        s32                     InstanceCount;      // How many of these do we have
        slot_id                 FirstType;          // First of this type in the obj_slot list
        slot_id                 FirstVis;           // First visible of this type
        s32                     nVis;               // Number visible of this type
        object_desc*            pDesc;              // Description of object type
    };

    //  This is used to store a list of all the objects currently in use in
    //  the game.  As objects are created an obj_slot is filled with info about
    //  the objects.
    //  Current object size is 14 bytes
    struct obj_slot
    {
        object*         pObject;            // The only place that actually holds a pointer to the object
        slot_id         Next;               // Points to index of next object of this type
        slot_id         Prev;               // Points to index of previous object of this type
        slot_id         NextSearch;         // Points to next object in a search result
        slot_id         NextVis;            // Points to next object in a visibility result
        slot_id         NextRenderable[2];  // Points to index of next renderable in the zone
        slot_id         PrevRenderable[2];  // Points to index of previous renderable in the zone
        slot_id         NextZone[2];        // Points to index of next object in the zone
        slot_id         PrevZone[2];        // Points to index of next object in the zone
        s32             Sequence;           // Sequence number
    };

    // This is used to store a list of objects connected to a zone
    struct obj_zone
    {
        slot_id         FirstZone[2];
        slot_id         FirstRenderable[2];
    };

//------------------------------------------------------------------------------
//  Special Functions
//------------------------------------------------------------------------------

public:
        void            DestroyObjectEx         ( guid ObjectGuid, xbool bRemoveNow );
        void            EmptyDeleteObjectList   ( void );

//------------------------------------------------------------------------------
//  Protected Functions
//------------------------------------------------------------------------------
protected:

        void            RemoveFromTypeLoops     ( slot_id Slot );
        object*         CreateObject            ( const object_desc& Desc, slot_id Slot );
        slot_id         AllocSlot               ( void );
        void            UnlinkAndFreeObject     ( slot_id Slot );
#ifndef X_RETAIL
        void            DisplayLocations        ( void );
        void            RenderCollision         ( void );
#endif

//------------------------------------------------------------------------------
//  Protected Data
//------------------------------------------------------------------------------
protected:

        guid_lookup     m_GuidLookup;                                   //  Guid lookup object                  Size = 28 bytess
        obj_type_node   m_ObjectType    [ object::TYPE_END_OF_LIST ];   //  Info about each type of object      Size = 40*numOfObjects
        obj_slot        m_ObjectSlot    [ MAX_OBJECTS   ];              //  The array that holds all pointers   Size = 14*4096
        obj_cell_link   m_ObjectLink    [ MAX_OBJECTS*8 ];              //  for the spatial structure           Size = 48*4096  
        obj_zone        m_ObjectZone    [ 256 ];                        //  for the zone linked-list            Size = 4*255

        slot_id         m_FirstFreeSlot;                                //  index to fist free slot                     2
        slot_id         m_FirstSearchResult;
        xbool           m_InVisLoop;                                    //                                              4
        xbool           m_InLoop;
        s32             m_Sequence;
        plane           m_Plane[6*2];     
        s32             m_PlaneMinIndex[6*2*3];
        s32             m_PlaneMaxIndex[6*2*3];
        object*         m_pAdvanceLogicActiveObject;                    // This is the current object that we are advancing the logic for
        object*         m_pProxyPlaySurface;                            // This is a proxy playsurface for doing collisions, etc.
        xtick           m_GameTime;
        xarray<slot_id> m_DeleteObject;
        f32             m_TimeDilation;
        bbox            m_SafeBBox;
        guid            m_ReservedGuid;

        // shadow information
        struct shad_projector
        {
            matrix4 L2W;
            bbox    CastWorldBBox;
            guid    Guid;
        };
        struct shad_receiver
        {
            guid    Guid;
            u64     Mask;
        };

        // TODO: Because we're only doing directional at the moment,
        // casters and projectors are identical. When we switch to
        // point (spot) projectors, a single projector could have
        // several casters
        xarray<shad_projector>  m_ShadowProjectors;
        xarray<shad_projector>  m_ShadowCasters;
        xarray<shad_receiver>   m_ShadowReceivers;
        xbool                   m_bRenderShadows;

        s32                     m_nLogicLoops;

//------------------------------------------------------------------------------
// DEBUG TRAPS
//------------------------------------------------------------------------------

        // Set these to hit a break before or after advancing logic
        guid                    m_TrapBeforeLogicGuid;
        guid                    m_TrapAfterLogicGuid;

        // Set this to hit a break before rendering
        guid                    m_TrapBeforeRenderGuid;

        // Other traps
        xbool                   m_bTrapOnCreate;
        xbool                   m_bTrapOnDestroy;
};
 
//==============================================================================
// OBJECT DESCRIPTION
//==============================================================================
//
// Create   -   Allocates and initializes an object.
//
//==============================================================================
class object_desc : public prop_interface
{
//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
public:
    CREATE_RTTI_BASE( object_desc );

    object::type        GetType                     ( void ) const;
    xbool               CanRender                   ( void ) const;
    const char*         GetTypeName                 ( void ) const;
    const char*         GetEditorGroupName          ( void ) const;
    u32                 GetAttrBits                 ( void ) const;
    
    xbool               UseInEditor                 ( void ) const;
    xbool               IsDynamic                   ( void ) const;
    xbool               TargetsOtherObjects         ( void ) const;
    xbool               NoAllowEditorCopy           ( void ) const;
    xbool               IsGlobalObject              ( void ) const;
    xbool               IsBurnVertexLighting        ( void ) const;
    xbool               IsIconSelect                ( void ) const;
    xbool               IsTempObject                ( void ) const;
    xbool               HasLogic                    ( void ) const;

#ifdef X_EDITOR
    virtual s32         OnEditorRender              ( object& Object ) const; 
    virtual void        OnEditorBandSelected        ( object& Object, plane DragSelect[5] ) const { (void)Object; (void)DragSelect; }
    virtual void        OnEditorSelectAll           ( object& Object ) const { (void)Object; }
#endif // X_EDITOR

    virtual const char* QuickResourceName           ( void ) const { return NULL; }
    virtual const char* QuickResourcePropertyName   ( void ) const { return NULL; }
    
    static object_desc* GetFirstType                ( void );
    static object_desc* GetNextType                 ( object_desc* pPrevious );

//------------------------------------------------------------------------------
// Protected functions
//------------------------------------------------------------------------------
protected:

    //--------------------------------------------------------------------------
    //
    // FLAGS_GENERIC_EDITOR_CREATE - Expouses the type in the editor so a user can create it from scrach.
    //
    // FLAGS_NO_ALLOW_COPY         - Tells the editor that this particular type can't be copy
    //
    // FLAGS_IS_DYNAMIC            - Tells the system that this object needs to be save/loaded every time the game starts up
    //
    // FLAGS_IS_GLOBAL             - Tells the editor that this object must be created only in the global layer.
    //  
    // FLAGS_DONT_RENDER           - Tells the editor that this type can't be render
    //
    // FLAGS_BURN_VERTEX_LIGHTING  - Tells the editor that when exporting this type it must bake in its lighting
    //
    // FLAGS_NO_ICON               - Tells the editor not to use the generic funtionality of selecting the object via an icon. So it is upto the object to solve that problem by it self.
    //
    // FLAGS_NO_EDITOR_RENDERABLE  - Tells the editor that this object type never renders in the editor
    //
    // FLAGS_TARGETS_OBJS          - Tells the editor that this object type may target other object guids
    //
    // FLAGS_EDITOR_TEMP           - Tells the editor to NOT save the object (object is for editor use only!)
    //
    //--------------------------------------------------------------------------
    enum flags
    {
        FLAGS_GENERIC_EDITOR_CREATE = (1<<0),
        FLAGS_NO_ALLOW_COPY         = (1<<1),
        FLAGS_IS_DYNAMIC            = (1<<2),
        FLAGS_IS_GLOBAL             = (1<<3),
        FLAGS_BURN_VERTEX_LIGHTING  = (1<<5),
        FLAGS_NO_ICON               = (1<<6),
        FLAGS_NO_EDITOR_RENDERABLE  = (1<<7),
        FLAGS_TARGETS_OBJS          = (1<<8),
        FLAGS_EDITOR_TEMP           = (1<<9),   
    };

//------------------------------------------------------------------------------
// Protected functions
//------------------------------------------------------------------------------
protected:

    virtual object*   Create            ( void          ) = 0;

    virtual xbool     OnBeginRender     ( void );
    virtual void      OnEndRender       ( void ) { CONTEXT( "object_desc::OnEndRender" ); }
    virtual xbool     OnBeginLogic      ( void );
    virtual void      OnEndLogic        ( void ){}

    virtual void      OnEnumProp        ( prop_enum&   List );
    virtual xbool     OnProperty        ( prop_query&  I    );

            void      AddObjectCount    ( s32 N ) const;

//------------------------------------------------------------------------------
// For the user to call
//------------------------------------------------------------------------------
protected:

                object_desc     ( object::type Type, 
                                  const char* pTypeName, 
                                  const char* pEditorGroupName, 
                                  u32 CommonAttrBit, 
                                  u32 TypeBits = 0 );

//------------------------------------------------------------------------------
//  Type Data
//------------------------------------------------------------------------------
            const char*             m_pEditorGroupName;
            const char*             m_pTypeName;

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
private:

            object::type            m_Type;
            object_desc*            m_pNext;
            object_desc*            m_pNextGroupType;
            u32                     m_CommonAttrBits;
    static  object_desc*            s_pHead;
            xbool                   m_bRender;
            xbool                   m_bLogic;
            xbool                   m_bDisplayLocation;
            xbool                   m_bTrapOnCreate;
            xbool                   m_bTrapOnDestroy;
            xbool                   m_bRenderHiCollision;
            xbool                   m_bRenderLoCollision;
    mutable s32                     m_ObjectCount;
            u32                     m_Flags;

    friend class obj_mgr;
    friend class object;
};

//==============================================================================
// INLINE
//==============================================================================

//==============================================================================
inline
xbool object_desc::IsTempObject( void ) const
{
    return !!(m_Flags & FLAGS_EDITOR_TEMP);
}

//==============================================================================
inline
u32 object_desc::GetAttrBits( void ) const
{
    return m_CommonAttrBits;
}

//==============================================================================

#ifdef X_EDITOR
inline
s32 object_desc::OnEditorRender( object& Object ) const
{
    if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        Object.OnDebugRender();

    return -1;
}
#endif // X_EDITOR

//==============================================================================

inline
const char* object_desc::GetEditorGroupName( void ) const
{
#ifdef X_EDITOR
    return m_pEditorGroupName;
#else // X_EDITOR
    return "<null>";
#endif // X_EDITOR
}

//==============================================================================
inline
xbool object_desc::IsIconSelect( void ) const
{
    return (m_Flags&FLAGS_NO_ICON) != FLAGS_NO_ICON;  
}

//==============================================================================
inline
xbool object_desc::IsBurnVertexLighting ( void ) const
{
    return (m_Flags&FLAGS_BURN_VERTEX_LIGHTING)  == FLAGS_BURN_VERTEX_LIGHTING;  
}

//==============================================================================
inline
xbool object_desc::IsGlobalObject( void ) const
{ 
    return (m_Flags&FLAGS_IS_GLOBAL)  == FLAGS_IS_GLOBAL; 
}

//==============================================================================
inline
xbool object_desc::UseInEditor( void ) const
{ 
    return (m_Flags&FLAGS_GENERIC_EDITOR_CREATE)  == FLAGS_GENERIC_EDITOR_CREATE; 
}

//==============================================================================

inline
xbool object_desc::IsDynamic( void ) const
{ 
    return (m_Flags&FLAGS_IS_DYNAMIC)     == FLAGS_IS_DYNAMIC;    
}

//==============================================================================

inline
xbool object_desc::TargetsOtherObjects( void ) const
{ 
    return (m_Flags&FLAGS_TARGETS_OBJS)     == FLAGS_TARGETS_OBJS;    
}

//==============================================================================

inline
xbool object_desc::NoAllowEditorCopy( void ) const
{ 
    return (m_Flags&FLAGS_NO_ALLOW_COPY)  == FLAGS_NO_ALLOW_COPY; 
}

//==============================================================================
inline
xbool object_desc::HasLogic( void ) const
{ 
    return m_bLogic && (m_CommonAttrBits&object::ATTR_NEEDS_LOGIC_TIME); 
}

//==============================================================================
inline
object_desc* object_desc::GetFirstType( void )
{
    return s_pHead;
}

//==============================================================================
inline 
object_desc* object_desc::GetNextType( object_desc* pPrevious )
{
    return pPrevious->m_pNext;
}

//==============================================================================
inline 
const char* object_desc::GetTypeName( void ) const
{
    return m_pTypeName;
}

//==============================================================================

inline
object::type object_desc::GetType( void ) const
{
    return m_Type;
}

//==============================================================================
inline
xbool object_desc::CanRender( void ) const
{
    return m_bRender && 
           ((m_Flags&FLAGS_NO_EDITOR_RENDERABLE) != FLAGS_NO_EDITOR_RENDERABLE);
}

//==============================================================================

inline 
object_desc::object_desc( object::type Type, const char* pTypeName, const char* pEditorGroupName, u32 CommonAttrBit, u32 TypeBits )
{
    ASSERT( Type> 0 );
    ASSERT( Type< object::TYPE_END_OF_LIST);
    ASSERT( pTypeName );

    m_Type             = Type; 
    m_pTypeName        = pTypeName; 
    m_pEditorGroupName = pEditorGroupName;
    m_CommonAttrBits   = CommonAttrBit;
    m_pNext            = s_pHead;
    s_pHead            = this;
    m_bRender          = TRUE;
    m_bLogic           = TRUE;
    m_bDisplayLocation = FALSE;
    m_bTrapOnCreate    = FALSE;
    m_bTrapOnDestroy   = FALSE;
    m_bRenderHiCollision = FALSE;
    m_bRenderLoCollision = FALSE;

    m_ObjectCount      = 0;
    m_Flags            = TypeBits;
}

//==============================================================================
inline
xbool object_desc::OnBeginRender( void )
{ 
    return CanRender() && m_CommonAttrBits&object::ATTR_RENDERABLE;
}

//==============================================================================
inline
xbool object_desc::OnBeginLogic( void )
{ 
    return m_bLogic && m_CommonAttrBits&object::ATTR_NEEDS_LOGIC_TIME; 
}

//==============================================================================
inline 
void object_desc::AddObjectCount( s32 N ) const
{
    m_ObjectCount += N;
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
// INLINE OBJECT MANAGER
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
inline 
f32 obj_mgr::GetGameDeltaTime( xtick LastTime ) const
{
    #ifdef TARGET_PS2
    s32 Delta = (s32)((m_GameTime - LastTime)>>9);
    f32 Time  = ((f32)Delta) * (512.0f * (1.0f/576.0f) * (1.0f/1000.0f));
    #else
    s64 Delta = ( m_GameTime - LastTime )/x_GetTicksPerMs();    
    f32 Time  = ((f32)Delta) * (1/1000.0f);
    #endif
    return Time;
}

//==============================================================================
inline
xtick obj_mgr::GetGameTime( void ) const
{    
    return m_GameTime;
}

//==============================================================================

inline
void obj_mgr::SetProxyPlaySurface( object* pObject )
{
    m_pProxyPlaySurface = pObject;
}

//==============================================================================

inline
slot_id obj_mgr::GetFirst( object::type Type )
{
    return m_ObjectType[Type].FirstType;
}


//==============================================================================

inline
slot_id obj_mgr::GetNext( slot_id SlotID )
{
    //
    // Keep going to the next for as long as there is deleted objects
    //
    while( m_ObjectSlot[SlotID].Next != SLOT_NULL )
    {
        ASSERT( GetObjectBySlot(SlotID) );
        object* pObject = GetObjectBySlot( m_ObjectSlot[SlotID].Next );
        if( !(pObject->GetAttrBits() & object::ATTR_DESTROY) ) break;
        SlotID = m_ObjectSlot[SlotID].Next;
    }

    ASSERT(GetObjectBySlot(SlotID) );
    return m_ObjectSlot[SlotID].Next;    
}

//==============================================================================

inline
object* obj_mgr::GetObjectBySlot( slot_id SlotID )
{
    if( SlotID == SLOT_NULL )
        return NULL;

    ASSERT( SlotID < MAX_OBJECTS );

    return m_ObjectSlot[SlotID].pObject;
}

//==============================================================================

inline
object* obj_mgr::GetObjectByGuid( guid Guid)
{
    if ( (Guid.Guid&(u64)0xffffffff) == (u64)0xffffffff )
    {
        // this is a play surface
        proxy_playsurface& Proxy = proxy_playsurface::GetSafeType( *m_pProxyPlaySurface );
        Proxy.SetSurface( Guid );
        return m_pProxyPlaySurface;
    }
    else
    {
        u32 Slot;
        if( m_GuidLookup.Find( Guid, Slot ) )
            return m_ObjectSlot[Slot].pObject;
        else
        {
            //x_DebugMsg("GetObjectByGuid() called for invalid guid.  Guid = %s, slot was %d\n",(const char*)guid_ToString(Guid),Slot );
            return NULL;
        }
    }
}

//==============================================================================

#ifdef USE_OBJECT_NAMES

inline
object* obj_mgr::GetObjectByName( const char* pName )
{
    for( s32 i=0 ; i<MAX_OBJECTS ; i++ )
    {
        object* pObject = m_ObjectSlot[i].pObject;
        if( pObject && (x_stricmp( pName, pObject->GetName() ) == 0) )
            return pObject;
    }

    return NULL;
}

#endif

//==============================================================================

inline
s32 obj_mgr::GetNumInstances( object::type Type )
{
    //ASSERT( (Type>=0) && (Type<TYPE_END_OF_LIST) );
    return m_ObjectType[Type].InstanceCount;
}

//==============================================================================

inline
slot_id     obj_mgr::StartLoop            ( void )     //  Functions for walking the list of all queries
{
    ASSERT(!m_InLoop);
    m_InLoop = true;

    return m_FirstSearchResult;
}


//==============================================================================

inline
slot_id     obj_mgr::GetNextResult        ( slot_id SlotID )
{
    ASSERT(m_InLoop);
    ASSERT(SlotID != SLOT_NULL && SlotID < MAX_OBJECTS );

    return m_ObjectSlot[SlotID].NextSearch;

}

//==============================================================================

inline
void        obj_mgr::EndLoop              ( void )
{
    ASSERT( m_InLoop );
    m_InLoop = false;

}

//==============================================================================

inline
void obj_mgr::SetNextSearchResult( slot_id SlotID )
{
    
    m_ObjectSlot[ SlotID ].NextSearch = m_FirstSearchResult;
    m_FirstSearchResult = SlotID;

}

//==============================================================================

inline 
slot_id obj_mgr::GetFirstVis( object::type Type )
{
    return m_ObjectType[Type].FirstVis;
}

//==============================================================================

inline
slot_id obj_mgr::GetNextVis( slot_id SlotID )
{
    ASSERT(m_ObjectSlot[ SlotID ].pObject );

    return m_ObjectSlot[ SlotID ].NextVis;

}

//==============================================================================

inline
void obj_mgr::DestroyObject( guid ObjectGuid )
{
    DestroyObjectEx( ObjectGuid, FALSE );
}

//==============================================================================

//
//  global instance is declared in the CPP file.  Declared extern here so it ca
//  be accessed by any file that includes the header.  Simple version of 
//  a singleton...
//
extern obj_mgr g_ObjMgr;

inline
s32 GetNLogicLoops( void ) { return g_ObjMgr.GetNLogicLoops(); }

//
//  Pip stuff
//
#ifdef TARGET_XBOX
    enum PipTarget
    {
        kTARGET_MAIN,
        kTARGET_PIP,
        kTARGET_OFF
    };
    void xbox_SetPipTarget( s32 PipTarget,s32 X,s32 Y );
#endif

//==============================================================================
// END
//==============================================================================
#endif

