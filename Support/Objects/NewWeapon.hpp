//=========================================================================
// New Weapon
//=========================================================================
#ifndef _NEWWEAPON_HPP
#define _NEWWEAPON_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "..\Support\Characters\factions.hpp"
#include "Inventory/Inventory2.hpp"
#include "Objects/Render/RigidInst.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//=========================================================================
// DEFINES
//=========================================================================

//#define DEBUG_WEAPON_MUZZLE_FLASH  //Define this if u want to draw debug spheres for the muzzle flashes.

//=========================================================================

class bullet_projectile;
class base_projectile;
class hud_object;
class player;


//=========================================================================

// Animation controller that positions the left/right weapons on each hand
// (since both weapons are part of the same geometry, it has to be done at
//  the compute matrices level in the animation player - this controller
//  adjust the keys of the correct weapon bones to achieve this)
class dual_weapon_anim_controller : public track_controller
{

public:
                    dual_weapon_anim_controller( void );
    virtual        ~dual_weapon_anim_controller( void )  {}

    // Initializes
            void    Init                (       guid                WeaponGuid,
                                          const anim_group::handle& hAnimGroup,
                                          const char*               pLeftBone,
                                          const char*               pRightBone );

    // Sets location of animation data package
    virtual void    SetAnimGroup        ( const anim_group::handle& hGroup );

    // Clears the animation to a safe unused state
    virtual void    Clear               ( void )          {}

    // Advances the current track by logic time
    virtual void    Advance             ( f32 DeltaTime ) { (void)DeltaTime; }

    // Controls the influence this anim has during the mixing process
    virtual void    SetWeight           ( f32 Weight )  { (void)Weight; }
    virtual f32     GetWeight           ( void )        { return 1.0f;  }

    // Returns the raw keyframe data
    virtual void    GetInterpKeys       ( anim_key* pKey )              { (void)pKey; }
    virtual void    GetInterpKey        ( s32 iBone, anim_key& Key )    { (void)iBone; (void)Key; }

    // Mixes the anims keyframes into the dest keyframes
    virtual void    MixKeys             ( anim_key* pDestKey );

    // Removes yaw from root node of turn animations
    virtual void    SetRemoveTurnYaw    ( xbool bRemove )   { (void)bRemove; }
            
// Data
protected:
    anim_group::handle  m_hAnimGroup;       // Group of anims we are using
    s32                 m_iLWeaponBone;     // Index of left weapon bone
    s32                 m_iRWeaponBone;     // Index of right weapon bone
    guid                m_WeaponGuid;       // Guid of weapon owner
};

//=========================================================================
class new_weapon : public object
{
public:
    
    //weapons can either be rendered by the player or if an NPC is holding them.
    enum render_state
    {
        RENDER_STATE_UNDEFINED = -1,
        RENDER_STATE_PLAYER,
        RENDER_STATE_NPC,
        RENDER_STATE_MAX
    };

    //slots for the weapon's ammo to be held
    enum ammo_priority
    {
        AMMO_PRIMARY = 0,
        AMMO_SECONDARY,
        AMMO_MAX
    };
    
    //type of projectile that this weapon uses for ammunition.
    enum projectile_type
    {
        PROJECTILE_UNDEFINED = -1,      //Not a valid value.
        PROJECTILE_NONE,                //Used for MagLight and LAB Scanner

        BULLET_MUTATION,
        BULLET_MACHINE_GUN,             

// KSS -- TO ADD NEW WEAPON - if weapon has ammo
        BULLET_SHOTGUN,                 

        BULLET_SNIPER_RIFLE,            
        BULLET_GAUSS_RIFLE, 
        BULLET_PISTOL,
        BULLET_RIFT_BOT,                
        BULLET_MHG,
        BULLET_MSN,
        BULLET_MSN_SECONDARY,
        
        GRENADE_FRAG,                   
        GRENADE_FLASH,  
        GRENADE_HAND_NUKE,
        
