//=============================================================================================
// NEW WEAPON
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "NewWeapon.hpp"
#include "e_ScratchMem.hpp"
#include "Render\SkinGeom.hpp"

#include "Entropy\e_Draw.hpp"
#include "player.hpp"
#include "ProjectileBullett.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "objects\GrenadeProjectile.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "render\LightMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "hud_Player.hpp"
#include "HudObject.hpp"
#include "hud_Ammo.hpp"
#include "OccluderMgr\OccluderMgr.hpp"
#include "Characters\character.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

#if !defined(X_EDITOR)
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#endif
#include "Gamelib/DebugCheats.hpp"

//=========================================================================
// DEBUG
//=========================================================================

//#define DEBUG_NPC_WEAPON
//#define DEBUG_WEAPON_MUZZLE_FLASH
//#define DEBUG_CUSTOM_SCOPE_TEXTURE

#ifdef  DEBUG_WEAPON_MUZZLE_FLASH
static float    s_SphereRadius = 10.0f;
static sphere   s_EditSphere(vector3(0,0,0), s_SphereRadius);
#endif

#ifdef DEBUG_NPC_WEAPON
static vector3 WeaponPos(0,0,0);
static matrix4 WeaponMatrix;
#endif

#ifdef DEBUG_CUSTOM_SCOPE_TEXTURE
#define LOG_SCOPE_PATCHING  1
#else
#define LOG_SCOPE_PATCHING  0
#endif

//=========================================================================
// STATICS
//=========================================================================

s32 new_weapon::m_OrigScopeVramId    = -1;
s32 new_weapon::m_ScopeRefCount      = 0;
s32 new_weapon::m_ScopeTextureVramId = -1;

#ifndef X_EDITOR
extern xbool s_bDegradeAim;
#endif

//=========================================================================
// CONSTS
//=========================================================================

static const s32 kScopeTextureW = 64;
static const s32 kScopeTextureH = 64;

static const f32   k_npc_frag_scaling_const = 2.5f;
static vector3 s_ZeroVec( 0.0f, 0.0f, 0.0f );

// message timing tweaks
tweak_handle Ammo_Full_Msg_FadeTimeTweak  ("Ammo_Full_Msg_FadeTime");
tweak_handle Ammo_Full_Msg_DelayTimeTweak ("Ammo_Full_Msg_DelayTime");


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct new_weapon_desc : public object_desc
{
    new_weapon_desc( void ) : object_desc( object::TYPE_WEAPON, 
                                        "Weapon Item",
                                        "WEAPON",
                                        object::ATTR_SPACIAL_ENTRY          |
                                        object::ATTR_NEEDS_LOGIC_TIME       |
                                        object::ATTR_SOUND_SOURCE           |
                                        object::ATTR_RENDERABLE,
                                        FLAGS_IS_DYNAMIC
                                        )
    {

    }

    virtual object* Create          ( void )
    {
        return new new_weapon;
    }

} s_NewWeaponDesc;

//==============================================================================

const object_desc& new_weapon::GetTypeDesc     ( void ) const
{
    return s_NewWeaponDesc;
}

//==============================================================================

const object_desc& new_weapon::GetObjectType   ( void )
{
    return s_NewWeaponDesc;
}

//==============================================================================
// WEAPON_DUAL_SMP_CONTROLLER
//==============================================================================

dual_weapon_anim_controller::dual_weapon_anim_controller( void ) :
    m_iLWeaponBone  ( -1 ),
    m_iRWeaponBone  ( -1 ),
    m_WeaponGuid    ( 0  )
{
}

//==============================================================================

// Initializes
void dual_weapon_anim_controller::Init(       guid                WeaponGuid,
                                        const anim_group::handle& hAnimGroup,
                                        const char*               pLeftBone,
                                        const char*               pRightBone )
{
    // Keep weapon guid
    m_WeaponGuid = WeaponGuid;
    
    // Switch to new anim group
    SetAnimGroup( hAnimGroup );

    // Init bone indices
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        // Lookup bones
        m_iLWeaponBone = pAnimGroup->GetBoneIndex( pLeftBone );        
        m_iRWeaponBone = pAnimGroup->GetBoneIndex( pRightBone );
        
        // Make sure they are present
        ASSERTS( m_iLWeaponBone != -1, xfs( "%s bone is missing from NPC dual .skingeom!", pLeftBone ) );
        ASSERTS( m_iRWeaponBone != -1, xfs( "%s bone is missing from NPC dual .skingeom!", pRightBone ) );
    }
}

//==============================================================================

// Sets location of animation data package
void dual_weapon_anim_controller::SetAnimGroup( const anim_group::handle& hGroup )
{
    // Keep group
    m_hAnimGroup = hGroup;
}

//==============================================================================

void dual_weapon_anim_controller::MixKeys( anim_key* pDestKey )
{
    // Lookup owning weapon
    object_ptr<new_weapon> pWeapon( m_WeaponGuid );
    if( !pWeapon )
        return;
        
    // Lookup owning actor
    object_ptr<actor> pActor( pWeapon->GetParentGuid() );
    if( !pActor )
        return;

    // Lookup actor loco
    loco* pLoco = pActor->GetLocoPointer();
    if( !pLoco )
        return;

    // Compute inverse root bone L2W        
    matrix4 InvRootL2W;
    InvRootL2W.Setup( vector3( 1, 1, 1 ),
                      pDestKey[0].Rotation,
                      pDestKey[0].Translation );
    InvRootL2W.InvertRT();
                          
    // Overwrite left weapon bone (all child bones will follow since they are relative)
    if( m_iLWeaponBone != -1 )
    {
        // Get world space weapon transform and make relative to parent bone
        matrix4 L2W = InvRootL2W * pLoco->GetWeaponL2W( 1 );
        pDestKey[ m_iLWeaponBone ].Translation = L2W.GetTranslation();
        pDestKey[ m_iLWeaponBone ].Rotation    = L2W.GetRotation();
    }

    // Overwrite right weapon bone (all child bones will follow since they are relative)
    if( m_iRWeaponBone != -1 )
    {
        // Get world space weapon transform and make relative to parent bone
        matrix4 L2W = InvRootL2W * pLoco->GetWeaponL2W( 0 );
        pDestKey[ m_iRWeaponBone ].Translation = L2W.GetTranslation();
        pDestKey[ m_iRWeaponBone ].Rotation    = L2W.GetRotation();
    }
}

//==============================================================================
// NEW WEAPON
//==============================================================================

new_weapon::ammo::ammo( void ) :
    m_AmmoAmount( 1 ),
    m_AmmoMax( 1 ),
    m_AmmoPerClip( 1 ),
    m_ProjectileTemplateID( -1 ),
    m_ProjectileType( new_weapon::PROJECTILE_UNDEFINED )    
{
}
    
//==============================================================================

new_weapon::new_weapon( void ) :
    m_OwnerGuid( 0 ),
    m_Flags( ~0 ),
    m_Item( INVEN_NULL ),
    
    m_CurrentRenderState( new_weapon::RENDER_STATE_NPC ),
    m_EnableMuzzleFx(TRUE),
    m_ReticleCenterPixelOffset( 8.0f ),
    m_AimDegradePrimary(0.0f),
    m_AimDegradeSecondary(0.0f),
    m_AimRecoverSpeed(1.0f),
    m_ShortRange(500.0f),
    m_LongRange(2000.0f),
    m_AccuracyPercentLongRange(25),
    m_IsVisible(TRUE),
    m_HasSecondaryAmmo(FALSE),
    m_ParentGuid(NULL_GUID),
    m_bIdleMode(FALSE),
    m_ZoomState(NO_ZOOM_WEAPON_STATE),
    m_CurrentViewX(R_0),
    m_CurrentViewY(R_0),
//    m_DropFadeTime(10.0f),
    m_ZoomMovementSpeed(1.0f),
    m_ZoomStep( 0 ),
    m_nZoomSteps( 0 ),
    m_TargetGuid( NULL_GUID ),
    m_ScopeMesh( -1 ),
    m_ScopeLensMesh( -1 ),
    m_bLockedOn(FALSE),
    m_bCanWarnLowAmmo( TRUE ),     
    m_fLastAmmoFullTime( 0.0f ),  // put this at 0 and not current time so the message will hit the first time
    m_AutoSwitchRating( 0 )       // what is the auto-switch rating of this weapon?  Larger number = better weapon
{
    s32 i;
    for( i = 0; i < FIRE_POINT_COUNT; i++ )
    {
        m_FiringPointBoneIndex      [i] = -1;
        m_AltFiringPointBoneIndex   [i] = -1;
        m_AimPointBoneIndex         [i] = -1;
        m_AltAimPointBoneIndex      [i] = -1;
    }

    for( i = 0; i < MAX_FACTION_COUNT; i++ )
    {
        m_FactionFireSfx[i] = -1;
    }

    // initialize flashlight bone index
    m_FlashlightBoneIndex = -1;

    m_ReticleRadiusParameters.m_MaxRadius                  = 20.0f;
    m_ReticleRadiusParameters.m_MinRadius                  = 6.5f;     
    m_ReticleRadiusParameters.m_CrouchBonus                = 5.0f;     
    m_ReticleRadiusParameters.m_MaxMovementPenalty         = 8.0f;
    m_ReticleRadiusParameters.m_MoveShrinkAccel            = 75.0f;       
    m_ReticleRadiusParameters.m_ShotShrinkAccel            = 75.0f;       
    m_ReticleRadiusParameters.m_GrowAccel                  = 1000.0f;
    m_ReticleRadiusParameters.m_PenaltyForShot             = 4.0f;    
    m_ReticleRadiusParameters.m_ShotPenaltyDegradeRate     = 17.0f;

    m_AltReticleRadiusParameters.m_MaxRadius                  = 20.0f;
    m_AltReticleRadiusParameters.m_MinRadius                  = 6.5f;     
    m_AltReticleRadiusParameters.m_CrouchBonus                = 5.0f;     
    m_AltReticleRadiusParameters.m_MaxMovementPenalty         = 8.0f;
    m_AltReticleRadiusParameters.m_MoveShrinkAccel            = 75.0f;       
    m_AltReticleRadiusParameters.m_ShotShrinkAccel            = 75.0f;       
    m_AltReticleRadiusParameters.m_GrowAccel                  = 1000.0f;
    m_AltReticleRadiusParameters.m_PenaltyForShot             = 4.0f;    
    m_AltReticleRadiusParameters.m_ShotPenaltyDegradeRate     = 17.0f;

    m_LastPosition.Zero();
    m_Velocity.Zero();
    m_LastRotation = 0;
    m_AngularSpeed = 0;
    
#ifndef TARGET_PC
    if( m_ScopeTextureVramId == -1 )
    {
        // register the custom scope texture
        m_ScopeTextureVramId = vram_RegisterLocked( kScopeTextureW, kScopeTextureH, 32 );
    }
#endif
}
    
//==============================================================================

new_weapon::~new_weapon()
{
    KillAllMuzzleFX();
    EndPrimaryFire();
}

//===========================================================================

void new_weapon::InitWeapon( const vector3& rInitPos, render_state rRenderState, guid OwnerGuid )
{
    //set the render state
    m_CurrentRenderState = rRenderState;

    //set the owner of this weapon
    m_OwnerGuid = m_ParentGuid = OwnerGuid;

    // Get zones of owner
    {
        object* pOwner = g_ObjMgr.GetObjectByGuid( OwnerGuid );
        if( pOwner )
        {
            SetZone1( pOwner->GetZone1() );
            SetZone2( pOwner->GetZone2() );
        }
    }

    //change the attribs flags...
    switch( m_CurrentRenderState )
    {
    case RENDER_STATE_PLAYER:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_NULL );
        }
        break;
    case RENDER_STATE_NPC:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_SOUND_SOURCE );
        }
        break;
    default:
        //no-op
        break;
    }

    //initialize the animations for this animation group.
    if( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_AnimPlayer[m_CurrentRenderState].SetAnimGroup( m_AnimGroup[m_CurrentRenderState] );
        m_AnimPlayer[m_CurrentRenderState].SetAnim( 0, TRUE, TRUE );

        m_FiringPointBoneIndex[FIRE_POINT_DEFAULT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );
        m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );
        m_AimPointBoneIndex[FIRE_POINT_DEFAULT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint" );
        m_AltAimPointBoneIndex[FIRE_POINT_DEFAULT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint" );

        m_FiringPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_left" );
        m_AltFiringPointBoneIndex[FIRE_POINT_LEFT]  = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_left" );
        m_AimPointBoneIndex[FIRE_POINT_LEFT]        = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_left" );
        m_AltAimPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_left" );

        m_FiringPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_right" );
        m_AltFiringPointBoneIndex[FIRE_POINT_RIGHT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_right" );
        m_AimPointBoneIndex[FIRE_POINT_RIGHT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_right" );
        m_AltAimPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_right" );
        
        m_WeaponInited = TRUE;
    }
    else
    {
        m_WeaponInited = FALSE;
    }

    //set initial position of the object.
    OnMove( rInitPos );

    m_ZoomStep = 0;
}

//==============================================================================

void new_weapon::InitWeapon( const char* pSkinFileName , const char* pAnimFileName , const vector3& rInitPos , 
                            const render_state& rRenderState, const guid& rParentGuid )
{   
    //set the render state
    m_CurrentRenderState = rRenderState;

    //set the owner of this weapon
    m_OwnerGuid = m_ParentGuid = rParentGuid;

    // Get zones of owner
    {
        object* pOwner = g_ObjMgr.GetObjectByGuid( rParentGuid );
        if( pOwner )
        {
            SetZone1( pOwner->GetZone1() );
            SetZone2( pOwner->GetZone2() );
        }
    }

    //change the attribs flags...
    switch( m_CurrentRenderState )
    {
    case RENDER_STATE_PLAYER:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_NULL );
        }
        break;
    case RENDER_STATE_NPC:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_SOUND_SOURCE          | 
                         object::ATTR_RENDERABLE        );
        }
        break;
    default:
        //no-op
        break;
    }

    //don't re-initialize the animation group ever.
    if ( ! m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        // Initialize the skin
        m_Skin[ m_CurrentRenderState ].OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pSkinFileName ) );

        //set up the animation groups.
        m_AnimGroup[ m_CurrentRenderState ].SetName( pAnimFileName );

        //initialize the animations for this animation group.
        if ( m_AnimGroup[m_CurrentRenderState].GetPointer() )
        {
            m_AnimPlayer[m_CurrentRenderState].SetAnimGroup( m_AnimGroup[m_CurrentRenderState] );
            m_AnimPlayer[m_CurrentRenderState].SetAnim( 0, TRUE, TRUE );

            m_FiringPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );
            m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );
            m_AimPointBoneIndex[FIRE_POINT_DEFAULT]             = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint" );
            m_AltAimPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint" );

            m_FiringPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_left" );
            m_AltFiringPointBoneIndex[FIRE_POINT_LEFT]  = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_left" );
            m_AimPointBoneIndex[FIRE_POINT_LEFT]        = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_left" );
            m_AltAimPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_left" );

            m_FiringPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_right" );
            m_AltFiringPointBoneIndex[FIRE_POINT_RIGHT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_right" );
            m_AimPointBoneIndex[FIRE_POINT_RIGHT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_right" );
            m_AltAimPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_right" );

            m_WeaponInited = TRUE;
        }
        else
        {
            m_WeaponInited = FALSE;
        }
    }

    //set initial position of the object.
    OnMove( rInitPos ); 

    m_ZoomStep = 0;
}

