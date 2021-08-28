//=========================================================================
// WEAPON SCANNER
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "ProjectileBullett.hpp"
#include "WeaponScanner.hpp"
#include "Debris\debris_mgr.hpp"
#include "Objects\Projector.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Player.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\LoreObject.hpp"
#include "Characters\Character.hpp"
#include "corpse.hpp"
#include "hud_Player.hpp"
#include "HudObject.hpp"
#include "hud_Scanner.hpp"

#ifndef X_EDITOR
#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#endif


//=========================================================================
// STATIC DEFINTIONS AND CONSTANTS
//=========================================================================
        xcolor g_ScanEnc_Color( 250, 49, 49, 100 );
        xcolor g_ScanEncFlash_Color( 255, 200, 200, 100 );
const   xcolor g_Scan_LaserColor( 49, 49, 250, 255 );
const   xcolor g_Scan2_LaserColor( 49, 250, 49, 255 );

f32 s_InvalidMsgFadeTime    = 2.0f;
xbool g_RenderScannerHit    = FALSE;
f32 g_ScanFlashTime         = 0.05f;
u8 Scanner_DrawType         = 0;
f32 g_Scan_OO               = 400.0f;// not on object offset
f32 g_Scan_O                = 150.0f;// object offset
u16  g_ScansPerFace         = 2;
xbool g_TestScanPolys       = FALSE;
u32 g_NumScanFlashes        = 3;
f32 g_LoreUpdateTime        = 0.6f;

static s32 Scanner_Laser_Alpha    = 100;
static f32 Scanner_Laser_Size      = 4.0f;
static s16 ScanTestPoly[] = {6,2,0} ;

#define MAX_SCAN_POINTS 24

tweak_handle Scanner_Max_Scan_TimeTweak   ("SCANNER_Max_Scan_Time");      // How long does a full scan take

// distances
tweak_handle Lore_Max_Detect_DistanceTweak ("Lore_Max_Detect_Distance");  // what is the max distance at which our Geiger will pick up a lore object
tweak_handle Lore_Mean_Detect_DistanceTweak("Lore_Mean_Detect_Distance"); // Mid-range distance in centimeters that a lore object can be detected (this is for changine sound frequencies).
tweak_handle Lore_Min_Detect_DistanceTweak ("Lore_Min_Detect_Distance");  // Within this distance is the minimum detect distance (used for changing sound frequency and scanner "search" distance).

tweak_handle Lore_Min_Detect_AngleTweak    ("Lore_Min_Detect_Angle");     // Within this angle is the where the lore object can be scanned.


struct VO_table_entry
{
    s32         Type;
    s32         IDStart;
    s32         IDEnd;
};

static VO_table_entry s_VOTable[] =
{
    { VO_SCAN_SELF,          1,  3    },
    { VO_SCAN_SELF_MUT,     14, 16    }, 
    { VO_SCAN_LORE,          4, 13    },
    { VO_SCAN_NOTHING,       7,  8    },
};


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct weapon_scanner_desc : public object_desc
{

    weapon_scanner_desc( void ) : object_desc( object::TYPE_WEAPON_SCANNER, 
                                        "Scanner",
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
        return new weapon_scanner;
    }

} s_Weapon_Scanner_Desc;

//=========================================================================

const object_desc&  weapon_scanner::GetTypeDesc( void ) const
{
    return s_Weapon_Scanner_Desc;
}

//=========================================================================

const object_desc&  weapon_scanner::GetObjectType( void )
{
    return s_Weapon_Scanner_Desc;
}

//=========================================================================

weapon_scanner::weapon_scanner( void )
{
	//initialize the ammo structures.
	m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileType = BULLET_PISTOL;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = 0;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
	m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = 0;
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
    m_AimDegradePrimary = 0.1f;
    m_AimRecoverSpeed = 0.9f;

    m_fScanTime     = GetMaxScanTime();
    m_ScanStartGuid = NULL_GUID;
    m_ScanEndGuid   = NULL_GUID;

    m_Item = INVEN_WEAPON_SCANNER;

    m_ScanTimesPerFace = 0;
    m_bCanScan = TRUE;
    m_bInitialScan = TRUE;
    m_bFlashEnclosures = FALSE;
    m_NumberOfFlashes = 0;

    m_LaserOnLoopId = 0;

    // force lore update to run on first pass
    m_LastLoreUpdate = 0.0f;

    // what are we scanning
    m_ScanState = SCAN_NONE;
    m_ScannedGuid = NULL_GUID;

    m_LaserBitmap.SetName( PRELOAD_FILE("Tracer_Laser.xbmp") );
    m_LaserFixupBitmap.SetName( PRELOAD_FILE("Tracer_Glow.xbmp") );    
    m_FXScannerBox.SetName( PRELOAD_FILE("Scanner_Box.fxo") );

#ifndef X_EDITOR
    if ( m_LaserBitmap.GetPointer() == NULL )
    {
        ASSERTS( 0, xfs( "Unable to load %s", m_LaserBitmap.GetName() ) );
    }

    if ( m_LaserFixupBitmap.GetPointer() == NULL )
    {
        ASSERTS( 0, xfs( "Unable to load %s", m_LaserFixupBitmap.GetName() ) );
    }

    if( m_FXScannerBox.GetPointer() == NULL )
    {
        ASSERTS( 0, xfs( "Unable to load %s", m_FXScannerBox.GetName() ) );
    }

#endif

    m_AutoSwitchRating = 0;
}

//=========================================================================

weapon_scanner::~weapon_scanner()
{    
    KillScanEffect();
}

//=========================================================================
xbool weapon_scanner::CanFire( xbool bIsAltFire )
{ 
    (void) bIsAltFire;

    return CanScan();
}

