
#include "AlienPlatformDock.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Objects\Player.hpp"


//=============================================================================
// CONSTANTS
//=============================================================================
static const f32 k_ORB_ARRIVE_AT_DISTANCE               = 11.0f;
static const s32 k_MAX_ORB_TARGETS                      = 16;
//=============================================================================
// SHARED
//=============================================================================
static guid     g_OrbTargetValidList    [ k_MAX_ORB_TARGETS ];
static guid     g_OrbTargetPotentialList[ k_MAX_ORB_TARGETS ];

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_platform_dock_desc : public object_desc
{
    alien_platform_dock_desc( void ) : object_desc( 
        object::TYPE_ALIEN_PLATFORM_DOCK, 
        "Alien Platform Dock", 
        "PROPS",
        object::ATTR_COLLIDABLE             |         
        object::ATTR_BLOCKS_ALL_PROJECTILES |         
        object::ATTR_BLOCKS_ALL_ACTORS      |         
        object::ATTR_BLOCKS_RAGDOLL         |         
        object::ATTR_BLOCKS_CHARACTER_LOS   |         
        object::ATTR_BLOCKS_PLAYER_LOS      |         
        object::ATTR_BLOCKS_PAIN_LOS        |         
        object::ATTR_BLOCKS_SMALL_DEBRIS    |         
        object::ATTR_RENDERABLE             |
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new alien_platform_dock; }

} s_AlienOrb_Desc;

//=============================================================================

alien_platform_dock::alien_platform_dock()
{
    m_DockedPlatform            = NULL_GUID;
    m_nDestinations             = 0;
    m_bPlayerOn                 = FALSE;
    m_bHighlighted              = FALSE;
    m_bActive                   = FALSE;
    m_State                     = STATE_IDLE;
    m_CurrentEffect             = EFFECT_UNKNOWN;

    s32 i;
    for (i=0;i<DOCK_MAX_DESTINATIONS;i++)
    {
        m_Destination[i].m_Dock.SetStaticGuid(NULL_GUID);
        m_Destination[i].m_Path.SetStaticGuid(NULL_GUID);
    }
}

//=============================================================================

alien_platform_dock::~alien_platform_dock()
{
}

//=============================================================================

const object_desc& alien_platform_dock::GetTypeDesc( void ) const
{
    return s_AlienOrb_Desc;
}

//=============================================================================

const object_desc& alien_platform_dock::GetObjectType( void )
{
    return s_AlienOrb_Desc;
}

//=============================================================================

void alien_platform_dock::OnColCheck( void )
{    
    anim_surface::OnColCheck();   
}

//=============================================================================

void alien_platform_dock::OnEnumProp      ( prop_enum&    List )
{
    anim_surface::OnEnumProp( List );

    List.PropEnumHeader( "Alien Dock", "Properties for the alien platform dock object", 0 );
    List.PropEnumGuid  ( "Alien Dock\\Activate Idle",        "Particle effect or object to activate when in idle state", 0 );
    List.PropEnumGuid  ( "Alien Dock\\Activate Active",      "Particle effect or object to activate when in active state", 0 );
    List.PropEnumGuid  ( "Alien Dock\\Activate Highlighted", "Particle effect or object to activate when in highlighted state", 0 );

    List.PropEnumButton   ( "Alien Dock\\Add Destination", 
        "Press the button to add a new destination.",
        PROP_TYPE_MUST_ENUM );

    s32 i;

    for (i=0;i<m_nDestinations;i++)
    {
        List.PropEnumString( xfs("Alien Dock\\Dest [%d]",i ),"", PROP_TYPE_HEADER );
        s32 iHeader = List.PushPath(xfs("Alien Dock\\Dest [%d]\\",i));

        m_Destination[i].m_Dock.OnEnumProp( List, "Dock" );
        m_Destination[i].m_Path.OnEnumProp( List, "Path" );

        List.PropEnumButton( "Remove", "Remove destination", PROP_TYPE_MUST_ENUM );

        List.PopPath(iHeader);          
    }  
}