//==============================================================================

void new_weapon::InitNPCDualAnimController(       dual_weapon_anim_controller* pController, 
                                            const char*                        pLeftRootBone, 
                                            const char*                        pRightRootBone )
{
    ASSERT( pController );
    ASSERT( pLeftRootBone );
    ASSERT( pRightRootBone );

    // Init the controller
    pController->Init( GetGuid(),                                           // WeaponGuid
                       m_AnimPlayer[RENDER_STATE_NPC].GetAnimGroupHandle(), // hAnimGroup
                       pLeftRootBone,                                       // pLeftBone
                       pRightRootBone );                                    // pRightBone

    // Add to animation player
    m_AnimPlayer[RENDER_STATE_NPC].SetTrackController( 1, pController );
}

//==============================================================================

xbool new_weapon::IsUsingSplitScreen( void )
{
#if defined( X_EDITOR )
    return( FALSE );
#else
    return( g_NetworkMgr.GetLocalPlayerCount() > 1 );
#endif
}

//==============================================================================

xbool new_weapon::IsWeaponReady( const ammo_priority& rAmmoPriority )
{
    return ( m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip > 0 );
}

//==============================================================================

s32 new_weapon::GetAmmoCount( void )
{
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip;
}

//==============================================================================

s32 new_weapon::GetSecondaryAmmoCount( void )
{
    return m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoInCurrentClip;
}

//==============================================================================

s32 new_weapon::GetAmmoPerClip( void )
{
    return m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
}

//==============================================================================

s32 new_weapon::GetSecondaryAmmoPerClip( void )
{
    return m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoPerClip;
}

//==============================================================================

s32 new_weapon::GetAmmoCount( ammo_priority Priority )
{
    return m_WeaponAmmo[ Priority ].m_AmmoInCurrentClip;
}

//==============================================================================

s32 new_weapon::GetAmmoPerClip( ammo_priority Priority )
{
    return m_WeaponAmmo[ Priority ].m_AmmoPerClip;
}

//==============================================================================

s32 new_weapon::GetAmmoAmount( ammo_priority Priority )
{
    return m_WeaponAmmo[ Priority ].m_AmmoAmount;
}

//==============================================================================

char *new_weapon::GetWeaponPrefixFromInvType2( inven_item WeaponItem )
{
    switch( WeaponItem )
    {
    case INVEN_WEAPON_DESERT_EAGLE:
        return "EGL";
        break;
    case INVEN_GRENADE_FRAG:
        return "FRG";
        break;
    case INVEN_GRENADE_GRAV:
        return "GRV";
        break;
    case INVEN_WEAPON_MESON_CANNON:
        return "MSN";
        break;

    case INVEN_WEAPON_BBG:
        return "BBG";
        break;

    case INVEN_WEAPON_TRA:
        return "TRA";
        break;

// KSS -- TO ADD NEW WEAPON
    case INVEN_WEAPON_SHOTGUN:
        return "SHT";
        break;
    case INVEN_WEAPON_SMP:
        return "SMP";
        break;
    case INVEN_WEAPON_SCANNER:
        return "SCN";
        break;
    case INVEN_WEAPON_SNIPER_RIFLE:
        return "SNI";
        break;
    case INVEN_WEAPON_MUTATION:
        return "MUT";
        break;
    }

    return NULL;
}

//==============================================================================

f32 new_weapon::GetAccuracyPercent( f32 distanceToTarget )
{
    if( distanceToTarget <= m_ShortRange )
    {
        return 1.0f;
    }
    else if( distanceToTarget >= m_LongRange )
    {
        return ((f32)m_AccuracyPercentLongRange)/100.0f;
    }
    else
    {
        // we are somewhere in between....
        f32 shortToLong = m_LongRange - m_ShortRange;
        f32 distToShort = distanceToTarget - m_ShortRange;
        f32 percentOfRange = distToShort/shortToLong;
        f32 finalAccuracy = 1.0f - (1.0f-(((f32)m_AccuracyPercentLongRange)/100.0f))*percentOfRange;
        return finalAccuracy;
    }
}

//==============================================================================
xbool new_weapon::GetFiringStartPosition( vector3 &Pos )
{
    (void) Pos;
    return FALSE;
}

//==============================================================================

xbool new_weapon::CanReload( const ammo_priority& Priority )
{
    return m_WeaponAmmo[Priority].m_AmmoAmount > m_WeaponAmmo[Priority].m_AmmoInCurrentClip && m_WeaponAmmo[Priority].m_AmmoInCurrentClip < m_WeaponAmmo[Priority].m_AmmoPerClip;
}

//==============================================================================

xbool new_weapon::CanFire( xbool bIsAltFire )
{
    s32 AmmoCount = 0;

    ammo_priority AmmoPriority 
        = bIsAltFire 
        ? GetSecondaryAmmoPriority() 
        : GetPrimaryAmmoPriority();
    AmmoCount = GetAmmoCount( AmmoPriority );

    return( AmmoCount > 0 );
}

//==============================================================================

void new_weapon::Reload( const ammo_priority& Priority )
{
    // For GameDay, we do not want Zach to run out of ammo.  Remove the next line after GameDay.
    //m_WeaponAmmo[Priority].m_AmmoAmount = m_WeaponAmmo[Priority].m_AmmoMax;
    s32 nNewAmmoCount = iMin( m_WeaponAmmo[Priority].m_AmmoAmount, m_WeaponAmmo[Priority].m_AmmoPerClip );

    m_WeaponAmmo[Priority].m_AmmoInCurrentClip = nNewAmmoCount;
}

//=========================================================================

void new_weapon::SetupDualAmmo( inven_item OtherWeaponItem )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

    // get our parent's pointer
    if( !pObj )
    {
        return;
    }

    actor* pParent = (actor*)pObj;

    // other weapon is the "parent" weapon, i.e. INVEN_WEAPON_SMP is the parent of INVEN_WEAPON_DUAL_SMP
    new_weapon* pOtherWeapon = pParent->GetWeaponPtr( OtherWeaponItem );

    // don't have it... get out.
    if( !pOtherWeapon )
    {
        return;
    }
    
    // get other weapon's ammo.
    s32 nCurrentAmmoCount  = pOtherWeapon->GetTotalPrimaryAmmo();
    s32 nAmmoBonus         = ( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip - pOtherWeapon->GetAmmoPerClip() );
    s32 nAmmoMissing       = ( pOtherWeapon->GetAmmoPerClip() - pOtherWeapon->GetAmmoCount() );

    // make sure our counts "match up".  We want this weapon to look like it just gave us X rounds for free
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = pOtherWeapon->GetMaxPrimaryAmmo() + nAmmoBonus;

    // give the new "dual" weapon the ammo from the "parent" weapon.
    // We have to do this the weird way because we are adding an absolute amount and there may be ammo missing.
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount         = nCurrentAmmoCount + nAmmoBonus + nAmmoMissing;
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip  = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
}

//=========================================================================

inven_item new_weapon::GetDualWeaponID( inven_item ParentItem )
{
    // get the dual item id for this parent object
    if( ParentItem == INVEN_WEAPON_SMP )
    {
        return INVEN_WEAPON_DUAL_SMP;
    }
    else
    if( ParentItem == INVEN_WEAPON_SHOTGUN )
    {        
        return INVEN_WEAPON_DUAL_SHT;
    }
    else
    if( ParentItem == INVEN_WEAPON_DESERT_EAGLE )
    {
        //return INVEN_WEAPON_DUAL_EAGLE;
    }

    return INVEN_NULL;
}

//=========================================================================

inven_item new_weapon::GetParentIDForDualWeapon( inven_item DualItem )
{
    // get the dual item id for this parent object
    if( DualItem == INVEN_WEAPON_DUAL_SMP )
    {
        return INVEN_WEAPON_SMP;
    }
    else
    if( DualItem == INVEN_WEAPON_DUAL_SHT )
    {        
        return INVEN_WEAPON_SHOTGUN;
    }
    else
    if( DualItem == INVEN_WEAPON_DUAL_EAGLE )
    {
        //return INVEN_WEAPON_DESERT_EAGLE;
    }

    return INVEN_NULL;
}

//=========================================================================

xbool new_weapon::FireWeapon( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    xbool Rval = FireWeaponProtected( InitPos, BaseVelocity, Power, InitRot, Owner, iFirePoint );

    if ( Rval == TRUE )
    {
        // Start the muzzle FX
        InitMuzzleFx( TRUE, iFirePoint );

        //if this is the player, degrade aim
        if (m_CurrentRenderState == RENDER_STATE_PLAYER)
        {
            object_ptr<player> PlayerObj(m_ParentGuid);
            
            if (PlayerObj.IsValid())
            {
                PlayerObj.m_pObject->DegradeAim(m_AimDegradePrimary);
                PlayerObj.m_pObject->SetAimRecoverSpeed(m_AimRecoverSpeed);
            }
        }
    }

    m_TargetGuid = NULL_GUID;

    return Rval;
}

//=========================================================================

xbool new_weapon::FireSecondary( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    xbool Rval = FireSecondaryProtected( InitPos, BaseVelocity, Power, InitRot, Owner, iFirePoint );

    if ( Rval == TRUE )
    {
        // Start the muzzle FX
        InitMuzzleFx( FALSE, iFirePoint );

        //if this is the player, degrade aim
        if (m_CurrentRenderState == RENDER_STATE_PLAYER)
        {
            object_ptr<player> PlayerObj(m_ParentGuid);
            
            if (PlayerObj.IsValid())
            {
                PlayerObj.m_pObject->DegradeAim(m_AimDegradeSecondary);
                PlayerObj.m_pObject->SetAimRecoverSpeed(m_AimRecoverSpeed);
            }
        }
    }
    
    return Rval;
}

//==============================================================================

xbool new_weapon::NPCFireWeapon( const vector3& BaseVelocity , const vector3& Target, const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{
    // If the weapon has no ammo left, do nothing.
    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip <= 0 )
    {
        return FALSE ;
    }

    xbool Rval = FireNPCWeaponProtected( BaseVelocity, Target, Owner, fDegradeMultiplier, isHit );

    if ( Rval == TRUE )
    {
        // Start the muzzle FX
        InitMuzzleFx( TRUE, FIRE_POINT_DEFAULT );

        // decrement count of bullets in current clip
        DecrementAmmo();
    }

    return Rval ;
}

//==============================================================================

void new_weapon::NPCFireSecondary ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit )
{
    xbool Rval = FireNPCSecondaryProtected( BaseVelocity, Target, Owner, fDegradeMultiplier, isHit );

    if( Rval )
    {
        // Start the muzzle FX
        InitMuzzleFx( FALSE, FIRE_POINT_DEFAULT );
    }
}

//==============================================================================

xbool new_weapon::FireGhostPrimary( s32 iFirePoint, xbool bUseFireAt, vector3& FireAt )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    // Ran out of ammo?
    if ( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID < 0 )
    {
        return FALSE;
    }

    // Start the muzzle FX
    InitMuzzleFx( TRUE, iFirePoint );

    // No fire point?
    if( m_FiringPointBoneIndex[ iFirePoint ] == -1 )
        return FALSE;

    // Compute position and direction of bullet to create                
    matrix4 Mat     = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[ iFirePoint ] );
    vector3 InitPos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iFirePoint ] );

    // THIS IS BAD, THANK YOU MAX EXPORTER.
    Mat.PreRotateY( R_180 );
    radian3 InitRot     = Mat.GetRotation();

    if( bUseFireAt )
    {
        vector3 AimVector = FireAt - InitPos;
        AimVector.GetPitchYaw( InitRot.Pitch, InitRot.Yaw );
    }

    // Create bullet
    pain_handle PainHandle( xfs("PLAYER_%s",GetLogicalName()) );

#ifndef X_EDITOR
    if( s_bDegradeAim )
    {
        vector3 Dir(0,0,1);
        Dir.RotateX( R_3*x_frand(-1.0f,1.0f) ); // Pitch Z-axis up or down by spread angle
        Dir.RotateZ( x_frand(0,R_360) );        // Roll dir around Z
        Dir.RotateX( InitRot.Pitch );           // Orient around original direction
        Dir.RotateY( InitRot.Yaw   );

        f32 P,Y;
        Dir.GetPitchYaw(P,Y);

        InitRot = radian3( P, Y, 0 );
    
        // Reset so next bullet doesn't try to degrade its aim.
        s_bDegradeAim = FALSE;
    }