//=========================================================================
xbool weapon_scanner::CanScan( void )
{
    if( m_bCanScan && m_bBootUpAnimFinished )
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================
void weapon_scanner::BeginSwitchFrom( void )
{
    ClearScan();
}

//=========================================================================
void weapon_scanner::BeginSwitchTo( void )
{
    m_bBootUpAnimFinished = FALSE;
}

//=========================================================================
void weapon_scanner::EndSwitchTo( void )
{
    m_bBootUpAnimFinished = TRUE;
}


xbool weapon_scanner::FireNPCWeaponProtected( const vector3& BaseVelocity , const vector3& Target , const guid& Owner, f32 fDegradeMultiplier, const xbool isHit  )
{
    (void)BaseVelocity;
    (void)Target;
    (void)fDegradeMultiplier;
    (void)isHit;
    
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

        //vector3 TargetVector   = Target - InitPos ; 
        //radian3 TargetRot(TargetVector.GetPitch(), TargetVector.GetYaw(), 0.0f );

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

xbool weapon_scanner::FireWeaponProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
{
    ( void )Power;
    ( void )iFirePoint;
    ( void )BaseVelocity;
    ( void )Owner;
    ( void )InitRot;

    m_ScannedGuid = NULL_GUID;
	
    // start scan but, don't pick up any lore objects if this isn't the first sample.
    if( m_ScanStartGuid == NULL_GUID && m_bInitialScan && CanScan() )
	{
        // initial scan
        m_bInitialScan = FALSE;
        
        PlayLaserSound();

        m_ScanStartGuid = CheckHitObject();

        object *pObj = g_ObjMgr.GetObjectByGuid( m_ScanStartGuid );

        // default
        m_ScanState = SCAN_AIR;

        if( pObj )
        {
            bbox BBox = pObj->GetBBox();
            if( pObj->IsKindOf( lore_object::GetRTTI() ) )
            {
                lore_object *pLO = (lore_object*)pObj;
                if( !pLO->IsTrueLoreObject() )
                {
                    //  KSS -- other types of objects here, i.e. walls, etc.
                    m_ScanState = SCAN_OBJECT;
                }
                else
                {
                    m_ScanState = SCAN_LORE;
                }

                BBox = pLO->GetFocusBBox();
            }
            else
            if( pObj->IsKindOf( character::GetRTTI() ) )
            {
                character *pCharacter = (character*)pObj;
                pCharacter->NotifyScanBegin();

                m_ScanState = SCAN_CHARACTER;
            }
            else
            if( pObj->IsKindOf( corpse::GetRTTI() ) )
            {
                m_ScanState = SCAN_CORPSE;
            }            
        
            // gather bounding box verts so that we can use them to plot points and bounding boxes with our scan and sampling lasers
            m_BBoxVerts[0].GetX() = BBox.Min.GetX();    m_BBoxVerts[0].GetY() = BBox.Min.GetY();    m_BBoxVerts[0].GetZ() = BBox.Min.GetZ();
            m_BBoxVerts[1].GetX() = BBox.Min.GetX();    m_BBoxVerts[1].GetY() = BBox.Min.GetY();    m_BBoxVerts[1].GetZ() = BBox.Max.GetZ();
            m_BBoxVerts[2].GetX() = BBox.Min.GetX();    m_BBoxVerts[2].GetY() = BBox.Max.GetY();    m_BBoxVerts[2].GetZ() = BBox.Min.GetZ();
            m_BBoxVerts[3].GetX() = BBox.Min.GetX();    m_BBoxVerts[3].GetY() = BBox.Max.GetY();    m_BBoxVerts[3].GetZ() = BBox.Max.GetZ();
            m_BBoxVerts[4].GetX() = BBox.Max.GetX();    m_BBoxVerts[4].GetY() = BBox.Min.GetY();    m_BBoxVerts[4].GetZ() = BBox.Min.GetZ();
            m_BBoxVerts[5].GetX() = BBox.Max.GetX();    m_BBoxVerts[5].GetY() = BBox.Min.GetY();    m_BBoxVerts[5].GetZ() = BBox.Max.GetZ();
            m_BBoxVerts[6].GetX() = BBox.Max.GetX();    m_BBoxVerts[6].GetY() = BBox.Max.GetY();    m_BBoxVerts[6].GetZ() = BBox.Min.GetZ();
            m_BBoxVerts[7].GetX() = BBox.Max.GetX();    m_BBoxVerts[7].GetY() = BBox.Max.GetY();    m_BBoxVerts[7].GetZ() = BBox.Max.GetZ();
        }

        // if we can scan, play effect        
        {
            // show scan effect
            vector3 Pos;
            GetFiringBonePosition(Pos);
            m_ScanEffect = particle_emitter::CreatePresetParticleAndOrient( 
                                                        "WPN_Scanner.fxo",
                                                        vector3(0.0f,0.0f,0.0f),
                                                        Pos );
                                                        
            // Update initial scanning laser position/rotation
            UpdateScanEffect();

            // may be scanning an air sample, i.e. no object
            if( pObj )
            {
                // Setup effect                
                m_ScanBoxHandle.InitInstance( m_FXScannerBox.GetPointer() );

                // get BBox position and size
                Pos = pObj->GetBBox().GetCenter();
                const vector3& BBoxSize = pObj->GetBBox().GetSize();
                m_ScanBoxHandle.SetTranslation( Pos );

                // set up scale values (particle is built at 1 meter = 100.0f cm)
                f32 Xscale = BBoxSize.GetX()/100.0f;
                f32 Yscale = BBoxSize.GetY()/100.0f;
                f32 Zscale = BBoxSize.GetZ()/100.0f;

                // don't let Y get too high or too low
                Yscale = x_clamp(Yscale, 0.25f, 5.0f);

                vector3 BoxScale = vector3(Xscale, Yscale, Zscale);

                // scan box source is 1 meter across
                m_ScanBoxHandle.SetScale( BoxScale );
            }
        }
		
        //add a muzzle light where the bullet was fired from (will fade out really quickly)
        (void)InitPos;

        //g_LightMgr.AddFadingLight( InitPos, xcolor( 76, 76, 38 ), 200.0f, 3.0f, 0.1f );

		return TRUE;
	}

    return TRUE;
}

//===========================================================================
void weapon_scanner::PlayLaserSound( void )
{
    if( CanScan() && (m_LaserOnLoopId == 0) && (m_CurrentRenderState == RENDER_STATE_PLAYER) )
    {
        m_LaserOnLoopId = g_AudioMgr.Play( "Scanner_Fire" );
    }
}

//===========================================================================
void weapon_scanner::KillLaserSound( void )
{
    if( m_LaserOnLoopId )
    {
        g_AudioMgr.Release(m_LaserOnLoopId, 0.0f);
        m_LaserOnLoopId = 0;
    }
}

f32 g_ScannerTestShereSize = 1.0f;
//===========================================================================
void weapon_scanner::RenderWeapon(xbool bDebug, const xcolor& Ambient, xbool Cloaked )
{
    new_weapon::RenderWeapon(bDebug, Ambient, Cloaked);

#ifndef X_RETAIL
    #ifdef ksaffel
        if( g_RenderScannerHit )
        {
            // Get the player.
            player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
            view &View = Player.GetView();

            // cast a sphere out of the player's eyes and look for a lore object
            vector3 StartPos = View.GetPosition();

            // check collisions
            vector3 EndPos;
            guid ObjGuid = NULL_GUID;
            if( !DoCollisionCheck( EndPos, ObjGuid ) )
            {
                radian Pitch, Yaw;
                Player.GetEyesPitchYaw( Pitch, Yaw );

                vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
                const vector3 ViewPos = View.GetPosition();
                Dest *= Lore_Max_Detect_DistanceTweak.GetF32();
                EndPos = ViewPos + Dest;
            }

            // default modifier to full distance in case the collision manager returns no collisions
            f32 DistModifier = 1.0f;

            // if we don't hit anything, T is undefined
            if( g_CollisionMgr.m_nCollisions > 0 )
            {
                DistModifier = g_CollisionMgr.m_Collisions[0].T;
            }

            // get our new end position
            EndPos = StartPos + (DistModifier*(EndPos-StartPos));

            draw_Line(StartPos, EndPos);
            draw_Sphere(EndPos, g_ScannerTestShereSize);
        }
    #endif
#endif
}

//===========================================================================
f32 g_CharDetectAngle = 0.72f;
xbool weapon_scanner::GetObjectPositionalInfo( guid objGuid, radian &AngleBetween, vector3 &StartPos, vector3 &EndPos )
{
    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
    view &View = Player.GetView();

    f32 MaxScanDist = Lore_Min_Detect_DistanceTweak.GetF32();
    radian MaxDetectAngle = Lore_Min_Detect_AngleTweak.GetRadian();

    // cast a sphere out of the player's eyes and look for a lore object
    StartPos = View.GetPosition();
    EndPos   = StartPos + View.GetViewZ() * MaxScanDist;

    object* pObj = g_ObjMgr.GetObjectByGuid( objGuid );

    // sent in bad guid
    if( !pObj )
    {
        return FALSE;
    }

    vector3 ObjPos = pObj->GetBBox().GetCenter();

    // if it's a corpse or a character, get the center of their BBox
    if( pObj->IsKindOf(character::GetRTTI()) ||
        pObj->IsKindOf(corpse::GetRTTI()) )
    {
        MaxDetectAngle = g_CharDetectAngle;
    }

    // get the vector to the lore object
    vector3 ToObjVector = ObjPos - StartPos;
    f32 theDist = ToObjVector.Length();

    if( pObj && theDist <= MaxScanDist )
    {
        // get the vector coming out of our eyes to the reticle
        vector3 EyeVector = EndPos - StartPos;                

        // Get angle difference between the vectors
        AngleBetween = v3_AngleBetween( EyeVector, ToObjVector );

        // good one?
        if( (AngleBetween <= MaxDetectAngle) )
        {
            EndPos = ObjPos;
            return TRUE;
        }
    }
    

    return FALSE;
}


//=========================================================================
f32 g_ScanSelfPitch = R_60;
xbool weapon_scanner::DoCollisionCheck( vector3 &EndPoint, guid &ObjGuid )
{   
    vector3 StartPos, EndPos;
    radian Angle;
    radian BestAngle = R_90;
    guid TempGuid = NULL_GUID;
    guid BestGuid = NULL_GUID;

    // clear it out just in case calling function didn't
    ObjGuid = NULL_GUID;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( !pObj || !pObj->IsKindOf( player::GetRTTI() ) )
    {
        ASSERT(0);
        return FALSE;
    }

    player *pPlayer = (player*)pObj;

    // Collect lore objects
    {
        slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_LORE_OBJECT );
        while( Slot != SLOT_NULL )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( Slot );
            Slot = g_ObjMgr.GetNext( Slot );

            lore_object *pLO = (lore_object*)pObj;
          
            if( pLO && pLO->IsActivated() && pLO->IsStandingIn() && !(pObj->GetAttrBits() & object::ATTR_DESTROY) )
            {
                TempGuid = pLO->GetGuid();
                
                if( GetObjectPositionalInfo( TempGuid, Angle, StartPos, EndPos ) )
                {
                    // check all collisions, don't exclude anything
                    g_CollisionMgr.RaySetup( m_ParentGuid, pPlayer->GetEyesPosition(), pLO->GetBBox().GetCenter() );
                    g_CollisionMgr.CheckCollisions(object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS);

                    // can we see THIS object
                    if( (g_CollisionMgr.m_nCollisions > 0) && 
                        (g_CollisionMgr.m_Collisions[0].ObjectHitGuid == TempGuid) &&
                        Angle < BestAngle )
                    {
                        // Save off best angle and guid.
                        BestAngle = Angle;
                        BestGuid = TempGuid;
                    }
                }
            }
        }
    }

    // Collect character objects
    {
        actor* pActor = actor::m_pFirstActive;
        while( pActor )
        {
            if( pActor->IsKindOf(character::GetRTTI() ) )
            {
                //if( pActor->GetAttrBits() & object::ATTR_LIVING )
                if( !(pActor->GetAttrBits() & object::ATTR_DESTROY) )
                {
                    TempGuid = pActor->GetGuid();

                    if( GetObjectPositionalInfo( TempGuid, Angle, StartPos, EndPos ) )
                    {
                        // go through and find best angle... always doing a collision check until the last best angle is reached.
                        if( Angle < BestAngle )
                        {
                            // check all collisions, don't exclude anything
                            g_CollisionMgr.RaySetup( m_ParentGuid, pPlayer->GetEyesPosition(), pActor->GetBBox().GetCenter() );

                            g_CollisionMgr.CheckCollisions(object::TYPE_ALL_TYPES, object::ATTR_LIVING);

                            // can we see THIS object?
                            if( (g_CollisionMgr.m_nCollisions > 0) && 
                                (g_CollisionMgr.m_Collisions[0].ObjectHitGuid == TempGuid) )
                            {
                                // Save off best angle and guid.
                                BestAngle = Angle;
                                BestGuid = TempGuid;
                            }
                        }
                    }
                }
            }

            pActor = pActor->m_pNextActive;
        }
    }

    // Collect corpse objects
    {
        slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_CORPSE );
        while( Slot != SLOT_NULL )
        {
            corpse* pCorpse = (corpse*)g_ObjMgr.GetObjectBySlot( Slot );
            Slot = g_ObjMgr.GetNext( Slot );

            if( pCorpse && !(pCorpse->GetAttrBits() & object::ATTR_DESTROY))
            {
                TempGuid = pCorpse->GetGuid();

                if( GetObjectPositionalInfo( TempGuid, Angle, StartPos, EndPos ) )
                {
                    // go through and find best angle... always doing a collision check until the last best angle is reached.
                    if( Angle < BestAngle )
                    {
                        // check all collisions, don't exclude anything
                        g_CollisionMgr.RaySetup( m_ParentGuid, pPlayer->GetEyesPosition(), pCorpse->GetBBox().GetCenter() );
                        g_CollisionMgr.CheckCollisions(object::TYPE_CORPSE, object::ATTR_ALL);

                        // can we see THIS object
                        if( (g_CollisionMgr.m_nCollisions > 0) && 
                            (g_CollisionMgr.m_Collisions[0].ObjectHitGuid == TempGuid) )
                        {
                            // Save off best angle and guid.
                            BestAngle = Angle;
                            BestGuid = TempGuid;
                        }
                    }
                }
            }
        }
    }

    // we have a good guid... see if we can see it.
    if( BestGuid != NULL_GUID )
    {
        EndPoint = EndPos;
        ObjGuid = BestGuid;

        return TRUE;
    }
    else
    {
        // check for player scan here        
        player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );
        
        // if we're lower than a certain angle and we don't have anything to scan, scan ourselves!
        if( Player.GetPitch() >= g_ScanSelfPitch )
        {
            ObjGuid = m_ParentGuid;
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================
guid weapon_scanner::CheckHitObject( void )
{
    vector3 EndPos;
    guid ObjGuid = NULL_GUID;

    // check collisions
    if( DoCollisionCheck( EndPos, ObjGuid ) )
    {
        // we have collisions or we are scanning ourselves, continue on with this object.
        if( g_CollisionMgr.m_nCollisions || (ObjGuid == m_ParentGuid) )
        {
            object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
            
            // we found an object
            if( pObject )
            {
                return ObjGuid;
            }
        }
    }

    return NULL_GUID;
}

//=========================================================================

xbool weapon_scanner::FireSecondaryProtected( const vector3& InitPos , const vector3& BaseVelocity, const f32& Power , const radian3& InitRot , const guid& Owner, s32 iFirePoint )
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

void weapon_scanner::FireBullet( const vector3& Pos, const radian3& Rot, const vector3& InheritedVelocity, guid Owner, const xbool bHitLiving )
{
    return;

    base_projectile* BP;
    ASSERT( m_WeaponAmmo[ AMMO_PRIMARY ].m_ProjectileTemplateID >= 0 );

    object* pOwnerObject = g_ObjMgr.GetObjectByGuid( Owner );
    if( !pOwnerObject )
        return;

    pain_handle PainHandle( xfs("%s_%s",pOwnerObject->GetLogicalName(),GetLogicalName()) );
    BP = CreateBullet( GetLogicalName(), Pos, Rot, InheritedVelocity, Owner, PainHandle, AMMO_PRIMARY, bHitLiving );
}

//==============================================================================
extern xbool s_bUseTestMap;
extern s32 s_ScannerTestMap;

void weapon_scanner::UpdateLoreCount(f32 DeltaTime)
{
#ifndef X_EDITOR
    m_LastLoreUpdate -= DeltaTime;

    // don't do this all of the time.
    // KSS -- FIXME -- Is this really that expensive that we need this?
    if( m_LastLoreUpdate <= F32_MIN )
    {
        s32 MapID = -1;
        if( s_bUseTestMap )
        {
            // For testing
            MapID = s_ScannerTestMap;
        }
        else
        {
            MapID = g_ActiveConfig.GetLevelID();
        }
               
        u32 TotalLore   = g_StateMgr.GetTotalLoreAcquired();
        u32 LevelLore   = g_StateMgr.GetLevelLoreAcquired(MapID);
        m_LastLoreUpdate = g_LoreUpdateTime;

        // hack into the ammo hud and put our number of lore acquired for this level into it.
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoPerClip = LevelLore;
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoInCurrentClip = LevelLore;

        // we have to trick the ammo hud into thinking that we're not pulling numbers out of the total
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax = TotalLore + LevelLore;
        m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoAmount = m_WeaponAmmo[ AMMO_PRIMARY ].m_AmmoMax;
    }

#endif
}

//==============================================================================
void weapon_scanner::OnAdvanceLogic( f32 DeltaTime )
{
    new_weapon::OnAdvanceLogic( DeltaTime );

    UpdateLoreCount(DeltaTime);

    // Get the player.
    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );

    if( Player.IsFiring() )
    {
        if( CanScan() && !m_bInitialScan )
        {
            //UpdateScanParticles(DeltaTime);
            m_fScanTime -= DeltaTime;

            if( m_ScanStartGuid != NULL_GUID )
            {                    
                vector3 StartPos, EndPos;
                radian Angle;

                // moved off of lore object, clear scan
                if( !GetObjectPositionalInfo( m_ScanStartGuid, Angle, StartPos, EndPos ) )
                {   
                    // play "fail" dialog here
                    char pIdentifier[256] = {'\0'};

                    // Get Lore Ident here
                    GetScannerVOIdent( pIdentifier, VO_SCAN_NOTHING );
                    g_AudioMgr.Play(pIdentifier);

#ifndef X_EDITOR
                    // Get the player.
                    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );

                    // invalid scan
                    MsgMgr.Message( MSG_SCAN_INVALID, Player.net_GetSlot() );
#endif

                    ClearScan();
                    return;
                }
                else
                {
                    object *pObj = g_ObjMgr.GetObjectByGuid( m_ScanStartGuid );

                    // move scan box with object
                    if( pObj && m_ScanBoxHandle.Validate() )
                    {
                        m_ScanBoxHandle.SetTranslation( pObj->GetBBox().GetCenter() );
                    }
                }
            }

            if( m_ScanIndex >= MAX_SCAN_POINTS )
            {                
                FlashLogic(DeltaTime);

                // See if our flashes are complete
                if( m_NumberOfFlashes >= g_NumScanFlashes )
                {
                    m_bFlashEnclosures = FALSE;

                    CheckForScanComplete();
                }
            }
            else if( m_ScanStartGuid == NULL_GUID )
            {
                m_bFlashEnclosures = FALSE;

                // make sure if we aren't scanning a lore object that we just use the timer.                
                CheckForScanComplete();                
            }
        }
    }
    else
    {
        ClearScan();
        m_bCanScan = TRUE;
        m_bInitialScan = TRUE;
    }

    // Update scanning laser position/rotation
    UpdateScanEffect();

    // make sure the scanner is going
    m_ScanBoxHandle.AdvanceLogic( DeltaTime );
}

