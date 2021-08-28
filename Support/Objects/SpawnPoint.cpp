//==============================================================================
//
//  spawn_point.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "SpawnPoint.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "InvisWall.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
const f32 c_Sphere_Radius = 50.0f;

static struct spawn_point_desc : public object_desc
{
    spawn_point_desc( void ) : object_desc( object::TYPE_SPAWN_POINT, 
                                            "SpawnPoint",
                                            "Multiplayer",
                                            object::ATTR_SPACIAL_ENTRY,
                                            FLAGS_GENERIC_EDITOR_CREATE | 
                                            FLAGS_IS_DYNAMIC ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new spawn_point; }

        //-------------------------------------------------------------------------

#ifdef X_EDITOR

        virtual s32  OnEditorRender( object& Object ) const
        {
            if( Object.IsKindOf( spawn_point::GetRTTI() ) )
            {
                spawn_point SpawnPt = spawn_point::GetSafeType( Object );
                xcolor      Color;

                switch( (SpawnPt.GetCircuit()).GetTeamBits() )
                {
                case 0x00000000:    Color = XCOLOR_YELLOW;      break;
                case 0x00000001:    Color = XCOLOR_RED;         break;
                case 0x00000002:    Color = XCOLOR_GREEN;       break;
                case 0xFFFFFFFF:    Color = XCOLOR_BLUE;        break;
                case 0xDEADC0DE:    Color = XCOLOR_WHITE;       break;
                default:            ASSERT( FALSE );            break;
                }

                EditorIcon_Draw( EDITOR_ICON_SPAWN_POINT, 
                    SpawnPt.GetL2W(), 
                    !!(SpawnPt.GetAttrBits() & object::ATTR_EDITOR_SELECTED), 
                    Color );

                if( mp_settings::s_Selected )
                    SpawnPt.GetCircuit().SpecialRender( SpawnPt.GetPosition() );
            }
            else
            {
                ASSERT( FALSE );
            }

            return( -1 );
        }

#endif // X_EDITOR

} s_spawn_point_Desc;

//==============================================================================

const object_desc& spawn_point::GetTypeDesc( void ) const
{
    return s_spawn_point_Desc;
}

//==============================================================================

const object_desc& spawn_point::GetObjectType( void )
{
    return s_spawn_point_Desc;
}

//==============================================================================
// spawn_point
//==============================================================================

spawn_point::spawn_point( void)
{
}

//==============================================================================

spawn_point::~spawn_point( void )
{
}

//==============================================================================

bbox spawn_point::GetLocalBBox( void ) const 
{ 
    return( bbox( vector3(0,0,0), c_Sphere_Radius ) );
}

//==============================================================================

void spawn_point::OnEnumProp( prop_enum& List )
{
    m_Circuit.OnEnumProp( List );
    object::OnEnumProp( List );

    u32 Flags = PROP_TYPE_DONT_SAVE | 
        PROP_TYPE_DONT_SHOW | 
        PROP_TYPE_DONT_EXPORT | 
        PROP_TYPE_DONT_SAVE_MEMCARD;

    // NOTE: These fields will be removed in future versions.  They are here 
    // only for backwards compatibility.  Data in these fields is only loaded.
    List.PropEnumHeader( "Spawn Point", "Spawns players for multiplayer games.", Flags );
    List.PropEnumEnum( "Spawn Point\\Team", 
                       "Team 0 (Alpha)\0Team 1 (Omega)\0All\0None\0", 
                       "Who can use this spawn point?", Flags );
}

//==============================================================================

xbool spawn_point::OnProperty( prop_query& Query )
{ 
    if( object::OnProperty( Query ) )
    {
        return TRUE;
    }

    if( m_Circuit.OnProperty( Query ) )
    {
        return( TRUE );
    }

    return( FALSE );
}

//=============================================================================