#endif

    CreateBullet( GetLogicalName(), InitPos, InitRot, s_ZeroVec, m_ParentGuid, PainHandle, AMMO_PRIMARY, TRUE, iFirePoint );

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
        // Do not generate bullet sounds for the SMP since it's built into the loop
        if( ( m_Item != INVEN_WEAPON_SMP ) && ( m_Item != INVEN_WEAPON_DUAL_SMP ) )
        {
            voice_id VoiceID = g_AudioMgr.Play( g_StringMgr.GetString( m_FactionFireSfx[ BitIndex ] ), 
                                                GetPosition(), GetZone1(), TRUE );
            g_AudioManager.NewAudioAlert( VoiceID, 
                                        audio_manager::GUN_SHOT, 
                                        GetPosition(), 
                                        GetZone1(), 
                                        GetGuid() );
        }
    }
        
    return TRUE;
}

//==============================================================================

xbool new_weapon::FireGhostSecondary( s32 iFirePoint )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );

    vector3 Dummy( 0.0f, 0.0f, 0.0f );
    xbool RetVal = FireGhostPrimary( iFirePoint, FALSE, Dummy );
    
    return RetVal;
}
 
//==============================================================================

xbool new_weapon::FireNPCWeaponProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
    //derived classes should overload this to fire the approriate bullets..
    
    (void)BaseVelocity;
    (void)Target;
    (void)Owner;
    (void)fDegradeMultiplier;
    (void)isHit;
    
    return FALSE;
}

//==============================================================================

xbool new_weapon::FireNPCSecondaryProtected ( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
    //derived classes should overload this to fire the approriate bullets..
    
    (void)BaseVelocity;
    (void)Target;
    (void)Owner;
    (void)fDegradeMultiplier;
    (void)isHit;

    return FALSE;
}

//==============================================================================

xbool new_weapon::FireWeaponProtected     ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint  )
{
    //derived classes should overload this to fire the approriate bullets..

    (void)InitPos;
    (void)BaseVelocity;
    (void)Power;
    (void)InitRot;
    (void)Owner;
    (void)iFirePoint;
    
    return FALSE;
}

//==============================================================================

xbool new_weapon::FireSecondaryProtected    ( const vector3& InitPos , const vector3& BaseVelocity , const f32& Power, const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{   
    //derived classes should overload this to fire the approriate bullets..

    (void)InitPos;
    (void)BaseVelocity;
    (void)Power;
    (void)InitRot;
    (void)Owner;
    (void)iFirePoint;
    
    return FALSE;
}

//==============================================================================

f32 new_weapon::GetFrameParametric( void )
{
    return m_AnimPlayer[ m_CurrentRenderState ].GetFrameParametric();
}

//==============================================================================

void new_weapon::SetFrameParametric( f32 Frame )
{
    m_AnimPlayer[ m_CurrentRenderState ].SetFrameParametric( Frame );
}
  
//==============================================================================

void new_weapon::OnRender( void )
{
    // If no one owns this weapon, render it.
    if ( m_OwnerGuid == NULL_GUID && m_CurrentRenderState == RENDER_STATE_NPC )
    {
        if( g_OccluderMgr.IsBBoxOccluded( GetBBox() ) )
            return;

        //get the current anim group that needs to be rendered.
        anim_group::handle& CurAnimGroup = m_AnimGroup[ m_CurrentRenderState ];
        
        if( CurAnimGroup.GetPointer() && m_Skin[ m_CurrentRenderState ].GetSkinGeom() )
        {
            u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

            s32            nBones    = m_AnimPlayer[ m_CurrentRenderState ].GetNBones();
            matrix4*       pBone     = (matrix4*)smem_BufferAlloc( nBones * sizeof( matrix4 ) );
            const matrix4* pAnimBone = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2Ws();

            for( s32 i=0; i<nBones; i++ )
            {
                pBone[i] = pAnimBone[i];
            }

            skin_inst& SkinInst = m_Skin[ m_CurrentRenderState ] ;
            SkinInst.Render( &GetL2W(),
                            pBone,
                            nBones,
                            Flags | GetRenderMode(),
                            SkinInst.GetLODMask(GetL2W()) );
        }        
        else
        {
//            draw_BBox( GetBBox() );
        }
    }

    else
    {
//        draw_BBox( GetBBox() );
    }
}

//===========================================================================
xbool g_DrawWeaponFireBone = FALSE;
void new_weapon::OnRenderTransparent(void)
{
    // default behavior
    object::OnRenderTransparent();

    // render all muzzle flashes
    RenderMuzzleFx();

#ifndef X_RETAIL
    #ifdef ksaffel
        if( g_DrawWeaponFireBone )
        {
            /*
            vector3 BonePos;
            GetFiringBonePosition(BonePos);
            draw_Sphere(BonePos, 5.0f);
            draw_Line(GetPosition(), BonePos);
            */

            //render aim points
            vector3 AimPos;
            vector3 FirePos;
            if (GetAimBonePosition(AimPos) && GetFiringBonePosition(FirePos))
            {
                vector3 TargetingDir = FirePos - AimPos;
                TargetingDir.Normalize();
                TargetingDir *= 2000; //give it some distance

                draw_Line(FirePos, FirePos + TargetingDir);

                draw_Sphere(FirePos, 6.0f, XCOLOR_GREEN);
                draw_Sphere(AimPos, 6.0f, XCOLOR_RED);
            }
        }
    #endif
#endif
}

//==============================================================================

void new_weapon::RenderWeapon( xbool bDebug, const xcolor& Ambient, xbool Cloaked )
{   
    (void)bDebug;
    if (!m_IsVisible)
        return;

    if( g_OccluderMgr.IsBBoxOccluded( GetBBox() ) )
        return;

#ifdef DEBUG_WEAPON_MUZZLE_FLASH
/*
    if (m_FiringPointBoneIndex!=-1)
    {
        const matrix4 BoneL2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex );
        draw_Sphere( BoneL2W.GetTranslation(), s_SphereRadius, xcolor(255,0,0) );

        SMP_UTIL_draw_MatrixAxis( BoneL2W );
    }
*/
#endif


#ifdef DEBUG_NPC_WEAPON
     draw_Sphere( WeaponPos, 100.0f, xcolor(255,0,0) );
     SMP_UTIL_draw_MatrixAxis( WeaponMatrix );
#endif


#ifdef X_EDITOR
     if (bDebug)
     {
        //render aim points
         vector3 AimPos;
         vector3 FirePos;
         if (GetAimBonePosition(AimPos) && GetFiringBonePosition(FirePos))
         {
            vector3 TargetingDir = FirePos - AimPos;
            TargetingDir.Normalize();
            TargetingDir *= 2000; //give it some distance

            draw_Line(FirePos, FirePos + TargetingDir);

            draw_Sphere(FirePos, 6.0f, XCOLOR_GREEN);
            draw_Sphere(AimPos, 6.0f, XCOLOR_RED);
         }
     }
#endif // X_EDITOR

    //get the current anim group that needs to be rendered.
    anim_group::handle& CurAnimGroup = m_AnimGroup[ m_CurrentRenderState ];

    if( CurAnimGroup.GetPointer() && m_Skin[ m_CurrentRenderState ].GetSkinGeom() )
    {
        // if we are owned by a player, then we need to ask for his offset
        object*        pOwner    = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
        vector3        Offset    ( 0.0f, 0.0f, 0.0f );
        if ( pOwner && pOwner->IsKindOf( player::GetRTTI() ) )
        {
            Offset = ((player*)pOwner)->GetCurrentWeaponCollisionOffset();
        }

        // accumulate clipping flags and whether or not we should glow
        u32 Flags;
        if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            Flags = render::CLIPPED | render::DISABLE_SPOTLIGHT;
            if ( g_RenderContext.m_bIsMutated && !g_RenderContext.m_bIsPipRender )
                Flags |= render::GLOWING;
        }
        else
        {
            const anim_group* pAnimGroup = CurAnimGroup.GetPointer();
            bbox RenderBBox = pAnimGroup->GetBBox();
            RenderBBox.Transform( GetL2W() );
            RenderBBox.Translate( Offset );

            // Perform clipping test, and skip render if outside the view
            s32 InView = g_ObjMgr.IsBoxInView( RenderBBox, XBIN(111111) );
            if( InView == -1 )
                return;
                
            // "InView" contains the # of planes straddled, so if it's not zero, clipping is needed
            Flags = ( InView != 0 ) ? render::CLIPPED : 0;
        }

        s32            nBones    = m_AnimPlayer[ m_CurrentRenderState ].GetNBones();
        matrix4*       pBone     = (matrix4*)smem_BufferAlloc( nBones * sizeof( matrix4 ) );
        const matrix4* pAnimBone = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2Ws();

        // if our owner is spawning, we need ALPHA for fading
        if (   pOwner 
            && pOwner->IsKindOf( actor::GetRTTI() ) 
            && (((actor*)pOwner)->GetSpawnFadeTime() > 0.0f) )
        {
            Flags |= render::FADING_ALPHA;
        }

        for( s32 i=0; i<nBones; i++ )
        {
            pBone[i] = pAnimBone[i];
            pBone[i].Translate( Offset );
        }

        skin_inst& SkinInst = m_Skin[ m_CurrentRenderState ] ;

        if ( Cloaked )
        {
            SkinInst.RenderDistortion( &GetL2W(),
                                       pBone,
                                       nBones,
                                       Flags,
                                       SkinInst.GetLODMask(GetL2W()),
                                       radian3(R_0,R_0,R_0),
                                       Ambient );
        }
        else
        {
            u64 MeshMask = SkinInst.GetLODMask( GetL2W() );

            // render the scope mesh if one is there
            if ( (m_ScopeMesh != -1)            &&
                 (MeshMask & (1<<m_ScopeMesh)) )
            {
                u32 ScopeMask = (1<<m_ScopeMesh);
                if ( m_ScopeLensMesh != -1 )
                    ScopeMask |= (1<<m_ScopeLensMesh);

                // render the scope mesh
                MeshMask &= ~ScopeMask;
                SkinInst.Render( &GetL2W(),
                                 pBone,
                                 nBones,
                                 Flags | render::FORCE_LAST,
                                 ScopeMask,
                                 Ambient );
            }

            // render the normal mesh
            SkinInst.Render( &GetL2W(),
                             pBone,
                             nBones,
                             Flags,
                             MeshMask,
                             Ambient );
        }
    }
}

//==============================================================================
f32 s_LowAmmoPercent     = 0.3f;
f32 s_LowAmmoMsgFadeTime = 2.5f;
void new_weapon::OnAdvanceLogic( f32 DeltaTime )
{
    const vector3 Position = GetPosition();
    m_Velocity = (Position - m_LastPosition) / DeltaTime;
    m_LastPosition = Position;

    const radian Rot = GetYaw();
    m_AngularSpeed = (Rot - m_LastRotation) / DeltaTime;
    m_LastRotation = Rot;

    vector3 Pos;
   
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        if ( m_AnimGroup[m_CurrentRenderState].GetPointer() )
        {
            //x_printfxy(0,4, "%s", m_AnimPlayer[m_CurrentRenderState].GetAnimInfo().GetName() );
            m_AnimPlayer[m_CurrentRenderState].Advance( DeltaTime , Pos );
        }

        // do we play locked-on sound
        UpdateReticle( DeltaTime );
    }
    
    xbool bHandleSuperEvents = TRUE;
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );

    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        // if this is a player and we're running a cinema, don't advance or handle logic 
        if( pPlayer && (pPlayer->IsCinemaRunning()) )
        {
            bHandleSuperEvents = FALSE;
            KillAllMuzzleFX();
        }
    }

    // should we handle super events?
    if( bHandleSuperEvents )
    {
        g_EventMgr.HandleSuperEvents( m_AnimPlayer[m_CurrentRenderState], this );
    }

    if( DEBUG_INFINITE_AMMO )
    {
        FillAmmo();
    }

    // make sure we advance the particle logic for muzzle flashes
    AdvanceMuzzleFX(DeltaTime);

/*
    if( m_ItemDroped && (m_DropFadeTime != -1.0f) )
    {
        m_DropFadeTime -= DeltaTime;
        if( m_DropFadeTime <= 0.0f )
        {
            g_ObjMgr.DestroyObject( GetGuid() );
        }
    }
*/

    //ProcessSfx();
}

//==============================================================================

void new_weapon::NotifyAmmoFull( player *pPlayer )
{
#if !defined(X_EDITOR)
    f32 currentTime = (f32)x_GetTimeSec();

    // make sure we don't flood the player with info.
    if( (currentTime - m_fLastAmmoFullTime) > Ammo_Full_Msg_DelayTimeTweak.GetF32() )
    {
        // tell the player their ammo is full
        MsgMgr.Message( MSG_FULL_AMMO, pPlayer->net_GetSlot(), m_Item );

        // reset time
        m_fLastAmmoFullTime = currentTime;
    }

#endif
}

//==============================================================================

void new_weapon::DegradeAim( radian3& Rot, radian Amt, const vector3& InitPos, guid Owner )
{
    // Shut off degradeaim.  Characters handle hit/miss on their own. -AndyT
    (void)Rot;
    (void)Amt;
    (void)InitPos;
    (void)Owner;

    /*
    vector3 Dir(0,0,1);
    Dir.RotateX( x_frand(-Amt,Amt) );   // Pitch Z-axis up or down by spread angle
    Dir.RotateZ( x_frand(0,R_360) );    // Roll dir around Z
    Dir.Rotate( Rot );                  // Orient around original direction

    // Calculate a fake target pos to test against
    vector3 vTarget = InitPos + (Dir * 500.0f); //lets check 5 meters out

    g_CollisionMgr.LineOfSightSetup( Owner, InitPos, vTarget) ;
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING)) ;
    if (g_CollisionMgr.m_nCollisions == 0) 
    {
        // Keep new firing direction
        Rot.Roll = 0;
        Dir.GetPitchYaw(Rot.Pitch,Rot.Yaw);
    }
    */
}

