//==============================================================================
// BASE PROJECTILES
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "BaseProjectile.hpp"
#include "Entropy\e_Draw.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Objects\ClothObject.hpp"
#include "Objects\Flag.hpp"
#include "Objects\Actor\Actor.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "Decals\DecalMgr.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\ProxyPlaySurface.hpp"
#include "Objects\ForceField.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "player.hpp"

xbool IsGameMultiplayer( void )
{
    #if defined(X_EDITOR)
        return FALSE;
    #else
        return GameMgr.IsGameMultiplayer();
    #endif
}

//==============================================================================
// GLOBALS
//==============================================================================

#define MAX_COLLISON_PER_BULLET  MAX_COLLISION_MGR_COLLISIONS

//==============================================================================

base_projectile::base_projectile( void ) :
    m_Speed                 ( 0.f               ),
    m_Velocity              ( 0.f , 0.f , 0.f   ),
    m_CurrentPosition       ( 0.f , 0.f , 0.f   ),
    m_InitialPosition       ( 0.f , 0.f , 0.f   ),
    m_OwnerGuid             ( NULL_GUID         ),
    m_TracerFadeTime        ( 0.0f              ),
    m_iFirePoint            ( 0                 ),
    m_bThroughGlass         ( FALSE             ),
    m_BulletNearFlyBySfxID  ( -1                ),
    m_BulletMedFlyBySfxID   ( -1                ),
    m_BulletFarFlyBySfxID   ( -1                )
{
    m_BulletSoundID = 0;
    m_TracerColor = xcolor( 90, 80, 40, 255 );
    m_bIsLargeProjectile = FALSE;
    m_bSplitScreenInitialPosReady = FALSE;
}

//==============================================================================

base_projectile::~base_projectile()
{
    m_BulletSoundID = 0;
}


//=============================================================================

bbox base_projectile::GetLocalBBox( void ) const 
{     
    return( bbox( vector3( 5.0f, 5.0f, 5.0f),
                  vector3(-5.0f,-5.0f,-5.0f) ) );
}

//==============================================================================

