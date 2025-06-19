#ifndef OBJECT_HPP
#define OBJECT_HPP
//////////////////////////////////////////////////////////////////////////////
//
//  Object.hpp
//
//      Defines base object class from which all game objects should inherit.
//      When creating a new type, inherit from this, add the logic to
//      obj_mgr for it's creation, and add a type for it here in the
//      object_type enum.  Constructor will be private to insure that only the
//      obj_mgr can create the object. - CDS
//
// TODO:
//------------------------------------------------------------------------------
// * We need to add the collision message
//  virtual     void        OnCollision ( const object& OtherObject, const collision& Info);
//------------------------------------------------------------------------------
// * We are thinking in adding a generic message system
//  virtual         bool        OnMessage       ( message& Message );
//------------------------------------------------------------------------------
// WARNING: Objects can't have any pointers
//------------------------------------------------------------------------------
// * Get and Set name needs to wait untill the string table gets in place
///////////////////////////////////////////////////////////////////////////////

#include "x_math.hpp"
#include "x_bitstream.hpp"
#include "MiscUtils\RTTI.hpp"
#include "MiscUtils\Property.hpp"
#include "Animation\AnimData.hpp"
#include "Render\CollisionVolume.hpp"
#ifdef X_EDITOR
#include "Render\Render.hpp"
#endif

//==============================================================================
// DEFINES
//==============================================================================

#define OBJECT_DATA_VERSION         1000
#define MAX_OBJECT_NAME_LENGTH 32

#if defined (X_EDITOR) || defined(CONFIG_DEBUG) || defined (CONFIG_OPTDEBUG) 
#define USE_OBJECT_NAMES
#endif

#if defined (X_EDITOR) || defined(CONFIG_DEBUG) || defined (CONFIG_OPTDEBUG) 
#define USE_OBJECT_DEBUGINFO
#endif

//==============================================================================
// PRE-DECLARE CLASSES
//==============================================================================

class  text_in;
class  object_desc;
class  netobj;
class  view;
struct geom;
class  render_inst;
class  pain;
struct event;
class  object;
class object_affecter;
class simple_anim_player;

//==============================================================================
// BASE OBJECT
//==============================================================================

class object : public prop_interface
{
//------------------------------------------------------------------------------
//  Public Types
//------------------------------------------------------------------------------
public:

    CREATE_RTTI_BASE( object );

    enum type
    {
        TYPE_NULL,

        // NOTE: Lights and projectors must be first in this list so that
        //       they are the first things rendered!
        TYPE_LIGHT,
        TYPE_CHARACTER_LIGHT,
        TYPE_TEAM_LIGHT,
        TYPE_DYNAMIC_LIGHT,
        TYPE_PROJECTOR,

        TYPE_TRIGGER,
        TYPE_TRIGGER_EX,

        // The audio driven cinema object needs to be above ALL THE OBJECTS IT CAN CONTROL
        // so that all cinema objects stay perfectly in sync and WYSIWYG from max.
        TYPE_AUDIO_DRIVEN_CINEMA,

        // Trackers are placed early so they can move objects before the object
        // runs it's logic.
        TYPE_TRACKER,       
        TYPE_CONTROLLER,      

        TYPE_PLAY_SURFACE,
        TYPE_ANIM_SURFACE,
        TYPE_PROP_SURFACE,
        TYPE_REACTIVE_SURFACE,

        TYPE_COUPLER, //-- Do not move the coupler has been placed here and both fixes (couple to anim ans well as elevator to player uses )

        TYPE_NPC,
        TYPE_NAVNODE,
        TYPE_NET_GHOST,        

        TYPE_LENS_FILTER,
        TYPE_SND_EMITTER,
        TYPE_PICKUP,
        TYPE_DOOR,
        TYPE_NAV_NODE_PLACE_HOLDER,
        TYPE_NAV_CONNECTION_PLACE_HOLDER,
        TYPE_EDITOR_STATIC_DECAL,
        TYPE_EDITOR_BLUEPRINT_ANCHOR,
        TYPE_EDITOR_NOTEPAD_OBJECT,
        TYPE_MARKER_OBJECT,
        TYPE_LOCOMOTION_OBJECT,
        TYPE_LOCOMOTION_TESTER,
        TYPE_SPAWN_POINT,

        TYPE_CORPSE,
        TYPE_GHOST_IMAGE,
        TYPE_ZONE_PORTAL,
        TYPE_NAV_CONNECTION_EDITOR,
        TYPE_NAV_NODE_EDITOR,
        TYPE_COVER_NODE,
        TYPE_ALARM_NODE,
        TYPE_SPATIAL_TRIGGER,
        TYPE_VIEWABLE_SPATIAL_TRIGGER,
        TYPE_STATIC_DECAL_OBJECT,
        TYPE_SKIN_PROP_ANIM_SURFACE,
        TYPE_MUTAGEN_RESERVOIR,