//==============================================================================

void new_weapon::OnColCheck( void )
{
    if ( m_OwnerGuid == NULL_GUID )
    {
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyAABBox( GetBBox() );
        g_CollisionMgr.EndApply();            
    }
}

//==============================================================================
s32 new_weapon::GetMaterial( void ) const
{
    return 0;
}

//==============================================================================

bbox new_weapon::GetLocalBBox( void ) const 
{
    geom* pGeom = m_Skin[ m_CurrentRenderState ].GetGeom();

    if ( pGeom ) 
    {
        return pGeom->m_BBox;
    }
    else
    {
        return bbox( vector3(-50, -50, -50), vector3(50,50,50) );
    }
    
}

//==============================================================================

char_anim_player& new_weapon::GetCurrentAnimPlayer( void )
{
    return m_AnimPlayer[m_CurrentRenderState];
}

//==============================================================================

const anim_group& new_weapon::GetCurrentAnimGroup( void )
{
    return m_AnimPlayer[m_CurrentRenderState].GetAnimGroup();
}

//==============================================================================

xbool new_weapon::HasAnimGroup( void )
{
    // Get the current anim group that needs to be rendered.
    anim_group::handle& CurAnimGroup = m_AnimGroup[ m_CurrentRenderState ];
    
    if( CurAnimGroup.GetPointer() && m_Skin[ m_CurrentRenderState ].GetSkinGeom() )
        return TRUE;
    
    return FALSE;
}

//==============================================================================

void new_weapon::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
   
    if ( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_AnimPlayer[m_CurrentRenderState].SetPosition( NewPos );
    }

    // for other weapons that need to do stuff
    MoveMuzzleFx();
}

//==============================================================================

void new_weapon::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    
    // for other weapons that need to do stuff
    MoveMuzzleFx();

#ifdef DEBUG_NPC_WEAPON   
    WeaponPos = L2W.GetTranslation();
#endif

    if ( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_AnimPlayer[m_CurrentRenderState].SetRotationAndPosition( L2W );
    }
}

//==============================================================================

void new_weapon::RenderMuzzleFx( void )
{
    // Do the players muzzle render logic...
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        // if we are owned by a player, then we need to ask for his offset
        object*        pOwner    = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
        vector3        Offset    ( 0.0f, 0.0f, 0.0f );
        if ( pOwner && pOwner->IsKindOf( player::GetRTTI() ) )
        {
            Offset = ((player*)pOwner)->GetCurrentWeaponCollisionOffset();
        }

        for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
        {
            if( (m_FiringPointBoneIndex[i] != -1) )
            {
                if ( m_MuzzleFX[i].Validate() )
                {
                    vector3 Pos( m_MuzzleFX[i].GetTranslation() );
                    m_MuzzleFX[i].SetTranslation( Pos + Offset );
                    m_MuzzleFX[i].Render();
                    m_MuzzleFX[i].SetTranslation( Pos );
                }
            }

            if( (m_AltFiringPointBoneIndex[i] != -1) )
            {
                m_MuzzleSecondaryFX[i].Render();
            }
        }
    }

    // Do the NPC muzzle render logic.. 
    // Don't need || IsUsingSplitScreen() because player logic handles the render state properly in this case.
    if ( m_CurrentRenderState == RENDER_STATE_NPC  )
    {         
        // render it
        for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
        {
            if( m_FiringPointBoneIndex[i] != -1 )
                m_MuzzleNPCFX[i].Render();
        }            
    }
}

//==============================================================================

void new_weapon::AdvanceMuzzleFX( f32 DeltaTime )
{
    // Do the players muzzle render logic...
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
        {            
            if( m_MuzzleFX[i].Validate() )
            {
                // if it finished, get rid of it
                if( m_MuzzleFX[i].IsFinished() )
                {
                    m_MuzzleFX[i].KillInstance();
                }
                else
                if( (m_FiringPointBoneIndex[i] != -1) )
                {
                    // transform the effect
                    matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[i] );
                    L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_FiringPointBoneIndex[i] ) );
                    m_MuzzleFX[i].SetTransform( L2W );

                    // the run the logic
                    m_MuzzleFX[i].AdvanceLogic( DeltaTime );
                }
            }

            if( m_MuzzleSecondaryFX[i].Validate() )
            {
                // if it finished, get rid of it
                if( m_MuzzleSecondaryFX[i].IsFinished() )
                {
                    m_MuzzleSecondaryFX[i].KillInstance();
                }
                else
                if( (m_AltFiringPointBoneIndex[i] != -1) )
                {
                    // transform the effect
                    matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_AltFiringPointBoneIndex[i] );
                    L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_AltFiringPointBoneIndex[i] ) );
                    m_MuzzleSecondaryFX[i].SetTransform( L2W );

                    // then run the logic
                    m_MuzzleSecondaryFX[i].AdvanceLogic( DeltaTime );
                }
            }            
        }
    }

    // Do the NPC muzzle move logic..
    if ( m_CurrentRenderState == RENDER_STATE_NPC || IsUsingSplitScreen() )
    {
        for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
        {            
            if( m_MuzzleNPCFX[i].Validate() )
            {
                // if it finished, get rid of it
                if( m_MuzzleNPCFX[i].IsFinished() )
                {
                    m_MuzzleNPCFX[i].KillInstance();
                }
                else
                if( (m_FiringPointBoneIndex[i] != -1) )
                {
                    vector3 AimPos;
                    vector3 BonePos;
                    radian Pitch, Yaw;
                    render_state OldState = m_CurrentRenderState;

                    // save off old state, just to be safe.
                    SetRenderState( RENDER_STATE_NPC );

                    // move the effect
                    if( GetAimBonePosition(AimPos, i) && GetFiringBonePosition(BonePos, i) )
                    {
                        vector3 Rot = AimPos-BonePos;
                        matrix4 M;

                        Rot.GetPitchYaw( Pitch,Yaw );
                        M.Setup( vector3(1.0f,1.0f,1.0f), radian3(Pitch,Yaw,0.0f), BonePos );

                        // point muzzle flash in a direction toward what we're aiming at
                        m_MuzzleNPCFX[i].SetTransform( M );
                    }
                    else
                    {
                        // transform the effect to the fire point so mp dual smps work
                        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FiringPointBoneIndex[i] );
                        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_FiringPointBoneIndex[i] ) );
                        m_MuzzleNPCFX[i].SetTransform( L2W );
                    }
                    
                    // then run the logic
                    m_MuzzleNPCFX[i].AdvanceLogic( DeltaTime );

                    // put this back, just in case.
                    SetRenderState( OldState );
                }
            }
        }
    }
}

//==============================================================================

void  new_weapon::InitMuzzleFx( xbool bIsPrimary, s32 iFirePoint )
{
    ASSERT( iFirePoint >= 0 );
    ASSERT( iFirePoint < FIRE_POINT_COUNT );
    
    // Start player muzzle flash?
    if( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        // make sure there's a name
        if( m_hMuzzleFXPrimary.GetPointer() && bIsPrimary )
        {
            // if it's still valid, restart it.
            if( m_MuzzleFX[iFirePoint].Validate() )
            {
                m_MuzzleFX[iFirePoint].Restart();
            }
            else
            if( m_FiringPointBoneIndex[iFirePoint] != -1 )
            {
                m_MuzzleFX[iFirePoint].InitInstance( m_hMuzzleFXPrimary.GetPointer() );
            }
        }
        else
        // make sure there's a name
        if( m_hMuzzleFXSecondary.GetPointer() )
        {
            // if it's still valid, restart it.
            if( m_MuzzleSecondaryFX[iFirePoint].Validate() )
            {
                m_MuzzleSecondaryFX[iFirePoint].Restart();
            }
            else
            if( m_AltFiringPointBoneIndex[iFirePoint] != -1 )
            {
                m_MuzzleSecondaryFX[iFirePoint].InitInstance( m_hMuzzleFXSecondary.GetPointer() );
            }
        }
    }
        
    // Start 3rd person muzzle flash?            
    if( m_CurrentRenderState == RENDER_STATE_NPC || IsUsingSplitScreen() )
    {
        // if it's still valid, restart it.
        if( m_MuzzleNPCFX[iFirePoint].Validate() )
        {
            m_MuzzleNPCFX[iFirePoint].Restart();
        }
        else
        {
            // make sure there's a name
            if( m_hMuzzleFXPrimary.GetPointer() )
            {
                m_MuzzleNPCFX[iFirePoint].InitInstance( m_hMuzzleFXPrimary.GetPointer() );
            }
        }
    }
}

//==============================================================================

void new_weapon::MoveMuzzleFx( void )
{
}

//==============================================================================

void new_weapon::KillAllMuzzleFX( void )
{
    // Clear all of the muzzle effect objects.
    for( s32 i = 0; i < FIRE_POINT_COUNT; i++ )
    {
        m_MuzzleFX[i].KillInstance();
        m_MuzzleSecondaryFX[i].KillInstance();
        m_MuzzleNPCFX[i].KillInstance();
    }
}

//==============================================================================

void new_weapon::SetAnimation( const s32& nAnimIndex , const f32& fBlendTime , const xbool& bResetFrames)
{
    if( m_CurrentRenderState == RENDER_STATE_NPC )
        return;

    //set the animation.
    m_AnimPlayer[m_CurrentRenderState].SetAnim( nAnimIndex , FALSE , FALSE , fBlendTime , bResetFrames );
}

//==============================================================================

void new_weapon::SetRotation( const f32& rPitch , const f32& rYaw )
{
    matrix4 L2W = GetL2W();
    //now we transform
    L2W.SetRotation(radian3(rPitch, rYaw, 0));
    OnTransform(L2W);
}

//==============================================================================

void new_weapon::OnEnumProp( prop_enum& PropEnumList )
{
    object::OnEnumProp( PropEnumList );    
    PropEnumList.PropEnumHeader(   "Inventory", "Stats for this item", PROP_TYPE_HEADER );
    
    s32 ID = PropEnumList.PushPath( "Inventory\\" );
    PropEnumList.PopPath( ID );

    PropEnumList.PropEnumHeader  ( "Inventory\\Useable", "Is this inventory item useable by the this strain", 0 );
    PropEnumList.PropEnumBool    ( "Inventory\\Useable\\Human", "Can a human use me", 0 );
    PropEnumList.PropEnumBool    ( "Inventory\\Useable\\Strain One", "Can a strain one use me", 0 );
    PropEnumList.PropEnumBool    ( "Inventory\\Useable\\Strain Two", "Can a strain two use me", 0 );
    PropEnumList.PropEnumBool    ( "Inventory\\Useable\\Strain Three", "Can a strain three use me", 0 );
//    PropEnumList.AddEnum    ( "Inventory\\DropFadeTime", "INF\0TEN SEC\0THIRTY SEC\0ONE MIN\0", "How long to wait before destroying the object, infinite, 10 sec, 30 sec or 1 min" );
    PropEnumList.PropEnumExternal( "Inventory\\Label", "Sound\0soundemitter\0","Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );


    PropEnumList.PropEnumHeader( "Render", "Render information.", 0 );
    
    //--------------------------------------------------------------------------
    // Render instance.
    //--------------------------------------------------------------------------
    PropEnumList.PropEnumHeader  ( "Render\\High res Inst", "Player weapon render instance", 0 );
    ID = PropEnumList.PushPath( "Render\\High res Inst\\" );
    m_Skin[ RENDER_STATE_PLAYER ].OnEnumProp( PropEnumList );
    PropEnumList.PropEnumExternal("RenderInst\\Anim", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM) ;
    PropEnumList.PopPath( ID );

    //--------------------------------------------------------------------------
    // Lo-res RenderInst.
    //--------------------------------------------------------------------------

    //Enumerate the Lo res Render instance and animation file
    PropEnumList.PropEnumHeader  ( "Render\\Lo res Inst", "NPC / Pickup weapon render instance", 0 );
    ID = PropEnumList.PushPath( "Render\\Lo res Inst\\" );
    m_Skin[ RENDER_STATE_NPC ].OnEnumProp( PropEnumList );
    PropEnumList.PropEnumExternal("RenderInst\\Anim", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM) ;
    PropEnumList.PopPath( ID );

    // Enumerate the ammo information
    PropEnumList.PropEnumHeader( "Ammunition", "Primary and secondary ammunition data", 0 );

    PropEnumList.PropEnumHeader( "Ammunition\\Primary Ammo", "Primary ammo info", 0 );
    ID = PropEnumList.PushPath( "Ammunition\\Primary Ammo\\" );
    m_WeaponAmmo[AMMO_PRIMARY].OnEnumProp( PropEnumList );
    PropEnumList.PopPath( ID );

    PropEnumList.PropEnumHeader( "Ammunition\\Secondary Ammo", "Secondary ammo info", 0 );
    ID = PropEnumList.PushPath( "Ammunition\\Secondary Ammo\\" );
    m_WeaponAmmo[AMMO_SECONDARY].OnEnumProp( PropEnumList );
    PropEnumList.PopPath( ID );


    PropEnumList.PropEnumHeader  ( "Weapon", "This is the properties that are unique for the weapons", 0 );
    PropEnumList.PropEnumExternal( "Weapon\\Audio Package",  "Resource\0audiopkg",   "The audio package associated with this weapon.", 0 );

    PropEnumList.PropEnumExternal( "Weapon\\Muzzle Flash Primary",  "Resource\0fxo",   "The particle effect associated with the primary muzzle flash.", 0 );
    PropEnumList.PropEnumExternal( "Weapon\\Muzzle Flash Secondary",  "Resource\0fxo",   "The particle effect associated with the secondary muzzle flash", 0 );

    PropEnumList.PropEnumExternal( "Weapon\\Reticle Center", "Resource\0xbmp\0", "The center reticle piece", 0 );
    PropEnumList.PropEnumExternal( "Weapon\\Reticle Edge", "Resource\0xbmp\0", "The edge reticle piece", 0 );
    PropEnumList.PropEnumFloat   ( "Weapon\\Reticle Center Pixel Offset", "How many pixels to add to the center reticle to offset the edge reticle by.", 0 );

    PropEnumList.PropEnumFloat   ( "Weapon\\Short Range", "Max distance considered short range for this weapon.", 0 );
    PropEnumList.PropEnumFloat   ( "Weapon\\Long Range", "Min distance considered long range for this weapon.", 0 );
    PropEnumList.PropEnumInt     ( "Weapon\\Accuracy Percent Long Range", "Accuracy is this percent of what it was at short range.", 0 );

    //-------------------------------------------------------------------------
    // Reticle Radius Parameters
    //-------------------------------------------------------------------------
    PropEnumList.PropEnumHeader( "Reticle Radius", "Parameters describing the behavior of the reticle's radius during play", 0 );

    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Maximum Radius", "Maximum reticle radius, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Minimum Radius", "Minimum reticle radius, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Crouch Bonus", "Reticle radius bonus for crouching, added to \"Maximum Radius,\" in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Maximum Movement Penalty", "Worst radius penalty for moving at speed, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Movement Shrink Acceleration", "Acceleration rate for shrinking the reticle from movement, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Shot Shrink Acceleration", "Acceleration rate for shrinking the reticle from a shot, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Grow Acceleration", "Acceleration rate for growing the reticle, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Shot Penalty", "Radius penalty for a single shot from this weapon, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Reticle Radius\\Shot Penalty Degrade Rate", "How fast the accumulated shot penalties degrade to zero, in screen pixels/second.", 0 );

    //-------------------------------------------------------------------------
    // Alt Reticle Radius Parameters
    //-------------------------------------------------------------------------
    PropEnumList.PropEnumHeader( "Alt Reticle Radius", "Parameters describing the behavior of the reticle's radius during play", 0 );

    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Maximum Radius", "Maximum reticle radius, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Minimum Radius", "Minimum reticle radius, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Crouch Bonus", "Reticle radius bonus for crouching, added to \"Maximum Radius,\" in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Maximum Movement Penalty", "Worst radius penalty for moving at speed, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Movement Shrink Acceleration", "Acceleration rate for shrinking the reticle from movement, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Shot Shrink Acceleration", "Acceleration rate for shrinking the reticle from a shot, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Grow Acceleration", "Acceleration rate for growing the reticle, in screen pixels/second^2.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Shot Penalty", "Radius penalty for a single shot from this weapon, in screen pixels.", 0 );
    PropEnumList.PropEnumFloat   ( "Alt Reticle Radius\\Shot Penalty Degrade Rate", "How fast the accumulated shot penalties degrade to zero, in screen pixels/second.", 0 );

    PropEnumList.PropEnumHeader( "Faction Fire Sound", "Weapon fire sounds for the factions.", 0 );
    for (s32 i = 0; i < (factions_manager::s_FactionList.GetCount()-1); i++)
    {
        PropEnumList.PropEnumExternal( xfs("Faction Fire Sound\\%s",factions_manager::s_FactionList.GetStringFromIndex(i)), "Sound\0soundexternal\0", 
                                            "What firing sound to play for this faction, this only for AI's", 0 );
    }

}