        MINE_GRAVITATIONAL_CHARGE,
        
        PROJECTILE_MAX
    };
    
    enum weapon_events
    {
        EVENT_NULL,

        EVENT_FIRE,
        EVENT_ALT_FIRE,
        EVENT_GRENADE,
        EVENT_ALT_GRENADE,

        EVENT_FIRE_LEFT,
        EVENT_ALT_FIRE_LEFT,
        EVENT_FIRE_RIGHT,
        EVENT_ALT_FIRE_RIGHT,
        EVENT_CINEMA_FIRE,
    };
    
    enum fire_points
    {
        FIRE_POINT_DEFAULT,
        FIRE_POINT_LEFT,
        FIRE_POINT_RIGHT,        
        
        FIRE_POINT_COUNT
    };

    enum zoom_state
    {
        NO_ZOOM_WEAPON_STATE,
        WEAPON_STATE_ZOOM_IN,
        WEAPON_STATE_ZOOM_OUT,
    };

    struct reticle_radius_parameters
    {
        f32 m_MaxRadius;                 // in screen pixels, how big the reticle can be
        f32 m_MinRadius;                 // in screen pixels, how small the reticle can be
        f32 m_CrouchBonus;               // in screen pixels, how much the radius increases over m_MaxRadius when crouching
        f32 m_MaxMovementPenalty;        // worst penalty, in screen pixels, for actor movement
        f32 m_MoveShrinkAccel;           // acceleration rate for shrinking the reticle from movement
        f32 m_ShotShrinkAccel;           // acceleration rate for shrinking the reticle from shot penalties
        f32 m_GrowAccel;                 // acceleration rate for growing the reticle
        f32 m_PenaltyForShot;            // penalty in screen pixels for a single shot
        f32 m_ShotPenaltyDegradeRate;    // how fast the shot penalty degrades, in pixels/second
    };

    CREATE_RTTI( new_weapon , object , object )

    new_weapon();
    
    virtual ~new_weapon();


//=========================================================================
//
// FireWeapon           -   Fires a projectile from the weapon.  Public interface for player.
// FireSecondary        -   Secondary fire for the weapon
// IsWeaponReady        -   Returns true if the weapon is ready to fire.  False otherwise.
// CanReload            -   Returns true if the weapon can be reloaded.
// Reload               -   Reloads the weapon (Updates ammo counts)
// GetCurrentAnimGroup  -   Returns the current AnimGroup (reference) to the caller.
// SetAnimation         -   Sets the animation on the weapon.  This will only be used by the player.
// SetRotation          -   Sets the Rotation of the weapon in world space.
// ProcessSfx           -   Handles sound related animation events.
// GetLocalBBox         -   Bounding box information for the weapon.
// GetMaterial          -   Legacy code.  Returns material type.
// OnEnumProp           -   Enumerates all the properties related to the player
// OnProperty           -   Set/Gets properties for the player
// OnAdvanceLogic       -   Called once per loop.  Advances animation player and makes the calls to handle animation events.
// OnMove               -   Updates objects position in the world.
// OnColCheck           -   Overload.  Does nothing (weapons do not currently collide with other objects.)
// OnColNotify          -   Overload.  Does nothing (weapons do not currently collide with other objects.)
// OnRender             -   Render routine for the weapons.
// GetRenderState       -   Tells if the weapon is being rendered by the player or an NPC.
// GetAmmoCount         -   Returns the ammo amount for the object.
// GetSecondaryAmmoCount-   Returns the ammo amount for the object.   
// InitWeapon           -   Initializes the weapon's animations, bind pose and position if the weapon is being created on the fly.
// IsInited             -   Tells if the weapon has been initialized.
// GetTypeDesc          -   Returns the type descriptor of the object.
// GetObjectType        -   Returns the type of the object.
//
//=========================================================================

    virtual        void         OnEnumProp              ( prop_enum& list );
    virtual        xbool        OnProperty              ( prop_query& rPropQuery );

