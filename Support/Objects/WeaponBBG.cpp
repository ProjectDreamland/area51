//=========================================================================
// WEAPON BBG (Bouncy Ball Gun) or NAW (New Alien Weapon)
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileEnergy.hpp"
#include "WeaponBBG.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Projector.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "render\LightMgr.hpp"
#include "Player.hpp"
#include "Objects/DestructibleObj.hpp"
#include "Objects/SuperDestructible.hpp"
#include "Objects/Corpse.hpp"
#include "Objects/Turret.hpp"
#include "Characters\character.hpp"

#if !defined(X_EDITOR)
#include "NetworkMgr/NetworkMgr.hpp"
#endif
#include "Gamelib/DebugCheats.hpp"

//const   xcolor  g_BBG_LaserColor( 49, 49, 250, 255 );//( 255, 0, 0, 255 );//( 90, 80, 40, 255 ); //( 49, 49, 250, 200 );
const   xcolor  g_BBG_LaserColor( 49, 250, 49, 255 );
const   xcolor  g_BBG_LaserColor_Locked( 250, 49, 49, 255 );

//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================

// defined in ProjectileEnergy.cpp
extern f32 BBG_BounceFactorRun;
extern f32 BBG_BounceFactorRise;

f32 s_BBGLockonBeepDelay            = 1.0f;

tweak_handle BBG_GainSecondsTweak ( "BBG_GainSeconds" );
tweak_handle BBG_BurnSecondsTweak ( "BBG_Secondary_BurnSeconds" );

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_bbg_desc : public object_desc
{

    weapon_bbg_desc( void ) : object_desc( object::TYPE_WEAPON_BBG, 
        "BBG",
        "WEAPON",
        object::ATTR_SPACIAL_ENTRY          |
        object::ATTR_NEEDS_LOGIC_TIME		|
        object::ATTR_SOUND_SOURCE			|
        object::ATTR_RENDERABLE				|
        object::ATTR_TRANSPARENT            |
        object::ATTR_COLLISION_PERMEABLE    ,
        FLAGS_IS_DYNAMIC
        //| FLAGS_GENERIC_EDITOR_CREATE                                                                       
        )
    {

    }

    //=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_bbg;
    }

} s_Weapon_bbg_Desc;

//=========================================================================

const object_desc&  weapon_bbg::GetTypeDesc     ( void ) const
{
    return s_Weapon_bbg_Desc;
}

//=========================================================================

const object_desc&  weapon_bbg::GetObjectType   ( void )
{
    return s_Weapon_bbg_Desc;
}

//=========================================================================

weapon_bbg::weapon_bbg( void )
{
    //initialize the ammo structures.
    m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_MSN;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 50;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 50;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;

    m_WeaponAmmo[ AMMO_SECONDARY ].m_ProjectileType = BULLET_MSN_SECONDARY;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    //Both primary and secondary fires use same ammo, so secondary ammo is only initialized
    //in the constructor of new_weapon and set to undefined.

    //set aim degradation
    m_AimDegradePrimary     = 0.2f;
    m_AimDegradeSecondary   = 0.25f;
    m_AimRecoverSpeed       = 0.9f;

    m_ReloadBoneIndex       = -1;
    m_CurrentWaitTime       = 0.0f;
    m_ReloadWaitTime        = 5.0f;
    m_bStartReloadFx        = FALSE; 
    
    m_LoopVoiceId           = 0;
    m_LockonLoopId          = 0;
    m_LaserOnLoopId         = 0;

    m_SecondaryFireBaseDamage   = 20.0f;
    m_SecondaryFireMaxDamage    = 35.0f;
    m_SecondaryFireSpeed        = 3000.0f;
    m_SecondaryFireBaseForce    = 10.0f;
    m_SecondaryFireMaxForce     = 20.0f;
    m_nZoomSteps                = 1;

    m_LastLockonTime            = s_BBGLockonBeepDelay;

    m_Item = INVEN_WEAPON_BBG;

    m_hMuzzleFXPrimary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );
    m_hMuzzleFXSecondary.SetName( PRELOAD_FILE("mhg_muzzleflash_000.fxo") );

    // initialize time
    m_LastUpdateTime = (f32)x_GetTimeSec();

    m_bIsAltFiring = FALSE;    
    m_LastAmmoBurnTime = m_LastUpdateTime;
    m_AmmoBurned = 0;

    // load up the bitmaps
    m_LaserBitmap.SetName( PRELOAD_FILE("Tracer_Laser.xbmp") );
    m_LaserFixupBitmap.SetName( PRELOAD_FILE("Tracer_Glow.xbmp") );

#ifndef X_EDITOR
    if ( m_LaserBitmap.GetPointer() == NULL )
    {
        ASSERTS( 0, xfs( "Unable to load %s", m_LaserBitmap.GetName() ) );
    }

    if ( m_LaserFixupBitmap.GetPointer() == NULL )
    {
        ASSERTS( 0, xfs( "Unable to load %s", m_LaserFixupBitmap.GetName() ) );
    }
#endif

    m_AutoSwitchRating = 5;
}

//=========================================================================

weapon_bbg::~weapon_bbg()
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }

    KillAllMuzzleFX();

    KillLaserSound();

    // get rid of reload particle if it exists
    DestroyReloadFx();
}

