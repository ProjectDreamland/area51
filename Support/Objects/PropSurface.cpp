//=============================================================================
//
// PropSurface.cpp
//
///=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================

#include "PropSurface.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"
#include "Objects\ParticleEmiter.hpp"

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

#ifdef X_EDITOR
extern xbool g_game_running;
#endif // X_EDITOR

//=============================================================================
static struct prop_surface_desc : public object_desc
{
    prop_surface_desc( void ) : object_desc( 
        object::TYPE_PROP_SURFACE, 
        "Prop Surface", 
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
            object::ATTR_NEEDS_LOGIC_TIME |
            object::ATTR_DAMAGEABLE, 

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_TARGETS_OBJS          |
            FLAGS_BURN_VERTEX_LIGHTING  ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new prop_surface;
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

} s_PropSurface_Desc;

//=============================================================================

const object_desc&  prop_surface::GetTypeDesc     ( void ) const
{
    return s_PropSurface_Desc;
}

//=============================================================================

const object_desc&  prop_surface::GetObjectType   ( void )
{
    return s_PropSurface_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

prop_surface::prop_surface( void ) :
    m_Health( 1000000.0f ),
    m_DebrisSet(debris_mgr::DEBRIS_SET_MECHANICAL ),
    m_PropFlags( PROP_FLAG_CAN_COLLIDE )
{
    m_hParticleEffect.SetName( PRELOAD_FILE("BloodSpray_001.fxo") );
}

//=============================================================================

prop_surface::~prop_surface( void )
{
}

//=============================================================================

bbox prop_surface::GetLocalBBox( void ) const
{
    return play_surface::GetLocalBBox();
}      

//=============================================================================

void prop_surface::OnPain ( const pain& Pain )   // Tells object to receive pain
{
    if( m_PropFlags & PROP_FLAG_DESTROYED )
        return;

    u32 flags;
    if( Pain.HasCollision() )
        flags = Pain.GetCollision().Flags;
    else
        flags = (u32)-1;

    health_handle HealthHandle(GetLogicalName());
    Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), Pain.GetPosition() );
    f32 Damage = Pain.GetDamage();
    
    // if they have collision data but not the glass flag, no damage
    if( GetOnlyGlassDestructible() && Pain.HasCollision() && (flags != MAT_TYPE_GLASS) )
        Damage = 0;

    m_Health -= Damage;

    if( m_Health <= 0.0f )
    {
        m_PropFlags |= PROP_FLAG_DESTROYED;
        
        //
        // Update PolyCache using the current bbox
        //
        {
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }

        if (m_ActivateOnDestruction != 0)
        {
            //activate this guid
            object* pObj = g_ObjMgr.GetObjectByGuid(m_ActivateOnDestruction);
            if (pObj)
            {
                pObj->OnActivate(TRUE);
            }
        }

        f32 debrisObjectCount = GetBBox().GetRadius();
        debrisObjectCount *= debrisObjectCount;
        debrisObjectCount /= 1500.0f;
        
        if( debrisObjectCount < 5.0f )
        {
            debrisObjectCount = 5.0f;
        }
        
        if( debrisObjectCount > 30.0f )
        {
            debrisObjectCount = 30.0f;
        }
        
        particle_emitter::CreateOnPainEffect( Pain, GetBBox().GetRadius(), particle_emitter::DIRT_PUFF, XCOLOR_WHITE );
       
        if (GetOnlyGlassDestructible())
        {
            debris_mgr::GetDebrisMgr()->CreateGlassFromRigidGeom( (play_surface*)this, &Pain );

            rigid_inst& RInst = GetRigidInst();
            const geom* pGeom = RInst.GetRigidGeom();
            if( pGeom && (pGeom->m_nVirtualMeshes == 2) )
            {
                u32 VMeshMask = RInst.GetVMeshMask().VMeshMask;
                VMeshMask ^= 0x3;   // swap the first two meshes
                RInst.SetVMeshMask( VMeshMask );
            }
        }
        else
        {                
            debris_mgr::GetDebrisMgr()->CreateDebrisFromObject( m_DebrisInst, GetGuid(), (s32)debrisObjectCount, Damage * 20.0f );
            g_ObjMgr.DestroyObject( GetGuid() );
        }


    }
}