    //These fire weapon functions are dispatch functions...
    void                        SetTarget               ( guid TargetGuid ) { m_TargetGuid = TargetGuid; }
    xbool                       FireWeapon              ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    xbool                       FireSecondary           ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    
    xbool                       NPCFireWeapon           ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier = 1.0f, xbool isHit = TRUE );
    void                        NPCFireSecondary        ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier = 1.0f, xbool isHit = TRUE );

    virtual xbool               FireGhostPrimary        ( s32 iFirePoint, xbool bUseFireAt, vector3& FireAt );
    virtual xbool               FireGhostSecondary      ( s32 iFirePoint );

            xbool               IsUsingSplitScreen      ( void ); // for multiplayer splitscreen
    virtual xbool               IsWeaponReady           ( const ammo_priority& rAmmoPriority );
    virtual xbool               CanReload               ( const ammo_priority& Priority );
    virtual void                Reload                  ( const ammo_priority& Priority );
    virtual void                SetupDualAmmo           ( inven_item OtherWeaponItem );
    virtual xbool               CanFire                 ( xbool bIsAltFire );

    char_anim_player&           GetCurrentAnimPlayer    ( void );
    const anim_group&           GetCurrentAnimGroup     ( void );
    xbool                       HasAnimGroup            ( void );
    virtual void                SetAnimation            ( const s32& nAnimIndex , const f32& fBlendTime , const xbool& bResetFrames = FALSE );
    virtual f32                 GetFrameParametric      ( void );
    virtual void                SetFrameParametric      ( f32 Frame );

    vector3                     GetVelocity             ( void ) { return m_Velocity; }
    radian                      GetAngularSpeed         ( void ) { return m_AngularSpeed; }
    radian                      GetYaw                  ( void ) { return (m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] >= 0) ? m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] ).GetRotation().Yaw : 0; }
    void                        SetRotation             ( const f32& rPitch , const f32& rYaw );
    virtual void                ProcessSfx              ( void );

    virtual void                BeginPrimaryFire        ( void );
    virtual void                EndPrimaryFire          ( void );
    virtual void                BeginAltFire            ( void );
    virtual void                EndAltFire              ( void );
    virtual void                ReleaseAudio            ( void );

    virtual void                BeginAltRampUp          ( void ){}
    virtual void                EndAltRampUp            ( xbool bGoingIntoHold, xbool bSwitchingWeapon ){(void) bGoingIntoHold; (void) bSwitchingWeapon; }
    virtual void                EndAltHold              ( xbool bSwitchingWeapon ){(void) bSwitchingWeapon;}
    virtual void                BeginIdle               ( xbool bNormalIdle = TRUE ){ (void) bNormalIdle; }
    virtual void                EndIdle                 ( void ){}
    virtual xbool               CanSwitchIdleAnim       ( void ){ return TRUE; }

    virtual void                BeginSwitchFrom         ( void ){}
    virtual void                BeginSwitchTo           ( void ){}
    virtual void                EndSwitchTo             ( void ){}

    //mandatory overloads
    virtual       bbox          GetLocalBBox            ( void ) const;      
    virtual       s32           GetMaterial             ( void ) const;
    
    virtual void                ResetWeapon             ( void );

    virtual void                OnAdvanceLogic          ( f32 DeltaTime );      
    virtual void                OnMove                  ( const vector3& NewPos   );      
    virtual void                OnTransform             ( const matrix4& L2W      ); 
  
    virtual void                OnColCheck              ( void );    