xbool base_projectile::OnProperty( prop_query& PropQuery )
{
    if ( object::OnProperty( PropQuery ) )
    {
        return TRUE;
    }

    //early out
    if( !PropQuery.IsBasePath("Bullet Properties") )
    {
        return FALSE;
    }
/*
    if ( PropQuery.VarFloat( "Bullet Properties\\Speed", m_Speed ) )
    {
        return TRUE;
    }
  */  
    if ( PropQuery.VarFloat( "Bullet Properties\\Tracer Fade Time", m_TracerFadeTime, 0.0f ) )
        return TRUE;
    
    if( PropQuery.IsVar( "Bullet Properties\\Bullet AudioPkg" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

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

    if( SMP_UTIL_IsAudioVar( PropQuery, "Bullet Properties\\Bullet Near Fly By Sound", m_hAudioPackage, m_BulletNearFlyBySfxID ) )
    {
        return( TRUE );
    }

    if( SMP_UTIL_IsAudioVar( PropQuery, "Bullet Properties\\Bullet Medium Fly By Sound", m_hAudioPackage, m_BulletMedFlyBySfxID ) )
    {
        return( TRUE );
    }

    if( SMP_UTIL_IsAudioVar( PropQuery, "Bullet Properties\\Bullet Far Fly By Sound", m_hAudioPackage, m_BulletFarFlyBySfxID ) )
    {
         return( TRUE );
    }

    if ( PropQuery.IsVar( "Bullet Properties\\Decal Package" ) )
    {
        if ( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            m_hDecalPackage.SetName( PropQuery.GetVarExternal() );
            m_hDecalPackage.GetPointer();
        }
    }
    
    if( PropQuery.VarColor( "Bullet Properties\\Tracer Color", m_TracerColor ) )
        return TRUE;
/*
    if ( PropQuery.IsBasePath("Bullet Properties\\Damage Loss"))
    {
        s32 iHeader = PropQuery.PushPath( "Bullet Properties\\Damage Loss\\" );      
        
        if ( PropQuery.VarFloat( "Distance", m_fDistIncrementForLoss, 0.0f ) )
            return TRUE;

        if ( PropQuery.VarFloat( "Amount", m_fDamageLostPerIncrement, 0.0f ) )
            return TRUE;

        if ( PropQuery.VarFloat( "MinDamage", m_fMinDamage, 0.0f ) )
            return TRUE;

        PropQuery.PopPath( iHeader );
    }
*/
    return FALSE;
}

//===========================================================================

void base_projectile::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
    List.PropEnumHeader  ( "Bullet Properties",                      "Properties of the bullet", PROP_TYPE_HEADER );
    //List.AddFloat   ( "Bullet Properties\\Speed",               "Scalar speed value in cm. / sec." );
    //List.AddFloat   ( "Bullet Properties\\Damage Amount",       "Number of damage hit points does this projectile cost" );
    //List.AddFloat   ( "Bullet Properties\\Force Amount",        "Relates to the amount of controller feedback and camera shake on impact." );
    //List.AddBool    ( "Bullet Properties\\Through glass",       "Does this bullet pass through glass?" );
    //List.AddBool    ( "Bullet Properties\\Through actors",      "Does this bullet pass through actors?" );
    List.PropEnumFloat   ( "Bullet Properties\\Tracer Fade Time",    "How long do the tracers last?", 0 );
    List.PropEnumColor   ( "Bullet Properties\\Tracer Color",        "The Color of the tracer", 0 );
    List.PropEnumExternal( "Bullet Properties\\Bullet AudioPkg",             "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Bullet Properties\\Bullet Near Fly By Sound",    "Sound\0soundexternal\0","What sound to play when the bullet flies by near you.", 0 );
    List.PropEnumExternal( "Bullet Properties\\Bullet Medium Fly By Sound",  "Sound\0soundexternal\0","What sound to play when the bullet flies by medium distance from you.", 0 );
    List.PropEnumExternal( "Bullet Properties\\Bullet Far Fly By Sound",     "Sound\0soundexternal\0","What sound to play when the bullet flies by far away form you.", 0 );
    List.PropEnumExternal( "Bullet Properties\\Decal Package",       "Resource\0decalpkg\0",  "What type of decals does this projectile use?", 0 );
    //List.AddHeader  ( "Bullet Properties\\Damage Loss",         "Does this projectile lose damage over time.", PROP_TYPE_HEADER );
    //List.AddFloat   ( "Bullet Properties\\Damage Loss\\Distance", "Lose Y damage every X distance. (this is X)" );
    //List.AddFloat   ( "Bullet Properties\\Damage Loss\\Amount",   "Lose Y damage every X distance. (this is Y)");
    //List.AddFloat   ( "Bullet Properties\\Damage Loss\\MinDamage","Minimum Damage this projectile will do");
}

//==============================================================================

s32 g_BulletVersion = 1;

xcolor g_color(120,230,230);
void base_projectile::OnRender( void )
{
    CONTEXT( "base_projectile::OnRender" );

#ifdef X_EDITOR
    
    if( g_BulletVersion == 0 )
    {
        draw_Sphere( GetBBox().GetCenter(),3, g_color );
    }
    else if ( g_BulletVersion == 1)
    {
//        vector3 tempVec3;
//        tempVec3 = m_LastPosition - GetBBox().GetCenter();

//        tempVec3.NormalizeAndScale(20.0f);

//        m_LastPosition = GetBBox().GetCenter();

//            draw_Line( GetBBox().GetCenter(),tempVec3,XCOLOR_GREEN );
    }
    else
    {
        draw_BBox( GetBBox() , XCOLOR_GREEN );
    }
#endif // X_EDITOR
}

//==============================================================================
//#define TIME_PROCESS_COLLISION

#ifdef X_RETAIL
    #undef TIME_PROCESS_COLLISION
#endif

#ifdef TIME_PROCESS_COLLISION
s32    NUM_BULLETS = 0;
f32    TOTAL_BULLET_TIME=0;
f32    TOTAL_COLL_TIME=0;
f32    TOTAL_RESPONSE_TIME=0;
f32    MAX_BULLET_TIME=0;
#endif

xbool base_projectile::OnProcessCollision( const f32& DeltaTime )
{
    //
    // Compute current and desired position
    //
    vector3 CurrentPos = m_CurrentPosition;
    vector3 NewPos     = m_CurrentPosition + m_Velocity * DeltaTime;
    
    object *pObj = g_ObjMgr.GetObjectByGuid(m_OwnerGuid);
    xbool bIsAvatar = FALSE;
    vector3 avatarCurrentPos = vector3(0.0f, 0.0f, 0.0f);
    vector3 avatarNewPos     = avatarCurrentPos;

    // SB: TO DO: Do this for ghosts too?
    // Did this bullet come from the player?
    if( pObj && pObj->IsKindOf(player::GetRTTI()) )
    {
        player *pPlayer = (player*)pObj;
        new_weapon* pWeapon = pPlayer->GetCurrentWeaponPtr();
        
        if (pWeapon)
        {
            // For split screen, update the start position of the tracer to be from the gun:
            // We shouldn't care about the first person tracer because you never see it anyway.
            if( pWeapon->IsUsingSplitScreen() )
            {         
                // set NPC render state so that the weapon will give us the proper bone position
                pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );

                // SH: I didn't want to risk breaking any of the existing mechanisms, so all we
                //     do here is update m_InitialPosition the first time through, and set a flag.
                //     On subsequent runs, we use the updated m_InitialPosition for the tracers.
                //     Without doing this, the tracers are glued to the fire point on the gun, and
                //     in split screen, large-kick weapons wave the start point of the tracers around.
                //  
                if (!m_bSplitScreenInitialPosReady)
                {
                    // get avatar's current firing bone position if available.
                    if( pWeapon->GetFiringBonePosition( avatarNewPos, m_iFirePoint ) )
                    {
                        bIsAvatar = TRUE;
                        avatarCurrentPos = NewPos;
                        m_InitialPosition = avatarNewPos;
                        m_bSplitScreenInitialPosReady = TRUE;
                    }
                }
                else
                {
                    avatarCurrentPos = NewPos;
                    avatarNewPos = m_InitialPosition;
                }

                // put the render state back to player in split screen
                pWeapon->SetRenderState( new_weapon::RENDER_STATE_PLAYER );
            }
        }
    }

    //
    // Fire up ray collision 
    //
    #ifdef TIME_PROCESS_COLLISION
    xtimer Timer;
    Timer.Reset();
    Timer.Start();
    #endif
    {
        //extern xbool COLL_DISPLAY_OBJECTS;
        //COLL_DISPLAY_OBJECTS = TRUE;
        g_CollisionMgr.RaySetup( GetGuid(), CurrentPos, NewPos );
        g_CollisionMgr.SetMaxCollisions( MAX_COLLISION_MGR_COLLISIONS );
        g_CollisionMgr.AddToIgnoreList( m_OwnerGuid );
        if( m_bHitLiving )
        {
            if( m_bIsLargeProjectile )
            {            
                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES );
            }
            else
            {
                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES );
            }
        }
        else
        {
            if( m_bIsLargeProjectile )
            {            
                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, ATTR_LIVING  );
            }
            else
            {
                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES, ATTR_LIVING  );
            }
        }
        //COLL_DISPLAY_OBJECTS = FALSE;
    }

    //
    // If no collisions then just move projectile to new position
    //
    if( g_CollisionMgr.m_nCollisions == 0 )
    {
        // Update the tracer
        if( bIsAvatar )
        {
            g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, avatarCurrentPos, avatarNewPos, m_TracerColor );
        }
        else
        {        
            // not an avatar, do tracer as normal
            g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, CurrentPos, NewPos, m_TracerColor );
        }

        m_CurrentPosition = NewPos;

        // Exit early
        return TRUE;

    }

    //
    // We have collisions !!!
    //

    //
    // Backup collisions
    //
    s32 nCollisions = g_CollisionMgr.m_nCollisions;
    ASSERT( nCollisions <= MAX_COLLISON_PER_BULLET );
    if( nCollisions > MAX_COLLISON_PER_BULLET ) nCollisions = MAX_COLLISON_PER_BULLET;

    collision_mgr::collision CollBackup[MAX_COLLISON_PER_BULLET];
    x_memcpy( CollBackup, g_CollisionMgr.m_Collisions, sizeof( collision_mgr::collision )*nCollisions );

    //
    // Setup tracer start and end points
    //
    vector3 TracerStart = CurrentPos;
    vector3 TracerEnd   = NewPos;

    #ifdef TIME_PROCESS_COLLISION
    Timer.Stop();
    f32 COLL_TIME = Timer.ReadMs();
    Timer.Reset();
    Timer.Start();
    #endif

    //
    // Loop through collisions and process
    //
    for( s32 i=0; i<nCollisions; i++ )
    {
        collision_mgr::collision& Coll = CollBackup[i];

        // Skip over collisions with unidentifiable objects
        if( Coll.ObjectHitGuid==0 )
        {
            //x_DebugMsg("NO GUID\n");
            continue;
        }

        // Let's talk to the object
        object* pObj = g_ObjMgr.GetObjectByGuid( Coll.ObjectHitGuid );
        if( !pObj )
        {
            //x_DebugMsg("NO OBJECT*\n");
            continue;
        }

        //
        // Is this a piece of cloth?
        //
        if( pObj->IsKindOf( cloth_object::GetRTTI() ) )
        {
            //x_DebugMsg("BULLET HIT CLOTH!!! %d of %d\n",i,nCollisions);

            // Notify the cloth of the bullet impact
            ((cloth_object*)pObj)->OnProjectileImpact( *this, m_Velocity, Coll.PrimitiveKey, Coll.Point );

            // Don't stop processing other collisions.  We're going to
            // go right through the cloth.
            continue;
        }

        //
        // Is this a flag?
        //
        if( pObj->IsKindOf( flag::GetRTTI() ) )
        {
            // Notify the flag of the bullet impact
            ((flag*)pObj)->OnProjectileImpact( *this, m_Velocity, Coll.PrimitiveKey, Coll.Point );

            // Don't stop processing other collisions.  We're going to
            // go right through the flag
            continue;
        }

        //
        // Is this object permeable?
        //
        if( pObj->GetAttrBits() & object::ATTR_COLLISION_PERMEABLE )
        {
            //x_DebugMsg("BULLET HIT PERMEABLE!!! %d of %d\n",i,nCollisions);

            pObj->OnColNotify( *this );

            // Don't stop processing other collisions.
            continue;
        }

        //
        // Is this a piece of glass
        //
        if( m_bThroughGlass && (Coll.Flags == object::MAT_TYPE_GLASS) )
        {
            //x_DebugMsg("BULLET HIT GLASS!!! %d of %d\n",i,nCollisions);

            particle_emitter::CreateProjectileCollisionEffect( Coll, m_OwnerGuid );
       
            pain Pain;
            Pain.Setup( m_PainHandle, m_OwnerGuid, Coll.Point );
            Pain.SetDirection( m_Velocity );
            Pain.SetDirectHitGuid( pObj->GetGuid() );
            Pain.SetCollisionInfo( Coll );
            Pain.SetCustomScalar( CalcDamageDegradation(Coll.Point) );
            Pain.ApplyToObject( pObj );

            // Don't stop processing other collisions.  We're going to
            // go right through the glass.
            continue;
        }

        //
        // Can we shoot through the actor?
        //
        if( m_bThroughActors && pObj->IsKindOf( actor::GetRTTI() ) )
        {
            //x_DebugMsg("SHOT THROUGH ACTOR!!! %d of %d  %08X%08X\n", i, nCollisions, (u32)(pObj->GetGuid()>>32), (u32)(pObj->GetGuid()>>0));

            particle_emitter::CreateProjectileCollisionEffect( Coll, m_OwnerGuid );
       
            pain Pain;
            Pain.Setup( m_PainHandle, m_OwnerGuid, Coll.Point );
            Pain.SetDirection( m_Velocity );
            Pain.SetDirectHitGuid( pObj->GetGuid() );
            Pain.SetCustomScalar( CalcDamageDegradation(Coll.Point) );
            Pain.SetCollisionInfo( Coll );
            Pain.ApplyToObject( pObj );

            // Don't stop processing other collisions.  We're going to
            // go right through the object.
            continue;
        }

        //
        // Is this a forcefield?
        //
        if( pObj->IsKindOf( force_field::GetRTTI() ) )
        {
            // Notify the forcefield of the bullet impact
            ((force_field*)pObj)->OnProjectileImpact( Coll.Point );
        }

        //
        // No special objects have skipped over us so we need to
        // stop the bullet on this collision
        //
        {

            m_CurrentPosition   = Coll.Point;
            TracerEnd           = Coll.Point;
            
            pain Pain;
            Pain.Setup( m_PainHandle, m_OwnerGuid, Coll.Point );
            Pain.SetDirection( m_Velocity );
            Pain.SetCollisionInfo( Coll );
            Pain.SetCustomScalar( CalcDamageDegradation(Coll.Point) );
            Pain.SetDirectHitGuid( pObj->GetGuid() );
            Pain.ApplyToWorld();

            //
            // If it is a player or creature make the tracer go a little further 
            //
            if( pObj->IsKindOf( actor::GetRTTI() ) )
            {
                vector3 Dir = Coll.Point - CurrentPos;
                Dir.Normalize();
                TracerEnd += 50.0f * Dir;
            }

            //
            // Process hit event...
            //
            //x_DebugMsg("BULLET HIT SOLID!!! %d of %d\n",i,nCollisions);
            if( bIsAvatar )
            {
                g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, avatarCurrentPos, avatarNewPos, m_TracerColor );
            }
            else
            {
                g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, TracerStart, TracerEnd, m_TracerColor );
            }

            #ifdef TIME_PROCESS_COLLISION
            {
                Timer.Stop();
                f32 RESPONSE_TIME = Timer.ReadMs();

                f32 TOTAL_TIME = COLL_TIME + RESPONSE_TIME;
                //if( TOTAL_TIME < 5.0f )
                {
                    NUM_BULLETS++;
                    TOTAL_BULLET_TIME += TOTAL_TIME;
                    TOTAL_COLL_TIME += COLL_TIME;
                    TOTAL_RESPONSE_TIME += RESPONSE_TIME;
                    if( (NUM_BULLETS==50) || (TOTAL_TIME>MAX_BULLET_TIME) )
                    {
                        if( MAX_BULLET_TIME < TOTAL_TIME )  MAX_BULLET_TIME = TOTAL_TIME;

                        x_DebugMsg("%8d  (%8.4f,%8.4f) (%8.4f,%8.4f) (%8.4f,%8.4f) (%8.4f)\n",
                            NUM_BULLETS,
                            TOTAL_TIME,
                            TOTAL_BULLET_TIME / (f32)NUM_BULLETS,
                            COLL_TIME,
                            TOTAL_COLL_TIME / (f32)NUM_BULLETS,
                            RESPONSE_TIME,
                            TOTAL_RESPONSE_TIME / (f32)NUM_BULLETS,
                            MAX_BULLET_TIME);

                        if( NUM_BULLETS==50 )
                        {
                            x_DebugMsg("----------------------------------\n");
                            MAX_BULLET_TIME = 0;
                            NUM_BULLETS = 0;
                            TOTAL_BULLET_TIME = 0;
                            TOTAL_COLL_TIME = 0;
                            TOTAL_RESPONSE_TIME = 0;
                        }
                    }
                }
            }
            #endif

            OnBulletHit( Coll, m_CurrentPosition );

            return FALSE;
        }

    }

    // Move the projectile
    //m_CurrentPosition = NewPos;

    //x_DebugMsg("NO SOLID COLLISIONS!!!\n");
    m_CurrentPosition = NewPos;

    // Add final bullet tracer
    if( bIsAvatar )
    {
        g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, avatarCurrentPos, avatarNewPos, m_TracerColor );
    }
    else
    {
        g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, m_TracerFadeTime, TracerStart, TracerEnd, m_TracerColor );
    }

    #ifdef TIME_PROCESS_COLLISION
    {
        Timer.Stop();
        f32 RESPONSE_TIME = Timer.ReadMs();

        f32 TOTAL_TIME = COLL_TIME + RESPONSE_TIME;
        if( TOTAL_TIME < 5.0f )
        {
            NUM_BULLETS++;
            TOTAL_BULLET_TIME += TOTAL_TIME;
            TOTAL_COLL_TIME += COLL_TIME;
            TOTAL_RESPONSE_TIME += RESPONSE_TIME;
            if( (NUM_BULLETS==50) || (TOTAL_TIME>MAX_BULLET_TIME) )
            {
                if( MAX_BULLET_TIME < TOTAL_TIME )  MAX_BULLET_TIME = TOTAL_TIME;

                x_DebugMsg("%8d  (%8.4f,%8.4f) (%8.4f,%8.4f) (%8.4f,%8.4f) (%8.4f)\n",
                    NUM_BULLETS,
                    TOTAL_TIME,
                    TOTAL_BULLET_TIME / (f32)NUM_BULLETS,
                    COLL_TIME,
                    TOTAL_COLL_TIME / (f32)NUM_BULLETS,
                    RESPONSE_TIME,
                    TOTAL_RESPONSE_TIME / (f32)NUM_BULLETS,
                    MAX_BULLET_TIME);

                if( NUM_BULLETS==50 )
                {
                    x_DebugMsg("----------------------------------\n");
                    MAX_BULLET_TIME = 0;
                    NUM_BULLETS = 0;
                    TOTAL_BULLET_TIME = 0;
                    TOTAL_COLL_TIME = 0;
                    TOTAL_RESPONSE_TIME = 0;
                }
            }
        }
    }
    #endif

    return TRUE;

    //OnProcessCollisionPermable( DeltaTime );
    //return OnProcessCollisionRigid( DeltaTime );
}