//==============================================================================
void weapon_scanner::FlashLogic( f32 DeltaTime )
{
    if( m_NumberOfFlashes < g_NumScanFlashes )
    {
        m_CurrentFlashTime -= DeltaTime;
        if( m_CurrentFlashTime <= F32_MIN )
        {   
            if( m_bFlashEnclosures )
            {
                // already flashing, turn them off
                m_bFlashEnclosures = FALSE;
                m_CurrentFlashTime = g_ScanFlashTime;
            }
            else
            {
                // flash enclosures
                m_bFlashEnclosures = TRUE;
                m_NumberOfFlashes++;

                g_AudioMgr.Play("Toggle");

                m_CurrentFlashTime = g_ScanFlashTime;
            }
        }
    }
}

//==============================================================================
void weapon_scanner::GetScannerVOIdent( char *pIdentifier, eVOIdentifiers Ident )
{
    // lore objects are from 4 to 13
    s32 index = x_irand(s_VOTable[Ident].IDStart, s_VOTable[Ident].IDEnd);

    x_sprintf(pIdentifier, "COLE_ANVO_%02d", index );
}

//==============================================================================
void weapon_scanner::CheckForScanComplete( void )
{
    m_ScannedGuid = NULL_GUID;    

    if( m_fScanTime <= F32_MIN )
    {
        char pIdentifier[256] = {'\0'};

        player *pPlayer = NULL;
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
        {
            pPlayer = (player*)pObj;
        }

        // scan complete
        m_ScanEndGuid = CheckHitObject();

        // same guid, finish it and notify lore item is acquired
        if( (m_ScanEndGuid == m_ScanStartGuid) && 
            m_ScanEndGuid   != NULL_GUID && 
            m_ScanStartGuid != NULL_GUID  )
        {
            if( m_ScanState == SCAN_LORE || m_ScanState == SCAN_OBJECT )
            {
                // deactivate lore item (should we delete?)
                lore_object *pLO = (lore_object*)g_ObjMgr.GetObjectByGuid(m_ScanStartGuid);

                // tell it we took/activated this item
                pLO->OnAcquire();

                // Get Lore Ident here
                GetScannerVOIdent( pIdentifier, VO_SCAN_LORE );
                g_AudioMgr.Play(pIdentifier);

                m_ScannedGuid = m_ScanEndGuid;
            }
            else  // do other non-lore samples here
            {
                if( m_ScanState == SCAN_CHARACTER )
                {
                    character *pCharacter = (character*)g_ObjMgr.GetObjectByGuid(m_ScanStartGuid);
                    pCharacter->NotifyScanEnd();
                }
                else
                if( m_ScanState == SCAN_CORPSE )
                {
                    // Get Lore Ident here
                    GetScannerVOIdent( pIdentifier, VO_SCAN_NOTHING );
                    g_AudioMgr.Play(pIdentifier);
                }
                else
                if( m_ScanEndGuid == m_ParentGuid)
                {
                    if( pPlayer )
                    {
                        // Get Lore Ident here
                        if( pPlayer->GetInventory2().HasItem( INVEN_WEAPON_MUTATION ) )
                        {
                            GetScannerVOIdent( pIdentifier, VO_SCAN_SELF_MUT );
                        }
                        else
                        {
                            GetScannerVOIdent( pIdentifier, VO_SCAN_SELF );
                        }

                        // play the sound
                        g_AudioMgr.Play(pIdentifier);
                    }
                }

                // keep this seperate so we don't interfere with next scan.
                m_ScannedGuid = m_ScanEndGuid;
            }
        }
        else
        {
#ifndef X_EDITOR
            // Get the player.
            //player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );

            // invalid scan
            //MsgMgr.Message( MSG_SCAN_INVALID, Player.net_GetSlot() );
#endif
            // Get Lore Ident here
            GetScannerVOIdent( pIdentifier, VO_SCAN_NOTHING );
            g_AudioMgr.Play(pIdentifier);
        }

        if( m_ScanState != SCAN_LORE && m_ScanState != SCAN_OBJECT )
        {
            if( pPlayer )
            {
                hud_object *Hud = pPlayer->GetHud();
                player_hud& PHud = Hud->GetPlayerHud( pPlayer->GetLocalSlot() );
                hud_scanner* pAHud = (hud_scanner*)(PHud.m_HudComponents[HUD_ELEMENT_SCANNER]);
                pAHud->NotifyScanComplete(pPlayer, m_ScannedGuid);
            }

            g_AudioMgr.Play("Scanner_Acquired");
        }

        ClearScan();
    }
}

