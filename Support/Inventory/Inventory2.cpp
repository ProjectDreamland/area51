//=========================================================================
// inventory2
//=========================================================================

#include "x_files.hpp"
#include "Inventory2.hpp"
#include "StringMgr\StringMgr.hpp"

//=========================================================================

#if defined( cgalley )
xbool g_LogInventory = FALSE;
#define INVENTORY_LOGGING g_LogInventory
#else
#define INVENTORY_LOGGING 0
#endif

//=========================================================================
// Inventory Info Table
//=========================================================================

struct table_entry
{
    s32         Item;
    const char* pName;
    const char* pDisplayName;
    f32         Min;
    f32         Max;            // NOTE: -1.0f is unavailable.
    const char* pPickupGeomName;
};

// KSS -- TO ADD NEW WEAPON
static table_entry s_Table[] =
{
    // Null entry
    { INVEN_NULL,                   "<null>",                "INVEN_NULL",                  0.0f,   0.0f, NULL },
                                                                                       
    // Weapons                                                
    { INVEN_WEAPON_SCANNER,         "Weapon Scanner",        "INVEN_WEAPON_SCANNER",        0.0f,   1.0f, PRELOAD_FILE( "WPN_EGL_Pickup.rigidgeom" ) },
    { INVEN_WEAPON_DESERT_EAGLE,    "Weapon Desert Eagle",   "INVEN_WEAPON_DESERT_EAGLE",   0.0f,   1.0f, PRELOAD_FILE( "WPN_EGL_Pickup.rigidgeom" ) },
    { INVEN_WEAPON_DUAL_EAGLE,      "Weapon Dual Eagle",     "INVEN_WEAPON_DUAL_EAGLE",     0.0f,   1.0f, NULL },// KSS -- FIXME -- need rigid
    { INVEN_WEAPON_SMP,             "Weapon SMP",            "INVEN_WEAPON_SMP",            0.0f,   1.0f, PRELOAD_FILE( "WPN_SMP_Pickup.rigidgeom" ) },
    { INVEN_WEAPON_DUAL_SMP,        "Weapon Dual SMP",       "INVEN_WEAPON_DUAL_SMP",       0.0f,   1.0f, NULL },    
    { INVEN_WEAPON_SHOTGUN,         "Weapon Shotgun",        "INVEN_WEAPON_SHOTGUN",        0.0f,   1.0f, PRELOAD_FILE( "WPN_SHT_Pickup.rigidgeom" ) },
    { INVEN_WEAPON_DUAL_SHT,        "Weapon Dual Shotguns",  "INVEN_WEAPON_DUAL_SHT",       0.0f,   1.0f, NULL },
    { INVEN_WEAPON_SNIPER_RIFLE,    "Weapon Sniper Rifle",   "INVEN_WEAPON_SNIPER_RIFLE",   0.0f,   1.0f, PRELOAD_FILE( "WPN_SNI_Pickup.rigidgeom" ) },    
    { INVEN_WEAPON_BBG,             "Weapon BBG",            "INVEN_WEAPON_BBG",            0.0f,   1.0f, PRELOAD_FILE( "WPN_BBG_Pickup.rigidgeom" ) },
    { INVEN_WEAPON_MESON_CANNON,    "Weapon Meson Cannon",   "INVEN_WEAPON_MESON_CANNON",   0.0f,   1.0f, PRELOAD_FILE( "WPN_MSN_Pickup.rigidgeom" ) },
    
    { INVEN_WEAPON_TRA,             "Weapon TRA",            "INVEN_WEAPON_TRA",            0.0f,   1.0f, NULL },    
    { INVEN_WEAPON_MUTATION,        "Weapon Mutation",       "INVEN_WEAPON_MUTATION",       0.0f,   1.0f, NULL },
                                                                                       
    // Weapon Ammo                                           
    { INVEN_AMMO_SMP,               "Ammo SMP",              "INVEN_AMMO_SMP",              0.0f,   0.0f, PRELOAD_FILE( "WPN_SMP_Pickup_Ammo_Small.rigidgeom" ) },
    { INVEN_AMMO_SHOTGUN,           "Ammo Shotgun",          "INVEN_AMMO_SHOTGUN",          0.0f,   0.0f, PRELOAD_FILE( "WPN_SHT_Pickup_Ammo_Small.rigidgeom" ) },
    { INVEN_AMMO_SNIPER_RIFLE,      "Ammo Sniper Rifle",     "INVEN_AMMO_SNIPER_RIFLE",     0.0f,   0.0f, PRELOAD_FILE( "WPN_SNI_Pickup_Ammo_Small.rigidgeom" ) },
    { INVEN_AMMO_MESON,             "Ammo Meson Cannon",     "INVEN_AMMO_MESON",            0.0f,   0.0f, PRELOAD_FILE( "WPN_MSN_Pickup_Ammo.rigidgeom" ) },
    { INVEN_AMMO_BBG,               "Ammo BBG",              "INVEN_AMMO_BBG",              0.0f,  -1.0f, PRELOAD_FILE( "WPN_GRAV_Bindpose.rigidgeom" ) }, // KSS -- FIXME -- new rigid
                                                                                       
    { INVEN_AMMO_DESERT_EAGLE,      "Ammo Desert Eagle",     "INVEN_AMMO_DESERT_EAGLE",     0.0f,   0.0f, PRELOAD_FILE( "WPN_EGL_Pickup_Ammo_Small.rigidgeom" ) },
                                                                                       
    // Grenades                                              
    { INVEN_GRENADE_FRAG,           "Grenade Frag",          "INVEN_GRENADE_FRAG",          0.0f,   5.0f, PRELOAD_FILE( "WPN_FRG_Pickup_Ammo_Small.rigidgeom" ) },
    { INVEN_GRENADE_GRAV,           "Grenade Grav",          "INVEN_GRENADE_GRAV",          0.0f,   5.0f, NULL },
    { INVEN_GRENADE_JBEAN,          "Grenade Jumping Bean",  "INVEN_GRENADE_JBEAN",         0.0f,   5.0f, PRELOAD_FILE( "WPN_GRAV_Bindpose.rigidgeom" ) },
    { INVEN_GRENADE_JBEAN_ENHANCE,  "Grenade JB Enhanced",   "INVENT_GRENADE_JBEAN_ENHANCE",0.0f,   5.0f, PRELOAD_FILE( "Pickup_KeyCard_GREEN.rigidgeom" ) },
                                                                                       
    // General Pickups                                       
    { INVEN_HEALTH,                 "Health",                "INVEN_HEALTH",                0.0f, 100.0f, PRELOAD_FILE( "Pickup_Health_Small.rigidgeom"  ) },
    { INVEN_MUTAGEN,                "Mutagen",               "INVEN_MUTAGEN",               0.0f, 100.0f, PRELOAD_FILE( "Pickup_Mutagen_Small.rigidgeom" ) },
    { INVEN_MUTAGEN_CORPSE,         "Mutagen Corpse",        "INVEN_MUTAGEN_CORPSE",        0.0f, 100.0f, PRELOAD_FILE( "Pickup_Contagious_Corpse.rigidgeom") },
    { INVEN_GLOVES,                 "Gloves",                "INVEN_GLOVES",                0.0f,   1.0f, NULL },
                                                                                       
    // Keycards                                              
    { INVEN_KEYCARD_RED,            "Keycard Red",           "INVEN_KEYCARD_RED",           0.0f,   0.0f, PRELOAD_FILE( "Pickup_KeyCard_RED.rigidgeom"    ) },
    { INVEN_KEYCARD_GREEN,          "Keycard Green",         "INVEN_KEYCARD_GREEN",         0.0f,   0.0f, PRELOAD_FILE( "Pickup_KeyCard_GREEN.rigidgeom"  ) },
    { INVEN_KEYCARD_BLUE,           "Keycard Blue",          "INVEN_KEYCARD_BLUE",          0.0f,   0.0f, PRELOAD_FILE( "Pickup_KeyCard_BLUE.rigidgeom"   ) },
    { INVEN_KEYCARD_YELLOW,         "Keycard Yellow",        "INVEN_KEYCARD_YELLOW",        0.0f,   0.0f, PRELOAD_FILE( "Pickup_KeyCard_YELLOW.rigidgeom" ) },
    { INVEN_KEYCARD_ORANGE,         "Keycard Orange",        "INVEN_KEYCARD_ORANGE",        0.0f,   0.0f, PRELOAD_FILE( "Pickup_KeyCard_ORANGE.rigidgeom" ) },
                                                                                       
    // Generic items for use as quest items in levels        
    { INVEN_ITEM_1,                 "Item 1",                "INVEN_ITEM_1",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_2,                 "Item 2",                "INVEN_ITEM_2",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_3,                 "Item 3",                "INVEN_ITEM_3",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_4,                 "Item 4",                "INVEN_ITEM_4",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_5,                 "Item 5",                "INVEN_ITEM_5",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_6,                 "Item 6",                "INVEN_ITEM_6",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_7,                 "Item 7",                "INVEN_ITEM_7",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_8,                 "Item 8",                "INVEN_ITEM_8",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_9,                 "Item 9",                "INVEN_ITEM_9",                0.0f,   0.0f, NULL },
    { INVEN_ITEM_10,                "Item 10",               "INVEN_ITEM_10",               0.0f,   0.0f, NULL },
};

