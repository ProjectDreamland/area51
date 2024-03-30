#include "debris_frag_explosion.hpp"
#include "NetworkMgr\Networkmgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Objects\Player.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Decals\DecalMgr.hpp"


rhandle<decal_package> debris_frag_explosion::s_hDecalPkg;


//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_frag_explosion_desc : public object_desc
{
    debris_frag_explosion_desc( void ) : object_desc( 
        object::TYPE_DEBRIS_FRAG_EXPLOSION, 
        "FragExplosionsDebris", 
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_TRANSPARENT        |
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC  )
    {}

    virtual object* Create          ( void )
    {
        return new debris_frag_explosion;
    }

} s_debris_frag_explosion_desc;

//=============================================================================================

const object_desc&  debris_frag_explosion::GetTypeDesc ( void ) const
{
    return s_debris_frag_explosion_desc;
}

//=============================================================================================

const object_desc&  debris_frag_explosion::GetObjectType ( void )
{
    return s_debris_frag_explosion_desc;
}


debris_frag_explosion::debris_frag_explosion()
{
    s_hDecalPkg.SetName( PRELOAD_FILE("CharMarks.decalpkg") );
}


debris_frag_explosion::~debris_frag_explosion()
{
}


void debris_frag_explosion::Create     ( const char*      pMeshName,
                                         const vector3&   Pos,
                                         u32              Zones,
                                         const vector3&   Dir,
                                         s32              nFragments )
{
    m_RigidInst.SetUpRigidGeom( pMeshName );
    rigid_geom* pGeom = m_RigidInst.GetRigidGeom();
    if (NULL == pGeom)
        return;

    SetZones( Zones );
/*
#ifdef TARGET_XBOX
#   define USE_LOTS_OF_MEMORY
#else
#   define nUSE_LOTS_OF_MEMORY
#endif
*/

// SH: Just using the define for now.  It can be removed later once 
// we've verified that everything is ok.  For now, it's simpler
// for someone else to re-disable the fx using this.
#define USE_LOTS_OF_MEMORY

#if defined( USE_LOTS_OF_MEMORY )
    InitializeFragmentsForPlayerDirectedExplosion(  Pos,                                         //Pos,
                                                    Dir,                                         //Dir,
                                                    nFragments,                                  //nFragments,
                                                    1000,                                        //MinSpeed,
                                                    5000,                                        //MaxSpeed,
                                                    0.7f,                                        //FaceShotPercentage,
                                                    2000,                                        //SlowTrailerThreshold,
                                                    0.4f,                                        //ABPercentage,
                                                    TRUE,                                        //bTypeASuspendOnRest,
                                                    3,                                           //MaxTypeA,
                                                    PRELOAD_FILE("DEB_fire_world_001.fxo"),      //TypeAFXName,
                                                    TRUE,                                        //bTypeBSuspendOnRest,
                                                    6,                                           //MaxTypeB,
                                                    PRELOAD_FILE("DEB_smoketrail.fxo"),          //TypeBFXName,
                                                    3000,                                        //FastTrailerThreshold,
                                                    0.7f,                                        //FastPercentage,
                                                    TRUE,                                        //bFastSuspendOnRest,
                                                    6,                                           //MaxFastTrailer,
                                                    PRELOAD_FILE("DEB_smoketrail_001.fxo"));     //FastFXName );
#else
    (void)nFragments;
#endif


    //==-------------------------------
    //  CREATE PARTICLE FX
    //==-------------------------------
    particle_emitter::CreatePresetParticleAndOrient( 
        particle_emitter::GRENADE_EXPLOSION, 
        vector3(0.0f,0.0f,0.0f), 
        Pos );

    //==-------------------------------
    //  KICK OFF AUDIO
    //==-------------------------------
    voice_id VoiceID = g_AudioMgr.Play( "Explosion_Powder_All", Pos, GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, Pos, GetZone1(), GetGuid() );

    // Kick in the sweetened sound
    g_AudioMgr.Play( "Explosion_LowFreq", Pos, GetZone1(), TRUE );        
    
    //==-------------------------------
    // CREATE DECAL
    //==-------------------------------
    vector3 nDir = Dir;
    nDir.NormalizeAndScale(6);

    vector3 StartPos = Pos + nDir;
    vector3 EndPos   = Pos - nDir;

    decal_package* pPackage = s_hDecalPkg.GetPointer();
    if ( pPackage )
    {
        decal_definition* pDef = pPackage->GetDecalDef( "CharMarks", 0 );
        if( pDef )
        {
            f32 Size        = x_frand( 150.0f, 200.0f );
            g_DecalMgr.CreateDecalFromRayCast( *pDef, StartPos, EndPos, vector2(Size,Size), pDef->RandomRoll() );                            
        }
    }
}