//==============================================================================

xbool new_weapon::OnProperty( prop_query& PropQuery )
{


    // Base classes
    if ( object::OnProperty( PropQuery ) )
    {
        return TRUE;
    }

/*
    if( PropQuery.IsVar( "Inventory\\DropFadeTime" ) )
    {
        if( PropQuery.IsRead() )
        {
            if( m_DropFadeTime == -1.0f )
                PropQuery.SetVarEnum( "INF" );
            else if( m_DropFadeTime == 10.0f )
                PropQuery.SetVarEnum( "TEN SEC" );
            else if( m_DropFadeTime == 30.0f )
                PropQuery.SetVarEnum( "THIRTT SEC" );
            else
                PropQuery.SetVarEnum( "ONE MIN" );
        }
        else
        {
            if( !x_stricmp( "INF", PropQuery.GetVarEnum()) )
                m_DropFadeTime = -1.0f;
            else if( !x_stricmp( "TEN SEC", PropQuery.GetVarEnum()) )
                m_DropFadeTime = 10.0f;
            else if( !x_stricmp( "THIRTY SEC", PropQuery.GetVarEnum()) )
                m_DropFadeTime = 30.0f;
            else 
                m_DropFadeTime = 60.0f;
        }
        
        return TRUE;
    }
*/

    // External
    if( PropQuery.IsVar( "Weapon\\Audio Package" ) )
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

    // External
    if( PropQuery.IsVar( "Weapon\\Muzzle Flash Primary" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hMuzzleFXPrimary.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hMuzzleFXPrimary.SetName( pString );                

                // Load the audio package.
                if( m_hMuzzleFXPrimary.IsLoaded() == FALSE )
                    m_hMuzzleFXPrimary.GetPointer();

            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Weapon\\Muzzle Flash Secondary" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_hMuzzleFXSecondary.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_hMuzzleFXSecondary.SetName( pString );                

                // Load the audio package.
                if( m_hMuzzleFXSecondary.IsLoaded() == FALSE )
                    m_hMuzzleFXSecondary.GetPointer();

            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Weapon\\Reticle Center" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ReticleCenter.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ReticleCenter.SetName( pString );                

                // Load the audio package.
                if( m_ReticleCenter.IsLoaded() == FALSE )
                    m_ReticleCenter.GetPointer();

            }
        }
        return( TRUE );
    }

    // External
    if( PropQuery.IsVar( "Weapon\\Reticle Edge" ) )
    {
        if( PropQuery.IsRead() )
        {
            PropQuery.SetVarExternal( m_ReticleEdge.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = PropQuery.GetVarExternal();

            if( pString[0] )
            {
                m_ReticleEdge.SetName( pString );                

                // Load the audio package.
                if( m_ReticleEdge.IsLoaded() == FALSE )
                    m_ReticleEdge.GetPointer();

            }
        }
        return( TRUE );
    }

    if( PropQuery.VarFloat( "Weapon\\Reticle Center Pixel Offset",  m_ReticleCenterPixelOffset ) )
    {
        m_ReticleCenterPixelOffset = MAX( 0.0f, m_ReticleCenterPixelOffset );
        return TRUE;
    }

    if( PropQuery.VarFloat( "Weapon\\Short Range",  m_ShortRange ) )
    {
        m_ShortRange = MAX( 0.0f, m_ShortRange );
        return TRUE;
    }
    
    if( PropQuery.VarFloat( "Weapon\\Long Range",  m_LongRange ) )
    {
        m_LongRange = MAX( 0.0f, m_LongRange );
        return TRUE;
    }

    if( PropQuery.VarInt( "Weapon\\Accuracy Percent Long Range",  m_AccuracyPercentLongRange ) )
    {
        m_AccuracyPercentLongRange = MAX( 0, m_AccuracyPercentLongRange );
        m_AccuracyPercentLongRange = MIN( 100, m_AccuracyPercentLongRange );
        return TRUE;
    }

    //
    // Human single handed.
    //
    s32 ID = PropQuery.PushPath( "Render\\High res Inst\\" );
    
    if ( m_Skin[ RENDER_STATE_PLAYER ].OnProperty( PropQuery ) )
    {
        if ( !PropQuery.IsRead() && PropQuery.IsVar("RenderInst\\File") )
        {
            InstallCustomScope();
        }
        return TRUE;
    }

    // Animation
    if (PropQuery.IsVar( "RenderInst\\Anim"))
    {
        if (PropQuery.IsRead())
            PropQuery.SetVarExternal(m_AnimGroup[ RENDER_STATE_PLAYER ].GetName(), RESOURCE_NAME_SIZE) ;
        else
        {
            if( x_strlen( PropQuery.GetVarExternal() ) > 0 )
            {
                m_AnimGroup[ RENDER_STATE_PLAYER ].SetName(PropQuery.GetVarExternal() );
                if ( m_AnimGroup[RENDER_STATE_PLAYER].GetPointer() )
                {
                    m_AnimPlayer[RENDER_STATE_PLAYER].SetAnimGroup( m_AnimGroup[RENDER_STATE_PLAYER] );
                    m_AnimPlayer[RENDER_STATE_PLAYER].SetAnim( 0, TRUE, TRUE );
                    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
                    OnTransform( GetL2W() );
                }
            }
        }
        return TRUE ;   
    }

    PropQuery.PopPath( ID );
  
    //--------------------------------------------------------------------------
    // Lo-res RenderInst.
    //-------------------------------------------   -------------------------------
  
    ID = PropQuery.PushPath( "Render\\Lo res Inst\\" );
    if ( m_Skin[ RENDER_STATE_NPC ].OnProperty( PropQuery ) )
    {
        return TRUE;
    }
    // Animation
    if (PropQuery.IsVar( "RenderInst\\Anim"))
    {
        if (PropQuery.IsRead())
            PropQuery.SetVarExternal(m_AnimGroup[ RENDER_STATE_NPC ].GetName(), RESOURCE_NAME_SIZE) ;
        else
        {
            // Anim changed?
            if( x_strlen( PropQuery.GetVarExternal() ) > 0 )
            {
                m_AnimGroup[ RENDER_STATE_NPC ].SetName(PropQuery.GetVarExternal()) ;
                if ( m_AnimGroup[RENDER_STATE_NPC].GetPointer() )
                {
                    m_AnimPlayer[RENDER_STATE_NPC].SetAnimGroup( m_AnimGroup[RENDER_STATE_NPC] );
                    m_AnimPlayer[RENDER_STATE_NPC].SetAnim( 0, TRUE, TRUE );                    
//                    m_AnimPlayer[m_CurrentRenderState].SetPosition( GetL2W().GetTranslation() );                    
                    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
                    OnTransform( GetL2W() );
                }
            }
        }
        return TRUE ;   
    }
    PropQuery.PopPath( ID );

    // Primary Ammo
    ID = PropQuery.PushPath( "Ammunition\\Primary Ammo\\" );
    if  ( m_WeaponAmmo[AMMO_PRIMARY].OnProperty( PropQuery ) )
    {
        m_OriginalWeaponAmmo[AMMO_PRIMARY] = m_WeaponAmmo[AMMO_PRIMARY];
        return TRUE;
    }
    PropQuery.PopPath( ID );
   
    // Secondary Ammo
    ID = PropQuery.PushPath( "Ammunition\\Secondary Ammo\\" );
    if  ( m_WeaponAmmo[AMMO_SECONDARY].OnProperty( PropQuery ) )
    {
        m_OriginalWeaponAmmo[AMMO_SECONDARY] = m_WeaponAmmo[AMMO_SECONDARY];
        return TRUE;
    }
    PropQuery.PopPath( ID );
    
    //
    // Reticle Radius Parameters
    //
    if( PropQuery.VarFloat( "Reticle Radius\\Maximum Radius",  m_ReticleRadiusParameters.m_MaxRadius ) )
    {
        m_ReticleRadiusParameters.m_MaxRadius = MAX( m_ReticleRadiusParameters.m_MinRadius, m_ReticleRadiusParameters.m_MaxRadius );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Minimum Radius",  m_ReticleRadiusParameters.m_MinRadius ) )
    {
        m_ReticleRadiusParameters.m_MinRadius = MIN( m_ReticleRadiusParameters.m_MinRadius, m_ReticleRadiusParameters.m_MaxRadius );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Crouch Bonus",  m_ReticleRadiusParameters.m_CrouchBonus) )
    {
        m_ReticleRadiusParameters.m_CrouchBonus = MAX( m_ReticleRadiusParameters.m_CrouchBonus, 0 );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Maximum Movement Penalty",  m_ReticleRadiusParameters.m_MaxMovementPenalty ) )
    {
        m_ReticleRadiusParameters.m_MaxMovementPenalty = MAX( 0.0f, m_ReticleRadiusParameters.m_MaxMovementPenalty );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Movement Shrink Acceleration",  m_ReticleRadiusParameters.m_MoveShrinkAccel ) )
    {
        m_ReticleRadiusParameters.m_MoveShrinkAccel = MAX( 0.0f, m_ReticleRadiusParameters.m_MoveShrinkAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Shot Shrink Acceleration",  m_ReticleRadiusParameters.m_ShotShrinkAccel ) )
    {
        m_ReticleRadiusParameters.m_ShotShrinkAccel = MAX( 0.0f, m_ReticleRadiusParameters.m_ShotShrinkAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Grow Acceleration",  m_ReticleRadiusParameters.m_GrowAccel ) )
    {
        m_ReticleRadiusParameters.m_GrowAccel = MAX( 0.0f, m_ReticleRadiusParameters.m_GrowAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Shot Penalty",  m_ReticleRadiusParameters.m_PenaltyForShot ) )
    {
        m_ReticleRadiusParameters.m_PenaltyForShot = MAX( 0.0f, m_ReticleRadiusParameters.m_PenaltyForShot );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Reticle Radius\\Shot Penalty Degrade Rate",  m_ReticleRadiusParameters.m_ShotPenaltyDegradeRate ) )
    {
        m_ReticleRadiusParameters.m_ShotPenaltyDegradeRate = MAX( 0.0f, m_ReticleRadiusParameters.m_ShotPenaltyDegradeRate );
        return TRUE;
    }

    //
    // Alt Reticle Radius Parameters
    //
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Maximum Radius",  m_AltReticleRadiusParameters.m_MaxRadius ) )
    {
        m_AltReticleRadiusParameters.m_MaxRadius = MAX( m_AltReticleRadiusParameters.m_MinRadius, m_AltReticleRadiusParameters.m_MaxRadius );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Minimum Radius",  m_AltReticleRadiusParameters.m_MinRadius ) )
    {
        m_AltReticleRadiusParameters.m_MinRadius = MIN( m_AltReticleRadiusParameters.m_MinRadius, m_AltReticleRadiusParameters.m_MaxRadius );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Crouch Bonus",  m_AltReticleRadiusParameters.m_CrouchBonus) )
    {
        m_AltReticleRadiusParameters.m_CrouchBonus = MAX( m_AltReticleRadiusParameters.m_CrouchBonus, 0 );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Maximum Movement Penalty",  m_AltReticleRadiusParameters.m_MaxMovementPenalty ) )
    {
        m_AltReticleRadiusParameters.m_MaxMovementPenalty = MAX( 0.0f, m_AltReticleRadiusParameters.m_MaxMovementPenalty );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Movement Shrink Acceleration",  m_AltReticleRadiusParameters.m_MoveShrinkAccel ) )
    {
        m_AltReticleRadiusParameters.m_MoveShrinkAccel = MAX( 0.0f, m_AltReticleRadiusParameters.m_MoveShrinkAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Shot Shrink Acceleration",  m_AltReticleRadiusParameters.m_ShotShrinkAccel ) )
    {
        m_AltReticleRadiusParameters.m_ShotShrinkAccel = MAX( 0.0f, m_AltReticleRadiusParameters.m_ShotShrinkAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Grow Acceleration",  m_AltReticleRadiusParameters.m_GrowAccel ) )
    {
        m_AltReticleRadiusParameters.m_GrowAccel = MAX( 0.0f, m_AltReticleRadiusParameters.m_GrowAccel );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Shot Penalty",  m_AltReticleRadiusParameters.m_PenaltyForShot ) )
    {
        m_AltReticleRadiusParameters.m_PenaltyForShot = MAX( 0.0f, m_AltReticleRadiusParameters.m_PenaltyForShot );
        return TRUE;
    }
    if( PropQuery.VarFloat( "Alt Reticle Radius\\Shot Penalty Degrade Rate",  m_AltReticleRadiusParameters.m_ShotPenaltyDegradeRate ) )
    {
        m_AltReticleRadiusParameters.m_ShotPenaltyDegradeRate = MAX( 0.0f, m_AltReticleRadiusParameters.m_ShotPenaltyDegradeRate );
        return TRUE;
    }

    // NPC faction fire sfx list.
    for (s32 i = 0; i < (factions_manager::s_FactionList.GetCount()-1); i++)
    {
        if( PropQuery.IsVar( xfs("Faction Fire Sound\\%s",factions_manager::s_FactionList.GetStringFromIndex(i) ) ) )
        {
            if( PropQuery.IsRead() )
            {
                const factions& rFaction = factions_manager::s_FactionList[i];

                if( m_FactionFireSfx[ factions_manager::GetFactionBitIndex(rFaction) ] != -1 )
                    PropQuery.SetVarExternal( g_StringMgr.GetString( m_FactionFireSfx[ factions_manager::GetFactionBitIndex(rFaction) ] ), 64 );
                else
                    PropQuery.SetVarExternal( "", 64 );
            }
            else
            {
                // Get the FileName
                xstring ExtString = PropQuery.GetVarExternal();
                if( !ExtString.IsEmpty() )
                {
                    xstring String( ExtString );

                    s32 PkgIndex = String.Find( '\\', 0 );
                
                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );
                    }

                    const factions& rFaction = factions_manager::s_FactionList[i];
                    m_FactionFireSfx[ factions_manager::GetFactionBitIndex(rFaction) ] = g_StringMgr.Add( String );
                }
            }
            return TRUE;
        }
    }


    return FALSE;
}

//===========================================================================

void new_weapon::ammo::OnEnumProp( prop_enum& List )
{
    List.PropEnumInt( "Max Ammunition", "The maximum ammount of ammo that can be carried", 0 );
    List.PropEnumInt( "Ammunition per clip", "Ammount of ammo per clip", 0 );
    List.PropEnumInt( "Ammo ammount", "Number of bullets in this weapon", 0 );
    List.PropEnumExternal( "BitmapResource", "Resource\0xbmp\0", "Bitmap resource.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL );

/*
    // TODO:  Add templated projectile types here.
    List.AddExternal( "Projectile Blueprint",
                      "File\0blueprint\0",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL );
*/    

    List.PropEnumFileName( "Projectile Blueprint Path",
                      "Projectile object blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM );
}

//===========================================================================

xbool new_weapon::ammo::OnProperty( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt( "Max Ammunition", m_AmmoMax ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarInt( "Ammunition per clip" , m_AmmoPerClip ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarInt( "Ammo ammount", m_AmmoAmount ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "BitmapResource" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_Bitmap.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            (void)pStr;
            m_Bitmap.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.IsVar( "Projectile Blueprint" ) )
    {
        return TRUE;
/*
        if( rPropQuery.IsRead() )
        {
            if( m_ProjectileTemplateID < 0 )
            {
                rPropQuery.SetVarExternal("",256);
            }
            else
            {
                rPropQuery.SetVarExternal( g_TemplateStringMgr.GetString( m_ProjectileTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarExternal() ) > 0 )
            {
                m_ProjectileTemplateID = g_TemplateStringMgr.Add( rPropQuery.GetVarExternal() );
            }

        }
        return TRUE;
*/
    }
    

    if( rPropQuery.IsVar( "Projectile Blueprint Path" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_ProjectileTemplateID < 0 )
            {
                rPropQuery.SetVarFileName("",256);
            }
            else
            {
                rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_ProjectileTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
            {
                m_ProjectileTemplateID = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
            }

        }
        return TRUE;
    }


    return FALSE;
}
    
//==============================================================================

void new_weapon::ResetWeapon( void )
{
    ReleaseAudio();
    ClearZoom();
}


//==============================================================================

void new_weapon::ProcessSfx( void )
{
}

//==============================================================================

void new_weapon::BeginPrimaryFire ( void )
{
}

//==============================================================================

void new_weapon::EndPrimaryFire ( void )
{
}

//==============================================================================

void new_weapon::BeginAltFire ( void )
{
}

//==============================================================================
void new_weapon::EndAltFire ( void )
{
}

//==============================================================================

void new_weapon::ReleaseAudio( void )
{
}

//==============================================================================
// This creates bullets through the template manager.
base_projectile* new_weapon::CreateBullet( 
    const char*                     pWeaponLogicalName,
    const vector3&                  InitPos, 
    const radian3&                  InitRot, 
    const vector3&                  InheritedVelocity,
    const guid                      OwnerGuid,
    const pain_handle&              PainHandle,
    new_weapon::ammo_priority       Priority,
    xbool                           bHitLiving,
    s32                             iFirePoint )
{
    object* pOwner = g_ObjMgr.GetObjectByGuid( OwnerGuid );
    ASSERT( pOwner );
    
    u16 Zone1 = pOwner->GetZone1();
    u16 Zone2 = pOwner->GetZone2();
    
    //create Bullet
    guid BulletID = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_WeaponAmmo[ Priority ].m_ProjectileTemplateID ), InitPos, InitRot, Zone1, Zone2 );
    object* pObject = g_ObjMgr.GetObjectByGuid( BulletID );
    ASSERT( pObject );
    ASSERT( pObject->IsKindOf( base_projectile::GetRTTI() ) );

    base_projectile* pBullet = (base_projectile*)pObject;
    
    ASSERT( pBullet );
    if( pBullet != NULL )
    {
        // Lookup speed
        tweak_handle SpeedTweak( xfs("%s_SPEED",pWeaponLogicalName) );
        pBullet->Initialize( InitPos, InitRot, InheritedVelocity, SpeedTweak.GetF32(), OwnerGuid, PainHandle, bHitLiving, iFirePoint );

        // Lookup pain degradation
        tweak_handle PainDropDistTweak ( xfs("%s_PainDropDist",pWeaponLogicalName) );
        tweak_handle PainDropScaleTweak( xfs("%s_PainDropScale",pWeaponLogicalName) );
        pBullet->SetPainDegradation( PainDropDistTweak.GetF32(), PainDropScaleTweak.GetF32() );
    }
    
    /*
    #ifdef X_LOGGING
    if( pBullet )
    {
        LOG_MESSAGE("new_weapon::CreateBullet","(%s) created a (%s) bullet",
            (pOwner) ? (pOwner->GetLogicalName()) : ("UNKNOWN") ,
            pWeaponLogicalName
            );
    }
    #endif
    */

    return pBullet;
}
//==============================================================================
 //change the attribs flags...
 
xbool new_weapon::GetFiringBonePosition ( vector3 &Pos, s32 iBone )
{
    if( m_FiringPointBoneIndex[ iBone ]!=-1 )
    {
        if( m_CurrentRenderState == RENDER_STATE_NPC )
        {
            // Using a dual weapon?
            if( ( m_Item == INVEN_WEAPON_DUAL_SMP ) || ( m_Item == INVEN_WEAPON_DUAL_SHT ) )
            {
                // Get position from anim player since special controller splits same geometry
                // weapon into correct positions so we can't use the faster method below
                Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iBone ] );
            }
            else
            {
                // Use optimized method that doesn't require computing animation matrices
                Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_FiringPointBoneIndex[ iBone ] );
                Pos = GetL2W().Transform( Pos );
            }
        }            
        else if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[ iBone ] );
        }
        
        return TRUE;
    }   

    return FALSE;
}

//==============================================================================

xbool new_weapon::GetAimBonePosition ( vector3 &Pos, s32 iBone )
{
    if( m_AimPointBoneIndex[ iBone ]!=-1 )
    { 
        if( m_CurrentRenderState == RENDER_STATE_NPC )
        {
            // Using a dual weapon?
            if( ( m_Item == INVEN_WEAPON_DUAL_SMP ) || ( m_Item == INVEN_WEAPON_DUAL_SHT ) )
            {
                // Get position from anim player since special controller splits same geometry
                // weapon into correct positions so we can't use the faster method below
                Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_AimPointBoneIndex[ iBone ] );
            }
            else
            {        
                // Use optimized method that doesn't require computing animation matrices
                Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_AimPointBoneIndex[ iBone ] );
                Pos = GetL2W().Transform( Pos );
            }
        }
        else if( m_CurrentRenderState == RENDER_STATE_PLAYER )
        {
            Pos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_AimPointBoneIndex[ iBone ] );
        }

        return TRUE;
    }   
    return FALSE;
}

//==============================================================================

xbool new_weapon::CheckFirePoint( void )
{
    return ( m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ] != -1 );
}

//==============================================================================

xbool new_weapon::CheckFlashlightPoint( void )
{
    return ( m_FlashlightBoneIndex != -1 );
}

//==============================================================================

xbool new_weapon::GetFlashlightTransformInfo( matrix4& incMatrix,  vector3 &incVect )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        // return the matrix and the bone vector
        incMatrix = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( m_FlashlightBoneIndex );
        incVect = m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( m_FlashlightBoneIndex );
        
        return TRUE;
    }

    // not in the proper state i.e. RENDER_STATE_NPC
    return FALSE;
}

//===========================================================================

/*
void new_weapon::OnColNotify( object& Object )
{
    xtimer OnColNotifyTimer;
    OnColNotifyTimer.Start();

    // Only when colliding with the player do we collect the weapon
    if ( !m_bCollected && Object.IsKindOf( player::GetRTTI() ) )
    {
        player&     Player   = player::GetSafeType( Object );
        inven_item  Item     = inventory2::ObjectTypeToItem( GetObjectType().GetType() );
        xbool       PickedUp = Player.AddItemToInventory2( Item );

        if( PickedUp )
        {
            Player.PickupWeapon2( Item );
            m_bCollected    = TRUE;
            m_ItemDroped    = FALSE;

            // Play the sound.
            if( m_SoundDescID > 0 )
            {
                g_AudioManager.PlayVolumeClipped( g_StringMgr.GetString( m_SoundDescID ), GetPosition(), GetZone1() );
            }
        }
		else
		{
			// give the ammo and self destruct
            if( Player.TryAddAmmo2( Item ) )
            {
                g_ObjMgr.DestroyObject( GetGuid() );
            }
		}
    }

    x_DebugMsg( "new_weapon::OnColNotify %f\n", OnColNotifyTimer.StopSec() );
}
*/

//===========================================================================

void new_weapon::OnKill( void )
{
    UninstallCustomScope();
}

//===========================================================================

void new_weapon::AddAmmoToWeapon( s32 nAmmoPrimary, s32 nAmmoSecondary )
{
    m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = iMin( m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount + nAmmoPrimary, m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax );
    m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = iMin( m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount + nAmmoSecondary, m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax );

    object* pOwner = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );
    
    // owner SHOULD be an actor
    if( pOwner && pOwner->IsKindOf( actor::GetRTTI() ) )
    {
        actor* pActor = (actor*)pOwner;
        inven_item DualItem = GetDualWeaponID( m_Item );
       
        // if we have a dual, add the ammo to it as well so that we display ammo correctly.
        if( DualItem != INVEN_NULL )
        {
            new_weapon* pDualWeapon = pActor->GetWeaponPtr( DualItem );
            if( pDualWeapon )
                pDualWeapon->AddAmmoToWeapon( nAmmoPrimary, nAmmoSecondary );
        }
    }
}

//===========================================================================
void new_weapon::RefillClip( ammo_priority Priority )
{
    s32 Amount = GetAmmoPerClip(Priority);

    // make sure we have full ammo in this weapon's clip
    m_WeaponAmmo[Priority].m_AmmoInCurrentClip = Amount;
}

//===========================================================================

void new_weapon::ClearAmmo( const ammo_priority& rAmmoPriority )
{
    // clear count of bullets in current clip
    if( m_WeaponAmmo[rAmmoPriority].m_ProjectileType != BULLET_MUTATION )
    {
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip = 0;

        if( !DEBUG_INFINITE_AMMO )
        {
            // clear all bullets    
            m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount = 0;
        }
    }
}

//===========================================================================

void new_weapon::ClearClipAmmo( const ammo_priority& rAmmoPriority  )
{
    // clear current weapon's clip ammo so that we'll have to reload once we dump the dual.
    // so if you had 30/270 when you picked up duals, you would have 0/270 when you discarded the dual.  (and 30/240 after reload).
    // this is basically just burning what is in the clip.
    {
        // how much ammo do we remove from the total?
        s32 nTotal  = m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount;
        s32 nClip   = m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip;

        // make sure we don't subtract more than we have.
        s32 nRemove = MIN(nTotal, nClip);

        // take it out
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount -= nRemove;
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip = 0; 
    }
}

//===========================================================================

void new_weapon::DecrementAmmo( const ammo_priority& rAmmoPriority, const s32& nAmt)
{
    // decrement count of bullets in current clip
    m_WeaponAmmo[ rAmmoPriority ].m_AmmoInCurrentClip -= nAmt;

    object *pObj = g_ObjMgr.GetObjectByGuid(m_ParentGuid);

    if( DEBUG_INFINITE_AMMO == FALSE && !pObj->IsKindOf( character::GetRTTI() ) )
    {
        // also, take away from total if we aren't unlimited ammo
        m_WeaponAmmo[ rAmmoPriority ].m_AmmoAmount -= nAmt;
    }
}

void new_weapon::FillAmmo( void )
{
    // make sure ammo is constantly full
    if( DEBUG_INFINITE_AMMO )
    {
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
        m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_SECONDARY ].m_AmmoMax;
    }
}

//===========================================================================

/*
void new_weapon::ActivateItem( void )
{
    // Gets called when we want to drop inventory items.
    m_OwnerGuid = NULL;
    inventory_item::ActivateItem();
}
*/

//==============================================================================

xbitmap* new_weapon::GetCenterReticleBmp( void )
{
    return m_ReticleCenter.GetPointer();
}

//==============================================================================

xbitmap* new_weapon::GetEdgeReticleBmp( void )
{
    return m_ReticleEdge.GetPointer();
}

//==============================================================================

xbool new_weapon::CanIntereptPrimaryFire( s32 nFireAnimIndex )
{
    // If we are currently playing the fire animation then see if its at end before we interupt it.
    if( m_AnimPlayer[ m_CurrentRenderState ].GetAnimIndex() == nFireAnimIndex )
    {
        return m_AnimPlayer[ m_CurrentRenderState ].IsAtEnd();
    }

    return TRUE;
}

//==============================================================================

xbool new_weapon::CanIntereptSecondaryFire( s32 nFireAnimIndex )
{
    // If we are currently playing the fire animation then see if its at end before we interupt it.
    if( m_AnimPlayer[ m_CurrentRenderState ].GetAnimIndex() == nFireAnimIndex )
    {
        return m_AnimPlayer[ m_CurrentRenderState ].IsAtEnd();
    }

    return TRUE;
}
//==============================================================================

void new_weapon::SetupRenderInformation( void )
{
    for( s32 i = 0; i < AMMO_MAX; i++ )
    {
        if( m_WeaponAmmo[i].m_AmmoInCurrentClip > m_WeaponAmmo[i].m_AmmoPerClip )
        {   
            m_WeaponAmmo[i].m_AmmoAmount += m_WeaponAmmo[i].m_AmmoInCurrentClip - m_WeaponAmmo[i].m_AmmoPerClip;
        }

        m_WeaponAmmo[i].m_AmmoMax       = m_OriginalWeaponAmmo[i].m_AmmoMax;
        m_WeaponAmmo[i].m_AmmoPerClip   = m_OriginalWeaponAmmo[i].m_AmmoPerClip;
    }

    // If we have a valid anim group fill out the bone indecies.
    anim_group::handle& CurAnimGroup =  m_AnimGroup[m_CurrentRenderState];
    if( CurAnimGroup.GetPointer() )
    {
        m_FiringPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );
        m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );
        m_AimPointBoneIndex[FIRE_POINT_DEFAULT]             = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint" );
        m_AltAimPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint" );

        m_FiringPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_left" );
        m_AltFiringPointBoneIndex[FIRE_POINT_LEFT]  = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_left" );
        m_AimPointBoneIndex[FIRE_POINT_LEFT]        = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_left" );
        m_AltAimPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_left" );

        m_FiringPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_right" );
        m_AltFiringPointBoneIndex[FIRE_POINT_RIGHT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_right" );
        m_AimPointBoneIndex[FIRE_POINT_RIGHT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_right" );
        m_AltAimPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_right" );
        
        // flashlight bone
        m_FlashlightBoneIndex                       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "lightpoint" );        

        // try and validate flashlight bone
        if( m_FlashlightBoneIndex == -1 )
        {
            // no flashlight point, try to put it on the firing point
            if( CheckFirePoint() )
            {
                m_FlashlightBoneIndex = m_FiringPointBoneIndex[ FIRE_POINT_DEFAULT ];
            }
            else  // try one more time to find a bone
            if( m_FiringPointBoneIndex[ FIRE_POINT_RIGHT ] != -1 )
            {
                m_FlashlightBoneIndex = m_FiringPointBoneIndex[ FIRE_POINT_RIGHT ];                
            }
            else
            {
                x_DebugMsg( "new_weapon::SetupRenderInformation - No flashlight bone\n");
            }
        }
    }
    
    KillAllMuzzleFX();
}

//==============================================================================

/*
inventory_item::use_item_err new_weapon::UseItem( void )
{
    // Inventory has requested to use this item.
    if( m_OwnerGuid == NULL )
    {
        return ERR_NO_OWNER;
    }
    
    object* pObj = g_ObjMgr.GetObjectByGuid( m_OwnerGuid );

    if( pObj )
    {
        actor& Actor = actor::GetSafeType( *pObj );
        Actor.SwitchWeapon2( inventory2::ObjectTypeToItem( GetTypeDesc().GetType() ) );
    }

    // If we return ERR_NONE, it will decremen the inventory count.
    return ERR_NO_NEED;
}
*/

//==============================================================================
void new_weapon::UpdateReticle( f32 DeltaTime )
{
    (void) DeltaTime;

    if( !ShouldUpdateReticle() )
    {
        return;
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    
    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        // if this is a player and we're not running a cinema, see if we are over a target with our reticle
        if( pPlayer && (!pPlayer->IsCinemaRunning()) && CheckReticleLocked() )
        {
            // reticle was NOT already on
            if( !m_bLockedOn )
            {
                m_bLockedOn = TRUE;
                g_AudioMgr.Play( "Reticule_Shift_Red" );
            }
        }
        else
        {
            m_bLockedOn = FALSE;
        }
    }
    else
    {
        m_bLockedOn = FALSE;
    }
}

//==============================================================================

void new_weapon::UpdateAmmoWarning( void )
{
    f32 fCurAmmo    = (f32)m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip;
    f32 fMaxAmmo    = (f32)m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip;
    s32 fTotalAmmo  = m_WeaponAmmo[AMMO_PRIMARY].m_AmmoAmount;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( pObj )
    {
        player* pPlayer = (player*)pObj;

        hud_object* Hud = pPlayer->GetHud();

        if( Hud && Hud->m_Initialized )
        {
            // if we have low ammo, warn the player
            if( (fCurAmmo/fMaxAmmo) <= s_LowAmmoPercent )
            {
                // make sure we can warn the player (so we don't get constant annoyances, just warn once).
                if( m_bCanWarnLowAmmo )
                {
#if !defined(X_EDITOR)
                    // does this load use a warning message for low clip ammo?
                    if( pPlayer->LoadWarnsLowAmmo() )
                    {
                        // set up message and sound and send to player
                        MsgMgr.Message( MSG_LOW_AMMO, pPlayer->net_GetSlot() );
                    }
#endif
                    m_bCanWarnLowAmmo = FALSE;
                }
            
                // out of ammo, tell player
                if( fCurAmmo == 0.0f )
                {
                    xbool bShouldFlash = (fTotalAmmo <= 0);
                 
                    // we can reload because we have more ammo, stop the flashing
                    Hud->SetElementPulseState( HUD_ELEMENT_AMMO_BAR,  bShouldFlash );
                }
                else
                {
                    // just low, clip not empty yet (NOTE: this will continue to flash if we are completely out of ammo)
                    Hud->SetElementPulseState( HUD_ELEMENT_AMMO_BAR, TRUE );
                }
                
                xcolor WarningColor( 150, 15, 15, 255 );
                SetAmmoHudColor(pPlayer, Hud, WarningColor);
            }
            else
            {
                // reset warning flag
                m_bCanWarnLowAmmo = TRUE;

                // turn off pulsing
                Hud->SetElementPulseState(HUD_ELEMENT_AMMO_BAR, FALSE);

                // set normal color
                SetAmmoHudColor(pPlayer, Hud, g_HudColor);
            }
        }
    }
}

//==============================================================================
void new_weapon::SetAmmoHudColor(player *pPlayer, hud_object* Hud, xcolor HudColor )
{
    ASSERT(Hud);
    ASSERT(pPlayer);
    player_hud& PHud = Hud->GetPlayerHud( pPlayer->GetLocalSlot() );
    hud_ammo* pAHud = (hud_ammo*)(PHud.m_HudComponents[HUD_ELEMENT_AMMO_BAR]);
    pAHud->SetRenderColor(HudColor);
}

//==============================================================================

xbool new_weapon::CheckReticleLocked( void )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        // if we are over something with the reticle, make sure it's an enemy
        if( pPlayer->ReticleOnTarget() && (pPlayer->GetEnemyOnReticle() != NULL_GUID) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

//==============================================================================
void new_weapon::SetRenderState( render_state RenderState )
{
    (void)RenderState;

    //set the render state
    m_CurrentRenderState = RenderState;
/*
    //change the attribs flags...
    switch( m_CurrentRenderState )
    {
    case RENDER_STATE_PLAYER:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_NULL );
        }
        break;
    case RENDER_STATE_NPC:
        {
            //remove the object from dBase
            g_ObjMgr.RemoveFromSpatialDBase( GetGuid() );
            
            //set appropriate attribute bits as needed
            SetAttrBits( object::ATTR_SOUND_SOURCE          | 
                         object::ATTR_RENDERABLE        );
        }
        break;
    default:
        //no-op
        break;
    }

    //initialize the animations for this animation group.
    if( m_AnimGroup[m_CurrentRenderState].GetPointer() )
    {
        m_AnimPlayer[m_CurrentRenderState].SetAnimGroup( m_AnimGroup[m_CurrentRenderState] );

        m_FiringPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint" );
        m_AltFiringPointBoneIndex[FIRE_POINT_DEFAULT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint" );
        m_AimPointBoneIndex[FIRE_POINT_DEFAULT]             = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint" );
        m_AltAimPointBoneIndex[FIRE_POINT_DEFAULT]          = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint" );

        m_FiringPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_left" );
        m_AltFiringPointBoneIndex[FIRE_POINT_LEFT]  = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_left" );
        m_AimPointBoneIndex[FIRE_POINT_LEFT]        = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_left" );
        m_AltAimPointBoneIndex[FIRE_POINT_LEFT]     = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_left" );

        m_FiringPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "firepoint_right" );
        m_AltFiringPointBoneIndex[FIRE_POINT_RIGHT] = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altfirepoint_right" );
        m_AimPointBoneIndex[FIRE_POINT_RIGHT]       = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "aimpoint_right" );
        m_AltAimPointBoneIndex[FIRE_POINT_RIGHT]    = m_AnimPlayer[m_CurrentRenderState].GetBoneIndex( "altaimpoint_right" );

        //set the muzzle FX 
        InitMuzzleFx( );
    }
*/
}

//==============================================================================

xbool new_weapon::ContinueReload( void )
{
    return FALSE;
}

//==============================================================================

f32 new_weapon::GetZoomLevel( void )
{
//  ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( pObj )
    {
        player* pPlayer = (player*)pObj;

        const player::view_info& ViewInfo = pPlayer->GetViewInfo();
        const player::view_info& OriginalViewInfo = pPlayer->GetOriginalViewInfo();

        f32 ZoomLevel = OriginalViewInfo.XFOV / ViewInfo.XFOV;
        
        return ZoomLevel;
    }
    else
    {
        return 0.0f;
    }
}

//==============================================================================

s32 new_weapon::GetZoomStep( void )
{
    return m_ZoomStep;
}

//==============================================================================

xbool new_weapon::IsZoomEnabled( void )
{   
    return (m_ZoomState != NO_ZOOM_WEAPON_STATE);
}

//==============================================================================

radian new_weapon::GetXFOV( void )
{
    ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );
    ASSERT( m_ZoomState != NO_ZOOM_WEAPON_STATE );

    return m_CurrentViewX;
}

//==============================================================================

f32 new_weapon::GetZoomMovementMod( void )
{
    ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );
    ASSERT( m_ZoomState != NO_ZOOM_WEAPON_STATE );

    return m_ZoomMovementSpeed;
}

//==============================================================================

s32 new_weapon::IncrementZoom( void )
{
    ASSERT( m_CurrentRenderState == RENDER_STATE_PLAYER );

    if ( m_ZoomStep == 0 )
    {
        // first time, start me up
        player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();
    
        m_CurrentViewX = OriginalViewInfo.XFOV;
        m_ZoomState = WEAPON_STATE_ZOOM_IN;
    }

    m_ZoomStep++;

    if ( m_ZoomStep > m_nZoomSteps )
    {
        m_ZoomState = WEAPON_STATE_ZOOM_OUT;
        m_ZoomStep = 0;
    }
    
    return m_ZoomStep;
}

//==============================================================================

void new_weapon::ClearZoom( void )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        object* pParent = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        ASSERT( pParent );
        player& Player = player::GetSafeType( *pParent );
        const player::view_info& OriginalViewInfo = Player.GetOriginalViewInfo();

        m_CurrentViewX = OriginalViewInfo.XFOV;
        m_ZoomState = NO_ZOOM_WEAPON_STATE;
        m_ZoomStep = 0;

        Player.ResetView();
    }
}

//==============================================================================

render_inst* new_weapon::GetRenderInstPtr( void )
{
    if(  IN_RANGE( 0, m_CurrentRenderState, RENDER_STATE_MAX ) )
    {
        return &m_Skin[ m_CurrentRenderState ];
    }
    else
    {
        return NULL;
    }
}

//==============================================================================

geom* new_weapon::GetGeomPtr( void )
{
    render_inst* pRenderInst = GetRenderInstPtr();
    return ( pRenderInst != NULL ) ? pRenderInst->GetGeom() : NULL;
}

anim_group::handle* new_weapon::GetAnimGroupHandlePtr( void )
{
    if( IN_RANGE( 0, m_CurrentRenderState, RENDER_STATE_MAX ) )
    {
        return &m_AnimGroup[ m_CurrentRenderState ];
    }
    else
    {
        return NULL;
    }
}

//==============================================================================

void new_weapon::GetAmmoState( ammo_priority Priority,
                               s32& AmmoAmount,
                               s32& AmmoMax,
                               s32& AmmoPerClip,
                               s32& AmmoInCurrentClip )
{
    AmmoAmount        = m_WeaponAmmo[ Priority ].m_AmmoAmount;
    AmmoMax           = m_WeaponAmmo[ Priority ].m_AmmoMax;
    AmmoPerClip       = m_WeaponAmmo[ Priority ].m_AmmoPerClip;
    AmmoInCurrentClip = m_WeaponAmmo[ Priority ].m_AmmoInCurrentClip;
}
                                                    
//==============================================================================

void new_weapon::SetAmmoState( ammo_priority Priority,
                               s32 AmmoAmount,
                               s32 AmmoMax,
                               s32 AmmoPerClip,
                               s32 AmmoInCurrentClip )
{
    m_WeaponAmmo[ Priority ].m_AmmoAmount         = AmmoAmount;
    m_WeaponAmmo[ Priority ].m_AmmoMax            = AmmoMax;
    m_WeaponAmmo[ Priority ].m_AmmoPerClip        = AmmoPerClip;
    m_WeaponAmmo[ Priority ].m_AmmoInCurrentClip  = AmmoInCurrentClip;
}

//==============================================================================

void new_weapon::SetAmmoState2( ammo_priority Priority,
                              s32 AmmoAmount,
                              s32 AmmoInCurrentClip )
{
    m_WeaponAmmo[ Priority ].m_AmmoAmount         = AmmoAmount;
    m_WeaponAmmo[ Priority ].m_AmmoInCurrentClip  = AmmoInCurrentClip;
}

//===========================================================================
// this function is used to orient a sprite to a collision plane to cover up the ugliness of two Oriented Quads intersecting
void new_weapon::DrawLaserFixupBitmap( xbitmap* pBitmap, f32 Radius, xcolor cColor, collision_mgr::collision& Coll )
{
    vector3 theNormal   = Coll.Plane.Normal;
    vector3 CenterPoint = Coll.Point;

    // set fixup texture
    draw_SetTexture( *pBitmap );

    // pull back point a tad along Normal
    CenterPoint = CenterPoint + theNormal * 0.5f;

    //
    // Get the plane-parallel axiis and build the three world-space positions.
    // Because of floating-point error, the ortho vectors can get turned 90 degrees
    // when hitting a wall. Handle that as a special-case by forcing the "up"
    // vector to be the Y axis.
    //
    vector3 AxisA, AxisB;
    if( theNormal.GetY() < 0.001f )
    {
        vector3 Dir( 0.0f, 1.0f, 0.0f );
        AxisA = theNormal.Cross( Dir );
        AxisB = theNormal.Cross( AxisA );
        AxisA.Normalize();
        AxisB.Normalize();
    }
    else
    {
        Coll.Plane.GetOrthoVectors( AxisA, AxisB );
    }

    // Scale the axis    
    AxisA *= Radius;
    AxisB *= Radius;

    // Vertices
    //  0,5 _____ 1
    //      |\  |
    //      | \ |
    //    4 |__\| 2,3

    // UVs
    //(0,0),(0,0)_____(1,0)
    //           |\  |
    //           | \ |
    //  (0,1)    |__\| (1,1), (1,1)

    // Build the quad points from the axis (Clock-wise)
    vector3 Pos[6];
    Pos[0] = CenterPoint + AxisA + AxisB;
    Pos[1] = CenterPoint + AxisA - AxisB;
    Pos[2] = CenterPoint - AxisA - AxisB;
    Pos[3] = CenterPoint - AxisA - AxisB;
    Pos[4] = CenterPoint - AxisA + AxisB;
    Pos[5] = CenterPoint + AxisA + AxisB;
    
    // set up the draw color to the same color as the "laser"
    draw_Color(cColor);

    draw_UV(0.0f, 0.0f);
    draw_Vertex(Pos[0]);

    draw_UV(1.0f, 0.0f);
    draw_Vertex(Pos[1]);

    draw_UV(1.0f, 1.0f);
    draw_Vertex(Pos[2]);

    draw_UV(1.0f, 1.0f);
    draw_Vertex(Pos[3]);

    draw_UV(0.0f, 1.0f);
    draw_Vertex(Pos[4]);

    draw_UV(0.0f, 0.0f);
    draw_Vertex(Pos[5]);
}

//=============================================================================

xbitmap* new_weapon::GetScopeTexture( void )
{
    if ( m_ScopeMesh < 0 )
        return NULL;

    skin_inst& Skin  = m_Skin[ RENDER_STATE_PLAYER ];
    geom*      pGeom = Skin.GetGeom();
    if ( pGeom == NULL )
        return NULL;

    rhandle<texture> RTex;
    geom::mesh&      Mesh    = pGeom->m_pMesh[m_ScopeMesh];
    geom::submesh&   SubMesh = pGeom->m_pSubMesh[Mesh.iSubMesh];
    geom::material&  Mat     = pGeom->m_pMaterial[SubMesh.iMaterial];
    
    RTex.SetName( pGeom->GetTextureName( Mat.iTexture ) );
    texture* pTexture = RTex.GetPointer();
    if ( !pTexture )
        return NULL;

    return &pTexture->m_Bitmap;
}

//=============================================================================

void new_weapon::InstallCustomScope( void )
{
    // It is possible that OnProperty gets called multiple times, or we could
    // be changing the mesh in the editor, so first uninstall whatever we
    // already had
    UninstallCustomScope();

#ifdef TARGET_PC
    // sorry, no sniper scope customization on the pc
    return;
#endif

    // no geometry == no scope texture
    skin_inst& Skin  = m_Skin[ RENDER_STATE_PLAYER ];
    geom*      pGeom = Skin.GetGeom();
    if ( pGeom == NULL )
        return;

    // is there a mesh called SCOPE?
    m_ScopeMesh = pGeom->GetMeshIndex( "SCOPE" );
    if ( m_ScopeMesh >= 0 )
    {
        CLOG_MESSAGE( LOG_SCOPE_PATCHING, "SCOPE", "Install Ref(%d), Guid(%08X:%08X)", m_ScopeRefCount, GetGuid().GetHigh(), GetGuid().GetLow() );

        // set up the overlay, since we'll need to force that to render last
        m_ScopeLensMesh = pGeom->GetMeshIndex( "SCOPE_LENS" );

        // set up the custom scope texture
        xbitmap* pBitmap = GetScopeTexture();
        ASSERT( pBitmap );
        if ( pBitmap  )
        {
            // patch the scope with our custom texture if no other
            // instance has done it yet
            s32 VramID = pBitmap->GetVRAMID();
            if( VramID != m_ScopeTextureVramId )
            {
                ASSERT( m_OrigScopeVramId == -1 );
                m_OrigScopeVramId = VramID;
                pBitmap->SetVRAMID( m_ScopeTextureVramId );
                CLOG_MESSAGE( LOG_SCOPE_PATCHING, "SCOPE", "Patched OrigId=%d, ScopeId=%d", m_OrigScopeVramId, m_ScopeTextureVramId );
            }

            // let everyone know we're using the custom texture now
            m_ScopeRefCount++;
        }
        
        // Force render library to use top mip (we only have 1!) always since the render
        // library thinks the bitmap has mips since we just cheated and set the bitmap vram id 
        // to use the pips!
        geom::mesh& Mesh = pGeom->m_pMesh[m_ScopeMesh];
        pGeom->m_pSubMesh[Mesh.iSubMesh].WorldPixelSize = 1000.0f;
    }
}

//=============================================================================

void new_weapon::UninstallCustomScope( void )
{
    // handle any reference counting or custom scope texture cleanup
    if( m_ScopeMesh != -1 )
    {
        CLOG_MESSAGE( LOG_SCOPE_PATCHING, "SCOPE", "Uninstall Ref(%d), Guid(%08X:%08X)", m_ScopeRefCount, GetGuid().GetHigh(), GetGuid().GetLow() );

        xbitmap* pBitmap = GetScopeTexture();
        ASSERT( pBitmap );
        if( pBitmap )
        {
            m_ScopeRefCount--;
            ASSERT( m_ScopeRefCount >= 0 );

            // if everyone is done using the custom texture, then restore the
            // geometry back to its original state
            if( m_ScopeRefCount == 0 )
            {
                CLOG_MESSAGE( LOG_SCOPE_PATCHING, "SCOPE", "Restore OrigId=%d, ScopeId=%d", m_OrigScopeVramId, m_ScopeTextureVramId );
                ASSERT( m_OrigScopeVramId != -1 );
                pBitmap->SetVRAMID( m_OrigScopeVramId );
                m_OrigScopeVramId = -1;
            }
        }

        m_ScopeMesh = -1;
    }
}

//=============================================================================

#define SCISSOR_LEFT    (2048-(VRAM_FRAME_BUFFER_WIDTH/2))
#define SCISSOR_TOP     (2048-(VRAM_FRAME_BUFFER_HEIGHT/2))

void new_weapon::CreateScopeTexture( void )
{
    if ( m_ScopeTextureVramId == -1 )
        return;

    //TODO:wouldn't it be nice if there was a generic way to do this?!?
#if defined(TARGET_PS2)
    // flush the diffuse bank so we're guaranteed to be on a page boundary
    vram_FlushBank( 0 );
    vram_Activate( m_ScopeTextureVramId );

    // copy out part of the frame buffer into this scope texture
    const view* pActiveView = eng_GetView();

    // grab the necessary texture addresses
    s32 VramAddr = vram_GetPixelBaseAddr( m_ScopeTextureVramId );
    s32 FBP      = eng_GetFrameBufferAddr( 0 ) / 2048;

    // We really want the center 128x128 portion of the screen, but we'd like
    // to keep the ratio similar for split-screen play. With a normal screen
    // size of 512x448, this is the resulting formula.
    s32 L, T, R, B;
    pActiveView->GetViewport( L, T, R, B );
    f32 U0 = L +  (3.0f/8.0f)*(f32)(R-L);
    f32 U1 = L +  (5.0f/8.0f)*(f32)(R-L);
    f32 V0 = T + (5.0f/14.0f)*(f32)(B-T);
    f32 V1 = T + (9.0f/14.0f)*(f32)(B-T);

    // now set up the appropriate frame buffer, texture, and alpha settings
    // and copy off that portion of the screen
    gsreg_Begin( 21 );
    gsreg_Set( SCE_GS_TEX0_1, SCE_GS_SET_TEX0( FBP * 32,
                                               VRAM_FRAME_BUFFER_WIDTH/64,
                                               SCE_GS_PSMCT32,
                                               vram_GetLog2(VRAM_FRAME_BUFFER_WIDTH),
                                               vram_GetLog2(VRAM_FRAME_BUFFER_HEIGHT),
                                               0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( VramAddr / 32,
                                                 kScopeTextureW/64,
                                                 SCE_GS_PSMCT32,
                                                 0x00000000 ) );
    gsreg_SetScissor( 0, 0, kScopeTextureW, kScopeTextureH );
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE( C_ZERO, C_ZERO, A_FIX, C_SRC ) );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 64, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_PRIM,     SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,    SCE_GS_SET_RGBAQ( 0x80, 0x80, 0x80, 0x80, 0x3f800000 ) );
    gsreg_Set( SCE_GS_UV,       SCE_GS_SET_UV( (s32)(U0*16.0f), (s32)(V0*16.0f) ) );
    gsreg_Set( SCE_GS_XYZ2,     SCE_GS_SET_XYZ2( SCISSOR_LEFT<<4, SCISSOR_TOP<<4, 0 ) );
    gsreg_Set( SCE_GS_UV,       SCE_GS_SET_UV( (s32)(U1*16.0f), (s32)(V1*16.0f) ) );
    gsreg_Set( SCE_GS_XYZ2,     SCE_GS_SET_XYZ2( (SCISSOR_LEFT+kScopeTextureW)<<4, (SCISSOR_TOP+kScopeTextureH)<<4, 0 ) );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( VramAddr / 32,
                                                 kScopeTextureW/64,
                                                 SCE_GS_PSMCT32,
                                                 0x00FFFFFF ) );
    gsreg_Set( SCE_GS_PRIM,     SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,    SCE_GS_SET_RGBAQ( 0x80, 0x80, 0x80, 64, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,     SCE_GS_SET_XYZ2( SCISSOR_LEFT<<4, SCISSOR_TOP<<4, 0 ) );
    gsreg_Set( SCE_GS_XYZ2,     SCE_GS_SET_XYZ2( (SCISSOR_LEFT+kScopeTextureW)<<4, (SCISSOR_TOP+kScopeTextureH)<<4, 0 ) );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_SetScissor( L, T, R, B );
    gsreg_SetFBMASK( 0x00000000 );  // will restore the frame buffer
    gsreg_End();

#elif defined(TARGET_XBOX)

    extern void xbox_FrameCopy( s32 VRAMID );
    {
        xbox_FrameCopy( m_ScopeTextureVramId );
    }

#endif // TARGET_??
}