//    virtual void                OnColNotify             ( object& Object );
    virtual void                OnKill                  ( void );

            void                DegradeAim              ( radian3& Rot, radian Amt, const vector3& InitPos, guid Owner ) ;

    virtual void                OnRender                ( void );
    virtual void                OnRenderTransparent     ( void );
    virtual void                RenderWeapon            ( xbool bDebug, const xcolor& Ambient, xbool Cloaked );
            void                SetVisible              ( xbool bVisible ) { m_IsVisible = bVisible; }

            void                GetAmmoState            ( ammo_priority Priority,
                                                          s32& AmmoAmount,
                                                          s32& AmmoMax,
                                                          s32& AmmoPerClip,
                                                          s32& AmmoInCurrentClip );
                                                    
            void                SetAmmoState            ( ammo_priority Priority,
                                                          s32 AmmoAmount,
                                                          s32 AmmoMax,
                                                          s32 AmmoPerClip,
                                                          s32 AmmoInCurrentClip );

            void                SetAmmoState2           ( ammo_priority Priority,
                                                          s32 AmmoAmount,
                                                          s32 AmmoInCurrentClip );

    static inven_item          GetDualWeaponID         ( inven_item ParentItem );
    static inven_item          GetParentIDForDualWeapon( inven_item DualItem );

            render_state        GetRenderState          ( void );

            virtual void        DrawLaserFixupBitmap    ( xbitmap* pBitmap, f32 Radius, xcolor cColor, collision_mgr::collision& Coll );

            xbool               IsReloadCompleted       ( void ) { return m_bCompletedReload; }
            void                SetReloadCompleted      ( xbool bCompleted ) { m_bCompletedReload = bCompleted; }

            void                RefillClip              (ammo_priority Priority );
            s32                 GetAmmoCount            ( void );
            s32                 GetAmmoCount            ( ammo_priority Priority );
            s32                 GetSecondaryAmmoCount   ( void );
            
            s32                 GetAmmoPerClip          ( void );
            s32                 GetAmmoPerClip          ( ammo_priority Priority );
            s32                 GetSecondaryAmmoPerClip ( void );

            s32                 GetAmmoAmount           ( ammo_priority Priority );

            s32                 GetTotalPrimaryAmmo     ( void );
    virtual s32                 GetTotalSecondaryAmmo   ( void );

            s32                 GetMaxPrimaryAmmo       ( void );
            s32                 GetMaxSecondaryAmmo     ( void );
            
            void                ClearAmmo               ( const ammo_priority& rAmmoPriority = AMMO_PRIMARY );
            void                ClearClipAmmo           ( const ammo_priority& rAmmoPriority = AMMO_PRIMARY );
            void                DecrementAmmo           ( const ammo_priority& rAmmoPriority = AMMO_PRIMARY, const s32& nAmt = 1);
    virtual void                FillAmmo                ( void );
    virtual void                NotifyAmmoFull          ( player *pPlayer );

    virtual guid                GetParentGuid           ( void )            { return m_ParentGuid; }            
            f32                 GetAccuracyPercent      ( f32 distance );
    
    virtual xbool               GetFiringStartPosition  ( vector3 &Pos );

            void                AddAmmoToWeapon         ( s32 nAmmoPrimary, s32 nAmmoSecondary );

    virtual void                InitWeapon              ( const char* pSkinFileName , const char* pAnimFileName , const vector3& rInitPos , const render_state& rRenderState = RENDER_STATE_PLAYER, const guid& rParentGuid = NULL );
    virtual void                InitWeapon              ( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid );
            
            void                InitNPCDualAnimController(       dual_weapon_anim_controller* pController, 
                                                           const char*                        pLeftRootBone, 
                                                           const char*                        pRightRootBone );

    inline  xbool               IsInited                ( void ){ return m_WeaponInited; }

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );

    xbool                       HasSecondaryAmmo        ( void ) { return m_HasSecondaryAmmo; }
    
    virtual xbool               GetFiringBonePosition   ( vector3 &Pos, s32 iBone = FIRE_POINT_DEFAULT );
    virtual xbool               GetAimBonePosition      ( vector3 &Pos, s32 iBone = FIRE_POINT_DEFAULT );

            xbool               CheckFirePoint          ( void );
            xbool               CheckFlashlightPoint    ( void );

