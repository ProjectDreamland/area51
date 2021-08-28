#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileBullett.hpp"
#include "WeaponSniper.hpp"
#include "e_View.hpp"
#include "Player.hpp"
#include "InputMgr\GamePad.hpp"
#include "Debris\debris_mgr.hpp"
#include "Objects\Projector.hpp"
#include "render\LightMgr.hpp"
#include "Objects\ParticleEmiter.hpp"

#ifndef X_EDITOR
#include "NetworkMgr\GameMgr.hpp"
#endif

//=========================================================================
// CONSTS
//=========================================================================
static const f32 s_ViewChangeRate = .187266f;       //number tested and agreed to by designers
static const f32 s_MinViewDimension = .0243645f;    //number tested and agreed to by designers

static f32 s_ViewClickChangeRate        = 1.0f;
static f32 s_SecondZoomStick            = 0.25f;
static f32 s_TenthZoomStick             = 0.2f;
static f32 s_FirstMagnification         = 2.0f;
static f32 s_SecondMagnification        = 10.0f;

static f32 s_MultiplayerSecondMagnification        = 6.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_sniper_rifle_desc : public object_desc
{

    weapon_sniper_rifle_desc( void ) : object_desc( object::TYPE_WEAPON_SNIPER, 
                                        "Sniper Rifle",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
                                        object::ATTR_NEEDS_LOGIC_TIME       |
                                        object::ATTR_SOUND_SOURCE           |
                                        object::ATTR_RENDERABLE,
                                        FLAGS_IS_DYNAMIC    
                                        )
    {

    }

//=========================================================================

    virtual object* Create          ( void )
    {
        return new weapon_sniper_rifle;
    }

} s_Weapon_Sniper_Rifle_Desc;

//=========================================================================

const object_desc&  weapon_sniper_rifle::GetTypeDesc     ( void ) const
{
    return s_Weapon_Sniper_Rifle_Desc;
}

//=========================================================================

const object_desc&  weapon_sniper_rifle::GetObjectType   ( void )
{
    return s_Weapon_Sniper_Rifle_Desc;
}


//=========================================================================

weapon_sniper_rifle::weapon_sniper_rifle( void ) :
    m_CurrentState( STATE_ZOOM_UNDEFINED ),
    m_RateOfViewChange( s_ViewChangeRate ), //play tested number decided by designers.
    m_ViewClickDimension( 0.0f )
{
    //initialize the ammo structures.
    m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_SNIPER_RIFLE;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 64;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 6;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;

    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip = 0;
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;

    m_NPCMuzzleSoundFx  = "Rifle_Primary_Fire";

    //set aim degradation
    m_AimDegradePrimary = 0.5f;
    m_AimRecoverSpeed = 0.5f;

    m_Item           = INVEN_WEAPON_SNIPER_RIFLE;
    m_SniperZoomSound   = 0;

    m_hMuzzleFXPrimary.SetName( PRELOAD_FILE("Muzzleflash_1stperson_Sniper_000.fxo") );

    m_nZoomSteps = 2;

    m_AutoSwitchRating = 2;
}

//=========================================================================

weapon_sniper_rifle::~weapon_sniper_rifle()
{
}

//=========================================================================

void weapon_sniper_rifle::InitWeapon            (   
                                                 const char* pSkinFileName , 
                                                 const char* pAnimFileName , 
                                                 const vector3& rInitPos , 
                                                 const render_state& rRenderState,
                                                 const guid& rParentGuid)
{
    //initialize the base class
    new_weapon::InitWeapon( pSkinFileName , pAnimFileName , rInitPos , rRenderState , rParentGuid);

    if ( rRenderState == RENDER_STATE_PLAYER )
    {
        SetState( STATE_NO_ZOOM );
    }
}

//===========================================================================

void weapon_sniper_rifle::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    new_weapon::InitWeapon( rInitPos, rRenderState, OwnerGuid );

    if ( rRenderState == RENDER_STATE_PLAYER )
    {
        SetState( STATE_NO_ZOOM );
    }
}

//==============================================================================

void weapon_sniper_rifle::ResetWeapon( void )
{
    SetState( STATE_NO_ZOOM );
}

//==============================================================================

void weapon_sniper_rifle::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "sniper_rifle::OnAdvanceLogic" );

    //update the base class
    new_weapon::OnAdvanceLogic( DeltaTime );

    //if the player is holding the sniper rifle, check for zoom mode
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        UpdateState( DeltaTime );
    }
}

//==============================================================================

void weapon_sniper_rifle::RenderWeapon( xbool bDebug, const xcolor& Ambient, xbool Cloaked )
{
    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );

    if( pOwner && (pOwner->GetType() == object::TYPE_PLAYER) )
    {
        player& Player = player::GetSafeType( *pOwner );
        if( Player.RenderSniperZoom() )
        {
            return;
        }
    }
    new_weapon::RenderWeapon( bDebug, Ambient, Cloaked );
}

//==============================================================================