//==============================================================================
static s32 BBG_Alpha    = 100;
static xbool g_bOldDraw = FALSE;
static f32 g_BBGFirstSize1 = 5.0f;
static f32 g_BBGFirstSize2 = 6.0f;
static f32 g_BBGLastSize1 = 15.0f;
static f32 g_BBGLastSize2 = 15.0f;

void weapon_bbg::OnRenderTransparent(void)
{
    // call base class render which basically just calles object::OnRenderTransparent()
    new_weapon::OnRenderTransparent();

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        draw_ClearL2W();

        // set up drawing
        draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_BLEND_ADD | DRAW_CULL_NONE );

        player* pPlayer = (player*)pObj;

        // is "laser" button held?
        if( m_ZoomStep != 0 && IsZoomInComplete() )
        {
            if( !g_bOldDraw )
            {
                // set draw texture
                draw_SetTexture( *m_LaserBitmap.GetPointer() );
            }
            
            vector3 StartPos, EndPos;
            // get the point from which the projectile will fire
            GetFiringStartPosition( StartPos );

            // get the position the "laser" will do a collision trace to
            pPlayer->GetProjectileHitLocation(EndPos, FALSE);

            // init bounce count
            u8 nBounces = 0;

            // save off old start position so we can set StartPos to next start point
            vector3 OldStartPos = StartPos;

            //////////////
            // setup collision info for firing "laser" from initial point to EndPos
            g_CollisionMgr.AddToIgnoreList( pPlayer->GetGuid() );
            g_CollisionMgr.RaySetup( pPlayer->GetGuid(), StartPos, EndPos );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES, object::ATTR_COLLISION_PERMEABLE);

            // default modifier to full distance in case the collision manager returns no collisions
            f32 DistModifier = 1.0f;

            // if we don't hit anything, T is undefined
            if( g_CollisionMgr.m_nCollisions > 0 )
            {
                DistModifier = g_CollisionMgr.m_Collisions[0].T;
            }

            // get our new end position
            EndPos = StartPos + (DistModifier*(EndPos-StartPos));            
            //////////////

            // get the object we hit
            guid HitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
            object* pObject = g_ObjMgr.GetObjectByGuid( HitGuid );

            // set flag for colliding with living objects
            xbool bStop = CheckLaserHitObject(pObject);

            // set up color and alpha; if we hit something, change color
            xcolor C = bStop ? g_BBG_LaserColor_Locked : g_BBG_LaserColor;

            C.A = (u8)BBG_Alpha;

            struct struct_CachedQuadInfo
            {
                vector3 StartPos;
                vector3 EndPos;
                collision_mgr::collision CachedCollision;

            };

            // +1 because we have an extra entry (the first one which is 0)
            struct_CachedQuadInfo CachedQuadInfo[MAX_ENERGY_PROJ_IMPACTS+1];

            CachedQuadInfo[nBounces].StartPos        = StartPos;
            CachedQuadInfo[nBounces].EndPos          = EndPos;
            // save off collision info (the first one is the initial collision).
            CachedQuadInfo[nBounces].CachedCollision = g_CollisionMgr.m_Collisions[0];

            if( g_bOldDraw )
            {
                // always draw the 1st "laser" textured quad (the one coming out of the gun)
                draw_OrientedQuad( StartPos, EndPos,
                    vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                    C, C,
                    g_BBGFirstSize1, g_BBGFirstSize2 );
            }

            // go through MAX impacts.  Stop if this is living or this is our first bounce
            if( !bStop )
            {
                while( nBounces < MAX_ENERGY_PROJ_IMPACTS )
                {
                    // put start position on our end postion
                    StartPos = EndPos;

                    nBounces++;

                    vector3 Direction = (StartPos-OldStartPos);
                    EndPos = GetLaserReflectVector( pPlayer, StartPos, Direction, g_CollisionMgr.m_Collisions[0] );

                    // save off info
                    CachedQuadInfo[nBounces].StartPos        = StartPos;
                    CachedQuadInfo[nBounces].EndPos          = EndPos;

                    // be sure we get the collision from GetLaserReflectVector() saved
                    CachedQuadInfo[nBounces].CachedCollision = g_CollisionMgr.m_Collisions[0];

                    // setup collision info
                    // REMEMBER, this collision is coming from GetLaserReflectVector()
                    HitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
                    pObject = g_ObjMgr.GetObjectByGuid( HitGuid );

                    // set flag for colliding with living objects
                    bStop = CheckLaserHitObject(pObject);

                    if( g_bOldDraw )
                    {
                        // draw the "laser" textured quad
                        draw_OrientedQuad( StartPos, EndPos,
                            vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                            C, C,
                            g_BBGLastSize1, g_BBGFirstSize2 );
                    }

                    if( bStop )
                    {                        
                        break;
                    }

                    OldStartPos = StartPos;
                }
            }

            // locked onto target
            if( bStop )
            {
                C = g_BBG_LaserColor_Locked;
                C.A = (u8)BBG_Alpha;

                m_bLockedOn = TRUE;
            }
            else
            {
                m_bLockedOn = FALSE;
            }

            // our special little counter
            u8 i = 0;

            // do this in a separate loop so we aren't flushing the draw pipeline and switching textures so much
            while( i <= nBounces )
            {
                if( !g_bOldDraw )
                {
                    // set draw texture
                    draw_SetTexture( *m_LaserBitmap.GetPointer() );

                    // make first quad smaller
                    if( i == 0 )
                    {
                        // get the point from which the "laser" emits
                        vector3 StartPos = GetLaserEmitPosition();
                        draw_OrientedQuad( StartPos, CachedQuadInfo[i].EndPos,
                            vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                            C, C,
                            g_BBGFirstSize1, g_BBGFirstSize2 );
                    }
                    else
                    {
                        draw_OrientedQuad( CachedQuadInfo[i].StartPos, CachedQuadInfo[i].EndPos,
                            vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
                            C, C,
                            g_BBGLastSize1, g_BBGLastSize2 );
                    }
                }

                // draw bitmap "error hider" for "laser"
                // change color if target is locked
                xcolor c = bStop ? g_BBG_LaserColor_Locked : g_BBG_LaserColor;

                new_weapon::DrawLaserFixupBitmap( m_LaserFixupBitmap.GetPointer(), 7.5f, c, CachedQuadInfo[i].CachedCollision );


                i++;
            }
        }
        else
        {
            // clear texture, alt fire isn't down
            draw_SetTexture();
        }

        // engine stop drawing
        draw_End();
    }

    RenderReloadFX();
}

