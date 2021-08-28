//=========================================================================
// Inventory2.hpp
//=========================================================================

#ifndef __INVENTORY2_HPP
#define __INVENTORY2_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "x_bitstream.hpp"
#include "objects/object.hpp"

//=========================================================================
// INVENTORY ENUMERATION
//=========================================================================

enum inven_item
{
    INVEN_NULL = 0,

    // KSS -- TO ADD NEW WEAPON
    // Weapons
    INVEN_WEAPON_SCANNER,
    
    INVEN_WEAPON_DESERT_EAGLE,
    INVEN_WEAPON_DUAL_EAGLE,
    
    INVEN_WEAPON_SMP,
    INVEN_WEAPON_DUAL_SMP,
    
    INVEN_WEAPON_SHOTGUN,
    INVEN_WEAPON_DUAL_SHT,
    
    INVEN_WEAPON_SNIPER_RIFLE,
    INVEN_WEAPON_BBG,
    INVEN_WEAPON_MESON_CANNON,
    INVEN_WEAPON_TRA,
    INVEN_WEAPON_MUTATION,
        INVEN_WEAPON_FIRST = INVEN_WEAPON_SCANNER,      // Used for range checks.
        INVEN_WEAPON_LAST  = INVEN_WEAPON_MUTATION,

    // Weapon Ammo
    INVEN_AMMO_SMP,
    INVEN_AMMO_SHOTGUN,
    INVEN_AMMO_SNIPER_RIFLE,
    INVEN_AMMO_MESON,
    INVEN_AMMO_BBG,
    INVEN_AMMO_DESERT_EAGLE,
        INVEN_AMMO_FIRST = INVEN_AMMO_SMP,         // Used for range checks.
        INVEN_AMMO_LAST  = INVEN_AMMO_DESERT_EAGLE,

    // Grenades
    INVEN_GRENADE_FRAG,
    INVEN_GRENADE_GRAV,
    INVEN_GRENADE_JBEAN,
    INVEN_GRENADE_JBEAN_ENHANCE,
        INVEN_GRENADE_FIRST = INVEN_GRENADE_FRAG,      // Used for range checks.
        INVEN_GRENADE_LAST  = INVEN_GRENADE_JBEAN_ENHANCE,

    // General Pickups
    INVEN_HEALTH,
    INVEN_MUTAGEN,
    INVEN_MUTAGEN_CORPSE,

    // Gloves (not a pickup)
    INVEN_GLOVES,
    
    // Keycards
    INVEN_KEYCARD_RED,
    INVEN_KEYCARD_GREEN,
    INVEN_KEYCARD_BLUE,
    INVEN_KEYCARD_YELLOW,
    INVEN_KEYCARD_ORANGE,

    // User items for use as quest items in levels
    INVEN_ITEM_1,
    INVEN_ITEM_2,
    INVEN_ITEM_3,
    INVEN_ITEM_4,
    INVEN_ITEM_5,
    INVEN_ITEM_6,
    INVEN_ITEM_7,
    INVEN_ITEM_8,
    INVEN_ITEM_9,
    INVEN_ITEM_10,
        INVEN_ITEM_FIRST = INVEN_HEALTH,      // Used for range checks.
        INVEN_ITEM_LAST  = INVEN_ITEM_10,

    // Count of enumeration entries
    INVEN_COUNT
};

#define INVEN_NUM_WEAPONS (INVEN_WEAPON_LAST+1) // Must include "NULL weapon".

//=========================================================================
// INVENTORY CLASS
//=========================================================================

class inventory2
{
public:

static  const char*         ItemToName              ( inven_item Item );
static  const xwchar*       ItemToDisplayName       ( inven_item Item );
static  inven_item          NameToItem              ( const char* pName );
static  s32                 GetNumItems             ( void );

static  inven_item          AmmoToWeapon            ( inven_item AmmoItem   );
static  inven_item          WeaponToAmmo            ( inven_item WeaponItem );
static  s32                 ItemToWeaponIndex       ( inven_item Item );
static  inven_item          WeaponIndexToItem       ( s32 iWeapon );
static  xbool               IsAWeapon               ( inven_item Item );
static  s32                 GetNumWeapons           ( void );

static  xstring             GetEnumString           ( void );
static  xstring             GetEnumStringWeapons    ( void );
static  xstring             GetEnumStringGrenades   ( void );

static  const char*         ItemToBlueprintName     ( inven_item Item );
static  const char*         ItemToPickupGeomName    ( inven_item Item, f32 Amount = -1.0f );

	                        inventory2();
                           ~inventory2();

        void                Init            ( void );

        void                Clear           ( void );

        inventory2&         operator =      ( const inventory2& Source );

        f32                 GetAmount       ( inven_item Item );
        void                SetAmount       ( inven_item Item, f32 Amount );
        void                AddAmount       ( inven_item Item, f32 Amount );
        void                RemoveAmount    ( inven_item Item, f32 Amount );
        f32                 GetMinAmount    ( inven_item Item );
        f32                 GetMaxAmount    ( inven_item Item );

        xbool               CanHoldMore     ( inven_item Item );
        xbool               HasItem         ( inven_item Item );

        void                Save            ( bitstream& Bitstream );
        void                Load            ( bitstream& Bitstream );

#ifdef ENABLE_DEBUG_MENU
        void                LogInventory    ( void );
#endif

protected:

            f32                 m_Data[INVEN_COUNT];
};

//===========================================================================
#endif // __INVENTORY2_HPP
//===========================================================================