// TODO: CJ: WEAPONS:    virtual use_item_err        UseItem                 ( void );
//    virtual        void         ActivateItem            ( void );

            xbitmap*            GetCenterReticleBmp     ( void );
            xbitmap*            GetEdgeReticleBmp       ( void );
            f32                 GetCenterPixelOffset    ( void );
    virtual ammo_priority       GetPrimaryAmmoPriority  ( void ){ return AMMO_PRIMARY; }
    virtual ammo_priority       GetSecondaryAmmoPriority( void ){ return AMMO_PRIMARY; }

    virtual xbool               CanFastTapFire          ( void ) { return FALSE; }
    virtual xbool               CanIntereptPrimaryFire  ( s32 nFireAnimIndex );
    virtual xbool               CanIntereptSecondaryFire( s32 nFireAnimIndex );

    virtual xbool               CanAltChainFire         ( void ) {return TRUE;}

    virtual xbool               ContinueReload          ( void );

    virtual void                SetupRenderInformation   ( void );
            xbool               GetIdleMode             ( void ){ return m_bIdleMode;   }
    static  char*               GetWeaponPrefixFromInvType2( inven_item WeaponItem );

    virtual void                SetRenderState          ( render_state RenderState );

    reticle_radius_parameters   GetReticleRadiusParameters      ( void ) const    { return m_ReticleRadiusParameters; }
    reticle_radius_parameters   GetAltReticleRadiusParameters   ( void ) const { return m_AltReticleRadiusParameters; }
    virtual xbool               ShouldDrawReticle               ( void ) { return TRUE; }

    virtual xbool               CheckReticleLocked              ( void );
    virtual void                UpdateReticle                   ( f32 DeltaTime );
    virtual xbool               ShouldUpdateReticle             ( void ) { return FALSE; }

            u8                  GetAutoSwitchRating             ( void ) const { return m_AutoSwitchRating; }

    virtual void                UpdateAmmoWarning               ( void );
    virtual void                SetAmmoHudColor                 ( player *pPlayer, hud_object* Hud, xcolor HudColor );

    virtual render_inst*        GetRenderInstPtr        ( void );
    virtual anim_group::handle* GetAnimGroupHandlePtr   ( void );
    virtual geom*               GetGeomPtr              ( void );

    //=========================================================================
    // ZOOM STATS
    //=========================================================================

            xbool               IsZoomEnabled           ( void );
            void                ZoomInComplete          ( xbool bZoomInComplete ) { m_bZoomComplete = bZoomInComplete; }
            xbool               IsZoomInComplete        ( void ) { return m_bZoomComplete; }
            radian              GetXFOV                 ( void );
            f32                 GetZoomLevel            ( void );
            s32                 GetZoomStep             ( void );
            s32                 GetnZoomSteps           ( void ) { return m_nZoomSteps; }
    virtual s32                 IncrementZoom           ( void );
    virtual void                ClearZoom               ( void );
            f32                 GetZoomMovementMod      ( void );

    //=========================================================================
    // FLASHLIGHT STUFF
    //=========================================================================    
    virtual xbool               HasFlashlight               ( void )        { return TRUE; }
    virtual xbool               GetFlashlightTransformInfo  ( matrix4& incMatrix,  vector3 &incVect );

    //=========================================================================
    // CUSTOM SNIPER SCOPE
    //=========================================================================    
    static  void                CreateScopeTexture          ( void );

    //this is a structure that contains ammunition information
    struct ammo
    {
        ammo( void );

        void    OnEnumProp      ( prop_enum& List );
        xbool   OnProperty      ( prop_query& rPropQuery );

        s32                 m_AmmoAmount;                                   //Amount of ammo.
        s32                 m_AmmoMax;                                      //Maximum amount of ammo.
        s32                 m_AmmoPerClip;                                  //Amount of ammo per clip
        s32                 m_AmmoInCurrentClip;                            //Amount of ammo in current clip

        s32                 m_ProjectileTemplateID;                         // Index to the g_TemplateDictionary for this weapons bullet.
        rhandle<xbitmap>    m_Bitmap;

        projectile_type     m_ProjectileType;
    };

