//=============================================================================
//
// GlassSurface.cpp
//
///=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================

#include "GlassSurface.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"


//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================
glass_surface_desc glass_surface::s_GlassSurface_Desc;

object* glass_surface_desc::Create( void )
{
    return new glass_surface;
}


/*
//=============================================================================
static struct glass_surface_desc : public object_desc
{
    glass_surface_desc( void ) : object_desc( 
        object::TYPE_GLASS_SURFACE, 
        "Glass Surface", 
            object::ATTR_COLLIDABLE | 
            object::ATTR_RENDERABLE |
            object::ATTR_SPACIAL_ENTRY |
            object::ATTR_DAMAGEABLE, 
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_BURN_VERTEX_LIGHTING  ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new glass_surface;
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

} s_GlassSurface_Desc;
*/

//=============================================================================

const object_desc&  glass_surface::GetTypeDesc     ( void ) const
{
    return glass_surface::s_GlassSurface_Desc;
}

//=============================================================================

const object_desc&  glass_surface::GetObjectType   ( void )
{
    return glass_surface::s_GlassSurface_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

glass_surface::glass_surface( void )
{
    m_Health    = 100.0f;
    m_DebrisSet = debris_mgr::DEBRIS_SET_GLASS;
    m_Destroyed = FALSE;

    m_hParticleEffect.SetName( PRELOAD_FILE("GlassShatter.fxo") );
}

//=============================================================================

glass_surface::~glass_surface( void )
{
}

//=============================================================================

bbox glass_surface::GetLocalBBox( void ) const
{
    return play_surface::GetLocalBBox();
}      

//=============================================================================

void glass_surface::OnColCheck( void )
{    
    play_surface::OnColCheck();   
}

//=============================================================================

void glass_surface::OnPain ( const pain& Pain )   // Tells object to recieve pain
{
    if( m_Destroyed )
        return;

    health_handle Handle(GetLogicalName());
    Pain.ComputeDamageAndForce( Handle, GetGuid(), GetBBox().GetCenter() );
    m_Health -= Pain.GetDamage();

    if( m_Health <= 0.0f )
    {
        m_Destroyed = TRUE;
/*
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
        
        if( Pain.Collision.IsValid() || Pain.Type == pain::TYPE_PROJECTILE_GRENADE || Pain.Type == pain::PAIN_ON_TRIGGER )
        {
            debris_mgr::GetDebrisMgr()->CreateGlassFromRigidGeom( (play_surface*)this, &Pain );

            particle_emitter::CreatePresetParticleAndOrient(  m_hParticleEffect.GetName(), 
                                                                Pain.Direction,                                                                
                                                                GetPosition(), GetZone1()  );
            // Switch to the broken glass LOD.
            if( GetRigidInst().GetLODCount() == 2 )
            {
                GetRigidInst().SwapLODs();
            }
        }
*/
    }
}

//=============================================================================

void glass_surface::OnEnumProp( prop_enum&    List )
{
    play_surface::OnEnumProp( List );

    List.PropEnumHeader( "Glass Surface", "Glass Surface Properties", 0 );
    List.PropEnumHeader( "Glass Surface\\Debris", "This is the debris that is created.", 0 );

    s32 ID = List.PushPath( "Glass Surface\\Debris\\" );
    m_DebrisInst.OnEnumProp( List );
    List.PopPath( ID );

    List.PropEnumHeader( "Glass Obj\\Particles", "This is the debris that is created.", 0 );

    List.PropEnumExternal( "Glass Obj\\Particles\\Particles Resource",
                      "Resource\0fxo\0",
                      "Particle Resource for this item",
                      PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Glass Obj\\Particles\\Scale", "The scale of the particle effect for this object.", 0 );

    List.PropEnumExternal(   "Glass Surface\\Audio Package", "Resource\0audiopkg\0","The audio package associated with this Glass object.", 0 );
    List.PropEnumFloat   (   "Glass Surface\\Hit Points","Number of hit points for this object", 0 );
}

//=============================================================================

xbool glass_surface::OnProperty( prop_query&   I    )
{
    if( play_surface::OnProperty( I ) )
        return TRUE;
    
    s32 ID = I.PushPath( "Glass Surface\\Debris\\" );
    if ( m_DebrisInst.OnProperty( I ) )
    {
        return TRUE;
    }
    I.PopPath( ID );

    // External
    if( I.IsVar( "Glass Surface\\Audio Package" ) )
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
    if( I.IsVar( "Glass Obj\\Particles\\Particles Resource" ) )
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

    if( I.IsVar("Glass Surface\\Hit Points" ) )
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

    return FALSE;
}




//=============================================================================
