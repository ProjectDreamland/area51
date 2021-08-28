//==============================================================================
//
//  Ghost.hpp
//
//==============================================================================

#ifndef GHOST_HPP  
#define GHOST_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\Actor\Actor.hpp"

#include "Animation\AnimPlayer.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "Locomotion\CharacterPhysics.hpp"

#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "EventMgr\EventMgr.hpp"

#include "Objects\Render\SkinInst.hpp"
#include "Objects\SpawnPoint.hpp"
#include "Objects\PlayerLoco.hpp"
#include "Objects\NewWeapon.hpp"

#include "Trigger\Actions\lock_player_view.hpp"

#include "NetworkMgr\NetObj.hpp"
#include "NetworkMgr\MoveMgr.hpp"
#include "NetworkMgr\Blender.hpp"

#include "Ragdoll/Ragdoll.hpp"
#include "Inventory/Inventory2.hpp"
//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_NET_PLAYERS     32
#define MAX_NAME_LENGTH     32

//==============================================================================
//  TYPES
//==============================================================================

class dead_body;
class third_person_camera;


//==============================================================================

class ghost : public actor
{
public:
    CREATE_RTTI( ghost, actor, object )


    //------------------------------------------------------------------------------
    // List of the different weapons
    enum player_virtual_weapon
    {
        WEAPON_UNDEFINED = -1,
        
        // Virtual weapons for the player
        MACHINE_GUN,        //SMP

// KSS -- TO ADD NEW WEAPON
        SHOTGUN,            //SHT
        GAUSS_RIFLE,        //GAS
        SNIPER_RIFLE,       //SNI
        DESERT_EAGLE,       //EGL
        MHG,                //MHG
        MSN,
        MUTATION,           // MUT

        DUAL_SMP,           //2MP        
        DUAL_EGL,           //2EG        
        
        //miscellaneous stuff (keycards, etc...)
        MISC_01,            //KEY

        //FRAG_GRENADE,       //FRG
        //FLASH_GRENADE,      //FLS
        //HAND_NUKE,          //NUK
        //GRAVITATION_CHARGE, //GRV

        WEAPON_VIRTUAL_MAX
    };
    
    enum player_weapon_obj
    {
        WEAPON_UNDEFINED_OBJ = -1,

        // main weapons for the player
        MACHINE_GUN_OBJ,        //SMP

// KSS -- TO ADD NEW WEAPON
        SHOTGUN_OBJ,            //SHT
        GAUSS_RIFLE_OBJ,        //GAS
        SNIPER_RIFLE_OBJ,       //SNI
        DESERT_EAGLE_OBJ,       //EGL
        MHG_OBJ,                //MHG
        MSN_OBJ,                //MSN
        MUTATION_OBJ,           //MUT

        WEAPON_MAX_OBJ
    };

    //------------------------------------------------------------------------------
    enum player_mutation_strain
    {
        STRAIN_INVALID = -1,

        STRAIN_HUMAN,
        //STRAIN_ONE,
        //STRAIN_TWO,
        //STRAIN_THREE,
        
        STRAIN_MAX
    };
    
                                ghost                   ( void );
virtual                        ~ghost                   ( void );
                                                        
                                                        
    // Object description.                              
virtual const   object_desc&    GetTypeDesc             ( void ) const;
static  const   object_desc&    GetObjectType           ( void );
                                                        
                vector3         GetEyesPosition         ( void ) const;
                void            GetEyesPitchYaw         ( radian& Pitch, radian& Yaw ) const;
virtual         vector3         GetPositionWithOffset   ( eOffsetPos offset );
                                                        
                f32             GetPitch                ( void ) {return m_Pitch;}
                f32             GetYaw                  ( void ) {return m_Yaw;}
                void            SetPitch                ( radian Pitch ) { m_Pitch = Pitch; }
                void            SetYaw                  ( radian Yaw   ) { m_Yaw   = Yaw;   }
        const   matrix4&        GetL2W                  ( void ) const;          

    // Static functions for enumeration needs.
    static  const char*                 GetStrainName       ( s32 Index );            
    static  const char*                 GetStrainEnum       ( void);
    static  player_mutation_strain      GetStrainByName     ( const char* pName );

        
virtual         f32             GetMaxHealth            ( void ) { return m_MaxHealth; }

virtual         xbool           AddMutagen              ( const f32& nDeltaMutagen );
virtual         f32             GetMutagen              ( void ) { return m_Mutagen; }
virtual         f32             GetMaxMutagen           ( void ) { return m_MaxMutagen; }
                                                        
//rtual         void            OnReset                 ( void );

virtual         void            OnMove                  ( const vector3& NewPos );
virtual         void            OnEvent                 ( const event& Event );
virtual         void            OnKill                  ( void );               
virtual         radian          GetSightYaw             ( void ) const;   
virtual         void            UpdateMovement          ( f32 DeltaTime );