#ifndef X_RETAIL
xbool s_Validated = FALSE;
#endif

#define NUM_TABLE_ENTRIES   ((s32)( sizeof(s_Table) / sizeof(table_entry) ))


//=========================================================================
//  Static members
//=========================================================================

//=========================================================================
const char* inventory2::ItemToName( inven_item Item )
{
    ASSERT( (Item >= 0) && (Item < INVEN_COUNT) );
    const char* pName = s_Table[Item].pName;
    CLOG_MESSAGE( INVENTORY_LOGGING, "Inventory::ItemToName", "%d = '%s'", Item, pName );
    return pName;
}

//=========================================================================
const xwchar* inventory2::ItemToDisplayName( inven_item Item )
{
    ASSERT( (Item >= 0) && (Item < INVEN_COUNT) );
    const xwchar* pName = g_StringTableMgr("Inventory", s_Table[Item].pDisplayName);
    CLOG_MESSAGE( INVENTORY_LOGGING, "Inventory::ItemToDisplayName", "%d = '%s'", Item, pName );
    return pName;
}

//=========================================================================

inven_item inventory2::NameToItem( const char* pName )
{
    // Search the table for the item name
    for( s32 i=0 ; i<NUM_TABLE_ENTRIES ; i++ )
    {
        if( x_strcmp( pName, s_Table[i].pName ) == 0 )
        {
            ASSERT( i == s_Table[i].Item );
            CLOG_MESSAGE( INVENTORY_LOGGING, "Inventory::NameToItem", "Found '%s' = %d", pName, i );
            return (inven_item)i;
        }
    }

    // Not found so NULL
    CLOG_ERROR( INVENTORY_LOGGING, "Inventory::NameToItem", " Not Found '%s'", pName );
    return INVEN_NULL;
}

