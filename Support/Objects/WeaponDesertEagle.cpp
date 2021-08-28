//=========================================================================
// WEAPON DESERT EAGLE
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileBullett.hpp"
#include "WeaponDesertEagle.hpp"
#include "Debris\debris_mgr.hpp"
#include "Objects\Projector.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Player.hpp"
#include "Objects\ParticleEmiter.hpp"


//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_desert_eagle_desc : public object_desc
{

    weapon_desert_eagle_desc( void ) : object_desc( object::TYPE_WEAPON_DESERT_EAGLE, 
                                        "Desert Eagle",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
                                        object::ATTR_NEEDS_LOGIC_TIME       |
                                        object::ATTR_SOUND_SOURCE           |
                                        object::ATTR_RENDERABLE,
                                        FLAGS_IS_DYNAMIC

                                        ) {}

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_desert_eagle;
    }

} s_Weapon_Desert_Eagle_Desc;

//=========================================================================

const object_desc&  weapon_desert_eagle::GetTypeDesc( void ) const
{
    return s_Weapon_Desert_Eagle_Desc;
}

//=========================================================================

const object_desc&  weapon_desert_eagle::GetObjectType( void )
{
    return s_Weapon_Desert_Eagle_Desc;
}

//=========================================================================

weapon_desert_eagle::weapon_desert_eagle( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_PISTOL;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 64;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 8;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
	
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
	m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    m_NPCMuzzleSoundFx  = "Shotgun_Primary_Fire";
    m_ZoomFOV           = R_48;
    m_ViewChangeRate    = 1.0f;
    m_StickSensitivity  = 0.25f;
    m_ZoomMovementSpeed = 0.9f;

    //set aim degradation
    m_AimDegradePrimary     = 0.2f;
    m_AimDegradeSecondary   = 0.6f;
    m_AimRecoverSpeed       = 0.4f;

    m_Item = INVEN_WEAPON_DESERT_EAGLE;
    m_nZoomSteps = 1;

    m_AutoSwitchRating = 1;
}

//=========================================================================

weapon_desert_eagle::~weapon_desert_eagle()
{
}

//=========================================================================

xbool weapon_desert_eagle::FireNPCWeaponProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{   
        vector3 InitPos;
    
        if (!GetFiringBonePosition(InitPos))
            return FALSE;

        vector3 TargetVector   = Target - InitPos ; 
        radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );

        DegradeAim( TargetRot, R_3*fDegradeMultiplier, InitPos, Owner );

        //pick a random point on a small box that is located 10 cm. away.
        vector3 RandomVector ( x_frand(-.3f , .3f ) , x_frand(-.3f , .3f ) , 5.f );

        //rotate that point into model space
        RandomVector.Rotate( TargetRot );

        //make a radian3 out of this vector
        radian3 RandomRot( RandomVector.GetPitch() , RandomVector.GetYaw() , 0.f );

        //create Bullet
        FireBullet(  InitPos, RandomRot, BaseVelocity, Owner, isHit );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // Play the sound associated with this actor's faction.
        object* pObj = g_ObjMgr.GetObjectByGuid( Owner );
        actor& OwnerActor = actor::GetSafeType( *pObj );

        factions Faction = OwnerActor.GetFaction();
        s32 BitIndex = factions_manager::GetFactionBitIndex( Faction );

        if( m_FactionFireSfx[ BitIndex ] != -1 )
        {
            voice_id VoiceID = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ),
                                                GetPosition(), GetZone1(), TRUE );
            g_AudioManager.NewAudioAlert( VoiceID, 
                                          audio_manager::GUN_SHOT, 
                                          GetPosition(), 
                                          GetZone1(), 
                                          GetGuid() );
        }

		return TRUE;
	}
}

//=========================================================================

xbool weapon_desert_eagle::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;

	//if there weapon is not ready, do nothing.
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	//otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{   
		//create Bullet
        FireBullet(  InitPos, InitRot, BaseVelocity, Owner, TRUE );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );
        
        // decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
}

//=========================================================================

xbool weapon_desert_eagle::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
	ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );
	ASSERT( m_AltFiringPointBoneIndex[ iFirePoint ] != -1 );

    (void)Power;
    (void)iFirePoint;
    
	//if there weapon is not ready, do nothing.  Shotgun uses primary ammo for both primary and secondary fire
	if ( ! IsWeaponReady( AMMO_PRIMARY ) )
	{
		return FALSE;
	}

	// otherwise, create a new bullet projectile, init it's position, and send it on it's way.
	else
	{
        FireBullet( InitPos, InitRot, BaseVelocity, Owner, TRUE );
 
        // add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // decrement count of bullets in current clip
        DecrementAmmo();

		return TRUE;
	}
	
	return FALSE;
}

//===========================================================================