//===========================================================================
xbool weapon_bbg::CheckLaserHitObject(object *pObject)
{
    if( (pObject->GetAttrBits() & object::ATTR_DESTROY) )
    {
        // this thing is being deleted
        return FALSE;
    }

    // object is inactive, get out.
    if( !pObject->IsActive() )
    {
        return FALSE;
    }

    if( pObject->IsKindOf( turret::GetRTTI() ) )
    {
        if( ((turret*)pObject)->IsDestroyed() )
        {
            // destroyed, don't stop laser
            return FALSE;
        }
        else
        {
            // active turret
            return TRUE;
        }
    }

    // list of valid objects here
    if( pObject->IsKindOf( actor::GetRTTI() ) )
    {
        return TRUE;
    }

    return FALSE;
}

//===========================================================================
void weapon_bbg::PlayLaserSound( void )
{
    if( (m_LaserOnLoopId == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
    {
        m_LaserOnLoopId = g_AudioMgr.Play( "BBG_LaserSight" );
    }
}

//===========================================================================
void weapon_bbg::KillLaserSound( void )
{
    if( m_LaserOnLoopId )
    {
        g_AudioMgr.Release(m_LaserOnLoopId, 0.0f);
        m_LaserOnLoopId = 0;

        g_AudioMgr.Play( "BBG_LaserSight_Off", GetPosition(), GetZone1(), TRUE );
    }

    m_bLockedOn = FALSE;
}

//===========================================================================

vector3 weapon_bbg::GetLaserReflectVector( player *pPlayer, const vector3& StartPos, const vector3& Direction, const collision_mgr::collision& Coll )
{
    radian Pitch;
    radian Yaw;
    vector3 Rise;
    vector3 Run;
    vector3 NewDirection, EndPos;

    // get reflect vector (NOTE: using this version because it automatically pulls out the components)
    Coll.Plane.GetComponents( Direction, Run, Rise );
    NewDirection = ( (Run * BBG_BounceFactorRun) + (Rise * BBG_BounceFactorRise) );

    // get rotation from direction
    NewDirection.GetPitchYaw(Pitch, Yaw);
    vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
    Dest *= 4500;

    // set up endpoint
    EndPos = StartPos + Dest;

    // set up collision
    g_CollisionMgr.AddToIgnoreList( pPlayer->GetGuid() );
    g_CollisionMgr.RaySetup( pPlayer->GetGuid(), StartPos, EndPos );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE);

    // default modifier to full distance in case the collision manager returns no collisions
    f32 DistModifier = 1.0f;

    // if we don't hit anything, T is undefined
    if( g_CollisionMgr.m_nCollisions > 0 )
    {
        DistModifier = Coll.T;
    }

    // get our new end position
    EndPos = StartPos + (DistModifier*(EndPos-StartPos));

    return EndPos;
}


//===========================================================================

void weapon_bbg::OnEnumProp( prop_enum& PropEnumList )
{
    new_weapon::OnEnumProp( PropEnumList );    

    PropEnumList.PropEnumHeader  ( "BBG", "Bouncy Ball Gun Weapon", 0 );
    PropEnumList.PropEnumExternal( "BBG\\Bullet Audio Package",   "Resource\0audiopkg", "The audio package associated with this weapon's bullets.", 0 );

    PropEnumList.PropEnumExternal( "BBG\\Reload Effect",          "Resource\0fxo",      "The particle effect for the reload", 0 );

    PropEnumList.PropEnumExternal( "BBG\\Primary Fire Effect",    "Resource\0fxo",      "The particle effect for the primary fire", 0 );
    PropEnumList.PropEnumExternal( "BBG\\Secondary Fire Effect",  "Resource\0fxo",      "The particle effect for the secondary fire", 0 );    
    PropEnumList.PropEnumExternal( "BBG\\Wall Bounce Efect",      "Resource\0fxo",      "The particle effect for when primary projectile hits a wall", 0 );

    PropEnumList.PropEnumFloat   ( "BBG\\Reload Wait Time",           "How long to wait before the BBG starts reloading.", 0 );
    PropEnumList.PropEnumFloat   ( "BBG\\Secondary Fire Base Damage", "The minimum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "BBG\\Secondary Fire Max Damage",  "The maximum amount of damage the secondary fire will cause.", 0 );
    PropEnumList.PropEnumFloat   ( "BBG\\Secondary Fire Speed",       "How fast is the secondary fire going to travel.", 0 );
    PropEnumList.PropEnumFloat   ( "BBG\\Secondary Fire Base Force",  "The minimum force the secondary fire going to emmit when it hits something.", 0 );
    PropEnumList.PropEnumFloat   ( "BBG\\Secondary Fire Max Force",   "The maximum force is the secondary fire going to emmit when it hits something.", 0 );
}

//===========================================================================

xbool weapon_bbg::OnProperty( prop_query& PropQuery )
{
    if ( new_weapon::OnProperty( PropQuery ) )
    {
        return TRUE;
    }

    if(  new_weapon::OnProperty( PropQuery ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Reload Wait Time",            m_ReloadWaitTime ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Secondary Fire Base Damage",  m_SecondaryFireBaseDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Secondary Fire Max Damage",   m_SecondaryFireMaxDamage ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Secondary Fire Speed",        m_SecondaryFireSpeed ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Secondary Fire Base Force",   m_SecondaryFireBaseForce ) )
        return TRUE;

    if(  PropQuery.VarFloat( "BBG\\Secondary Fire Max Force",    m_SecondaryFireMaxForce ) )
        return TRUE;

    // External
    if( PropQuery.IsVar( "BBG\\Reload Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ReloadFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ReloadFx.SetName( pString );                

                // Load the audio package.
                if( m_ReloadFx.IsLoaded() == FALSE )
                    m_ReloadFx.GetPointer();
            }
        }
        return( TRUE );
    }
    
    // External
    if( PropQuery.IsVar( "BBG\\Bullet Audio Package" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hBulletAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hBulletAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hBulletAudioPackage.IsLoaded() == FALSE )
                    m_hBulletAudioPackage.GetPointer();

            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "BBG\\Primary Fire Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_PrimaryProjectileFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_PrimaryProjectileFx.SetName( pString );                

                // Load the audio package.
                if( m_PrimaryProjectileFx.IsLoaded() == FALSE )
                    m_PrimaryProjectileFx.GetPointer();
            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "BBG\\Secondary Fire Effect" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_SecondaryProjectileFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_SecondaryProjectileFx.SetName( pString );                

                // Load the audio package.
                if( m_SecondaryProjectileFx.IsLoaded() == FALSE )
                    m_SecondaryProjectileFx.GetPointer();
            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "BBG\\Wall Bounce Efect") )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_WallBounceFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_WallBounceFx.SetName( pString );                

                // Load the audio package.
                if( m_WallBounceFx.IsLoaded() == FALSE )
                    m_WallBounceFx.GetPointer();
            }
        }
        return( TRUE );
    }


    return FALSE;
}