//=========================================================================

s32 inventory2::GetNumItems( void )
{
    return INVEN_COUNT;
}

//=========================================================================

inven_item inventory2::AmmoToWeapon( inven_item AmmoItem )
{
    switch( AmmoItem )
    {
    case INVEN_AMMO_SMP:          return( INVEN_WEAPON_SMP );           break;
    case INVEN_AMMO_SHOTGUN:      return( INVEN_WEAPON_SHOTGUN );       break;
    case INVEN_AMMO_SNIPER_RIFLE: return( INVEN_WEAPON_SNIPER_RIFLE );  break;
    case INVEN_AMMO_DESERT_EAGLE: return( INVEN_WEAPON_DESERT_EAGLE );  break;
    case INVEN_AMMO_MESON:        return( INVEN_WEAPON_MESON_CANNON );  break;
    case INVEN_AMMO_BBG:          return( INVEN_WEAPON_BBG );           break;
    }

    return( INVEN_NULL );
}

//=========================================================================

inven_item inventory2::WeaponToAmmo( inven_item WeaponItem )
{
    switch( WeaponItem )
    {
    case INVEN_WEAPON_SMP:          return( INVEN_AMMO_SMP );           break;
    case INVEN_WEAPON_SHOTGUN:      return( INVEN_AMMO_SHOTGUN );       break;
    case INVEN_WEAPON_SNIPER_RIFLE: return( INVEN_AMMO_SNIPER_RIFLE );  break;
    case INVEN_WEAPON_DESERT_EAGLE: return( INVEN_AMMO_DESERT_EAGLE );  break;
    case INVEN_WEAPON_DUAL_SMP:     return( INVEN_AMMO_SMP );           break;
    case INVEN_WEAPON_DUAL_SHT:     return( INVEN_AMMO_SHOTGUN );       break;
    case INVEN_WEAPON_DUAL_EAGLE:   return( INVEN_AMMO_DESERT_EAGLE );  break;
    case INVEN_WEAPON_MESON_CANNON: return( INVEN_AMMO_MESON );         break;
    case INVEN_WEAPON_BBG:          return( INVEN_AMMO_BBG );           break;
    }

    return( INVEN_NULL );
}