        TYPE_BLACK_OPPS,
        TYPE_HAZMAT,
        TYPE_GRUNT,
        TYPE_MUTANT_TANK,
        TYPE_GOD,
        TYPE_STAGE5,
        TYPE_GRAY,
        TYPE_GENERIC_NPC,
        TYPE_TURRET,
        TYPE_DEBRIS,
        TYPE_DEBRIS_RIGID,
        TYPE_DEBRIS_GLASS,
        TYPE_DEBRIS_FRAG_EXPLOSION,
        TYPE_DEBRIS_ALIEN_GRENADE_EXPLOSION,
        TYPE_DEBRIS_MESON_EXPLOSION,

        TYPE_LADDER_FIELD,
        TYPE_SPAWNER_OBJECT,
        TYPE_SIMPLE_SND_EMITTER,
        TYPE_FRIENDLY_SCIENTIST,
        TYPE_FRIENDLY_SOLDIER,
        TYPE_PLAYER,

        TYPE_PARTICLE,
        TYPE_PARTICLE_EVENT_EMITTER,

        TYPE_WEAPON,
        TYPE_WEAPON_SMP,
        TYPE_WEAPON_DUAL_SMP,
        TYPE_WEAPON_DUAL_SHT,
        TYPE_WEAPON_SHOTGUN,
        TYPE_WEAPON_SCANNER,
        TYPE_WEAPON_SNIPER,
        TYPE_WEAPON_GAUSS,
        TYPE_WEAPON_DESERT_EAGLE,
        TYPE_WEAPON_FRAG_GRENADE,
        TYPE_WEAPON_MUTATION,
        TYPE_WEAPON_MHG,
        TYPE_WEAPON_GRAV_CHARGE,        
        TYPE_WEAPON_MSN,
        TYPE_WEAPON_BBG,
        TYPE_WEAPON_TRA,

        PROJECTILE_BEGIN,
        TYPE_SPIKE_PROJECTILE,
        TYPE_BULLET_PROJECTILE,
        TYPE_GRENADE_PROJECTILE,
        TYPE_JUMPING_BEAN_PROJECTILE,
        TYPE_GRAV_CHARGE_PROJECTILE,
        TYPE_ENERGY_PROJECTILE,
        TYPE_SEEKER_PROJECTILE,
        TYPE_MESONSEEKER_PROJECTILE,
        TYPE_HOMING_PROJECTILE,
        TYPE_MUTANT_PARASITE_PROJECTILE,
        TYPE_MUTANT_CONTAGION_PROJECTILE,
        TYPE_MUTANT_TENDRIL_PROJECTILE,
        PROJECTILE_END,

        TYPE_CLOTH_OBJECT,
        TYPE_CHARACTER_TASK,

        TYPE_INVENTORY_ITEM,
        TYPE_INPUT_SETTINGS,
        TYPE_LEVEL_SETTINGS,        
        TYPE_GLASS_SURFACE,

        TYPE_MANIPULATOR,
        TYPE_ANIMATION_OBJECT,
        TYPE_FOCUS_OBJECT,
        TYPE_LORE_OBJECT,
        TYPE_HUD_OBJECT,

        TYPE_PLAY_SURFACE_PROXY,
        
        TYPE_EVENT_SND_EMITTER,
        TYPE_DAMAGE_FIELD,
        
        TYPE_DESTRUCTIBLE_OBJ,
        
        TYPE_NAV_CONNECTION2_EDITOR,
        TYPE_NAV_CONNECTION2_ANCHOR,
    
        TYPE_PATH,
        TYPE_CAMERA,
        TYPE_PIP,
        TYPE_THIRD_PERSON_CAMERA,
        TYPE_NAV_POINT,
        TYPE_EXPLOSIVE_BULLET_PROJECTILE,
        
        TYPE_INVENTORY_HEALTH_ITEM,

        TYPE_INVENTORY_MUTAGEN_ITEM,

        TYPE_ALIEN_TURRET_PROJECTILE,
        TYPE_ALIEN_ORB,
        TYPE_ALIEN_SPOTTER,
        TYPE_ALIEN_ORB_SPAWNER,
        TYPE_ALIEN_SPAWN_TUBE,
        TYPE_ALIEN_GLOB,
        TYPE_ALIEN_SHIELD,

        TYPE_GZ_CORE_OBJ,
        TYPE_FEEDBACK_EMITTER,

        TYPE_VOLUMETRIC_LIGHT,
        TYPE_BOMB,
        TYPE_COKE_CAN,
        TYPE_SUPER_DESTRUCTIBLE_OBJ,

        TYPE_INVISIBLE_WALL_OBJ,

        TYPE_DEBRIS_GLASS_CLUSTER,

        TYPE_GROUP,

        TYPE_ALIEN_PLATFORM,
        TYPE_ALIEN_PLATFORM_DOCK,

        TYPE_VIDEO_WALL,

