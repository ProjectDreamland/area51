
#include "reactivesurface.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Player.hpp"

xstring g_ReactiveSurfaceStringList;

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct reactive_surface_desc : public object_desc
{
    reactive_surface_desc( void ) : object_desc( 
        object::TYPE_REACTIVE_SURFACE, 
        "Reactive Surface", 
        "PROPS",
        object::ATTR_COLLIDABLE       | 
        object::ATTR_BLOCKS_ALL_PROJECTILES | 
        object::ATTR_BLOCKS_ALL_ACTORS | 
        object::ATTR_BLOCKS_RAGDOLL | 
        object::ATTR_BLOCKS_CHARACTER_LOS | 
        object::ATTR_BLOCKS_PLAYER_LOS | 
        object::ATTR_BLOCKS_PAIN_LOS | 
        object::ATTR_BLOCKS_SMALL_DEBRIS | 
        object::ATTR_RENDERABLE       |
        object::ATTR_SPACIAL_ENTRY    |
        object::ATTR_NEEDS_LOGIC_TIME, 

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING  ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new reactive_surface; }

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

} s_ReactiveSurface_Desc;

//=============================================================================

reactive_surface::reactive_surface() :
m_ActiveDistance(800.0f),
m_iEnterReaction(-1),
m_iExitReaction(-1),    
m_iInactiveIdle(-1),
m_iActiveIdle(-1),
m_State(OUT_OF_RANGE)
{
    // Disable logic.  
    // SetAttrBits( GetAttrBits() & (~(object::ATTR_NEEDS_LOGIC_TIME) ));
}

//=============================================================================

reactive_surface::~reactive_surface()
{
}

//=============================================================================

const object_desc& reactive_surface::GetTypeDesc( void ) const
{
    return s_ReactiveSurface_Desc;
}

//=============================================================================

const object_desc& reactive_surface::GetObjectType( void )
{
    return s_ReactiveSurface_Desc;
}

//=============================================================================

void reactive_surface::OnEnumProp      ( prop_enum&    List )
{
    anim_surface::OnEnumProp( List );

    List.PropEnumHeader( "ReactiveSurface", "Which anims to play for reactions of player proximity", 0 );
    List.PropEnumFloat("ReactiveSurface\\ActiveRange", "Range which the player needs to be within to activate.", PROP_TYPE_EXPOSE);

    if( m_hAnimGroup.GetPointer() )
    {
        //
        // TODO: HACK: We need a better way to do this in the future
        //
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        g_ReactiveSurfaceStringList.Clear();

        s32 i;
        for( i=0; i<pAnimGroup->GetNAnims(); i++ )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );

            g_ReactiveSurfaceStringList += AnimInfo.GetName();
            g_ReactiveSurfaceStringList += "~";
        }

        for( i=0; g_ReactiveSurfaceStringList[i]; i++ )
        {
            if( g_ReactiveSurfaceStringList[i] == '~' )
                g_ReactiveSurfaceStringList[i] = 0;
        }

        List.PropEnumEnum( "ReactiveSurface\\EnterReaction", g_ReactiveSurfaceStringList, "When approaching the object, what anim to play.", PROP_TYPE_EXPOSE );
        List.PropEnumEnum( "ReactiveSurface\\ExitReaction", g_ReactiveSurfaceStringList, "When moving away from the object, what anim to play.", PROP_TYPE_EXPOSE );
        List.PropEnumEnum( "ReactiveSurface\\ActiveIdle", g_ReactiveSurfaceStringList, "Idle Anim for when the player is within Range.", PROP_TYPE_EXPOSE );
        List.PropEnumEnum( "ReactiveSurface\\InactiveIdle", g_ReactiveSurfaceStringList, "Idle Anim for when the player is far away.", PROP_TYPE_EXPOSE );
    }
}

//=============================================================================

xbool reactive_surface::OnProperty( prop_query&   I    )
{
    if (anim_surface::OnProperty( I ))
    {
        return TRUE;
    }

    if( I.VarFloat("ReactiveSurface\\ActiveRange", m_ActiveDistance, 100.0f ) )
    {
        return TRUE;
    }

    if( I.IsVar( "ReactiveSurface\\EnterReaction" ) )
    {
        if( I.IsRead() )
        {
            if (m_iEnterReaction >= 0)
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iEnterReaction ) );
            }
            else
            {
                I.SetVarEnum("");
            }
        }
        else
        {
            m_iEnterReaction = g_StringMgr.Add( I.GetVarEnum() );
        }
        return TRUE;
    }

    if( I.IsVar( "ReactiveSurface\\ExitReaction" ) )
    {
        if( I.IsRead() )
        {
            if (m_iExitReaction >= 0)
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iExitReaction ) );
            }
            else
            {
                I.SetVarEnum("");
            }
        }
        else
        {
            m_iExitReaction = g_StringMgr.Add( I.GetVarEnum() );
        }
        return TRUE;
    }

    if( I.IsVar( "ReactiveSurface\\InactiveIdle" ) )
    {
        if( I.IsRead() )
        {
            if (m_iInactiveIdle >= 0)
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iInactiveIdle ) );
            }
            else
            {
                I.SetVarEnum("");
            }
        }
        else
        {
            m_iInactiveIdle = g_StringMgr.Add( I.GetVarEnum() );
        }
        return TRUE;
    }

    if( I.IsVar( "ReactiveSurface\\ActiveIdle" ) )
    {
        if( I.IsRead() )
        {
            if (m_iActiveIdle >= 0)
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iActiveIdle ) );
            }
            else
            {
                I.SetVarEnum("");
            }
        }
        else
        {
            m_iActiveIdle = g_StringMgr.Add( I.GetVarEnum() );
        }
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