//=========================================================================

s32 inventory2::ItemToWeaponIndex( inven_item Item )
{
    ASSERT( (Item == INVEN_NULL) || ((Item >= INVEN_WEAPON_FIRST) && (Item <= INVEN_WEAPON_LAST)) );
    if( Item == INVEN_NULL )
        return 0;
    else
        return Item - INVEN_WEAPON_FIRST + 1;
}

//=========================================================================

inven_item inventory2::WeaponIndexToItem( s32 iWeapon )
{
    ASSERT( (iWeapon >= 0) && (iWeapon < (INVEN_NUM_WEAPONS)) );

    if( iWeapon == 0 )
        return INVEN_NULL;
    else
        return (inven_item)(iWeapon + INVEN_WEAPON_FIRST - 1);
}

//=========================================================================

xbool inventory2::IsAWeapon( inven_item Item )
{
    xbool bIsAWeapon = false;

    if( (Item >= INVEN_WEAPON_FIRST) && (Item <= INVEN_WEAPON_LAST) )
    {
        bIsAWeapon = true;
    }

    return bIsAWeapon;
}

//=========================================================================

s32 inventory2::GetNumWeapons( void )
{
    return INVEN_NUM_WEAPONS;
}

//=========================================================================

xstring inventory2::GetEnumString( void )
{
    xstring EnumString;

    for( s32 i=0 ; i<GetNumItems() ; i++ )
    {
        EnumString += ItemToName( (inven_item)i );
        EnumString += '\0';
    }

    return EnumString;
}

//=========================================================================

xstring inventory2::GetEnumStringWeapons( void )
{
    xstring EnumString;

    for( s32 i=0 ; i<GetNumWeapons() ; i++ )
    {
        EnumString += ItemToName( (inven_item)WeaponIndexToItem(i) );
        EnumString += '\0';
    }

    return EnumString;
}

//=========================================================================

xstring inventory2::GetEnumStringGrenades( void )
{
    xstring EnumString;

    EnumString += ItemToName( INVEN_NULL );
    EnumString += '\0';
    EnumString += ItemToName( INVEN_GRENADE_FRAG );
    EnumString += '\0';
    EnumString += ItemToName( INVEN_GRENADE_GRAV );
    EnumString += '\0';
    EnumString += ItemToName( INVEN_GRENADE_JBEAN );
    EnumString += '\0';

    return EnumString;
}

//==============================================================================

const char* inventory2::ItemToBlueprintName( inven_item Item )
{    
    const char* pBlueprintName = NULL;

    // KSS -- TO ADD NEW WEAPON

    switch( Item )
    {
    case INVEN_WEAPON_SMP:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_SMP.bpx";
        break;
    case INVEN_WEAPON_SHOTGUN:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_Shotgun.bpx";
        break;
    case INVEN_WEAPON_SNIPER_RIFLE:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_SniperRifle.bpx";
        break;
    case INVEN_WEAPON_DESERT_EAGLE:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_DesertEagle.bpx";
        break;
    case INVEN_WEAPON_SCANNER:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_Scanner.bpx";
        break;
    case INVEN_WEAPON_MESON_CANNON:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_MasonCannon.bpx";
        break;
    case INVEN_WEAPON_BBG:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_BBG.bpx";
        break;
    case INVEN_WEAPON_TRA:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_FPTurretA.bpx";
        break;
    case INVEN_WEAPON_DUAL_SMP:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_Dual_SMP.bpx";
        break;
    case INVEN_WEAPON_DUAL_SHT:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_Dual_SHT.bpx";
        break;
    case INVEN_WEAPON_MUTATION:
        pBlueprintName = "C:\\GameData\\A51\\Source\\Themes\\Weapons\\Blueprint\\WPN_Mutation.bpx";
        break;
    }

    return pBlueprintName;
}