void weapon_sniper_rifle::BeginNoZoom( void )
{
    // Get View from player and reset it.
    
    if (m_CurrentRenderState == RENDER_STATE_PLAYER)
    { 
        object_ptr<player> PlayerObj(m_ParentGuid);

        if (PlayerObj.IsValid())
        {
            PlayerObj.m_pObject->ResetView();
            PlayerObj.m_pObject->ResetStickSensitivity();
        }
    }

    m_EnableMuzzleFx = TRUE;
}

//==============================================================================

void weapon_sniper_rifle::UpdateNoZoom( const f32& rDeltaTime )
{
    (void)rDeltaTime;

}

//==============================================================================

void weapon_sniper_rifle::BeginZooming( void )
{
    if (m_CurrentRenderState == RENDER_STATE_PLAYER)
    { 
        object_ptr<player> PlayerObj(m_ParentGuid);
        
        if (PlayerObj.IsValid())
        {
            view& rView = PlayerObj.m_pObject->GetView();

            m_CurrentViewX = rView.GetXFOV() - R_20;
            m_CurrentViewY = rView.GetYFOV() - R_20;
        }
    }
    
    //Disable muzzle flash for zoom mode, as it gets in the way..
    m_EnableMuzzleFx = FALSE;
}

//==============================================================================

void weapon_sniper_rifle::UpdateZooming( const f32& rDeltaTime )
{
    m_CurrentViewX -= s_ViewClickChangeRate * rDeltaTime;
    m_CurrentViewY -= s_ViewClickChangeRate * rDeltaTime;

    m_CurrentViewX = fMax( m_CurrentViewX , m_ViewClickDimension );     
    m_CurrentViewY = fMax( m_CurrentViewY , m_ViewClickDimension );     //number tested and agreed to by designers

}

//==============================================================================

void weapon_sniper_rifle::UpdateWaitForReset( const f32& rDeltaTime )
{
    (void)rDeltaTime;
}

//==============================================================================

void weapon_sniper_rifle::SetState( const sniper_state& rNewState )
{
    if ( m_CurrentState == rNewState )
        return;

    EndState();

    m_CurrentState = rNewState;

    BeginState();
}

//==============================================================================

void weapon_sniper_rifle::BeginState( void )
{
    switch ( m_CurrentState )
    {
        case STATE_NO_ZOOM:
            BeginNoZoom();
            break;

        case STATE_ZOOMING:
            BeginZooming();
            break;

        case STATE_WAITING_FOR_RESET:
            break;
            
        default:
            break;
    }
}

//==============================================================================

void weapon_sniper_rifle::UpdateState( const f32& rDeltaTime )
{
    switch ( m_CurrentState )
    {
        case STATE_NO_ZOOM:
            UpdateNoZoom( rDeltaTime );
            break;

        case STATE_ZOOMING:
            UpdateZooming( rDeltaTime );
            break;

        case STATE_WAITING_FOR_RESET:
            UpdateWaitForReset( rDeltaTime );
            break;
    
        default:
            break;
    }
}

//==============================================================================

void weapon_sniper_rifle::EndState( void )
{
    switch ( m_CurrentState )
    {
        case STATE_NO_ZOOM:
            break;

        case STATE_ZOOMING:
            ClearZoom();
            break;

        case STATE_WAITING_FOR_RESET:
            break;

        default:
        break;
    }
}

//=========================================================================