//==============================================================================

xbool weapon_scanner::CanIntereptPrimaryFire( s32 nFireAnimIndex )
{
    (void)nFireAnimIndex;
    return TRUE;
}

//==============================================================================
void weapon_scanner::OnRenderTransparent(void)
{
    // call base class render which basically just calls object::OnRenderTransparent()
    new_weapon::OnRenderTransparent();

    player& Player = player::GetSafeType( *g_ObjMgr.GetObjectByGuid( m_ParentGuid ) );

    // we're firing, we can scan and we've started the process (m_bInitialScan set to false)
    if( Player.IsFiring() && Player.AllowedToFire() && CanScan() && !m_bInitialScan )
    {
        DrawScanningLaser();
    }

    m_ScanBoxHandle.Render();
}

//==============================================================================
static s16 g_ScanIndex[MAX_SCAN_POINTS]= {6,2, 0,4, // 0 - 3
                                          6,7, 5,4, // 4 - 7
                                          7,3, 2,6, // 8 - 11
                                          7,3, 1,5, // 12 - 15
                                          2,3, 1,0, // 16 - 19
                                          1,0, 4,5};// 20 - 23
//            7____________3
//           /|           /|
//         /  |         /  |
//       /    |       /    |
//      6____________2     |
//      |     |      |     |
//      |     |      |     |
//      |     |      |     | 
//      |     5____________1
//      |    /       |    /
//      |  /         |  /
//      |/           |/
//      4____________0  