        // BEGIN - Multiplayer specific objects.
     // TYPE_SPAWN_REGION,
        TYPE_GAME_PROP,
        TYPE_FLAG,
        TYPE_BLUEPRINT_BAG,
        TYPE_MP_SETTINGS,
        TYPE_JUMP_PAD,
        TYPE_TELEPORTER,
        TYPE_TEAM_PROP,
        TYPE_FORCE_FIELD,
        TYPE_CAP_POINT,
        TYPE_FLAG_BASE,
     // TYPE_NEXUS,
     // TYPE_GATEWAY,
     // TYPE_HOLOGRAM,
        // END   - Multiplayer specific objects.        

        TYPE_END_OF_LIST,
        TYPE_ALL_TYPES = 0xFFFFFFFF
    };

    enum object_attr
    {
        // WARNING: DANGER WILL ROBINSON!
        // If you add any bits, make sure the attribute AND flags masks are updated,
        // and make sure you haven't clobbered any flags below!
        ATTR_NULL                       =      0,
        ATTR_NEEDS_LOGIC_TIME           = BIT( 0),      // This flag indicates that this object needs time for logic
        ATTR_DRAW_2D                    = BIT( 1),      // Objects that are rendered in 2D.
        ATTR_COLLIDABLE                 = BIT( 2),      // This object can collide with other objects
        ATTR_RENDERABLE                 = BIT( 3),      // This object is actually renderable in the normal game
        ATTR_TRANSPARENT                = BIT( 4),      // This is use with the render flag to indicate whether this objects uses a specialice pipe line.
        ATTR_PLAYER                     = BIT( 5),      // This is a player, either local human player, remote human player, or monkey
        ATTR_DAMAGEABLE                 = BIT( 6),      // This object can be destroyed
        ATTR_SOUND_SOURCE               = BIT( 7),      // This object can create emit sounds    
        ATTR_SPACIAL_ENTRY              = BIT( 8),      // Obj mgr knows when to add/remove from the spacial
        ATTR_DESTROY                    = BIT( 9),      // Flag should be set when the object is marked for death
        ATTR_NO_RUNTIME_SAVE            = BIT(10),      // This dynamic object should not be saved as part of save load
        ATTR_COLLISION_PERMEABLE        = BIT(11),      // This flag makes a collidable object not stop movement of other objects
        ATTR_EDITOR_SELECTED            = BIT(12),      // This flag is set when the editor selects an object.
        ATTR_EDITOR_BLUE_PRINT          = BIT(13),      // This flags is set if the object is part of blue print.
        ATTR_EDITOR_TEMP_OBJECT         = BIT(14),      // This object is a temporary editor only object (not exported)
        ATTR_EDITOR_PLACEMENT_OBJECT    = BIT(15),      // This object is a temporary editor only object used for placement
        ATTR_LIVING                     = BIT(16),      // This object is a living type object.  Should inclue all things that inherit from actor
        ATTR_CHARACTER_OBJECT           = BIT(17),      // This object is derived from the character class
        ATTR_DISABLE_PROJ_SHADOWS       = BIT(18),      // This object cannot receive projector shadows (artist-placed)
        ATTR_CAST_SHADOWS               = BIT(19),      // This object can cast dynamic shadows
        ATTR_RECEIVE_SHADOWS            = BIT(20),      // This object can receive dynamic shadows
        ATTR_DESTRUCTABLE_OBJECT        = BIT(21),      // This object is a destructible or superdestructable object
        ATTR_ACTOR_RIDEABLE             = BIT(22),      // This object will apply collision data to bullets NOTE!!! Only use this on decendants of anim_surface for now.  CharacterPhysics does some blind casts, and we would need to implement GetBoneL2W at the object level to support this anywhere
        ATTR_BLOCKS_PLAYER              = BIT(23),      // This object blocks player movement
        ATTR_BLOCKS_CHARACTER           = BIT(24),      // This object blocks character movement
        ATTR_BLOCKS_RAGDOLL             = BIT(25),      // This object blocks ragdoll movement
        ATTR_BLOCKS_SMALL_PROJECTILES   = BIT(26),      // This object blocks small projectiles
        ATTR_BLOCKS_LARGE_PROJECTILES   = BIT(27),      // This object blocks large projectiles
        ATTR_BLOCKS_CHARACTER_LOS       = BIT(28),      // This object blocks character LOS
        ATTR_BLOCKS_PLAYER_LOS          = BIT(29),      // This object blocks player LOS i.e. AimAssist
        ATTR_BLOCKS_PAIN_LOS            = BIT(30),      // This object blocks pain LOS
        ATTR_BLOCKS_SMALL_DEBRIS        = BIT(31),      // This object blocks small debris movement
        ATTR_ALL                        = 0xFFFFFFFF,   // All attributes
        ATTR_BLOCKS_ALL_PROJECTILES     = (ATTR_BLOCKS_SMALL_PROJECTILES | ATTR_BLOCKS_LARGE_PROJECTILES),
        ATTR_BLOCKS_ALL_ACTORS          = (ATTR_BLOCKS_PLAYER | ATTR_BLOCKS_CHARACTER),
    };