protected:
    // functions for the custom sniper scope texture
            xbitmap*            GetScopeTexture             ( void );
            void                InstallCustomScope          ( void );
            void                UninstallCustomScope        ( void );

    //The protected weapon fire functions are the real overloaded funcitons...
    virtual xbool               FireWeaponProtected         ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint );
    virtual xbool               FireSecondaryProtected      ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint );
     
    virtual xbool               FireNPCWeaponProtected      ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
    virtual xbool               FireNPCSecondaryProtected   ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit );
  
    virtual void                MoveMuzzleFx                ( void );
    virtual void                RenderMuzzleFx              ( void );
    virtual void                AdvanceMuzzleFX             ( f32 DeltaTime );

    //Overload this to define the type of muzzle flash for a particular gun, can be NULL
public:    
    virtual void                InitMuzzleFx                ( xbool bIsPrimary, s32 iFirePoint );
            void                KillAllMuzzleFX             ( void );

protected:
            base_projectile*  CreateBullet                (     const char*                 pWeaponLogicalName,
                                                                const vector3&              InitPos, 
                                                                const radian3&              InitRot, 
                                                                const vector3&              InheritedVelocity,
                                                                const guid                  OwnerGuid,
                                                                const pain_handle&          PainHandle,
                                                                new_weapon::ammo_priority   Priority = AMMO_PRIMARY, 
                                                                xbool                       bHitLiving = TRUE,
                                                                s32                         iFirePoint = FIRE_POINT_DEFAULT
                                                            );

//=========================================================================
// Functions & Data from inventory_item
//=========================================================================

public:
    inline  inven_item          GetInvenItem        ( void )        { return m_Item; }
    inline  rigid_inst&         GetRigidInst        ( void )        { return m_Inst; }
    inline  void                SetOwnerGuid        ( guid Guid )   { m_OwnerGuid = Guid; m_ParentGuid = Guid;}
//            u8                  GetFlags            ( void )        { return m_Flags; }
protected:
    rigid_inst                  m_Inst;                 // Render Instance for the item.
    guid                        m_OwnerGuid;            // Who is carrying this inventory item.
    u8                          m_Flags;                // Any special flags associated with the inventory item.
    inven_item                  m_Item;                 // Inventory item corresponding to this weapon

//=========================================================================
//=========================================================================

public:
    ammo                    m_WeaponAmmo[ AMMO_MAX ];                           // Actual ammo information