//=========================================================================

const char* inventory2::ItemToPickupGeomName( inven_item Item, f32 Amount /* = -1.0f */ )
{
    ASSERT( IN_RANGE( INVEN_NULL, Item, INVEN_COUNT) );

    const char* pGeomName = s_Table[Item].pPickupGeomName;

    // This is mildly HACKish, but it is better than the previous way of doing it--which was multiple ways
    // in a couple of different places.  I also check to make sure that any of these variable quantity pickups
    // are indeed created to correspond to their amount.
    switch ( Item )
    {
        case INVEN_GRENADE_FRAG:
            ASSERT( Amount > 0.0f );

            if( Amount >= 2.0f )
                pGeomName = PRELOAD_MP_FILE( "WPN_FRG_Pickup_Ammo_Large.rigidgeom" );
            break;

        case INVEN_GRENADE_GRAV:
        case INVEN_GRENADE_JBEAN:
            ASSERT( Amount > 0.0f );

            if( Amount >= 2.0f )
                pGeomName = PRELOAD_MP_FILE( "WPN_GRAV_Pickup_Large.rigidgeom" );
            break;

        case INVEN_HEALTH:
            ASSERT( Amount > 0.0f );
            
            if( Amount >= 51.0f )
                pGeomName = PRELOAD_MP_FILE( "Pickup_Health_Large.rigidgeom" );
            
            break;

        case INVEN_MUTAGEN:
            ASSERT( Amount > 0.0f );

            if( Amount >= 26.0f )
                pGeomName = PRELOAD_MP_FILE( "Pickup_Mutagen_Large.rigidgeom" );

            break;

        case INVEN_AMMO_SMP:
            ASSERT( Amount > 0.0f );

            if( Amount >= 41.0f )
                pGeomName = PRELOAD_MP_FILE( "WPN_SMP_Pickup_Ammo_Large.rigidgeom" );
            break;

        case INVEN_AMMO_SHOTGUN:
            ASSERT( Amount > 0.0f );

            if( Amount >= 9.0f )
                pGeomName = PRELOAD_MP_FILE( "WPN_SHT_Pickup_Ammo_Large.rigidgeom" );
            break;

        case INVEN_AMMO_DESERT_EAGLE:
            ASSERT( Amount > 0.0f );

            if( Amount >= 9.0f )
                pGeomName = PRELOAD_MP_FILE( "WPN_EGL_Pickup_Ammo_Large.rigidgeom" );
            break; 

        default:
            break;
    }

    CLOG_MESSAGE( INVENTORY_LOGGING, "Inventory::ItemToPickupGeomName", "%d = '%s'", Item, pGeomName );
    return pGeomName;
}

//=========================================================================


//=========================================================================
//  Instance members
//=========================================================================

inventory2::inventory2()
{
    // Clear the data
    Clear();
}

//=========================================================================

inventory2::~inventory2()
{
}

//=========================================================================

void inventory2::Init( void )
{
    CLOG_MESSAGE( INVENTORY_LOGGING, "inventory::Init", "" );

#if !defined(X_RETAIL)
    // Run validation on the data table to make sure nothing got screwed up with our enums
    if( !s_Validated )
    {
        ASSERT( GetNumItems() == NUM_TABLE_ENTRIES );

        for( s32 i=0 ; i<NUM_TABLE_ENTRIES ; i++ )
        {
            ASSERT( s_Table[i].Item == i );
            if( s_Table[i].Item != i )
            {
                LOG_ERROR( "Inventory", "Error in inventory table, entry %d = %d '%s'", i, s_Table[i].Item, s_Table[i].pName );
            }
        }

        // Done validating
        s_Validated = TRUE;
    }
#endif // X_RETAIL

    // Clear the data
    Clear();
}

//=========================================================================

void inventory2::Clear( void )
{
    // Zero out the data
    x_memset( m_Data, 0, sizeof( m_Data ) );
}

//=========================================================================