//==============================================================================

f32 base_projectile::CalcDamageDegradation( vector3& DamagePos )
{
    if( m_PainDropDist == 0.0f )
        return 1.0f;

    f32 DistTraveled = (m_InitialPosition - DamagePos).Length();
    f32 T = DistTraveled / m_PainDropDist;
    T = x_clamp( T, 0, 1 );
    T = 1 + T*(m_PainDropScale - 1.0f);

    return T;
}

//==============================================================================
// BW 4/29/03 - The PC compiler was generating an incorrect stack frame for argument passing to
// DestroyObject. Nothing, other than inline, would stop it generating the bad code. Please
// see Biscuit before removing this inline.
inline void base_projectile::OnBulletHit ( collision_mgr::collision& rColl, const vector3& HitPos  )
{
    ( void )rColl;
    ( void )HitPos;
    
    // Start the flyby.
    StartFlyby();

    particle_emitter::CreateProjectileCollisionEffect( rColl, m_OwnerGuid );
    CreateDecal( rColl );

    // SB 6/10/03 - Adding to Brian's fun - this fixes the damn crash on my PC that I keep getting!
    //              I setup a variable instead of using: "g_ObjMgr.DestroyObject( GetGuid() );" directly.
    guid Guid = GetGuid();

   
    //destroy the projectile.
    g_ObjMgr.DestroyObject( Guid );
}