//==============================================================================

void weapon_bbg::InitWeapon			(   
                                     const char* pSkinFileName , 
                                     const char* pAnimFileName , 
                                     const vector3& rInitPos , 
                                     const render_state& rRenderState,
                                     const guid& rParentGuid )
{
    new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos ,rRenderState, rParentGuid);

    m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );

    if( m_AnimGroup[rRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[rRenderState].GetBoneIndex( "firepoint" );

        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//===========================================================================

void weapon_bbg::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

    m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );

    if( m_AnimGroup[rRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[rRenderState].GetBoneIndex( "firepoint" );

        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//=========================================================================
xbool weapon_bbg::ShouldDrawReticle( void )
{
    return ( m_ZoomStep == 0 );
}

//=========================================================================
s32 weapon_bbg::IncrementZoom( void )
{
    // let base weapon code go through its stuff.
    new_weapon::IncrementZoom();

    // we don't need the "zoom out" state, that's only for weapons that change the FOV
    // if we're at 0, base weapon code is killing the zoom.
    if( m_ZoomStep == 0 )
    {
        ClearZoom();
    }

    return m_ZoomStep;
}

//=========================================================================
void weapon_bbg::RenderReloadFX( void )
{
    // Do the players reload render logic...
    if( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        // if we are owned by a player, then we need to ask for his offset
        object*        pOwner    = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
        vector3        Offset    ( 0.0f, 0.0f, 0.0f );
        if ( pOwner && pOwner->IsKindOf( player::GetRTTI() ) )
        {
            Offset = ((player*)pOwner)->GetCurrentWeaponCollisionOffset();
        }

        if( m_ReloadBoneIndex != -1 )
        {
            if( m_ReloadFxHandle.Validate() )
            {
                vector3 Pos( m_ReloadFxHandle.GetTranslation() );
                m_ReloadFxHandle.SetTranslation( Pos + Offset );
                m_ReloadFxHandle.Render();
                m_ReloadFxHandle.SetTranslation( Pos );
            }
        }
    }

    // Do the NPC reload render logic.. 
    // Don't need || IsUsingSplitScreen() because player logic handles the render state properly in this case.
    if ( m_CurrentRenderState == RENDER_STATE_NPC  )
    {         
        // render it
        for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
        {
            if( m_ReloadBoneIndex != -1 )
            {
                m_NPCReloadFxHandle.Render();
            }
        }            
    }
}

//=========================================================================

void weapon_bbg::OnAdvanceLogic( f32 DeltaTime )
{
    f32 currentTime = (f32)x_GetTimeSec();

    // we are in idle mode or zoom idle, so recharge
    if( m_bIdleMode && m_ZoomStep == 0 )
    {   
        m_CurrentWaitTime += ((currentTime - m_LastUpdateTime));

        if( (m_CurrentWaitTime >= m_ReloadWaitTime) && 
            (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) )
        {
            s32 Ammo = (s32)((m_CurrentWaitTime-m_ReloadWaitTime)/BBG_GainSecondsTweak.GetF32());

            // clamp ammo
            Ammo = MIN( Ammo,   ((m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) - (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount)) );

            // Did the ammo count change.
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount != (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount+Ammo) )
            {
                // Update the ammo and set the time back.
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount += Ammo;
                m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip    = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount;
                m_CurrentWaitTime = m_ReloadWaitTime;
            }

            // All maxed out.
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount == m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax )
            {
                SuspendReloadFX();
            }
            else
            {   
                InitReloadFx();
            }
        }        
    }
    else
    {
        // REMEMBER: Alt fire needs to take ammo from primary
        if( m_ZoomStep != 0 )
        {
            if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount > 0 )
            {
                f32 checkTime = (currentTime - m_LastAmmoBurnTime);

                if( checkTime > BBG_BurnSecondsTweak.GetF32() )
                {
                    // Decrement count of bullets in current clip
                    // REMEMBER: this needs to take ammo from primary
                    DecrementAmmo(AMMO_PRIMARY);
                    m_AmmoBurned++;
                    m_LastAmmoBurnTime = (f32)x_GetTimeSec();
                }

                PlayLaserSound();
            }
            else
            {
                object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
                if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
                {
                    // tell the player to play zoom out anim
                    player* pPlayer = (player*)pObj;

                    pPlayer->EndZoomState();

                    KillLaserSound();
                }
            }
        }
        else
        {
            KillLaserSound();
        }
    }

    m_LastUpdateTime = (f32)x_GetTimeSec();

    // Do this here.  Otherwise, the FX will trail a frame behind
    AdvanceReloadFx( DeltaTime );

    new_weapon::OnAdvanceLogic( DeltaTime );
}