//==============================================================================
void weapon_scanner::DrawEnclosures( void )
{
    draw_Begin(DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_CULL_NONE );
    draw_ClearL2W();    

    if( m_bFlashEnclosures )
    {
        draw_Color(g_ScanEncFlash_Color);
    }
    else
    {
        draw_Color(g_ScanEnc_Color);
    }
    
    s32 num = m_ScanIndex/4;

    s32 ind = 0;    
    
    // build quads
    for( s32 i = 0; i < num; i++ )
    {
        ind = i * 4;

        // first tri
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind]   ]);
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind+1] ]);
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind+2] ]);

        // second tri
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind+2] ]);
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind+3] ]);
        draw_Vertex(m_BBoxVerts[ g_ScanIndex[ind]   ]);

    }
    draw_End();
}

//==============================================================================
void weapon_scanner::TESTEnclosures( void )
{
    draw_Begin(DRAW_TRIANGLES, DRAW_USE_ALPHA);
    draw_ClearL2W();
    draw_Color(g_ScanEnc_Color);

    draw_Vertex(m_BBoxVerts[ ScanTestPoly[0] ] );
    draw_Vertex(m_BBoxVerts[ ScanTestPoly[1] ] );
    draw_Vertex(m_BBoxVerts[ ScanTestPoly[2] ] );

    draw_End();
}