void reactive_surface::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "reactive_surface::OnAdvanceLogic" );

    anim_surface::OnAdvanceLogic( DeltaTime );

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    switch(m_State)
    {
    case OUT_OF_RANGE:
        if( pPlayer )
        {
            vector3 PlayerPos = pPlayer->GetPosition();
            f32 Dist = (GetPosition() - PlayerPos).LengthSquared();
            if (Dist <= (m_ActiveDistance * m_ActiveDistance))
            {
                //entering Range
                m_State = ENTERING_RANGE;
                PlayAnim(m_iEnterReaction);
            }
        }
        break;
    case ENTERING_RANGE:
        if (m_AnimPlayer.IsAtEnd())
        {
            m_State = IN_RANGE;
            PlayAnim(m_iActiveIdle);
        }
        break;
    case IN_RANGE:
        {
            if (m_AnimPlayer.AnimDone())
            {
                m_AnimPlayer.SetStopAtEnd(FALSE);

                //anim has ended, exit range
                m_State = EXITING_RANGE;
                PlayAnim(m_iExitReaction);  
            }
            else if( pPlayer )
            {
                vector3 PlayerPos = pPlayer->GetPosition();
                f32 Dist = (GetPosition() - PlayerPos).LengthSquared();
                if (Dist > (m_ActiveDistance * m_ActiveDistance))
                {
                    //exiting Range
                    m_AnimPlayer.SetStopAtEnd(TRUE);
                }            
            }
        }
        break;
    case EXITING_RANGE:
        if (m_AnimPlayer.IsAtEnd())
        {
            m_State = OUT_OF_RANGE;
            PlayAnim(m_iInactiveIdle);
        }
        break;
    }
}

//=============================================================================

void reactive_surface::PlayAnim( s16 AnimStringIndex )
{
    if (AnimStringIndex == -1)
    {
        LOG_ERROR( "GAMEPLAY", "Bad Animation Index in Reactive Surface" );                        
        return;
    }

    if( m_hAnimGroup.GetPointer() )
    {
        s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( AnimStringIndex ) );
        if( Index != -1 )
        {
            m_AnimPlayer.SetSimpleAnim( Index );
        }
        else
        {
            LOG_ERROR( "GAMEPLAY", "No animation found with name (%s) in anim group (%s) for Reactive Surface",
                g_StringMgr.GetString( AnimStringIndex ), m_hAnimGroup.GetName() );                        
        }
    }
}

//=============================================================================

#ifdef X_EDITOR

s32 reactive_surface::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = anim_surface::OnValidateProperties( ErrorMsg );

    if( !m_hAnimGroup.GetPointer() )
    {
        nErrors++;
        ErrorMsg += "ERROR: Bad Anim Group\n";    
    }
    else
    {
        if (m_iEnterReaction != -1)
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iEnterReaction ) );
            if( Index == -1 )
            {
                nErrors++;
                ErrorMsg += "ERROR: Missing EnterReaction Anim\n";    
            }        
        }
        else
        {
            nErrors++;
            ErrorMsg += "ERROR: Bad EnterReaction Anim Index\n";    
        }

        if (m_iExitReaction != -1)
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iExitReaction ) );
            if( Index == -1 )
            {
                nErrors++;
                ErrorMsg += "ERROR: Missing ExitReaction Anim\n";    
            }        
        }
        else
        {
            nErrors++;
            ErrorMsg += "ERROR: Bad ExitReaction Anim Index\n";    
        }

        if (m_iInactiveIdle != -1)
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iInactiveIdle ) );
            if( Index == -1 )
            {
                nErrors++;
                ErrorMsg += "ERROR: Missing InactiveIdle Anim\n";    
            }        
        }
        else
        {
            nErrors++;
            ErrorMsg += "ERROR: Bad InactiveIdle Anim Index\n";    
        }

        if (m_iActiveIdle != -1)
        {
            s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iActiveIdle ) );
            if( Index == -1 )
            {
                nErrors++;
                ErrorMsg += "ERROR: Missing ActiveIdle Anim\n";    
            }        
        }
        else
        {
            nErrors++;
            ErrorMsg += "ERROR: Bad ActiveIdle Anim Index\n";    
        }
    }

    return nErrors;
}

#endif