guid spawn_point::GetSpawnInfo( guid     ActorGuid, 
                                vector3& Position, 
                                radian3& Rotation, 
                                u16&     Zone1, 
                                u16&     Zone2 ) const
{
    // Default to using spawn point info in case there is nothing below it
    Position = GetPosition();
    Rotation = GetL2W().GetRotation();
    Zone1    = GetZone1();
    Zone2    = GetZone2();

    // Clear zone object
    guid ZoneGuid = 0;

    // Cast a cylinder down to find objects below
    vector3 Start = Position + vector3( 0,   10, 0 );
    vector3 End   = Position + vector3( 0, -100, 0 );
    g_CollisionMgr.CylinderSetup( ActorGuid, Start, End, 30.0f, 180.0f );
    g_CollisionMgr.SetMaxCollisions( 5 );

    // Did we hit anything?    
    if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                        object::ATTR_BLOCKS_PLAYER,
                                        object::ATTR_COLLISION_PERMEABLE ) )
    {
        // For position, always use the 1st object that was hit
        // (this could be an invisible wall that the player needs to walk on)
        Position = g_CollisionMgr.m_Collisions[0].Point;
    
        // Loop over all collisions looking for the 1st render-able play surface
        // (we can't just use the invisible walls zone because some of them span more than 1 zone!)
        for( s32 i = 0; i < g_CollisionMgr.m_nCollisions; i++ )
        {
            // Lookup hit object
            guid    ObjectHitGuid = g_CollisionMgr.m_Collisions[i].ObjectHitGuid;
            object* pObjectHit    = g_ObjMgr.GetObjectByGuid( ObjectHitGuid );
            ASSERT( pObjectHit );

            // Is this object a play surface? (which has to be in the correct zone to render!)
            if(     ( pObjectHit->GetType() == object::TYPE_PLAY_SURFACE_PROXY )
                ||  ( pObjectHit->GetType() == object::TYPE_PLAY_SURFACE       ) )
            {
                // Use zone info from play surface
                ZoneGuid = ObjectHitGuid;
                Zone1    = pObjectHit->GetZone1();
                Zone2    = pObjectHit->GetZone2();
                
                // Exit for loop
                break;
            }                
        }
    }
    else
    {
#ifndef X_EDITOR
        // Spawn points MUST have a play-surface within 1 meter below them so zone information can be used
        ASSERTS( 0, xfs( "Spawn point %08X:%08X does not have a valid play-surface within 1 meter below!", GetGuid().GetHigh(), GetGuid().GetLow() ) );
#endif    
    }

    // Return object that zone info was obtained from (or 0 if none)            
    return ZoneGuid;
}

//=============================================================================

#ifdef X_EDITOR

s32 spawn_point::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = object::OnValidateProperties( ErrorMsg );

    // Validate spawn info
    vector3 Position;
    radian3 Rotation;
    u16     Zone1;
    u16     Zone2;
    guid    ZoneGuid = GetSpawnInfo( 0, Position, Rotation, Zone1, Zone2 );

    // Did we hit a play surface?
    if( ZoneGuid )
    {
        // Lookup object that zone was obtained from
        object* pZoneObject = g_ObjMgr.GetObjectByGuid( ZoneGuid );
        ASSERT( pZoneObject );
    
        // Make sure hit spawn point is in the same zone as the hit object
        if( ( GetZone1() != pZoneObject->GetZone1() ) ||
            ( GetZone2() != pZoneObject->GetZone2() ) )
        {
            nErrors++;
            ErrorMsg += "ERROR: Spawn point is not in the same zone as the 1st play surface below it: " + guid_ToString( ZoneGuid ) + "\n";
            ErrorMsg += "Either the spawn point zone or play surface zone is wrong -\n";
            ErrorMsg += "You MUST fix this issue to avoid spawn point zone render problems!\n\n";
        }
    }
    else
    {
        // No zone object was found
        nErrors++;
        ErrorMsg += "ERROR: Spawn point is more than 1m above a playsurface\n";
    }

    return nErrors;
}

#endif

//===========================================================================