//==============================================================================
void weapon_bbg::UpdateReticle( f32 DeltaTime )
{
    if( !ShouldUpdateReticle() )
    {
        return;
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        if( pPlayer->IsCinemaRunning() )
        {
            return;
        }

        // reticle is on or laser is locked on
        // NOTE: we hijacked m_bLockedOn to tell if laser locked onto an enemy
        if( CheckReticleLocked() || m_bLockedOn )
        {
            m_LastLockonTime += DeltaTime;

            if( m_LastLockonTime >= s_BBGLockonBeepDelay )
            {
                g_AudioMgr.Play( "Reticule_Shift_Red" );
                m_LastLockonTime = 0.0f;
            }
        }
        else
        {
            // set delay so as soon as we lock on again, sound will beep.
            m_LastLockonTime = s_BBGLockonBeepDelay;
        }
    }
    else
    {
        // set delay so as soon as we lock on again, sound will beep.
        m_LastLockonTime = s_BBGLockonBeepDelay;
    }
}

//==============================================================================

void weapon_bbg::SetupRenderInformation( )
{    
    new_weapon::SetupRenderInformation( );

    if( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_ReloadBoneIndex    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );

        ASSERT( m_ReloadBoneIndex != -1 );
    }
}

//==============================================================================
vector3 weapon_bbg::GetLaserEmitPosition( void )
{
    // get the position that the "laser" emits from
    return m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT] ) + GetWeaponCollisionOffset();
}

//==============================================================================
xbool weapon_bbg::GetFiringStartPosition( vector3 &Pos )
{
    // get the "firepoint" position for this gun
    Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] ) + GetWeaponCollisionOffset();
    return TRUE;
}

//=========================================================================