//==============================================================================

void base_projectile::SetPainDegradation( f32 PainDropDist, f32 PainDropScale )
{
    m_PainDropDist  = PainDropDist;
    m_PainDropScale = PainDropScale;
}

//==============================================================================

void base_projectile::Initialize( const vector3&        InitPos,
                                  const radian3&        InitRot,
                                  const vector3&        InheritedVelocity,
                                        f32             Speed,
                                        guid            OwnerGuid,
                                        pain_handle     PainHandle,
                                        xbool           bHitLiving,
                                        s32             iFirePoint )
{

    // Setup the defaults
    m_PainDropDist      = 0.0f;
    m_PainDropScale     = 1.0f;

    m_bThroughActors    = FALSE;

    // Set owner and a few other values
    m_OwnerGuid = OwnerGuid;
    m_PainHandle = PainHandle;
    m_bHitLiving = bHitLiving;
    m_iFirePoint = iFirePoint;
    
    // We need to set our zone properly!
    object* pOwnerObject = g_ObjMgr.GetObjectByGuid(OwnerGuid);
    if( pOwnerObject )
    {
        SetZone1(pOwnerObject->GetZone1());
    }

    // Set transform
    matrix4 InitMat( vector3( 1.0f , 1.0f , 1.0f ), InitRot, InitPos );

    // Set transform
    OnTransform( InitMat ); 


    // Record current position
    m_CurrentPosition = InitPos;
    m_InitialPosition = InitPos;

    // Set speed
    m_Speed         = Speed;

    // Set velocity
    m_Velocity = vector3( 0.0f , 0.0f , m_Speed);
    m_Velocity.Rotate( InitRot );
    m_Velocity += InheritedVelocity;
}