//==============================================================================
void weapon_scanner::GetLaserHitLocation( player* pPlayer, vector3& EndPos, xbool bIsOnLoreObject )
{   
    radian Pitch;
    radian Yaw;

    // the view's rotation
    view &View = pPlayer->GetView();

    pPlayer->GetEyesPitchYaw( Pitch, Yaw );
   
    vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
    const vector3 ViewPos = View.GetPosition();
    Dest *= Lore_Max_Detect_DistanceTweak.GetF32();
    EndPos = ViewPos + Dest;

    // make laser "zip around"
    if( bIsOnLoreObject )
    {
        switch( Scanner_DrawType )
        {
        case 0:
            {
                if( m_ScanIndex < MAX_SCAN_POINTS )
                {
                    EndPos = m_BBoxVerts[ g_ScanIndex[m_ScanIndex] ];
                    
                    // at last vert on this run
                    if( (m_ScanIndex%4) == 3 )
                    {
                        m_ScanTimesPerFace++;
                        if( m_ScanTimesPerFace < g_ScansPerFace )
                        {
                            m_ScanIndex -= 3;
                        }
                        else
                        {
                            g_AudioMgr.Play("ScreenWipe");
                            m_ScanIndex++;
                            m_ScanTimesPerFace = 0;
                        }
                    }
                    else
                    {
                        m_ScanIndex++;
                    }
                }
            }
            break;
        default:
            {
                // point sampling
                EndPos += vector3(  x_frand(-g_Scan_O , g_Scan_O ) , 
                                    x_frand(-g_Scan_O , g_Scan_O ) , 
                                    x_frand(-g_Scan_O , g_Scan_O ) );
            }
            break;
        }
    }
    else
    {
        EndPos += vector3(  x_frand(-g_Scan_OO , g_Scan_OO ) , 
                            x_frand(-g_Scan_OO , g_Scan_OO ) , 
                            x_frand(-g_Scan_OO , g_Scan_OO ) );
    }

    g_CollisionMgr.AddToIgnoreList( pPlayer->GetGuid() );
    g_CollisionMgr.RaySetup( pPlayer->GetGuid(), ViewPos, EndPos );
    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE);

    // default modifier to full distance in case the collision manager returns no collisions
    f32 DistModifier = 1.0f;

    // if we don't hit anything, T is undefined
    if( g_CollisionMgr.m_nCollisions > 0 )
    {
        DistModifier = g_CollisionMgr.m_Collisions[0].T;
    }

    // get our new end position
    EndPos = ViewPos + (DistModifier*(EndPos-ViewPos));
}

