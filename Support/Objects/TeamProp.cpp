//=========================================================================
//
// TeamProp.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "TeamProp.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "x_context.hpp"
#include "NetworkMgr\NetObjMgr.hpp"
#include "Player.hpp"
#include "Render\LightMgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"

#ifndef X_EDITOR
#include "GameLib\RenderContext.hpp"
#endif

//=========================================================================

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct team_prop_desc : public object_desc
{
    team_prop_desc( void ) : object_desc( 
        object::TYPE_TEAM_PROP, 
        "Team Prop",
        "Multiplayer",
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS      | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_PAIN_LOS        | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_RECEIVE_SHADOWS        |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_NEEDS_LOGIC_TIME,       

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING
    ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) {                                  
        return new team_prop; 
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }
} s_Team_Prop_Desc;

//=========================================================================

const object_desc& team_prop::GetTypeDesc( void ) const
{
    return s_Team_Prop_Desc;
}

//=========================================================================

const object_desc& team_prop::GetObjectType( void )
{
    return s_Team_Prop_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

team_prop::team_prop( void )
{ 
    m_NewState = FRIENDLY_ALL;
    m_OldState = FRIENDLY_ALL;
    m_TransitionValue = 1.0f;
}

//=========================================================================

void team_prop::SetL2W( const matrix4& L2W )
{
    SetTransform( L2W );
}

//=========================================================================

void team_prop::SetGeom( char* pString )
{ 
    m_Inst.SetUpRigidGeom( pString );
}

//=========================================================================

team_prop::~team_prop( void )
{ 
}

//=========================================================================

void team_prop::OnInit( void )
{
}

//=========================================================================

void team_prop::OnRender( void )
{
    CONTEXT( "team_prop::OnRender" );

#ifndef X_EDITOR
    if( m_Circuit.GetCircuit() == 15 )
    {
        return;
    }
#endif

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();

    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        if ( pRigidGeom->m_nBones > 1 )
        {
#ifdef X_EDITOR
            // only display the error message once
            static xbool Once = TRUE;
            if ( Once )
            {
                Once = FALSE;
                x_try;
                x_throw( xfs( "Play surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
                x_catch_display;
            }
#else
            ASSERTS( 0, xfs( "Play surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
#endif
        }
        else
        {
#ifndef X_EDITOR
            player* pPlayer = (player*)NetObjMgr.GetObjFromSlot( g_RenderContext.NetPlayerSlot );
            u32 PlayerTeamBits = pPlayer->net_GetTeamBits();
            u32 States[ 2 ]    = { m_OldState, m_NewState };
#else
            // Assume you're viewing as if you're on Omega in the editor (To be consistent with other circuit based objects).
            u32 PlayerTeamBits = 0x00000002;
            u32 States[ 2 ]    = { m_Circuit.GetTeamBits(), m_Circuit.GetTeamBits() };
#endif

            u32 FadePoints[ 2 ] = { FRIEND_TO_ALL, FRIEND_TO_ALL };

            // We know what the alignment is, just check how that relates to the player viewing it.
            for( s32 i = 0; i < 2; i++ )
            {
                switch( States[ i ] )
                {
                case FRIENDLY_NONE:
                    FadePoints[ i ] = FRIEND_TO_NONE;
                    break;

                case FRIENDLY_ALPHA:
                    FadePoints[ i ] = (PlayerTeamBits & States[ i ]) ? FRIEND_TO_TEAM_ALPHA : FRIEND_TO_ENEMY_ALPHA;
                    break;

                case FRIENDLY_OMEGA:
                    FadePoints[ i ] = (PlayerTeamBits & States[ i ]) ? FRIEND_TO_TEAM_OMEGA : FRIEND_TO_ENEMY_OMEGA;
                    break;

                case FRIENDLY_ALL:
                    FadePoints[ i ] = FRIEND_TO_ALL;
                    break;

                default:
                    break;
                }
            }
                 
            // This will always render the newest state at full alpha.  If we just started a transition,
            // the old state will also be at full alpha, but when it drops to 0 it won't be rendered at all.
            {
                m_Inst.Render( &GetL2W(), Flags | GetRenderMode(), FadePoints[ 0 ], (s32)((1.0f - m_TransitionValue) * 255) );
                m_Inst.Render( &GetL2W(), Flags | GetRenderMode(), FadePoints[ 1 ], 255 );
            }   

#ifdef X_EDITOR
            if( mp_settings::s_Selected )
            {
                m_Circuit.SpecialRender( GetPosition() );
            }
#endif
        }
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=========================================================================

void team_prop::OnAdvanceLogic ( f32 DeltaTime )
{
    CONTEXT( "team_prop::OnAdvanceLogic" );

    u32 TeamBits = m_Circuit.GetTeamBits();
    
    // If the state has changed, swap the state values and reset the 
    // transition time to start the fading process.
    if( m_NewState != TeamBits )
    {
        m_OldState = m_NewState;
        m_NewState = TeamBits;
        m_TransitionValue = 0.0f;
    }

    f32 TransitionTime = 2.0f;

    m_TransitionValue = MINMAX( 0.0f, 
                                m_TransitionValue + (DeltaTime / TransitionTime), 
                                1.0f );
}

//==============================================================================

void team_prop::OnEnumProp( prop_enum& List )
{      
    m_Circuit.OnEnumProp( List );
    play_surface::OnEnumProp( List );
}

//==============================================================================

xbool team_prop::OnProperty( prop_query&  Query )
{
    if( play_surface::OnProperty( Query ) )
    {
        return( TRUE );
    }
    if( m_Circuit.OnProperty( Query ) )
    {
        return( TRUE );
    }

    return( FALSE );
}

//=============================================================================

#if defined(X_EDITOR)
s32 team_prop::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = play_surface::OnValidateProperties( ErrorMsg );

    const rigid_geom* pGeom = m_Inst.GetRigidGeom();
    if( pGeom && pGeom->m_nVirtualMaterials < NUM_FRIEND_TO_OPTIONS )
    {
        nErrors++;
        ErrorMsg += "ERROR: Team prop must have at least 6 virtual materials\n";
    }

    return nErrors;
}
#endif

//=============================================================================