    // Make sure that this matches with the material type in the editor.
    enum material_type
    {
        MAT_TYPE_NULL                   = 0,
        MAT_TYPE_EARTH,
        MAT_TYPE_ROCK,
        MAT_TYPE_CONCRETE,
        MAT_TYPE_SOLID_METAL,
        MAT_TYPE_HOLLOW_METAL,    
        MAT_TYPE_METAL_GRATE,
        MAT_TYPE_PLASTIC,
        MAT_TYPE_WATER,
        MAT_TYPE_WOOD,
        MAT_TYPE_ENERGY_FIELD,
        MAT_TYPE_BULLET_PROOF_GLASS,
        MAT_TYPE_ICE,

        MAT_TYPE_LEATHER,
        MAT_TYPE_EXOSKELETON,
        MAT_TYPE_FLESH,
        MAT_TYPE_BLOB,
        
        MAT_TYPE_FIRE,
        MAT_TYPE_GHOST,
        MAT_TYPE_FABRIC,
        MAT_TYPE_CERAMIC,
        MAT_TYPE_WIRE_FENCE,

        MAT_TYPE_GLASS,
        MAT_TYPE_RUBBER,

        MAT_TYPE_CARPET,
        MAT_TYPE_CLOTH,
        MAT_TYPE_DRYWALL,
        MAT_TYPE_FLESHHEAD,
        MAT_TYPE_MARBLE,
        MAT_TYPE_TILE,

        MAT_TYPE_LAST,
    };

    struct detail_tri
    {
        vector3                     Vertex[3];
        vector3                     Normal[3];
        vector2                     UV    [3];
        xcolor                      Color [3];
        collision_data::mat_info    MaterialInfo;
    }; 

    struct debug_info
    {
        object_desc* m_pDesc;
    };

//------------------------------------------------------------------------------
// PUBLIC MESSAGES
//------------------------------------------------------------------------------
// Public messages are messages that objects can send to other objects including
// them selves. When handling the message must call the parent class version.
//------------------------------------------------------------------------------
//
// OnMove        - Sets the absolute position for the object.
//
// OnMoveRel     - Moves the object relative to current pos.
//
// OnTransfrom   - Moves/Rotate/Scale Object. Note that this is the most optimal function.
//
// OnTriggerTransform - Some objects need to re-initialize some of their internal data when moved
//                      through the trigger system.  This allows those objects to do so.
//
// OnEnumProp           - Enumerates all the properties that can be saved/loaded/edited.
//
// OnProperty           - Read/writes property. Can be called from UI edit, or save/load.
//
// OnValidateProperties - Used by editor only to make sure properties are valid.
//
// OnLoad        - Loads an object. Note it has to came from a text_in. Also the property has the initial onload
//
// OnColNotify  - Some one is colling with you. The object is past as a parameter.
//
// OnActivate   - Used within the Trigger system to notifiy an object to activate
//
//------------------------------------------------------------------------------
public:

    virtual void                OnMove              ( const vector3& NewPos   );      
    virtual void                OnMoveRel           ( const vector3& DeltaPos );    
    virtual void                OnTransform         ( const matrix4& L2W      );
    virtual void                OnTriggerTransform  ( const matrix4& L2W      );
    virtual void                OnEnumProp          ( prop_enum&     List     );
    virtual xbool               OnProperty          ( prop_query&    I        );

#ifdef X_EDITOR
    // Property validation utility functions
    virtual s32                 OnValidateGeomAnim  ( xstring& Desc, render_inst* pRenderInst, anim_group::handle* pAnimGroupHandle, xstring& ErrorMsg );
    virtual s32                 OnValidateObject    ( xstring& Desc, guid* pGuid, xstring& ErrorMsg );
    virtual s32                 OnValidateObject    ( xstring& Desc, object_affecter* pAffecter, xstring& ErrorMsg );
    virtual s32                 OnValidateAnim      ( xstring& Desc, s32* pAnimGroupName, s32 AnimName, xstring& ErrorMsg );
    virtual s32                 OnValidateSound     ( xstring& Desc, rhandle<char>* phSoundPackage, s32 SoundName, xstring& ErrorMsg );
    virtual s32                 OnValidateGlobal    ( xstring& Desc, s32* pGlobalName, xstring& ErrorMsg );
    virtual s32                 OnValidateProperty  ( xstring& Desc, object_affecter* pAffecter, s32* pPropertyName, s32 PropertyType, xstring& ErrorMsg );
    virtual s32                 OnValidateTemplate  ( xstring& Desc, s32* pTemplateName, xstring& ErrorMsg );

    // Top level validation function
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

    virtual void                OnLoad              ( text_in&       TextIn   );
    virtual void                OnPaste             ( const xarray<prop_container>& Container );