void weapon_desert_eagle::FireBullet( const vector3& Pos, const radian3& Rot, const vector3& InheritedVelocity, guid Owner, const xbool bHitLiving )
{
    base_projectile* BP;
    ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return;

    pain_handle PainHandle( xfs("%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName()) );
    BP = CreateBullet( GetLogicalName(), Pos, Rot, InheritedVelocity, Owner, PainHandle, AMMO_PRIMARY, bHitLiving );
}

//==============================================================================

void weapon_desert_eagle::OnAdvanceLogic( f32 DeltaTime )
{
    new_weapon::OnAdvanceLogic( DeltaTime );

    // Are we in zoom mode.
    if( IsZoomEnabled() )
    {
        ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );
           
        // Get the player.
        player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

        if( m_ZoomState == WEAPON_STATE_ZOOM_IN )
        {
            // Zooming in.
            m_CurrentViewX -= m_ViewChangeRate * DeltaTime;
            m_CurrentViewX = fMax( m_CurrentViewX , m_ZoomFOV );     
        }
        else
        {
            // Zooming Out.
            m_CurrentViewX += m_ViewChangeRate * DeltaTime;
            m_CurrentViewX = fMin( m_CurrentViewX , OriginalViewInfo.XFOV );     
            if( m_CurrentViewX >= OriginalViewInfo.XFOV )
                ClearZoom();
        }
    }
}

//==============================================================================

void weapon_desert_eagle::ProcessSfx( void )
{
/*
    // Check all events in current animation
    for (s32 i = 0; i < m_AnimPlayer[m_CurrentRenderState].GetNEvents(); i++)
    {
        // Is this event active?
        if (m_AnimPlayer[m_CurrentRenderState].IsEventActive(i))
        {
            // Lookup event and world position
            const anim_event& Event  = m_AnimPlayer[m_CurrentRenderState].GetEvent(i) ;
            vector3           Pos    = m_AnimPlayer[m_CurrentRenderState].GetEventPosition(i) ;

            // Old event?
            if (x_stricmp(Event.GetType(), "Old Event") == 0)
            {
                switch(Event.GetInt( anim_event::INT_IDX_OLD_TYPE ))
                {
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_LR:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_LL:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_UR:
                    case ANIM_EVENT_WPN_PRIMARY_FIRE_UL:
                    case ANIM_EVENT_PRIMARY_FIRE:
                        //g_AudioManager.Play( "Shotgun_Primary_Fire", Pos );
                        g_AudioManager.Play( "Shotgun_Primary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                        GetZone1(), GetGuid() );

                    break;
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_LL:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UR:
                    case ANIM_EVENT_WPN_SECONDARY_FIRE_UL:
	                case ANIM_EVENT_SECONDARY_FIRE:
                        //g_AudioManager.Play( "Shotgun_Secondary_Fire", Pos );
                        g_AudioManager.Play( "Shotgun_Secondary_Fire", audio_manager::GUN_SHOT, GetPosition(), 
                                        GetZone1(), GetGuid() );
                    break;
                    case ANIM_EVENT_SFX_WEAPONLOOP:
                        g_AudioManager.Play( "Shotgun_Primary_Fire", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_TO:
                        g_AudioManager.Play( "Shotgun_Ready", Pos );
                    break;
                    case ANIM_EVENT_SFX_SWITCH_FROM:
                        g_AudioManager.Play( "Shotgun_Unready", Pos );
                    break;
                    case ANIM_EVENT_SFX_RELOAD:
                        g_AudioManager.Play( "Shotgun_Reload", Pos );
                    break;
                    case ANIM_EVENT_SFX_ALT_RELOAD:
                        g_AudioManager.Play( "Shotgun_Reload", Pos );
                    break;
                    default:
                        x_DebugMsg( "Anim Event: Shotgun Event[%d]\n", Event.GetInt( anim_event::INT_IDX_OLD_TYPE ) );
                    break;
                }
            }
        }
    }
*/
}

//==============================================================================

void weapon_desert_eagle::OnEnumProp( prop_enum& List )
{
    new_weapon::OnEnumProp( List );

    List.PropEnumHeader  ( "Desert Eagle", "", 0 );
    List.PropEnumAngle   ( "Desert Eagle\\Zoom FOV", "Whats the field of view when this weapon is zoomed in", 0 );
    List.PropEnumFloat   ( "Desert Eagle\\View Change Rate", "How fast is the view going to change.", 0 );
    List.PropEnumFloat   ( "Desert Eagle\\Zoom Stick Sensitivity", "How sensitive is the stick going to be when zoomed in", 0 );
    List.PropEnumFloat   ( "Desert Eagle\\Zoom Movement Speed", "How much to slow the player down by.", 0 );
}

//==============================================================================

xbool weapon_desert_eagle::OnProperty( prop_query& rPropQuery )
{
    if( new_weapon::OnProperty( rPropQuery ) )
    {
#ifndef X_EDITOR
        // in multiplayer, set ammo amounts.
        if( GameMgr.IsGameMultiplayer() )
        {
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 40;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = 24;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 8;
        }
#endif
        return TRUE;
    }

    if( rPropQuery.IsVar( "Desert Eagle\\Zoom FOV" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarAngle( m_ZoomFOV );
        }
        else
        {
            m_ZoomFOV = rPropQuery.GetVarAngle();
        }    
        return TRUE;
    }

    if( rPropQuery.VarFloat( "Desert Eagle\\View Change Rate", m_ViewChangeRate ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Desert Eagle\\Zoom Stick Sensitivity", m_StickSensitivity, 0.1f, 1.0f ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Desert Eagle\\Zoom Movement Speed", m_ZoomMovementSpeed, 0.1f, 1.0f ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

xbool weapon_desert_eagle::GetFlashlightTransformInfo( matrix4& incMatrix, vector3& incVect )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        return new_weapon::GetFlashlightTransformInfo( incMatrix, incVect );
    }
    else
    {
        incMatrix = GetL2W();
        incVect.Set( 0.0f, 1.0f, -23.0f );
        return TRUE;
    }
}

//==============================================================================
xbool weapon_desert_eagle::ShouldUpdateReticle( void )
{
    // if we are zoomed in, play sound
    if( m_ZoomStep > 0 )
    {
        return TRUE; 
    }

    return FALSE;
}

//=========================================================================
xbool weapon_desert_eagle::ShouldDrawReticle( void )
{
    return ( m_ZoomStep == 0 );
}