xbool weapon_bbg::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;
    ( void )InitPos;    // we don't need InitPos because this weapon fires from its FIRE_POINT_DEFAULT

    ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );

    //if there weapon is not ready, do nothing.
    if ( ! IsWeaponReady( AMMO_PRIMARY ) )
    {
        return FALSE;
    }

    //otherwise, create a new bullet projectile, init it's position, and send it on it's way.
    else
    {
        vector3 StartPosition;
        GetFiringStartPosition( StartPosition );

        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iFirePoint ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

        //        base_projectile* pBaseProjectile;
        //        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;
        pain_handle PainHandle( xfs("%s_%s_PRIMARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        // Create the Meson Projectile
        guid ProjectileID = CREATE_NET_OBJECT( energy_projectile::GetObjectType(), netobj::TYPE_BBG_1ST );
        energy_projectile* pProjectile = (energy_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
        ASSERT( pProjectile );

        // Compute velocity
        tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
        vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
        Velocity.Rotate( InitRot );
        Velocity += BaseVelocity;

        pProjectile->Setup( Owner,
            pOwnerObject->net_GetSlot(),
            StartPosition,
            radian3(0.0f,0.0f,0.0f), //InitRot
            Velocity,
            GetZone1(),
            GetZone2(),
            0.0f,
            PainHandle );

        /*
        new_weapon::CreateBullet

        // Lookup speed
        tweak_handle SpeedTweak( xfs("%s_SPEED",pWeaponLogicalName) );
        pBullet->Initialize( InitPos, InitRot, InheritedVelocity, SpeedTweak.GetF32(), OwnerGuid, PainHandle, bHitLiving );

        // Lookup pain degradation
        tweak_handle PainDropDistTweak ( xfs("%s_PainDropDist",pWeaponLogicalName) );
        tweak_handle PainDropScaleTweak( xfs("%s_PainDropScale",pWeaponLogicalName) );
        pBullet->SetPainDegradation( PainDropDistTweak.GetF32(), PainDropScaleTweak.GetF32() );
        */
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Decrement count of bullets in current clip
        DecrementAmmo();

        return TRUE;
    }
}

//=========================================================================

xbool weapon_bbg::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;
    ( void )InitPos;    // we don't need InitPos because this weapon fires from its FIRE_POINT_DEFAULT
    (void)BaseVelocity;
    (void)Power;
    (void)InitRot;
    (void)Owner;

    // put in our "Laser"

    return FALSE;
}

//=========================================================================

xbool weapon_bbg::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{   
    (void)isHit;
    (void) fDegradeMultiplier;
    vector3 StartPosition;

    //if there weapon is not ready, do nothing.
    if ( ! IsWeaponReady( AMMO_PRIMARY ) )
    {
        return FALSE;
    }
    //otherwise, create a new bullet projectile, init it's position, and send it on it's way.
    else
    {
        GetFiringStartPosition( StartPosition );

        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] );
        radian3 Rot = L2W.GetRotation();
        (void)Rot;

        vector3 toTarget = Target - StartPosition;
        radian3 InitRot(toTarget.GetPitch(),toTarget.GetYaw(),0.0f);

        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;

        pain_handle PainHandle( xfs("%s_%s_PRIMARY",pOwnerObject->GetLogicalName(),GetLogicalName()) );

        //        base_projectile* pBaseProjectile;
        //        pBaseProjectile = CreateBullet( GetLogicalName(), StartPosition, InitRot, BaseVelocity, Owner, PainHandle, AMMO_PRIMARY, isHit );
        //        ((energy_projectile*)pBaseProjectile)->LoadEffect( m_PrimaryProjectileFx.GetName(), StartPosition, Rot );

        // Create the Meson Projectile
        guid ProjectileID = CREATE_NET_OBJECT( energy_projectile::GetObjectType(), netobj::TYPE_BBG_1ST );
        energy_projectile* pProjectile = (energy_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
        ASSERT( pProjectile );

        // Compute velocity
        tweak_handle SpeedTweak( xfs("%s_SPEED", GetLogicalName()) );
        vector3 Velocity( 0.0f , 0.0f , SpeedTweak.GetF32() );
        Velocity.Rotate( InitRot );
        Velocity += BaseVelocity;

        pProjectile->Setup( Owner,
            pOwnerObject->net_GetSlot(),
            StartPosition,
            radian3(0.0f,0.0f,0.0f), //InitRot
            Velocity,
            GetZone1(),
            GetZone2(),
            0.0f,
            PainHandle );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( StartPosition, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Play the sound associated with this actor's faction.
        object* pObj = g_ObjMgr.GetObjectByGuid( Owner );
        actor& OwnerActor = actor::GetSafeType( *pObj );

        factions Faction = OwnerActor.GetFaction();
        s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

        if( m_FactionFireSfx[ BitIndex ] != -1 )
        {
            m_LoopVoiceId = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), 
                GetPosition(), GetZone1(), TRUE );
            g_AudioManager.NewAudioAlert( m_LoopVoiceId, 
                audio_manager::GUN_SHOT, GetPosition(), 
                GetZone1(), GetGuid() );
        }

        return TRUE;
    }
}

//=========================================================================

xbool weapon_bbg::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{
    ( void ) BaseVelocity;
    ( void ) Target;
    ( void ) Owner;
    ( void ) fDegradeMultiplier;
    ( void ) isHit;

    return FALSE;
}

//=========================================================================

xbool weapon_bbg::FireGhostPrimary( s32 iFirePoint, xbool , vector3& )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    // Don't create anything, the projectile is networked and will create itself on the other machines,

    // Start the muzzle FX
    InitMuzzleFx( TRUE, iFirePoint );

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

    vector3 InitPos     = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] ) + GetWeaponCollisionOffset();

    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID < 0 )
    {
        return FALSE;
    }

    // add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    // Play the sound associated with this actor's faction.
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    actor& OwnerActor = actor::GetSafeType( *pObj );

    factions Faction = OwnerActor.GetFaction();
    s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

#if !defined(X_EDITOR)
    if( g_NetworkMgr.IsOnline() )
    {
        BitIndex = 0;
    }
#endif
    if( m_FactionFireSfx[ BitIndex ] != -1 )
    {
        voice_id VoiceID = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), GetPosition(), GetZone1(), TRUE );
        g_AudioManager.NewAudioAlert( VoiceID, audio_manager::GUN_SHOT, GetPosition(), GetZone1(), GetGuid() );
    }

    return TRUE;
}

//=========================================================================

xbool weapon_bbg::FireGhostSecondary( s32 iFirePoint )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    // Don't create anything, the projectile is networked and will create itself on the other machines,

    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

    vector3 InitPos     = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] ) + GetWeaponCollisionOffset();

    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID < 0 )
    {
        return FALSE;
    }

    // add a muzzle light where the bullet was fired from (will fade out really quickly)
    g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

    // Play the sound associated with this actor's faction.
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    actor& OwnerActor = actor::GetSafeType( *pObj );

    factions Faction = OwnerActor.GetFaction();
    s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

#if !defined(X_EDITOR)
    if( g_NetworkMgr.IsOnline() )
    {
        BitIndex = 0;
    }
