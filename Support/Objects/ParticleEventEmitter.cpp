//==============================================================================
// PARTICLE EVENT EMITTER
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "ParticleEventEmitter.hpp"
#include "Animation\AnimData.hpp"
#include "Objects\Event.hpp"
#include "gamelib\StatsMgr.hpp"
#include "ParticleEmiter.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct particle_event_emitter_desc : public object_desc
{
    particle_event_emitter_desc( void ) : object_desc( 
        object::TYPE_PARTICLE_EVENT_EMITTER, 
        "Particle Event Emitter", 
        "EFFECTS",
            object::ATTR_NEEDS_LOGIC_TIME,
            FLAGS_IS_DYNAMIC) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new particle_event_emitter;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_PARTICLE_EMITTER;
    }

#endif // X_EDITOR

} s_ParticleEventEmitter_Desc;

//=========================================================================

const object_desc&  particle_event_emitter::GetTypeDesc( void ) const
{
    return s_ParticleEventEmitter_Desc;
}

//=========================================================================

const object_desc&  particle_event_emitter::GetObjectType( void )
{
    return s_ParticleEventEmitter_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

particle_event_emitter::particle_event_emitter( void ) 
{
    m_FxName[0]     = 0;
    m_EventID       = -1;
    m_ParentGuid    = NULL;
    m_ParticleGuid  = NULL;
    m_LogicRunning  = TRUE;
    m_EventActive   = FALSE;
}

//=========================================================================

void particle_event_emitter::OnAdvanceLogic ( f32 DeltaTime )
{
    LOG_STAT(k_stats_ParticleSystem);
    (void)DeltaTime;
    
    if( m_EventActive )
    {
        // If this object wasn't updated last cycle then that means that we are done so stop the effect and destory
        // this object.
        if( m_LogicRunning == FALSE )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ParticleGuid );
            if( pObj )
            {
                particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
                Particle.DestroyParticle();
            }
            
            g_ObjMgr.DestroyObject( GetGuid() );
        }
        
        m_LogicRunning = FALSE;
    }
    else
    {
        // If the particle is done playing, kill this object.
        if( g_ObjMgr.GetObjectByGuid( m_ParticleGuid ) == NULL )
            g_ObjMgr.DestroyObject( GetGuid() );
    }
}

//=========================================================================

void particle_event_emitter::StartEmitter( const char*      pFx,
                                           const vector3&   Position,
                                           const vector3&   Rotation,
                                           u16              ZoneID,
                                           guid             ParentGuid,
                                           s32              EventID,
                                           xbool            EventActive )
{
    x_strncpy( m_FxName, pFx, 64 );
    m_EventID       = EventID;
    m_ParentGuid    = ParentGuid;
    m_EventActive   = EventActive;
    m_LogicRunning  = TRUE;

    m_ParticleGuid  = particle_emitter::CreatePresetParticleAndOrient( m_FxName, Rotation, Position, ZoneID );

    OnMove( Position );
}   

//=========================================================================

void particle_event_emitter::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
    
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParticleGuid );

    if( pObj )
        pObj->OnMove( NewPos );

    // If we are getting updated that mean this particle is still "active".
    m_LogicRunning = TRUE;
}

//=========================================================================

void particle_event_emitter::OnTransform( const matrix4& L2W      )
{
    object::OnTransform( L2W );

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParticleGuid );

    if( pObj )
        pObj->OnTransform( L2W );

    // If we are getting updated that mean this particle is still "active".
    m_LogicRunning = TRUE;
}

//=========================================================================

bbox particle_event_emitter::GetLocalBBox( void ) const
{
    bbox Box( vector3(0.0f, 0.0f, 0.0f), 0.0f );
    return Box;
}

//=========================================================================