//=============================================================================

void prop_surface::OnAdvanceLogic( f32 DeltaTime )
{
    m_Inst.OnAdvanceLogic( DeltaTime );
}

//=============================================================================

void prop_surface::OnEnumProp( prop_enum&    List )
{
    play_surface::OnEnumProp( List );

    List.PropEnumHeader( "Prop Surface", "Prop Surface Properties", 0 );
    List.PropEnumHeader( "Prop Surface\\Debris", "This is the debris that is created.", 0 );

    s32 ID = List.PushPath( "Prop Surface\\Debris\\" );
    m_DebrisInst.OnEnumProp( List );
    List.PopPath( ID );
    
//    List.PropEnumExternal( "Prop Surface\\Debris\\Geometry", "Resource\0rigidgeom\0", "Resource File", PROP_TYPE_MUST_ENUM );

    List.PropEnumExternal(   "Prop Surface\\Audio Package", "Resource\0audiopkg\0","The audio package associated with this prop object.", 0 );

    List.PropEnumHeader(     "Prop Surface\\Particles", "This is the debris that is created.", 0 );

    List.PropEnumExternal(   "Prop Surface\\Particles\\Particles Resource",
                                "Resource\0fxo\0",
                                "Particle Resource for this item",
                                PROP_TYPE_MUST_ENUM );

    List.PropEnumFloat(      "Prop Surface\\Hit Points","Number of hit points for this object", PROP_TYPE_EXPOSE );
    List.PropEnumEnum(       "Prop Surface\\Debris Set", "Flesh\0Electronic\0Mechanical\0Glass\0","What type of debris the object should use when destroyed", PROP_TYPE_EXPOSE );
    List.PropEnumBool(       "Prop Surface\\Only Glass Destructible", "Mark this piece so that only the glass portion is destructible", 0 );
    List.PropEnumBool(       "Prop Surface\\Can Collide",    "Does this surface have collision?", PROP_TYPE_EXPOSE);
    List.PropEnumGuid(       "Prop Surface\\Activate OnDestroy", "Activate this guid on destruction of this object.", 0 );
}

//=============================================================================