#endif
    voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Fire_Charge",GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );

    return TRUE;
}

//==============================================================================

void weapon_bbg::ProcessSfx( void )
{

}

//==============================================================================

void weapon_bbg::BeginAltFire( void )
{
    /*
    if( (m_LoopVoiceId == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
    {
    // Use the weapons position as the place where the sound start since the next time the event for
    // the secondary fire comes it will move it to the muzzle.
    //m_LoopVoiceId = g_AudioManager.Play( "Pistol_Secondary_Fire_Loop", GetPosition() );
    m_LoopVoiceId = g_AudioManager.Play( "Pistol_Secondary_Fire_Loop", audio_manager::GUN_SHOT, GetPosition(), 
    GetZone1(), GetGuid() );
    }
    */
}

//==============================================================================

void weapon_bbg::EndAltFire( void )
{
    // homing = false
    /*
    if( m_LoopVoiceId )
    {
    vector3 Pos( 0.0f, 0.0f, 0.0f);
    g_AudioManager.GetPosition( m_LoopVoiceId, Pos );

    g_AudioManager.Play( "Pistol_Primary_Fire", Pos );

    g_AudioManager.Release( m_LoopVoiceId, 0.0f );
    m_LoopVoiceId = 0;
    }
    */
}

//==============================================================================
void weapon_bbg::BeginAltRampUp( void )
{
    m_AmmoBurned = 0;
    m_bIsAltFiring = TRUE;
    m_LastAmmoBurnTime = (f32)x_GetTimeSec();    
}

//==============================================================================
void weapon_bbg::EndAltRampUp( xbool bGoingIntoHold, xbool bSwitchingWeapon )
{
    (void)bGoingIntoHold;

    if( bSwitchingWeapon )
    {
        // switching weapons, just destroy the projectile
        if( m_SecondaryFireProjectileGuid )
        {
            g_ObjMgr.DestroyObjectEx( m_SecondaryFireProjectileGuid, TRUE );
            m_SecondaryFireProjectileGuid = 0;
            return;
        }
    }
}

void weapon_bbg::EndAltHold( xbool bSwitchingWeapon )
{
    if( bSwitchingWeapon )
    {
        // switching weapons, just destroy the projectile
        if( m_SecondaryFireProjectileGuid )
        {
            g_ObjMgr.DestroyObjectEx( m_SecondaryFireProjectileGuid, TRUE );
            m_SecondaryFireProjectileGuid = 0;
        }
    }
}

//==============================================================================

xbool weapon_bbg::CanReload( const ammo_priority& Priority )
{
    (void)Priority;
    return FALSE;
}

//==============================================================================

xbool weapon_bbg::CanFire( xbool bIsAltFire )
{
    s32 AmmoCount = 0;

    // REMEMBER: alt and primary fire use the same ammo
    ammo_priority AmmoPriority = GetPrimaryAmmoPriority();
    AmmoCount = GetAmmoCount( AmmoPriority );

    if( bIsAltFire )
    {        
        // Can we fire our "LASER" ?
        return TRUE;
    }

    // normal fire will fire 3 rounds
    return( AmmoCount >= 3 );
}

//==============================================================================

void weapon_bbg::BeginPrimaryFire ( void )
{   
}

//==============================================================================

void weapon_bbg::EndPrimaryFire ( void )
{
}

//==============================================================================

void weapon_bbg::ReleaseAudio( void )
{
    if( m_LoopVoiceId )
    {
        g_AudioMgr.Release( m_LoopVoiceId, 0.0f );
        m_LoopVoiceId = 0;
    }

    KillLaserSound();
}

//==============================================================================

void weapon_bbg::InitReloadFx( void )
{
    if( m_bStartReloadFx == FALSE )
    {
        // if we are recharging, kill sound.
        KillLaserSound();

        // Start player reload fx?
        if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            // make sure there's a name
            if( m_ReloadFx.GetPointer() )
            {
                // if it's still valid, restart it.
                if( m_ReloadFxHandle.Validate() )
                {
                    m_ReloadFxHandle.Restart();
                }
                else
                if( m_ReloadBoneIndex != -1 )
                {
                    m_ReloadFxHandle.InitInstance( m_ReloadFx.GetPointer() );
                }
            }
        }

        // Start 3rd person reload fx?            
        if( m_CurrentRenderState == RENDER_STATE_NPC || IsUsingSplitScreen() )
        {
            // make sure there's a name
            if( m_ReloadFx.GetPointer() )
            {
                // if it's still valid, restart it.
                if( m_NPCReloadFxHandle.Validate() )
                {
                    m_NPCReloadFxHandle.Restart();
                }
                else
                if( m_ReloadBoneIndex != -1 )
                {
                    m_NPCReloadFxHandle.InitInstance( m_ReloadFx.GetPointer() );
                }
            }
        }

        m_bStartReloadFx = TRUE;
    }
}

//==============================================================================