    virtual void                OnPain              ( const pain& Pain ) ;  // Tells object to recieve pain
    virtual xbool               OnChildPain         ( guid ChildGuid, const pain& Pain );
    
    virtual void                OnColNotify         ( object& Object ) {(void)Object;}    
    virtual void                OnActivate          ( xbool Flag );            
    virtual void                OnEvent             ( const event& Event );
    virtual vector3             GetSubPosition      ( s32 ID );

    virtual xbool               GetColDetails       ( s32 Key, detail_tri& Tri );
    virtual void                OnPolyCacheGather   ( void );
    virtual const char*         GetLogicalName      ( void );

    virtual void                OnAddedToGroup      ( guid gGroup );

//------------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//------------------------------------------------------------------------------
//
// GetLocalBBox    - User must provide a way to get the local bbox
//
// GetType         - Gets the object type. Must be provided by the user.
//
// GetBBox         - Gets the world bbox
//
// GetPosition     - Gets the position of the object
//
// GetVelocity     - Gets the movement velocity of the object
//
// ScaleVelocity   - Scales velocity along a plane
//                         PlaneNormal = Collision plane
//                         PerpScale   = Scales velocity along plane normal
//                         ParaScale   = Scales velocity parallel to plane
//
// GetLookAtExtent - Gets the fraction of the height where the object should be looked at
//
// GetL2W          - Get the Local to World Matrix   
//
// GetW2L          - Gets the World to Local Matrix
//
// OnDebugRender   - Renders the object in a non-standard way. Default is just a bbox.
//
// GetAttrBits     - Get the attribute bits
//
// SetAttrBits     - Overwrites the attribute bits with a new set    
//
// GetFlagBits     - Get the flag bits
//
// SetFlagBits     - Overwrites the flag bits with a new set    
//
// GetGuid         - Gets the guid of the object
//
// EnumAttachPoints         - Returns a string of attachpoints formatted suitably for 
//                            the AddEnum property call
//
// GetAttachPointIDByName   - Gets the s32 ID of an attach point
//  
// GetAttachPointNameByID   - Gets the string name of an attach point
//
// GetAttachPointData       - Retrieves a matrix describing the attach point
//
// OnAttachedMove           - Performs an attached move operation
// 
//------------------------------------------------------------------------------
public:

    virtual                    ~object          ( void );                                
    virtual       bbox          GetLocalBBox    ( void ) const = 0;      
    virtual       s32           GetMaterial     ( void ) const = 0;
    virtual const object_desc&  GetTypeDesc     ( void ) const = 0;
    virtual       f32           GetLookAtExtent ( void ) const { return .95f; }
                  type          GetType         ( void ) const;      

            const bbox&         GetBBox         ( void );                
    virtual       bbox          GetColBBox      ( void );                
    virtual       bbox          GetScreenBBox   ( const view& rView );
                  vector3       GetPosition     ( void ) const;          
    virtual       vector3       GetVelocity     ( void ) const;          
    virtual       void          ScaleVelocity   ( const vector3& PlaneNormal, f32 PerpScale, f32 ParaScale );
    
            const matrix4&      GetL2W          ( void ) const;          

            u32                 GetRenderMode   ( void ) const;
            u32                 GetAttrBits     ( void ) const;
            void                SetAttrBits     ( u32 NewBits );
            void                TurnAttrBitsOn  ( u32 BitsToTurnOn );
            void                TurnAttrBitsOff ( u32 BitsToTurnOff );
            u32                 GetFlagBits     ( void ) const;
            void                SetFlagBits     ( u32 NewBits );
    virtual xbool               IsActive        ( void );

            guid                GetGuid         ( void ) const; 
            s32                 GetSlot         ( void ) const;

            u16                 GetZones        ( void ) const;
            u16                 GetZone1        ( void ) const;
            u16                 GetZone2        ( void ) const;            

            void                SetZones        ( u16 Zones );
            void                SetZone1        ( u16 Zone1 );
            void                SetZone2        ( u16 Zone2 );

            void                LoadStart       ( void );
            void                LoadEnd         ( void );

#ifdef X_EDITOR
    void                        EnableDrawBBox  ( void ) { m_bDrawBBox = true;    }
    void                        DisableDrawBBox ( void ) { m_bDrawBBox = false;   }
    virtual void                EditorPreGame   ( void ) {};
#endif// X_EDITOR

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );   
    void                        RenderCollision ( xbool bRenderHigh ) { OnColRender(bRenderHigh); }
