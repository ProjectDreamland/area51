//==============================================================================
//
//  FlagBase.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "FlagBase.hpp"
#include "GameLib/RenderContext.hpp"
#include "TemplateMgr/TemplateMgr.hpp"
#include "Entropy/e_Draw.hpp"
#include "Objects/Actor/Actor.hpp"

#ifndef X_EDITOR
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/logic_Base.hpp"
#endif

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct flag_base_desc : public object_desc
{
    flag_base_desc( void ) 
        :   object_desc( object::TYPE_FLAG_BASE, 
                         "FlagBase",
                         "Multiplayer",
                         object::ATTR_NEEDS_LOGIC_TIME     |
                         object::ATTR_COLLIDABLE           |
                         object::ATTR_BLOCKS_ALL_PROJECTILES |
                         object::ATTR_BLOCKS_ALL_ACTORS    |
                         object::ATTR_BLOCKS_RAGDOLL       |
                         object::ATTR_BLOCKS_SMALL_DEBRIS  |
                         object::ATTR_SPACIAL_ENTRY        |
                         object::ATTR_NO_RUNTIME_SAVE,
                         FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new flag_base ); 
    }

} s_flag_base_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& flag_base::GetTypeDesc( void ) const
{
    return( s_flag_base_Desc );
}

//==============================================================================

const object_desc& flag_base::GetObjectType( void )
{
    return( s_flag_base_Desc );
}

//==============================================================================

flag_base::flag_base( void )
{
    m_bInitialized = FALSE;

    #ifndef X_EDITOR
    m_NetTeamBits = 0x00000000;
    #endif
}

//==============================================================================

void flag_base::Init( s32 BaseCircuit, const matrix4& L2W )
{
    ASSERT( IN_RANGE( 0, BaseCircuit, 3 ) );

    #ifndef X_EDITOR
    if( BaseCircuit == 3 )  m_NetTeamBits = 0x00000000;
    else                    m_NetTeamBits = BaseCircuit;
    #endif

    SetTransform( L2W );

    if( !m_bInitialized )
    {
        m_BaseGuid    = g_ObjMgr.CreateObject( team_prop::GetObjectType() );
        object* pBase = g_ObjMgr.GetObjectByGuid( m_BaseGuid );

        if( pBase )
        {
            ((team_prop*)pBase)->SetGeom( "MP_team_flag_base_000.rigidgeom" );
            ((team_prop*)pBase)->SetCircuit( BaseCircuit );
            ((team_prop*)pBase)->SetL2W( L2W );
        }
    }

    m_bInitialized = TRUE;
}

//==============================================================================

flag_base::~flag_base( void )
{
    // Dispose of geometry and effects.
    g_ObjMgr.DestroyObject( m_BaseGuid );
}

//==============================================================================

bbox flag_base::GetLocalBBox( void ) const 
{ 
    bbox BBox( vector3( -100.0f, 25.0f, -100.0f ),
               vector3( +100.0f,  0.0f, +100.0f ) );
    return( BBox );
}

//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void flag_base::net_AcceptUpdate( const bitstream& BS )
{
    // netobj::ACTIVATE_BIT
    if( BS.ReadFlag() )
    {
        vector3 Position;
        radian3 Orientation;
        s32     Zone1;
        s32     Zone2;
        s32     Circuit;

        Orientation.Zero();

        BS.ReadVector( Position );
        BS.ReadF32( Orientation.Yaw );
        BS.ReadRangedS32( Zone1, 0, 255 );
        BS.ReadRangedS32( Zone2, 0, 255 );
        BS.ReadRangedS32( Circuit, 0, 3 );

        SetZone1( Zone1 );
        SetZone2( Zone2 );
        
        matrix4 L2W;
        L2W.Identity();
        L2W.SetRotation( Orientation );
        L2W.SetTranslation( Position );

        Init( Circuit, L2W );
    }
}

//==============================================================================

void flag_base::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{
//  if( DirtyBits & netobj::ACTIVATE_BIT )
//      DirtyBits |= DIRTY_ALL;

    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        vector3 Position    = GetL2W().GetTranslation();
        radian3 Orientation = GetL2W().GetRotation();
        s32     Circuit;

        if( m_NetTeamBits == 0xFFFFFFFF )   Circuit = 0;
        else                                Circuit = (s32)m_NetTeamBits;

        BS.WriteVector( Position );
        BS.WriteF32( Orientation.Yaw );
        BS.WriteRangedS32( GetZone1(), 0, 255 );
        BS.WriteRangedS32( GetZone2(), 0, 255 );
        BS.WriteRangedS32( Circuit, 0, 3 );
        
        DirtyBits &= ~netobj::ACTIVATE_BIT;
    }
}

//==============================================================================
#endif // X_EDITOR
//==============================================================================