protected:    
    ammo                    m_OriginalWeaponAmmo[ AMMO_MAX ];                   // Actual ammo information
    render_state            m_CurrentRenderState;
    skin_inst               m_Skin[ RENDER_STATE_MAX ];       // Geometry use to render the weapon
    anim_group::handle      m_AnimGroup[ RENDER_STATE_MAX ];  // Animation file for the weapon
    rhandle<char>           m_hAudioPackage;                  // Audio package for the weapon.
    char_anim_player        m_AnimPlayer[ RENDER_STATE_MAX ]; // Animation player used for weapon animations.
    xbool                   m_WeaponInited;
    s32                     m_FlashlightBoneIndex;                              // The bone index of the flashlight point
    s32                     m_FiringPointBoneIndex[ FIRE_POINT_COUNT ];          // Bone index on the weapon of the firing point.
    s32                     m_AltFiringPointBoneIndex[ FIRE_POINT_COUNT ];       // Alt firing point for the weapon.
    s32                     m_AimPointBoneIndex[ FIRE_POINT_COUNT ];             
    s32                     m_AltAimPointBoneIndex[ FIRE_POINT_COUNT ];              
    vector3                 m_LastPosition;
    vector3                 m_Velocity;
    radian                  m_LastRotation;                                     // just tracking yaw here
    radian                  m_AngularSpeed;                                     // just tracking yaw here

    xbool                   m_EnableMuzzleFx;                                   // Flag to enable muzzle effects..
    rhandle<char>           m_hMuzzleFXPrimary;                                 // Primary muzzle effect for the weapon.
    rhandle<char>           m_hMuzzleFXSecondary;                               // Secondary muzzle effect for the weapon.
    fx_handle               m_MuzzleNPCFX[ FIRE_POINT_COUNT ];                   // Unique ID for NPC/Avatar muzzle flash
    fx_handle               m_MuzzleFX[ FIRE_POINT_COUNT ];                      // Unique ID for muzzle flash
    fx_handle               m_MuzzleSecondaryFX[ FIRE_POINT_COUNT ];             // Unique ID for the secondary muzzle flash
    const char*             m_NPCMuzzleSoundFx;                                 // Name of sound effect for muzzle flash for NPC
    
    rhandle<xbitmap>        m_ReticleCenter;
    rhandle<xbitmap>        m_ReticleEdge;
    f32                     m_ReticleCenterPixelOffset;

    f32                     m_AimDegradePrimary;                // how much to degrade aim for each primary fire
    f32                     m_AimDegradeSecondary;              // how much to degrade aim for each secondary fire
    f32                     m_AimRecoverSpeed;                  // how quickly to recover from fire

    reticle_radius_parameters m_ReticleRadiusParameters;
    reticle_radius_parameters m_AltReticleRadiusParameters;

    f32                     m_ShortRange;                       // max distance considered short rnage for this weapon
    f32                     m_LongRange;                        // min distance considered long range for this weapon
    s32                     m_AccuracyPercentLongRange;         // percent of short ranges accuracy at long range

    xbool                   m_IsVisible;                        // Is the weapon visible (used for thrown grenades)  
    xbool                   m_HasSecondaryAmmo;
    guid                    m_ParentGuid;                       // guid to the owner of this weapon.. 
    xbool                   m_bIdleMode;
    zoom_state              m_ZoomState;
    radian                  m_CurrentViewX;                     // Current x-view
    radian                  m_CurrentViewY;                     // Current y-view
//    f32                     m_DropFadeTime;
    f32                     m_ZoomMovementSpeed;
    s32                     m_FactionFireSfx[ MAX_FACTION_COUNT ];
    s32                     m_ZoomStep;
    s32                     m_nZoomSteps;
    xbool                   m_bZoomComplete;
    guid                    m_TargetGuid;

    s32                     m_ScopeMesh;                        // sniper-scope custom mesh (-1 if not present)
    s32                     m_ScopeLensMesh;                    // sniper-scope custom lens mesh (-1 if not present)

    static s32              m_OrigScopeVramId;                  // original vram id of scope texture
    static s32              m_ScopeRefCount;                    // reference count to scope texture
    static s32              m_ScopeTextureVramId;               // custom texture pulled from the screen

    xbool                   m_bLockedOn;                        // so we can tell a sound to play
    xbool                   m_bCompletedReload;                 // are we past the required reload threshold?
    xbool                   m_bCanWarnLowAmmo;                  // flag so we don't flood the player with low ammo warnings.
    f32                     m_fLastAmmoFullTime;                // what was the last time we told the player that they were full of a particular ammo
    u8                      m_AutoSwitchRating;                 // what is the auto-switch rating of this weapon?  Larger number = better weapon
};

//===========================================================================

inline
s32 new_weapon::GetTotalPrimaryAmmo( void )
{
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
}

//===========================================================================

inline
s32 new_weapon::GetTotalSecondaryAmmo( void )
{
    return m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount;
}

//===========================================================================

inline
s32 new_weapon::GetMaxPrimaryAmmo( void )
{
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
}

//===========================================================================

inline
s32 new_weapon::GetMaxSecondaryAmmo( void )
{
    return m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax;
}

//===========================================================================

inline
f32 new_weapon::GetCenterPixelOffset( void )
{
    return m_ReticleCenterPixelOffset;
}

//==============================================================================

inline
new_weapon::render_state new_weapon::GetRenderState( void )
{
    return m_CurrentRenderState;
}

//===========================================================================
// END.
//===========================================================================
#endif