#endif // X_RETAIL

    // Life functions
    virtual xbool               IsAlive         ( void ) ;              // Returns true if alive
    virtual f32                 GetHealth       ( void ) ;              // Returns amount of life left

    enum 
    {
        ATTACH_USE_WORLDSPACE = 1,      // Return the attachpoint in worldspace
    };

    virtual void                EnumAttachPoints      ( xstring& String   ) const;
    virtual s32                 GetAttachPointIDByName( const char* pName ) const;
    virtual xstring             GetAttachPointNameByID( s32 iAttachPt     ) const;
    virtual xbool               GetAttachPointData    ( s32      iAttachPt,
                                                        matrix4& L2W,
                                                        u32      Flags = 0 );
    virtual void                OnAttachedMove        ( s32             iAttachPt,
                                                        const matrix4&  L2W );

    virtual guid                GetParentGuid         ( void ) { return 0; }
    
    virtual render_inst*        GetRenderInstPtr      ( void ) { return NULL; }
    virtual geom*               GetGeomPtr            ( void );
    virtual const char*         GetGeomName           ( void );

    virtual simple_anim_player* GetSimpleAnimPlayer   ( void ) { return NULL; }
    virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return NULL; }
    virtual anim_group*         GetAnimGroupPtr       ( void );
    virtual const char*         GetAnimGroupName      ( void );

#if defined( USE_OBJECT_NAMES )
    const char *                GetName                 ( void );
    void                        SetName                 ( const char *pNewObjectName);
#endif //defined( USE_OBJECT_NAMES )

#ifdef X_EDITOR
    xbool                       IsHidden                ( void );
    xbool                       IsSelectable            ( void );
    virtual xbool               IsIconSelectable        ( void ) { return TRUE; }
    void                        SetHidden               ( xbool bHidden );
    void                        SetSelectable           ( xbool bSelectable );
#endif// X_EDITOR

    virtual s32                 net_GetSlot             ( void ) const { return -1; }

//------------------------------------------------------------------------------
// PROTECTED TYPES
//------------------------------------------------------------------------------
protected:

    enum flags
    {
        // WARNING: DANGER WILL ROBINSON!
        // If you add any bits, make sure the attribute AND flags masks are updated,
        // and make sure you haven't clobbered any attributes above!
        FLAG_CHECK_PLANES          = BIT( 0) | BIT( 1) | BIT( 2) | BIT( 3) | BIT( 4)| BIT( 5),
        FLAG_CHECK_PLANES_SHIFT    = 0,        // make sure this matches the flags above!
        FLAG_LOADING               = BIT( 8),  // The object is in a loading state. This is set/clear when the object loads
        FLAG_DIRTY_TRANSLATION     = BIT( 9),
        FLAG_DIRTY_ROTATION        = BIT(10),
        FLAG_DIRTY_TRANSFORM       = FLAG_DIRTY_TRANSLATION | FLAG_DIRTY_ROTATION,
    };

//------------------------------------------------------------------------------
// PROTECTED MESSAGES
//------------------------------------------------------------------------------
// Protected messages are messages that only the system can send to objects.
// All messages are hirarchical so user must notify parent class.
//------------------------------------------------------------------------------
//
// OnInit               - Initialize the object. This is only call when the object is
//                        created, not imported. TODO: This is not 100% decided.
//
// OnKill               - When the object is decided to be kill then this function is call.
// 
// OnAdvanceLogic       - This function is call when the object is requested to advance its logid.
//
// OnSpacialUpdate      - Updates the spacial data base with the new bbox
//
// OnColCheck           - Talk to the collision manager to be tested for collisions.
//
// OnRender             - Official function to render the object in the game.
//
// OnRenderCloth        - Special function for rendering cloth on the xbox
//
// OnColRender          - Render the collision structure that this object uses to collide with.
//
// OnRenderTransparent  - Get called after all normal objects are done rendering, requires ATTR_TRANSPARENT
//
//------------------------------------------------------------------------------
protected:

    virtual void                OnInit              ( void );     
    virtual void                OnKill              ( void );               
    virtual void                OnAdvanceLogic      ( f32 DeltaTime );      
    virtual void                OnSpacialUpdate     ( void );               
    virtual void                OnColCheck          ( void );    

    virtual void                OnRender                ( void );
#ifdef TARGET_XBOX
    virtual void                OnRenderCloth           ( void );
#endif    
    virtual void                OnRenderShadowCast      ( u64 ProjMask );
    virtual void                OnRenderShadowReceive   ( u64 ProjMask );
#ifndef X_RETAIL
    virtual void                OnColRender             ( xbool bRenderHigh );
#endif
    virtual void                OnRenderTransparent     ( void );
            xbool               NeedsClipping           ( void );
    virtual void                OnAnimEvent             ( void );

//------------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------------
//
// object           - This constructor should be call by the descendent object
//
// SetTransform     - Overwrites the L2W matrix but doesn't update anything else
//
// UpdateTransform  - Updates so that it is upto date with the spacial database etc.
//
//------------------------------------------------------------------------------
protected:
                                object          ( void );
    void                        SetTransform    ( const matrix4& L2W );
    void                        UpdateTransform ( void );
    xbool                       IsLoading       ( void );
    
    
    void                        SetNewZoneInfo          ( u16 ZoneInfo );
    void                        UpdateRenderableLinks   ( void );