inventory2& inventory2::operator =( const inventory2& Source )
{
    // Just copy the data from the Source inventory
    x_memcpy( m_Data, Source.m_Data, sizeof(m_Data) );

    // Return me
    return *this;
}

//=========================================================================

f32 inventory2::GetAmount( inven_item Item )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );

    CLOG_MESSAGE( INVENTORY_LOGGING, "inventory::GetAmount", "'%s' = %.3f", ItemToName(Item), m_Data[Item] );

    return m_Data[Item];
}

//=========================================================================

void inventory2::SetAmount( inven_item Item, f32 Amount )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );

    CLOG_MESSAGE( INVENTORY_LOGGING, "inventory::SetAmount", "'%s' = %.3f, was %.3f", ItemToName(Item), Amount, m_Data[Item] );

    m_Data[Item] = Amount;
}

//=========================================================================

void inventory2::AddAmount( inven_item Item, f32 Amount )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );

    CLOG_MESSAGE( INVENTORY_LOGGING, "inventory::AddAmount", "'%s' = %.3f, added %.3f", ItemToName(Item), m_Data[Item], Amount );
    
    // Add the amount
    m_Data[Item] += Amount;

    // Clamp if we have a max
    if( s_Table[Item].Max > 0.0f )
    {
        m_Data[Item] = MIN( m_Data[Item], s_Table[Item].Max );
    }
}

//=========================================================================

void inventory2::RemoveAmount( inven_item Item, f32 Amount )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );

    CLOG_MESSAGE( INVENTORY_LOGGING, "inventory::RemoveAmount", "'%s' = %.3f, removed %.3f", ItemToName(Item), m_Data[Item], Amount );

    m_Data[Item] = MAX( m_Data[Item] - Amount, s_Table[Item].Min );
}

//=========================================================================

f32 inventory2::GetMinAmount( inven_item Item )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );
    return s_Table[Item].Min;
}

//=========================================================================

f32 inventory2::GetMaxAmount( inven_item Item )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );

    if( s_Table[Item].Max == -1.0f )
    {
        return 0.0f;
    }

    return s_Table[Item].Max;
}

//=========================================================================

xbool inventory2::CanHoldMore( inven_item Item )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );
    return  ( (m_Data[Item] < s_Table[Item].Max) || (s_Table[Item].Max == 0.0f) && !(s_Table[Item].Max == -1.0f));
}

//=========================================================================

xbool inventory2::HasItem( inven_item Item )
{
    ASSERT( (Item >= INVEN_NULL) && (Item < INVEN_COUNT) );
    return m_Data[Item] > 0.0f;
}

//=========================================================================

void inventory2::Save( bitstream& Bitstream )
{
    s32 nItems = GetNumItems();

    Bitstream.WriteMarker();
    {
        x_DebugMsg("-----------SAVE INVENTORY---------\n");
        x_DebugMsg("nItems: %d\n",nItems);

        Bitstream.WriteS32( nItems );

        for( s32 i=0 ; i<nItems ; i++ )
        {
            x_DebugMsg("%3d] %f\n",i,m_Data[i]);
            Bitstream.WriteF32( m_Data[i] );
        }
    }
    Bitstream.WriteMarker();
}

//=========================================================================

void inventory2::Load( bitstream& Bitstream )
{
    Clear();
    s32 nItems = GetNumItems();

    Bitstream.ReadMarker();
    {
        x_DebugMsg("-----------RESTORE INVENTORY---------\n");

        s32 nSavedItems;
        Bitstream.ReadS32( nSavedItems );
        x_DebugMsg("nItems: %d\n",nSavedItems);

        for( s32 i=0 ; i<nSavedItems ; i++ )
        {
            f32 Temp;
            Bitstream.ReadF32( Temp );
            x_DebugMsg("%3d] %f\n",i,Temp);

            if( i < nItems )
            {
                m_Data[i] = Temp;
            }
        }
    }
    Bitstream.ReadMarker();
}

//=========================================================================

#ifdef ENABLE_DEBUG_MENU

void inventory2::LogInventory( void )
{
    for( s32 i=0 ; i<INVEN_COUNT ; i++ )
    {
        LOG_MESSAGE( "inventory::LogInventory", "'%s' = %.3f", ItemToName( i ), m_Data[i] );
    }
}

#endif

//=========================================================================