//==============================================================================

void base_projectile::CreateDecal( collision_mgr::collision& rColl )
{
    decal_package* pPackage = m_hDecalPackage.GetPointer();
    if ( !pPackage )
        return;

    const char* pGroupName = "";

    switch ( rColl.Flags )
    {
    default:
    case MAT_TYPE_ROCK:
    case MAT_TYPE_CONCRETE:
    case MAT_TYPE_SOLID_METAL:
    case MAT_TYPE_HOLLOW_METAL:    
    case MAT_TYPE_METAL_GRATE:
        pGroupName = "Hard";
        break;

    case MAT_TYPE_PLASTIC:
    case MAT_TYPE_WATER:
    case MAT_TYPE_WOOD:
    case MAT_TYPE_ICE:
    case MAT_TYPE_EARTH:
        pGroupName = "Soft";
        break;

    case MAT_TYPE_LEATHER:
    case MAT_TYPE_EXOSKELETON:
    case MAT_TYPE_FLESH:
    case MAT_TYPE_BLOB:
    case MAT_TYPE_ENERGY_FIELD:
        break;

    case MAT_TYPE_BULLET_PROOF_GLASS:
    case MAT_TYPE_GLASS:
        pGroupName = "Glass";
        break;
    }

    const decal_definition* pDef = pPackage->GetDecalDef( pGroupName, 0 );
    if ( pDef )
    {
        #if 1
        {
            // Get triangle information from collision
            object* pObject = g_ObjMgr.GetObjectByGuid( rColl.ObjectHitGuid );
            if( pObject )
            {
                object::detail_tri Tri;

                if( pObject->IsKindOf( proxy_playsurface::GetRTTI() ) )
                {
                    ((proxy_playsurface*)pObject)->GetColDetails( rColl.PrimitiveKey, Tri );
                    g_DecalMgr.CreateBulletHole( *pDef, rColl.Point, rColl.Plane, &Tri.Vertex[0] );
                }
            }
        }
        #else
        {
            g_DecalMgr.CreateDecalAtPoint( *pDef, rColl.Point, rColl.Plane.Normal );
        }
        #endif
    }
}
//==============================================================================

const char* base_projectile::GetNearFlyByDescriptor( void )
{ 
    return g_StringMgr.GetString( m_BulletNearFlyBySfxID ); 
}

//==============================================================================

const char* base_projectile::GetMediumFlyByDescriptor( void )
{ 
    return g_StringMgr.GetString( m_BulletMedFlyBySfxID ); 
}

//==============================================================================

const char* base_projectile::GetFarFlyByDescriptor( void )
{ 
    return g_StringMgr.GetString( m_BulletFarFlyBySfxID ); 
}

//==============================================================================

void base_projectile::SetTarget( guid TargetGuid ) 
{ 
    (void)TargetGuid; 
}

//==============================================================================

void base_projectile::StartFlyby( void )
{
}

//==============================================================================