private:
    xbool                       HandleAttribute( prop_query& I, const char* PropName, u32 AttrBit );
    object&                     operator =      ( const object& Object );

//------------------------------------------------------------------------------
// NETWORKING HOOK
//------------------------------------------------------------------------------
//
// AsNetObj         - If implemented, returns object as a netobj*.
//
//------------------------------------------------------------------------------
public:

    virtual     netobj*         AsNetObj        ( void )  { return( NULL ); };

//------------------------------------------------------------------------------
//
//
// !!! WARNING !!! - Be sure to consider alignment when adding member variables
//                   so the class size doesn't grow unexpectedly
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PRIVATE VARS
//------------------------------------------------------------------------------
private:
                                                    //  ( 4) Virtual table ptr
    u32                 m_AttrBits;                 //  ( 4) Attribute bits
    guid                m_Guid;                     //  ( 8) Unique ID for the object
    matrix4             m_L2W;                      //  (64) Takes the object to the world
    bbox                m_WorldBBox;                //  (32) World BBox.
    u32                 m_FlagBits;                 //  ( 4) Flag bits
    u16                 m_SlotID;                   //  ( 2) What is the slot of the object

//------------------------------------------------------------------------------
// PROTECTED VARS
//------------------------------------------------------------------------------
protected:

#if defined( USE_OBJECT_NAMES )
    char m_Name[MAX_OBJECT_NAME_LENGTH];
#endif

private:
    u16                 m_ZoneInfo;                 // Objects can be in 2 zones (each 8 bits)
    
protected:
#ifdef X_EDITOR
    xbool               m_bDrawBBox;                // TODO: This needs to be deleted
    u16                 m_bHidden:1;                // Object is hidden in the editor
    u16                 m_bSelectable:1;            // Object is selectable in the editor
#endif // X_EDITOR

//------------------------------------------------------------------------------
// DATA ADDED TO OBJECT ONLY WHEN DEBUGGING
//------------------------------------------------------------------------------

#if defined( USE_OBJECT_DEBUGINFO )
public:
    debug_info          m_DebugInfo;    // Contains useful debugging info
#endif

//------------------------------------------------------------------------------

    friend class obj_mgr;
    friend class collision_mgr;
};


//==============================================================================
// INLINE IMPLEMENTATION
//==============================================================================

//===========================================================================

inline
bbox object::GetColBBox( void )
{
    return GetBBox();
}
    
//==============================================================================

inline
const bbox& object::GetBBox( void )
{
    if( m_FlagBits & FLAG_DIRTY_TRANSFORM )
        UpdateTransform();

    return m_WorldBBox;
}

//==============================================================================
inline 
void object::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "object::OnAdvanceLogic" );

    // KSS -- commenting this out until Craig gets a chance to look into it further -- As per Craig.
    //ASSERTS( 0, "Why are we advancing logic for for a base object?" );

    (void)DeltaTime;
}

//==============================================================================

inline
vector3 object::GetPosition( void ) const
{
    return m_L2W.GetTranslation();
}

//==============================================================================

inline
vector3 object::GetVelocity( void ) const
{
    return vector3( 0.0f, 0.0f, 0.0f );
}

//==============================================================================

inline
void object::ScaleVelocity( const vector3& PlaneNormal, f32 PerpScale, f32 ParaScale )
{
    (void)PlaneNormal;
    (void)PerpScale;
    (void)ParaScale;
}

//==============================================================================

inline
vector3 object::GetSubPosition( s32 ID )
{
    (void)ID;

    return m_L2W.GetTranslation();
}

//==============================================================================

inline
xbool object::GetColDetails( s32 Key, object::detail_tri& Tri )
{
    (void)Key;
    (void)Tri;
    return( FALSE );
}

//==============================================================================

inline
const matrix4& object::GetL2W( void ) const
{
    return m_L2W;
}

//==============================================================================