void weapon_bbg::AdvanceReloadFx( f32 DeltaTime )
{
    // Do the players reload render logic...
    if( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        if( m_ReloadFxHandle.Validate() )
        {
            // if it finished, get rid of it
            if( m_ReloadFxHandle.IsFinished() )
            {
                m_ReloadFxHandle.KillInstance();
            }
            else
            if( m_ReloadBoneIndex != -1 )
            {
                // transform the effect
                matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
                L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );

                // Add in the collision offset
                matrix4 CollisionOffset;
                CollisionOffset.Identity();
                CollisionOffset.SetTranslation( GetWeaponCollisionOffset() );
                L2W = CollisionOffset * L2W;

                m_ReloadFxHandle.SetTransform( L2W );

                // the run the logic
                m_ReloadFxHandle.AdvanceLogic( DeltaTime );
            }
        }
    }

    // Do the NPC reload move logic..
    if ( m_CurrentRenderState == RENDER_STATE_NPC || IsUsingSplitScreen() )
    {
        if( m_NPCReloadFxHandle.Validate() )
        {
            // if it finished, get rid of it
            if( m_NPCReloadFxHandle.IsFinished() )
            {
                m_NPCReloadFxHandle.KillInstance();
            }
            else
            if( m_ReloadBoneIndex != -1 )
            {
                vector3 AimPos;
                vector3 BonePos;
                radian Pitch, Yaw;
                render_state OldState = m_CurrentRenderState;

                // save off old state, just to be safe.
                SetRenderState( RENDER_STATE_NPC );

                // Add in the collision offset
                matrix4 CollisionOffset;
                CollisionOffset.Identity();
                CollisionOffset.SetTranslation( GetWeaponCollisionOffset() );                

                // move the effect
                if( GetAimBonePosition(AimPos) && GetFiringBonePosition(BonePos) )
                {
                    vector3 Rot = AimPos-BonePos;
                    matrix4 M;

                    Rot.GetPitchYaw( Pitch,Yaw );
                    M.Setup( vector3(1.0f,1.0f,1.0f), radian3(Pitch,Yaw,0.0f), BonePos );

                    M = CollisionOffset * M;

                    // point reload fx in a direction toward what we're aiming at
                    m_NPCReloadFxHandle.SetTransform( M );
                }
                else
                {
                    // transform the effect to the fire point so mp dual smps work
                    matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_ReloadBoneIndex );
                    L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_ReloadBoneIndex ) );
                    L2W = CollisionOffset * L2W;
                    m_NPCReloadFxHandle.SetTransform( L2W );
                }

                // then run the logic
                m_NPCReloadFxHandle.AdvanceLogic( DeltaTime );

                // put this back, just in case.
                SetRenderState( OldState );
            }
        }
    }
}

//==============================================================================

void weapon_bbg::DestroyReloadFx( void )
{
    // Destroy the particles.
    m_ReloadFxHandle.KillInstance();
    m_NPCReloadFxHandle.KillInstance();
}

//==============================================================================

void weapon_bbg::SuspendReloadFX( void )
{
    m_ReloadFxHandle.SetSuspended( TRUE );
    m_NPCReloadFxHandle.SetSuspended( TRUE );
}

//==============================================================================
xbool weapon_bbg::CanSwitchIdleAnim( void )
{
    if( (m_CurrentWaitTime < m_ReloadWaitTime) &&
        (m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount < m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax) )
    {
        return FALSE;
    }
    else
    {
        if( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount == m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax )
            return TRUE;
        else
            return FALSE;
    }
}

//==============================================================================
void weapon_bbg::BeginSwitchTo( void )
{
    // reset initializer
    m_bStartReloadFx = FALSE;
}

//==============================================================================

void weapon_bbg::BeginSwitchFrom( void )
{
    // reset initializer
    m_bStartReloadFx = TRUE;

    DestroyReloadFx();
    KillLaserSound();
}

//==============================================================================

void weapon_bbg::BeginIdle( xbool bNormalIdle )
{
    // keep reload FX from playing if this isn't a normal idle (i.e. for cinematics)
    m_bStartReloadFx = !bNormalIdle;

    // set idle mode so that weapon will recharge
    m_bIdleMode = TRUE;
    m_CurrentWaitTime = 0.0f;
}

//==============================================================================

void weapon_bbg::EndIdle( void )
{
    m_bIdleMode         = FALSE;
    m_bStartReloadFx    = FALSE;

    SuspendReloadFX();
}

//==============================================================================

void weapon_bbg::ClearAmmo( const ammo_priority& rAmmoPriority  )
{
    if( DEBUG_INFINITE_AMMO == FALSE )
    {
        // clear count of bullets in current clip
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip = 0;

        // clear all bullets    
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount = 0;
    }
}

//===========================================================================

void weapon_bbg::DecrementAmmo( const ammo_priority& rAmmoPriority, const s32& nAmt)
{
    object *pObj = g_ObjMgr.GetObjectByGuid(m_ParentGuid);

    if( DEBUG_INFINITE_AMMO == FALSE && !pObj->IsKindOf( character::GetRTTI() ) )
    {
        // decrement count of bullets in current clip
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip -= nAmt;

        // also, take away from total if we aren't unlimited ammo
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount -= nAmt;
    }
}

//===========================================================================

void weapon_bbg::FillAmmo( void )
{
    // make sure ammo is constantly full - BBG works a bit differently
    if( DEBUG_INFINITE_AMMO )
    {
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    }
}

//===========================================================================

vector3 weapon_bbg::GetWeaponCollisionOffset( void )
{
    vector3 CollisionOffset( 0.0f, 0.0f, 0.0f );
    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
    if ( pOwner && (pOwner->GetType() == TYPE_PLAYER) )
    {
        CollisionOffset = ((player*)pOwner)->GetCurrentWeaponCollisionOffset();
    }
    return CollisionOffset;
}