xbool prop_surface::OnProperty( prop_query&   I    )
{
    if( play_surface::OnProperty( I ) )
        return TRUE;
    
    s32 ID = I.PushPath( "Prop Surface\\Debris\\" );
    if ( m_DebrisInst.OnProperty( I ) )
    {
        return TRUE;
    }
    I.PopPath( ID );

    // External
    if( I.IsVar( "Prop Surface\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );
    }

    // External Particle.
    if( I.IsVar( "Prop Surface\\Particles\\Particles Resource" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hParticleEffect.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hParticleEffect.SetName( pString );                

                // Load the particle effect.
                if( m_hParticleEffect.IsLoaded() == FALSE )
                    m_hParticleEffect.GetPointer();
            }
        }
        return( TRUE );
    }

    if( I.IsVar("Prop Surface\\Hit Points" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_Health );
        }
        else
        {
            m_Health = I.GetVarFloat();
        }
        return TRUE;
    }

    if( I.IsVar("Prop Surface\\Only Glass Destructible" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( (m_PropFlags & PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE) == PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE );
        }
        else
        {
            if (I.GetVarBool())
                m_PropFlags |= PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE;
            else
                m_PropFlags &= ~PROP_FLAG_ONLY_GLASS_DESTRUCTIBLE;
        }
        return TRUE;
    }

    if( I.IsVar("Prop Surface\\Can Collide") )
    {
        if ( I.IsRead() )
        {
            I.SetVarBool( (m_PropFlags & PROP_FLAG_CAN_COLLIDE) == PROP_FLAG_CAN_COLLIDE );
        }
        else
        {
            if (I.GetVarBool())
                m_PropFlags |= PROP_FLAG_CAN_COLLIDE;
            else
                m_PropFlags &= ~PROP_FLAG_CAN_COLLIDE;
        }
        return TRUE;
    }

    if( I.IsVar( "Prop Surface\\Debris Set") )
    {

        if ( I.IsRead() )
        {
            switch( m_DebrisSet ) 
            {
            case debris_mgr::DEBRIS_SET_FLESH :
                {
                    I.SetVarEnum("Flesh");
                }
            	break;
            case debris_mgr::DEBRIS_SET_MECHANICAL :
                {
                    I.SetVarEnum("Mechanical");
                }
                break;
            case debris_mgr::DEBRIS_SET_GLASS :
                {
                    I.SetVarEnum("Glass");
                }
                break;
            case debris_mgr::DEBRIS_SET_ELECTRONICS :
                {
                    I.SetVarEnum("Electronic");
                }
                break;
            default:
//                ASSERT(FALSE);
                break;
            
            }
        }
        else
        {
            if(         !x_strcmp( "Flesh", I.GetVarEnum() ) )
            {
                m_DebrisSet = debris_mgr::DEBRIS_SET_FLESH;
            }
            else if(    !x_strcmp( "Mechanical", I.GetVarEnum() ) )
            {
                m_DebrisSet = debris_mgr::DEBRIS_SET_MECHANICAL;
                
            }
            else if(    !x_strcmp( "Glass", I.GetVarEnum() ) )
            {
                m_DebrisSet = debris_mgr::DEBRIS_SET_GLASS;
                
            }
            else if(    !x_strcmp( "Electronic", I.GetVarEnum() ) )
            {
                m_DebrisSet = debris_mgr::DEBRIS_SET_ELECTRONICS;
               
            }
            else
            {
//                ASSERT(FALSE);
            }

        }
        return (TRUE);
    }

    if (I.VarGUID("Prop Surface\\Activate OnDestroy",m_ActivateOnDestruction))
        return TRUE;

    return FALSE;
}

//=============================================================================

void prop_surface::OnRender( void )
{
    CONTEXT( "prop_surface::OnRender" );

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
    if( pRigidGeom && !(m_PropFlags & PROP_FLAG_BBOX_ONLY) )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        if ( pRigidGeom->m_nBones > 1 )
        {
            m_PropFlags |= PROP_FLAG_BBOX_ONLY;
            x_throw( xfs( "Prop surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
        }
        else
        {
            m_Inst.Render( &GetL2W(), Flags | GetRenderMode() );
        }
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=============================================================================

void prop_surface::OnPolyCacheGather( void )
{
    if (m_PropFlags & PROP_FLAG_CAN_COLLIDE)
    {
        play_surface::OnPolyCacheGather();
    }
}

//=============================================================================

void prop_surface::OnColCheck( void )
{
#ifdef X_EDITOR
    //for editor selection
    if (!g_game_running)
    {
        play_surface::OnColCheck( );
        return;
    }
#endif // X_EDITOR

    if (m_PropFlags & PROP_FLAG_CAN_COLLIDE)
    {
        play_surface::OnColCheck( );
    }
}

//=============================================================================

void prop_surface::OnMove( const vector3& NewPos )
{
    bbox BBox = GetBBox() ;
    
    object::OnMove( NewPos );

    BBox += GetBBox() ;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );
}

//=============================================================================


void prop_surface::OnTransform( const matrix4& L2W )
{
    bbox BBox = GetBBox() ;

    object::OnTransform( L2W );

    BBox += GetBBox() ;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );
}

//=============================================================================

#if defined(X_EDITOR)
s32 prop_surface::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = play_surface::OnValidateProperties( ErrorMsg );
    
    const rigid_geom* pGeom = m_DebrisInst.GetRigidGeom();
    if( pGeom && (pGeom->m_nBones>1) )
    {
        nErrors++;
        ErrorMsg += "ERROR: Prop debris can't use geometry with more than one bone\n";
    }

    return nErrors;
}
#endif

//=============================================================================