#define MAX_SAMPLE_POINTS 8
static s16 g_SampleIndex[MAX_SAMPLE_POINTS]= {0,1,2,3,4,5,6,7};

//==============================================================================
void weapon_scanner::GetSamplingLaserHitLocation( player* pPlayer, vector3& EndPos, xbool bIsOnLoreObject )
{   
    radian Pitch;
    radian Yaw;

    // the view's rotation
    view &View = pPlayer->GetView();

    pPlayer->GetEyesPitchYaw( Pitch, Yaw );

    vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
    const vector3 ViewPos = View.GetPosition();
    Dest *= Lore_Max_Detect_DistanceTweak.GetF32();
    EndPos = ViewPos + Dest;

    // make laser "zip around"
    if( bIsOnLoreObject )
    {
        switch( Scanner_DrawType )
        {
        case 0:
            {
                // sample points all over the bbox
                s32 num = MAX_SAMPLE_POINTS-1;
                s32 i1 = x_irand(0, num);
                vector3 p1 = m_BBoxVerts[ g_SampleIndex[i1] ];
                s32 i2 = x_irand(0, num);
                
                // get another number if the points are the same
                if( i1 == i2 )
                {
                    i2 = (i2 + 1) % (MAX_SAMPLE_POINTS-1);
                }
                vector3 p2 = m_BBoxVerts[ g_SampleIndex[i2] ];

                // get a point somewhere on the BBox between two selected verts
                f32 T = x_frand(0.0f, 1.0f);
                vector3 V12 = (p2 - p1);                
                V12 = p1 + (T*(V12));
                EndPos = V12;
            }
            break;
        default:
            {
                // point sampling
                f32 x = x_frand(-g_Scan_O , g_Scan_O );
                f32 y = x_frand(-g_Scan_O , g_Scan_O );
                f32 z = x_frand(-g_Scan_O , g_Scan_O );
                EndPos += vector3( x, y, z );
            }
            break;
        }
    }
    else
    {
        f32 x = x_frand(-g_Scan_O , g_Scan_O );
        f32 y = x_frand(-g_Scan_O , g_Scan_O );
        f32 z = x_frand(-g_Scan_O , g_Scan_O );
        EndPos += vector3( x, y, z );
    }

    g_CollisionMgr.AddToIgnoreList( pPlayer->GetGuid() );
    g_CollisionMgr.RaySetup( pPlayer->GetGuid(), ViewPos, EndPos );
    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE);

    // default modifier to full distance in case the collision manager returns no collisions
    f32 DistModifier = 1.0f;

    // if we don't hit anything, T is undefined
    if( g_CollisionMgr.m_nCollisions > 0 )
    {
        DistModifier = g_CollisionMgr.m_Collisions[0].T;
    }

    // get our new end position
    EndPos = ViewPos + (DistModifier*(EndPos-ViewPos));
}