//=============================================================================

xbool alien_platform_dock::OnProperty      ( prop_query&   I    )
{
    if (anim_surface::OnProperty( I ))
    {
        return TRUE;
    }   
    else if (I.VarGUID( "Alien Dock\\Activate Idle", m_Effect[ EFFECT_IDLE ] ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Dock\\Activate Active", m_Effect[ EFFECT_ACTIVE ] ))
    {
        return TRUE;
    }
    else if (I.VarGUID( "Alien Dock\\Activate Highlighted", m_Effect[ EFFECT_HIGHLIGHTED ] ))
    {
        return TRUE;
    }
    else if( I.IsVar( "Alien Dock\\Add Destination" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Destination" );
        }
        else
        {
            if (m_nDestinations < DOCK_MAX_DESTINATIONS)
            {                
                m_nDestinations++;
                m_Destination[ m_nDestinations ].m_Dock.SetStaticGuid(NULL_GUID);
                m_Destination[ m_nDestinations ].m_Path.SetStaticGuid(NULL_GUID);
            }
        }
        return TRUE;
    }
    else 
    if (I.IsSimilarPath( "Alien Dock\\Dest" ) )
    {
        s32 i = I.GetIndex(0);
        if (i>=0)
        {
            if (i >= m_nDestinations)
            {
                // We are accessing beyond what we have stored
                s32 j;
                for (j=m_nDestinations;j<i;j++)
                {
                    m_Destination[j].m_Path.SetStaticGuid(NULL_GUID);
                    m_Destination[j].m_Dock.SetStaticGuid(NULL_GUID);
                }
                m_nDestinations = i+1;
            }

            if (I.IsSimilarPath("Alien Dock\\Dest []\\Dock"))
            {
                s32 iPath = I.PushPath("Alien Dock\\Dest []\\");
                if (m_Destination[i].m_Dock.OnProperty( I, "Dock" ))
                {                   
                    I.PopPath( iPath );
                    return TRUE;
                }
                I.PopPath( iPath );
            }
            else 
            if (I.IsSimilarPath("Alien Dock\\Dest []\\Path"))
            {
                s32 iPath = I.PushPath("Alien Dock\\Dest []\\");
                if (m_Destination[i].m_Path.OnProperty( I, "Path" ))
                {                   
                    I.PopPath( iPath );
                    return TRUE;
                }
                I.PopPath( iPath );
            }
            else
            if ( I.IsVar( "Alien Dock\\Dest []\\Remove" ) )
            {
                if( I.IsRead() )
                {
                    I.SetVarButton( "Remove" );
                }
                else
                {
                    // Delete this child
                    s32 j;
                    for (j=i;j<m_nDestinations-1;j++)
                    {
                        m_Destination[j] = m_Destination[j+1];
                    }

                    m_nDestinations--;                    
                }
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

//=============================================================================

void alien_platform_dock::OnAdvanceLogic  ( f32 DeltaTime )
{
    anim_surface::OnAdvanceLogic( DeltaTime );
/*
    xbool m_bPlayerWasOn = m_bPlayerOn;

    m_bPlayerOn = FALSE;

    // Is player standing on the dock?
    if (IsPlayerOn())
    {
        m_bPlayerOn = TRUE;    
    }

    if (m_bPlayerWasOn != m_bPlayerOn)
    {
        if (m_bPlayerOn)
        {
            // Player has stepped onto the dock
            m_bActive = TRUE;
        }
        else
        {
            // Player has stepped off of the dock
            m_bActive = FALSE;
        }
    }
*/
    state NewState = STATE_IDLE;
    if (m_bActive)
        NewState = STATE_ACTIVE;
    if (m_bHighlighted)
        NewState = STATE_HIGHLIGHTED;

    if (NewState != m_State)
    {
        SwitchState( NewState );
    }   
}

//=============================================================================

void alien_platform_dock::SwitchState( state NewState )
{
    // No transitions for now
    m_State = NewState;

    switch( m_State )
    {
    case STATE_IDLE:        
        ActivateEffect( EFFECT_IDLE );
        break;
    case STATE_ACTIVE:        
        ActivateEffect( EFFECT_ACTIVE );
        break;
    case STATE_HIGHLIGHTED:        
        ActivateEffect( EFFECT_HIGHLIGHTED );
        break;
    }    
}

//=============================================================================

#ifdef X_EDITOR

s32 alien_platform_dock::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = anim_surface::OnValidateProperties( ErrorMsg );

    return nErrors;
}

#endif

//=============================================================================

#ifndef X_RETAIL
void alien_platform_dock::OnDebugRender( void )
{
    anim_surface::OnDebugRender();
}
#endif // X_RETAIL


//=============================================================================

xbool alien_platform_dock::IsPlayerOn( void )
{
    // ASSUMING THIS WILL NEVER BE IN MULTIPLAYER
    // since it's a moving platform
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( !pPlayer )
        return FALSE;
    
    bbox Box = pPlayer->GetBBox();

    Box.Inflate( 10,10,10 );

    if (Box.Intersect( GetBBox() ))
        return TRUE;

    return FALSE;
}

//=============================================================================

#if !defined( CONFIG_RETAIL )

void alien_platform_dock::OnRender( void )
{
    anim_surface::OnRender();

    if (0)
    {
        if (m_bPlayerOn)
        {
            bbox Box = GetBBox();
            Box.Inflate( 50,50,50 );

            draw_BBox( Box, XCOLOR_YELLOW );
        }
        if (m_bHighlighted)
        {
            bbox Box = GetBBox();
            Box.Inflate( 50,50,50 );

            draw_BBox( Box, XCOLOR_RED );
        }

        sphere Sphere(GetBBox());
        draw_Sphere( Sphere.Pos, Sphere.R, XCOLOR_RED );
    }

}

#endif // !defined( CONFIG_RETAIL )

//=============================================================================

s32 alien_platform_dock::GetDestinationCount( void )
{
    return m_nDestinations;
}

//=============================================================================

xbool alien_platform_dock::GetDestination( s32 iDest, guid& OutDock, guid& OutPath )
{
    OutDock = NULL_GUID;
    OutPath = NULL_GUID;

    if ((iDest < 0) || (iDest >= m_nDestinations))
        return FALSE;

    OutDock = m_Destination[ iDest ].m_Dock.GetGuid();
    OutPath = m_Destination[ iDest ].m_Path.GetGuid();

    return TRUE;
}

//=============================================================================

void alien_platform_dock::Highlight( void )
{
    m_bHighlighted = TRUE;
}

//=============================================================================

void alien_platform_dock::Unhighlight( void )
{
    m_bHighlighted = FALSE;
}

//=============================================================================

void alien_platform_dock::ActivateEffect( effect Effect )
{
    if (Effect == m_CurrentEffect)
        return;

    guid OldEffect = NULL_GUID;
    guid NewEffect = NULL_GUID;
    
    if (m_CurrentEffect < EFFECT_COUNT)
        OldEffect = m_Effect[ m_CurrentEffect ];

    if (Effect < EFFECT_COUNT)
        NewEffect = m_Effect[ Effect ];

    // Deactivate old effect
    if (NULL_GUID != OldEffect)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( OldEffect );
        if (NULL != pObj)
        {
            pObj->OnActivate( FALSE );
        }
    }
    
    // Activate old effect
    if (NULL_GUID != NewEffect)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( NewEffect );
        if (NULL != pObj)
        {
            pObj->OnActivate( TRUE );
        }
    }

    m_CurrentEffect = Effect;
}

//=============================================================================

void alien_platform_dock::ActivateDock( xbool bActive )
{
    m_bActive = bActive;
}

//=============================================================================

//=============================================================================