xbool weapon_sniper_rifle::FireWeaponProtected( const vector3& InitPos , const vector3& InheritedVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    CONTEXT( "sniper_rifle::FireWeapon" );

    ( void )InitPos;
    ( void )Power;
    ( void )iFirePoint;
        
    ASSERT( m_FiringPointBoneIndex[ iFirePoint ] != -1 );

    //if there weapon is not ready, do nothing.
    if ( ! IsWeaponReady( AMMO_PRIMARY ) )
    {
        return FALSE;
    }

    //otherwise, create a new bullet projectile, init it's position, and send it on it's way.
    else
    {
        // Create the bullet.
        base_projectile* pBullet;
        ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

        object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
        if( !pOwnerObject )
            return FALSE;

        char DescName[64] = {"\0"};

        // zoomed in... normal pain profile
        if( m_ZoomStep > 0 )
        {
            x_sprintf( DescName, "%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName() );
        }
        else
        {
            x_sprintf( DescName, "%s_%s_NOZOOM",pOwnerObject->GetLogicalName(),GetLogicalName() );        
        }

        pain_handle PainHandle( DescName );
        pBullet = CreateBullet( GetLogicalName(), InitPos, InitRot, InheritedVelocity, Owner, PainHandle );

        ASSERT( pBullet );

        pBullet->SetThroughGlass( TRUE );

//    if( IsGameMultiplayer() && g_MultiplayerPainTweaks.m_bUseTweaks )
//            pBullet->SetDamageAmount( g_MultiplayerPainTweaks.m_SniperDamage );

        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

        // decrement count of bullets in current clip
        DecrementAmmo();

        return TRUE;
    }
}

//==============================================================================

xbool weapon_sniper_rifle::FireNPCWeaponProtected ( const vector3& InheritedVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, xbool isHit )
{   
    //if there weapon is not ready, do nothing.
    if ( ! IsWeaponReady( AMMO_PRIMARY ) )
    {
        return FALSE;
    }
    
    vector3 InitPos;
    
    if (!GetFiringBonePosition(InitPos))
        return FALSE;
    
    vector3 TargetVector   = Target - InitPos ;
    radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );

    DegradeAim( TargetRot, R_1*fDegradeMultiplier, InitPos, Owner );

    base_projectile* pBullet;

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return FALSE;
    
    char DescName[64] = {"\0"};

    // zoomed in... normal pain profile
    if( m_ZoomStep > 0 )
    {
        x_sprintf( DescName, "%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName() );
    }
    else
    {
        x_sprintf( DescName, "%s_%s_NOZOOM",pOwnerObject->GetLogicalName(),GetLogicalName() );        
    }

    pain_handle PainHandle( DescName );
    pBullet = CreateBullet( GetLogicalName(), InitPos, TargetRot, InheritedVelocity, Owner, PainHandle, AMMO_PRIMARY, isHit );
     
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

//=========================================================================

xbool weapon_sniper_rifle::FireNPCSecondaryProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, xbool isHit )
{
    ( void ) BaseVelocity;
    ( void ) Target;
    ( void ) Owner;
    ( void ) fDegradeMultiplier;
    ( void ) isHit;

    return FALSE;
}

//==============================================================================

s32 weapon_sniper_rifle::IncrementZoom( void )
{
    s32 ZoomStep = new_weapon::IncrementZoom();

    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_OwnerGuid ) );

    switch ( ZoomStep )
    {
    case 0:
        ClearZoom();
        SetState( STATE_NO_ZOOM );
        break;

    case 1:
    {
        view& rView = Player.GetView();

        m_ViewClickDimension = rView.GetXFOV() / s_FirstMagnification;
        Player.ResetStickSensitivity();
        Player.SetStickSensitivity( s_SecondZoomStick ); 

        // Release the current zoom sound, if any, play the next zoom sound.
        if( g_AudioMgr.IsValidVoiceId( m_SniperZoomSound ) )
            g_AudioMgr.Release( m_SniperZoomSound, 0.0f );
        m_SniperZoomSound = g_AudioMgr.Play( "SNI_Zoom_x2" );

        SetState( STATE_ZOOMING );

        break;
    }
    case 2:
    {
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();
        if( m_ViewClickDimension == (OriginalViewInfo.XFOV / s_FirstMagnification) )
        {
#ifndef X_EDITOR
            // if this is multiplayer, get different second zoom FOV
            if( GameMgr.IsGameMultiplayer() )
            {
                m_ViewClickDimension = OriginalViewInfo.XFOV / s_MultiplayerSecondMagnification;
            }
            else
#endif
            // not multiplayer or in the editor... continue as normal
            {
                m_ViewClickDimension = OriginalViewInfo.XFOV / s_SecondMagnification;
            }

            //m_ViewClickDimension = s_TenthZoomLevel; 
            Player.ResetStickSensitivity();
            Player.SetStickSensitivity( s_TenthZoomStick ); 

            // Release the current zoom sound, if any, play the next zoom sound.
            if( g_AudioMgr.IsValidVoiceId( m_SniperZoomSound ) )
                g_AudioMgr.Release( m_SniperZoomSound, 0.0f );
            m_SniperZoomSound = g_AudioMgr.Play( "SNI_Zoom_x10" );

            SetState( STATE_ZOOMING );
        }
        else if( m_ViewClickDimension == (OriginalViewInfo.XFOV / 10.0f) )
        {
            ClearZoom();
            SetState( STATE_NO_ZOOM );
        }

        break;
    }
    default:
        ASSERT( FALSE );
    }

    return ZoomStep;
}

//==============================================================================

void weapon_sniper_rifle::ClearZoom( void )
{
    new_weapon::ClearZoom();

    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_OwnerGuid ) );
    Player.ResetStickSensitivity();

    m_ViewClickDimension = 0.0f;

    // Release the current zoom sound, if any
    if( g_AudioMgr.IsValidVoiceId( m_SniperZoomSound ) )
    {
        g_AudioMgr.Release( m_SniperZoomSound, 0.0f );
    }

}

//==============================================================================
xbool weapon_sniper_rifle::ShouldUpdateReticle( void )
{
    // if we are zoomed in, play sound
    if( m_ZoomStep > 0 )
    {
        return TRUE; 
    }

    return FALSE;
}

//==============================================================================
xbool weapon_sniper_rifle::OnProperty( prop_query& rPropQuery )
{
    if( new_weapon::OnProperty( rPropQuery ) )
    {
#ifndef X_EDITOR
        // in multiplayer, set ammo amounts.
        if( GameMgr.IsGameMultiplayer() )
        {
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 12;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = 6;
            m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 6;
        }
#endif
        return TRUE;
    }

    return FALSE;
}