    //------------------------------------------------------------------------------
    // Determines whether or not we want to run the normal logic for this guy.
    //------------------------------------------------------------------------------

                skin_inst&      GetThirdPersonInst  ( void ) { return m_SkinInst; }
            anim_group::handle& GetAnimGroupHandle  ( void ) { return m_hAnimGroup; }
virtual         ragdoll::type   GetRagdollType      ( void ) { return ragdoll::TYPE_PLAYER; }


virtual         void            OnAliveLogic        ( f32 DeltaTime );
virtual         void            OnDeathLogic        ( f32 DeltaTime );
virtual         void          UpdateFellFromAltitude( void );
virtual         void            TakeFallPain        ( void );
                
virtual         f32             GetCollisionHeight  ( void );
virtual         f32             GetCollisionRadius  ( void );

            void        LogWeapons                  ( void );

protected:

    virtual s32         GetMaterial                 ( void ) const { return MAT_TYPE_FLESH; }
    virtual void        OnAdvanceLogic              ( f32 DeltaTime );
    virtual void        OnTransform                 ( const matrix4& L2W );    
            const char* GetVirtualWeaponName        ( player_virtual_weapon VirtualWeapon );
            const char* GetWeaponObjName            ( player_weapon_obj WeaponObj );

            void        ParseOnPainForEffects       ( const pain& Pain );

    virtual void        SwitchWeapon                ( new_weapon* pWeapon );
    virtual radian3     GetProjectileTrajectory     ( void );
    player_weapon_obj   GetWeaponObjFromVirtual     ( player_virtual_weapon VirtualWeapon );
            s32         GetRenderIndexFromPlayer    ( void );
    inventory_item::inv_type GetInventoryType       ( player_virtual_weapon VirtualWeapon );
    virtual void        AddNewWeapon                ( guid WeaponGuid );
    player_virtual_weapon GetWeaponStateFromType    ( object::type Type ); 
    virtual s32         GetWeaponRenderState        ( void );



public:
    virtual void                OnColCheck                  ( void );    
    virtual bbox                GetLocalBBox                ( void ) const { return m_Physics[ m_CurrentStrain ].GetBBox(); }      

#ifndef X_RETAIL
    virtual void                OnColRender                 ( xbool bRenderHigh );
#endif // X_RETAIL

    //------------------------------------------------------------------------------
    // Input handlers
    //------------------------------------------------------------------------------
            void                    OnPrimaryFire               ( void );
            void                    OnSecondaryFire             ( void );
            void                    OnReload                    ( void );

    virtual f32         GetMovementNoiseLevel ( void );


protected:
    f32                     m_Mutagen;
    f32                     m_MaxMutagen;                   // Player's maximum mutagen level
    f32                     m_MaxHealth;                    // Player's maximum health
    vector3                 m_EyesPosition;                 // Where the player's eyes are after the last ComputeView() call
    radian                  m_EyesPitch;                    // Eye's pitch after the last ComputeView() call
    radian                  m_EyesYaw;                      // Eye's yaw after the last ComputeView() call

    radian                  m_Pitch;                        // Head Logical orientation. Used for movement
    radian                  m_Yaw;                          // Head Logical orientation. Used for movement

    xbool                   m_bRenderSkeleton;              // Whether to render the skeleton or not
    xbool                   m_bRenderSkeletonNames;         // When ever the skeleton gets render whther to also print the name of the bones
    xbool                   m_bRenderBBox;                  // Renders the bbox of the player this is use mainly in the editor
    xbool                   m_bIsCrouching;                 // Is the player crouching
    f32                     m_fCrouchChangeRate;

public:
    xbool                   m_bWeaponInInventory[ WEAPON_VIRTUAL_MAX ]; //Inventory of player's weapons.
    guid                    m_GuidWeaponArray[ WEAPON_MAX_OBJ ];//Guid's for weapons that the player may carry.
    new_weapon*             m_pCurrentWeapon;               // Pointer to the current weapon.
    player_virtual_weapon   m_CurrentVirtualWeapon;         // current weapon that the player is carrying
    player_weapon_obj       m_CurrentWeaponObj;
    player_virtual_weapon   m_PreviousVirtualWeapon;        // Previous weapon that the player was carrying
    player_virtual_weapon   m_NextVirtualWeapon;            // Next weapon that the player is carrying

protected:
    player_mutation_strain  m_CurrentStrain;
    player_mutation_strain  m_NextStrain;

    // Locomotion                               
    character_physics       m_Physics[ STRAIN_MAX ];            // Physics to drive the motion of the player
    player_loco             m_Loco;
    f32                     m_DeathTime;

    xbool                   m_bFalling; // used for pain when we land
    xbool                   m_bJustLanded;
    vector3                 m_DeltaPos; // current velocity / delta time


    f32                     m_DeltaTime;



};

//===========================================================================


//==============================================================================
#endif // GHOST_HPP
//==============================================================================