//==============================================================================
void weapon_scanner::DrawScanningLaser( void )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if( !pObj || !pObj->IsKindOf( player::GetRTTI() ) )
    {
        return;
    }

    player* pPlayer = (player*)pObj;

    draw_ClearL2W();

    // set up drawing
    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_CULL_NONE );

    // set draw texture
    draw_SetTexture( *m_LaserBitmap.GetPointer() );

    // get the point from which the "laser" emits
    vector3 StartPos, EndPos;
    
    StartPos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( m_FiringPointBoneIndex[FIRE_POINT_DEFAULT] );

    // get the position the "laser" will do a collision trace to
    xbool bHitLoreObject = (m_ScanStartGuid != NULL_GUID);
    GetLaserHitLocation(pPlayer, EndPos, bHitLoreObject);
    
    xcolor C = g_Scan_LaserColor;
    C.A = (u8)Scanner_Laser_Alpha;

    // always draw the 1st "laser" textured quad (the one coming out of the gun)
    draw_OrientedQuad( StartPos, EndPos,
        vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
        C, C,
        Scanner_Laser_Size, Scanner_Laser_Size );

    C = g_Scan2_LaserColor;
    C.A = (u8)Scanner_Laser_Alpha;

    GetSamplingLaserHitLocation(pPlayer, EndPos, bHitLoreObject);

    // always draw the 2nd "laser" textured quad (sampling laser)
    draw_OrientedQuad( StartPos, EndPos,
        vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ),
        C, C,
        Scanner_Laser_Size, Scanner_Laser_Size );
    
    // now draw sampling "hit" bitmap
    DrawLaserFixupBitmap((m_LaserFixupBitmap.GetPointer()), (Scanner_Laser_Size+0.5f), g_Scan2_LaserColor, g_CollisionMgr.m_Collisions[0]);

    // engine stop drawing
    draw_End();

    if( g_TestScanPolys )
    {
        if( bHitLoreObject )
        {
            TESTEnclosures();
        }
    }
    else
    {
        //DrawEnclosures();
    }
}

//==============================================================================

void weapon_scanner::ClearScan( void )
{    
    KillScanEffect();

    // stopped scanning, clear everything
    m_fScanTime = GetMaxScanTime();
    m_ScanStartGuid = NULL_GUID;
    m_ScanEndGuid = NULL_GUID;
    m_ScanIndex = 0;
    m_ScanTimesPerFace = 0;
    m_bCanScan = FALSE;
    m_bInitialScan = TRUE;
    m_bFlashEnclosures = FALSE;
    m_NumberOfFlashes = 0;
    m_CurrentFlashTime = g_ScanFlashTime;
}

//==============================================================================

void weapon_scanner::UpdateScanEffect( void )
{
    object* pScanEffectObj  = g_ObjMgr.GetObjectByGuid( m_ScanEffect );
    if( pScanEffectObj != NULL )
    {
        vector3 StartPos, EndPos;
        s32 iBone = m_FiringPointBoneIndex[FIRE_POINT_DEFAULT];

        // get the point from which the "laser" emits
        StartPos = m_AnimPlayer[ m_CurrentRenderState ].GetBonePosition( iBone );
        matrix4 L2W = m_AnimPlayer[ m_CurrentRenderState ].GetBoneL2W( iBone );
        L2W.PreTranslate( m_AnimPlayer[ m_CurrentRenderState ].GetBindPosition( iBone ) );

        radian Pitch, Yaw;

        player *pPlayer = NULL;
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
        {
            pPlayer = (player*)pObj;
        }
        else
        {
            // not a player????
            ASSERT(0);
            return;
        }

        // get where this projectile would hit
        pPlayer->GetProjectileHitLocation(EndPos, FALSE);

        // create vector
        vector3 ToVector = EndPos - StartPos;

        // pull pitch and yaw from it for rotation
        ToVector.GetPitchYaw(Pitch, Yaw);

        // set proper effect rotation
        radian3 ParticleRotation( -Pitch, Yaw-R_180, R_0 );

        L2W.SetRotation( ParticleRotation );

        pScanEffectObj->OnTransform( L2W );
        pScanEffectObj->OnMove( StartPos );
    }
}

//==============================================================================

void weapon_scanner::KillScanEffect( void )
{
    KillLaserSound();

    // destroy scan effect
    if( m_ScanEffect != NULL_GUID )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ScanEffect );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_ScanEffect = NULL_GUID;
    }

    m_ScanBoxHandle.KillInstance();
}

//==============================================================================

f32 weapon_scanner::GetMaxScanTime( void )
{
    return Scanner_Max_Scan_TimeTweak.GetF32();
}

//==============================================================================

void weapon_scanner::OnEnumProp( prop_enum& List )
{
    new_weapon::OnEnumProp( List );

    List.PropEnumHeader  ( "Scanner", "", 0 );
    List.PropEnumAngle   ( "Scanner\\Zoom FOV", "Whats the field of view when this weapon is zoomed in", 0 );
    List.PropEnumFloat   ( "Scanner\\View Change Rate", "How fast is the view going to change.", 0 );
    List.PropEnumFloat   ( "Scanner\\Zoom Stick Sensitivity", "How sensitive is the stick going to be when zoomed in", 0 );
    List.PropEnumFloat   ( "Scanner\\Zoom Movement Speed", "How much to slow the player down by.", 0 );

    SMP_UTIL_EnumHiddenManualResource( List, "Scanner\\ScanEffect", SMP_FXO );
}

//==============================================================================

xbool weapon_scanner::OnProperty( prop_query& rPropQuery )
{
    if( new_weapon::OnProperty( rPropQuery ) )
        return TRUE;

    if( SMP_UTIL_IsHiddenManualResource( rPropQuery, "Scanner\\ScanEffect", "WPN_Scanner.fxo" ))
        return TRUE;

    if( rPropQuery.IsVar( "Scanner\\Zoom FOV" ) )
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

    if( rPropQuery.VarFloat( "Scanner\\View Change Rate", m_ViewChangeRate ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Scanner\\Zoom Stick Sensitivity", m_StickSensitivity, 0.1f, 1.0f ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Scanner\\Zoom Movement Speed", m_ZoomMovementSpeed, 0.1f, 1.0f ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

xbool weapon_scanner::GetFlashlightTransformInfo( matrix4& incMatrix, vector3& incVect )
{
    if ( m_CurrentRenderState == RENDER_STATE_PLAYER )
    {
        return new_weapon::GetFlashlightTransformInfo( incMatrix, incVect );
    }
    else
    {
        incMatrix = GetL2W();
        incVect.Set( 0.0f, 11.0f, -18.0f );
        return TRUE;
    }
}

//==============================================================================