inline
u32 object::GetRenderMode( void ) const
{
#ifdef X_EDITOR
    if( GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT )
    {
        return render::PULSED;
    }
    else if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        if( GetAttrBits() & ATTR_EDITOR_BLUE_PRINT )
            return render::WIREFRAME2;
        else
            return render::WIREFRAME;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

//==============================================================================

inline
u32 object::GetAttrBits( void ) const
{
    return m_AttrBits;
}

//==============================================================================

inline
void object::SetAttrBits( u32 NewBits ) 
{
    if( (NewBits & object::ATTR_RENDERABLE) != (m_AttrBits & object::ATTR_RENDERABLE) )
    {
        m_AttrBits = NewBits;
        UpdateRenderableLinks();
    }
    else
    {
        m_AttrBits = NewBits;
    }

#ifdef X_EDITOR
    // For the editor all the objects are in space in order to be editable
    m_AttrBits |=  ATTR_SPACIAL_ENTRY;
#endif // X_EDITOR
}

//==============================================================================

inline
void object::TurnAttrBitsOn( u32 BitsToTurnOn )
{
    SetAttrBits( m_AttrBits | BitsToTurnOn );
}

//==============================================================================

inline
void object::TurnAttrBitsOff( u32 BitsToTurnOff )
{
    SetAttrBits( m_AttrBits & (~BitsToTurnOff) );
}

//==============================================================================

inline
u32 object::GetFlagBits( void ) const
{
    return m_FlagBits;
}

//==============================================================================

inline
void object::SetFlagBits( u32 NewBits ) 
{
    m_FlagBits = NewBits;
}

//==============================================================================

inline
guid object::GetGuid( void ) const
{
    return m_Guid;
}

//==============================================================================

inline
s32 object::GetSlot( void ) const
{
    return m_SlotID;
}

//==============================================================================

inline
void object::OnRender( void ) 
{
    CONTEXT( "object::OnRender" );

    ASSERT( 0 );
}

//==============================================================================

#ifdef TARGET_XBOX

inline
void object::OnRenderCloth( void )
{
    CONTEXT( "object::OnRenderCloth" );

    ASSERT( 0 );
}

#endif

//==============================================================================

inline
void object::OnRenderShadowCast( u64 ProjMask )
{
    (void)ProjMask;
}

//==============================================================================

inline
void object::OnRenderShadowReceive( u64 ProjMask )
{
    (void)ProjMask;
}

//==============================================================================
inline
xbool object::NeedsClipping( void )
{
    return (m_FlagBits & FLAG_CHECK_PLANES) != 0;
}

//==============================================================================
inline
void object::OnAnimEvent( void )
{
    ASSERT( 0 );
}

//==============================================================================
inline
xbool object::IsLoading( void )
{
    return ( m_FlagBits & FLAG_LOADING ) !=0;
}

//==============================================================================
inline
void object::SetTransform( const matrix4& L2W )
{
    m_FlagBits |= FLAG_DIRTY_TRANSFORM;
    m_L2W = L2W;
}

//==============================================================================

// Life functions
inline
xbool object::IsAlive( void )
{
    f32 Health = GetHealth( );
    return( Health > 0.0f );
}

//==============================================================================

inline
f32 object::GetHealth( void )
{
    return 100.0f;
}

//==============================================================================

inline
u16 object::GetZones( void ) const
{
    return m_ZoneInfo;
}

//==============================================================================

inline
u16 object::GetZone1( void ) const
{
    return m_ZoneInfo&0xff;
}

//==============================================================================

inline
u16 object::GetZone2( void ) const
{
    return (m_ZoneInfo>>8);
}

//==============================================================================

inline
void object::SetZones( u16 Zones )
{
    SetZone1( Zones&0xff );
    SetZone2( (Zones&0xff00)>>8 );
}

//==============================================================================

inline
void object::SetZone1( u16 Zone1 )
{
    u16 NewZoneInfo = (m_ZoneInfo&0xff00)|Zone1;
    if( NewZoneInfo != m_ZoneInfo )
    {
        SetNewZoneInfo(NewZoneInfo);
    }
}

//==============================================================================

inline
void object::SetZone2( u16 Zone2 )
{
    u16 NewZoneInfo = (m_ZoneInfo&0xff)|(Zone2<<8);
    if( NewZoneInfo != m_ZoneInfo )
    {
        SetNewZoneInfo(NewZoneInfo);
    }
}

//==============================================================================

inline
void object::LoadStart       ( void )
{
    ASSERT( (m_FlagBits&FLAG_LOADING) == 0 );
    // Okay indicate that we are loading
    m_FlagBits |= FLAG_LOADING;
}

//==============================================================================

inline
void object::LoadEnd         ( void )
{
//    ASSERT( m_FlagBits & FLAG_LOADING );

    // Indicate that we are done loading
    m_FlagBits &= ~FLAG_LOADING;

    //
    // Now that we are done loading lets make sure that we end up in the spacial data base
    //
    OnSpacialUpdate();
}

//==============================================================================

inline
xbool object::IsActive         ( void )
{
    return (m_AttrBits & ATTR_NEEDS_LOGIC_TIME) != 0;
}

//==============================================================================

inline
void object::OnPolyCacheGather( void )
{
}

//==============================================================================

#ifdef X_EDITOR

inline
xbool object::IsHidden( void ) 
{
    return m_bHidden;
}

#endif

//==============================================================================

#ifdef X_EDITOR

inline
xbool object::IsSelectable( void ) 
{
    return m_bSelectable;
}

#endif

//==============================================================================

#ifdef X_EDITOR

inline
void  object::SetHidden( xbool bHidden ) 
{
    m_bHidden = bHidden;
}

#endif

//==============================================================================

#ifdef X_EDITOR

inline
void  object::SetSelectable( xbool bSelectable ) 
{
    m_bSelectable = bSelectable;
}    

#endif

//==============================================================================
#endif // OBJECT_HPP
//==============================================================================



