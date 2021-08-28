//=============================================================================
//  MUTAGEN_RESERVOIR.CPP
//=============================================================================
//
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================
#include "MutagenReservoir.hpp"
#include "player.hpp"

//=============================================================================
// TWEAKS
//=============================================================================
struct reservoir_tweaks
{
    reservoir_tweaks( void );

    f32 Radius;
};

reservoir_tweaks::reservoir_tweaks( void ) :
    Radius( 100.0f )
{
}

reservoir_tweaks g_ReservoirTweaks;

//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct mutagen_reservoir_desc : public object_desc
{
    mutagen_reservoir_desc( void ) : object_desc( 
            object::TYPE_MUTAGEN_RESERVOIR, 
            "Mutagen Reservoir",
            "PROPS",
            object::ATTR_RENDERABLE             |
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_SPACIAL_ENTRY,
            FLAGS_GENERIC_EDITOR_CREATE ) 
    {}
    
    virtual object* Create( void ) { return new mutagen_reservoir; }
    
} s_mutagen_reservoir_desc ;

//=============================================================================================

const object_desc&  mutagen_reservoir::GetTypeDesc ( void ) const
{
    return s_mutagen_reservoir_desc;
}

//=============================================================================================

const object_desc&  mutagen_reservoir::GetObjectType ( void )
{
    return s_mutagen_reservoir_desc;
}

//=============================================================================================
mutagen_reservoir::mutagen_reservoir()
{
}

//=============================================================================================

mutagen_reservoir::~mutagen_reservoir()
{
}

//=============================================================================

void mutagen_reservoir::OnInit( void )
{
    object::OnInit();
}

//=============================================================================================

void mutagen_reservoir::OnAdvanceLogic( f32 DeltaTime )
{
    object::OnAdvanceLogic( DeltaTime );
}

//=============================================================================================

void mutagen_reservoir::OnRender( void )
{
}

//=============================================================================

bbox mutagen_reservoir::GetLocalBBox( void ) const
{
    return bbox( vector3( 0.0f, 0.0f, 0.0f ), g_ReservoirTweaks.Radius );
}

//=============================================================================

s32 mutagen_reservoir::GetMaterial( void ) const
{
    return 0xffffffff;
}
