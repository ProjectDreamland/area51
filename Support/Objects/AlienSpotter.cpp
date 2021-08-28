
#include "AlienOrb.hpp"
#include "AlienSpotter.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "render\LightMgr.hpp"
#include "audiomgr\audiomgr.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameLib\RenderContext.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Characters\Soldiers\soldier.hpp"


//=============================================================================
// CONSTANTS
//
//  k_SECURITY_UPDATE_RATE  = How often spotters update turrets and blackops
//                            about targets
//
//=============================================================================
static const f32            k_SECURITY_UPDATE_RATE          = 2.0f;

//=============================================================================
// SHARED
//=============================================================================

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_spotter_desc : public object_desc
{
    alien_spotter_desc( void ) : object_desc( 
        object::TYPE_ALIEN_SPOTTER, 
        "Alien Spotter", 
        "AI",
        object::ATTR_COLLIDABLE             | 
        object::ATTR_BLOCKS_ALL_PROJECTILES | 
        object::ATTR_RENDERABLE             |
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_TARGETS_OBJS          |
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new alien_spotter; }

} s_AlienSpotter_Desc;

//=============================================================================
//=============================================================================

const object_desc& alien_spotter::GetTypeDesc( void ) const
{
    return s_AlienSpotter_Desc;
}

//=============================================================================

const object_desc& alien_spotter::GetObjectType( void )
{
    return s_AlienSpotter_Desc;
}

//=============================================================================

alien_spotter::alien_spotter()
{
    m_SightConeAngle    = R_15;
    m_nOrbsInPool       = 4;
    m_nMaxOrbsActive    = 1;
    m_nActiveOrbs       = 0;
    m_OrbSpawnDelay     = 2.0f;
    m_OrbSpawnObj       = NULL_GUID;
    m_TimeSinceLastOrbLaunch = 0;
    m_OrbTemplateID     = -1;

    m_bTrackingLocked   = FALSE;

    m_LastTargetLockDir.Set(0,0,0);

    m_AimStartLoopSoundID   = -1;
    m_AimStopSoundID        = -1;
    m_AimStartLoopVoiceID   = -1;

    m_nOrbsToSpawnOnDeath   = 0;

    m_TimeSinceLastSecurityUpdate = 0;
    s32 i;
    for (i=0;i<MAX_ACTIVE_ORBS;i++)
    {
        m_ActiveOrbs[i] = NULL_GUID;
    }
}

//=============================================================================

alien_spotter::~alien_spotter()
{
}

//=============================================================================

void alien_spotter::OnEnumProp( prop_enum&    List )
{
    turret::OnEnumProp( List );

    List.PropEnumHeader( "Alien Spotter",                    "Alien Spotter Properties", 0 );
    List.PropEnumFileName( "Alien Spotter\\Orb Blueprint Path",
                      "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM );

    List.PropEnumExternal( "Alien Spotter\\Audio Package",     "Resource\0audiopkg\0", "The audio package associated with this alien spotter object.", 0 );
    List.PropEnumExternal( "Alien Spotter\\Aim Start Sound",   "Sound\0soundexternal\0","The sound to play when the orb is created", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Alien Spotter\\Aim Stop Sound",    "Sound\0soundexternal\0","The sound to play when the orb collides with something", PROP_TYPE_MUST_ENUM );


    List.PropEnumAngle ( "Alien Spotter\\Sight Cone Angle",      "", 0 );    
    List.PropEnumInt   ( "Alien Spotter\\Number of Orbs",        "Number of orbs the spotter is capable of launching", 0 );
    List.PropEnumInt   ( "Alien Spotter\\Number of Active Orbs", "Number of orbs that can be out at a time", 0 );
    List.PropEnumInt   ( "Alien Spotter\\Number of Death Orbs",  "Number of orbs to spawn at death", 0 );
    List.PropEnumFloat ( "Alien Spotter\\Orb Spawn Delay",       "Seconds between orb spawns", 0 );
    List.PropEnumGuid  ( "Alien Spotter\\Orb Spawn Point",       "Where the orbs spawn from", 0 );

}

//=============================================================================

xbool alien_spotter::OnProperty( prop_query&   I    )
{
    if (turret::OnProperty(I))
    {}
    else if (I.VarAngle( "Alien Spotter\\Sight Cone Angle", m_SightConeAngle ))
    {}
    else if (I.VarInt( "Alien Spotter\\Number of Orbs", m_nOrbsInPool ))
    {}
    else if (I.VarInt( "Alien Spotter\\Number of Active Orbs", m_nMaxOrbsActive ))
    {}
    else if (I.VarInt( "Alien Spotter\\Number of Death Orbs", m_nOrbsToSpawnOnDeath ))
    {}
    else if (I.VarFloat( "Alien Spotter\\Orb Spawn Delay", m_OrbSpawnDelay ))
    {}
    else if (I.VarGUID( "Alien Spotter\\Orb Spawn Point", m_OrbSpawnObj ))
    {}
    else if( I.IsVar( "Alien Spotter\\Orb Blueprint Path" ) )
    {
        if( I.IsRead() )
        {
            if( m_OrbTemplateID < 0 )
            {
                I.SetVarFileName("",256);
            }
            else
            {
                I.SetVarFileName( g_TemplateStringMgr.GetString( m_OrbTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( I.GetVarFileName() ) > 0 )
            {
                m_OrbTemplateID = g_TemplateStringMgr.Add( I.GetVarFileName() );
            }
        }        
    }
    else if( I.IsVar( "Alien Spotter\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hSpotterAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hSpotterAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                    m_hSpotterAudioPackage.GetPointer();
            }
        }
    }
    else if( I.IsVar( "Alien Spotter\\Aim Start Sound" ) )
    {
        if( I.IsRead() )
        {
            if( m_AimStartLoopSoundID != -1 )
                I.SetVarExternal( g_StringMgr.GetString( m_AimStartLoopSoundID ), 64 );
            else
                I.SetVarExternal( "", 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                s32 PkgIndex = String.Find( '\\', 0 );

                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    String.Delete( 0, PkgIndex+1 );

                    m_hSpotterAudioPackage.SetName( Pkg );                

                    // Load the audio package.
                    if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                        m_hSpotterAudioPackage.GetPointer();
                }

                m_AimStartLoopSoundID = g_StringMgr.Add( String );
            }
        }
    }
    else if( I.IsVar( "Alien Spotter\\Aim Stop Sound" ) )
    {
        if( I.IsRead() )
        {
            if( m_AimStopSoundID != -1 )
                I.SetVarExternal( g_StringMgr.GetString( m_AimStopSoundID ), 64 );
            else
                I.SetVarExternal( "", 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                s32 PkgIndex = String.Find( '\\', 0 );

                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    String.Delete( 0, PkgIndex+1 );

                    m_hSpotterAudioPackage.SetName( Pkg );                

                    // Load the audio package.
                    if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                        m_hSpotterAudioPackage.GetPointer();
                }

                m_AimStopSoundID = g_StringMgr.Add( String );
            }
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================

void alien_spotter::OnAdvanceLogic  ( f32 DeltaTime )
{
    turret::OnAdvanceLogic( DeltaTime );

    m_TimeSinceLastOrbLaunch += DeltaTime;
    
    if (m_LastAimedTrackingStatus == TRACK_LOCKED)
    {
        // See if it's time to launch an orb
        if (m_TimeSinceLastOrbLaunch > m_OrbSpawnDelay)
        {
            if (m_nOrbsInPool != 0)
            {
                LaunchOrb();
            }
        }
    }
/*
    m_TimeSinceLastSecurityUpdate += DeltaTime;
    if (m_TimeSinceLastSecurityUpdate > k_SECURITY_UPDATE_RATE )
    {         
        m_TimeSinceLastSecurityUpdate = 0;

        u16 Zone1 = GetZone1();
        u16 Zone2 = GetZone2();
    
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_TURRET );

        while (SLOT_NULL != SlotID)
        {
            object* pObj = g_ObjMgr.GetObjectBySlot(SlotID);
            if (pObj)
            {            
                u16 Z1,Z2;
                xbool bValid = FALSE;

                Z1 = pObj->GetZone1();
                Z2 = pObj->GetZone2();

                if (Zone1 != 0)
                {
                    if ((Z1 == Zone1) || (Z2 == Zone1))
                        bValid = TRUE;
                }
                if (Zone2 != 0)
                {
                    if ((Z1 == Zone2) || (Z2 == Zone2))
                        bValid = TRUE;
                }

                if (bValid)
                {               
                    if (pObj->IsKindOf( turret::GetRTTI() ))
                    {                        
                        turret& Turret = turret::GetSafeType( *pObj );

                        if ((m_LastAimedTrackingStatus == TRACK_AIMING) ||
                            (m_LastAimedTrackingStatus == TRACK_LOCKED))
                        {
                            //Turret.LockForTarget( m_TargetGuid );
                            Turret.OfferNewObjectiveGuid( m_TargetGuid );
                        }                                                        
                    }
                    else if (pObj->IsKindOf( blackopps::GetRTTI() ))
                    {
                        blackopps& BO = blackopps::GetSafeType( *pObj );

                        if ((m_LastAimedTrackingStatus == TRACK_AIMING) ||
                            (m_LastAimedTrackingStatus == TRACK_LOCKED))
                        {
                            BO.OfferOverrideTarget( m_TargetGuid );
                        }                                                                               
                    }
                }
            }
            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }
    */
}

//=============================================================================

xbool alien_spotter::LaunchOrb( void )
{
    
    // Bail if we are empty
    if ( m_nOrbsInPool == 0 )
        return FALSE;
    
    s32 i;
    s32 iSlot = -1;

    for (i=0;i<m_nMaxOrbsActive;i++)
    {
        if (m_ActiveOrbs[i] == NULL_GUID)
        {
            iSlot = i;
            break;
        }
        else
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ActiveOrbs[i] );
            if (NULL == pObj)
            {
                iSlot=i;
                break;
            }
             
            if (!pObj->IsKindOf( alien_orb::GetRTTI() ))
            {                
                iSlot=i;
                break;
            }             
        }
    }
    if (iSlot == -1)
        return FALSE;

    m_ActiveOrbs[iSlot] = NULL_GUID;

    vector3 Pos,Dir;
    radian3 Rot;

    //Get Launch Point
    if (m_OrbSpawnObj != NULL_GUID)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OrbSpawnObj );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();

            Pos = L2W.GetTranslation();
            Rot = L2W.GetRotation();
            Dir.Set(0,0,1);
            Dir.Rotate(Rot);
        }
    }
    else
    {
        GetLaunchPoint(Pos,Dir,Rot);
    }

    // create orb
    guid gObj = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_OrbTemplateID ), Pos, Rot, GetZone1(), GetZone2() );
    object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
    if (pObj)
    {        
        alien_orb& Orb = alien_orb::GetSafeType( *pObj );  
        Orb.SetSpawner( GetGuid() );

        matrix4 L2W( vector3(1,1,1), Rot, Pos );
        Orb.OnTransform( L2W );  

        Dir = Pos - GetPosition();
        Dir.NormalizeAndScale(100.0f);
        
        Orb.Launch( Pos, Pos + Dir );
    }

    m_ActiveOrbs[ iSlot ] = gObj;
    
    m_TimeSinceLastOrbLaunch = 0;

    if (m_nOrbsInPool > 0)    
        m_nOrbsInPool--;

    return TRUE;
}

//=========================================================================

turret::tracking_status alien_spotter::TrackTarget( f32 DeltaTime )
{
    tracking_status Ret = TRACK_NO_TARGET;

    if (m_TargetGuid == 0)
    {        
        return TRACK_NO_TARGET;
    }
    else
    if (!CanSenseTarget() && !CanFireAtTarget())
    {
        return TRACK_NO_TARGET;
    }
    else
    if (!IsTargetValid())
    {
        return TRACK_NO_TARGET;
    }
    else
    {   
        vector3 TargetPos = GetTargetPos();
        vector3 SensorPos;
        radian3 SensorRot;
        GetSensorInfo( SensorPos, SensorRot );

        // We want to work with position and angles in local turret space
        matrix4 W2L = GetL2W();

        W2L.InvertRT();

        TargetPos = W2L.Transform( TargetPos );
        SensorPos = W2L.Transform( SensorPos );

        vector3 Delta = TargetPos - SensorPos;
        m_LocalTargetDir = Delta;
        m_LocalTargetDir.Normalize();

        f32 Dot = m_LocalTargetDir.Dot( m_LastTargetLockDir );
        

        if (( Dot > 0.8f ) && ( Dot <= 1.0f ))
        {
            // Target is still inside the sight cone.
            // We return the happy "TRACK_LOCKED" in this case
            // to force the turret to think it is locked on.
            Ret = TRACK_LOCKED;
        }
        else
        {            
            Ret = turret::TrackTarget( DeltaTime );
            if (Ret == TRACK_AIMING)
            {
                
            }
            if (Ret == TRACK_LOCKED)
            {
                m_LastTargetLockDir = m_LocalTargetDir;
            }
        }
    }

    if (Ret == TRACK_NO_TARGET)
    {
        SetTargetGuid(0);
        StopAiming();
    }
    else
    {
        if (Ret == TRACK_AIMING)
        {            
            StartAiming();
        }
        else if ((Ret == TRACK_LOCKED) || (Ret == TRACK_OUTSIDE_FOF))
        {
            StopAiming();
        }
    }
    return Ret;
}

//=========================================================================

void alien_spotter::SetTargetGuid   ( guid Guid )
{
    if ((Guid == NULL_GUID) || (Guid != m_TargetGuid))
    {
        m_bTrackingLocked = FALSE;
        m_LastTargetLockDir.Set(0,0,0);
    }

    turret::SetTargetGuid( Guid );
}

//=========================================================================

xbool alien_spotter::GetLaunchPoint( vector3& Pos, vector3& Dir, radian3& Rot )
{
    xbool bValid = FALSE;

    if( m_hAnimGroup.IsLoaded() )
    {     
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        s32   iBone  = pAnimGroup->GetBoneIndex( "orb_spawn" );
        
        if (iBone != -1)
        {   
            const matrix4* pL2W = m_AnimPlayer.GetBoneL2W( iBone, TRUE );
            if (NULL != pL2W)
            {
                matrix4 L2W = *pL2W;

                L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iBone ) );

                Pos = L2W.GetTranslation();
                Rot = L2W.GetRotation();
                Dir.Set(0,0,1);
                Dir.Rotate( Rot );
                
                bValid = TRUE;
            }
        }
    }
    if (!bValid)
    {
        // Just use the position of the turret
        const matrix4& L2W = GetL2W();
        Pos = L2W.GetTranslation();
        Rot = L2W.GetRotation();
        Dir.Set(0,0,1);
        Dir.Rotate( Rot );
        return FALSE;
    }    
    return TRUE;
}

//=========================================================================

void alien_spotter::OnEnterState( state NewState )
{   
    turret::OnEnterState( NewState );

    if (NewState == STATE_DESTROYED)
    {
        // Spawn orbs when we die
        if (m_nOrbsInPool > 0)
        {
            DeathSpawnOrbs();
        }
    }
}

//=========================================================================

void alien_spotter::StartAiming()
{
    if (m_AimStartLoopVoiceID != -1)
        return;

    if (m_AimStartLoopSoundID != -1)
    {
        const char* pSoundName = g_StringMgr.GetString( m_AimStartLoopSoundID );
        if (pSoundName)
        {
            const matrix4& L2W = GetL2W();
            m_AimStartLoopVoiceID = g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
        }
    }
}

//=========================================================================

void alien_spotter::StopAiming()
{
    if (m_AimStartLoopVoiceID != -1)
    {
        g_AudioMgr.Release( m_AimStartLoopVoiceID, 0.0f );

        m_AimStartLoopVoiceID = -1;

        if (m_AimStopSoundID != -1)
        {
            const char* pSoundName = g_StringMgr.GetString( m_AimStopSoundID );
            if (pSoundName)
            {
                const matrix4& L2W = GetL2W();
                g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
            }
        }
    }
}

//=========================================================================

void alien_spotter::DeathSpawnOrbs( void )
{
    // Create multiple orbs, and don't bother tracking their guids.
    // We're dead anyway... :(

    vector3 Pos,Dir;
    radian3 Rot;

    //Get Launch Point
    if (m_OrbSpawnObj != NULL_GUID)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OrbSpawnObj );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();

            Pos = L2W.GetTranslation();
            Rot = L2W.GetRotation();
            Dir.Set(0,0,1);
            Dir.Rotate(Rot);
        }
    }
    else
    {
        GetLaunchPoint(Pos,Dir,Rot);
    }

    // create orb
    s32     i;
    for (i=0;i<m_nOrbsToSpawnOnDeath;i++)
    {    
        guid gObj = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_OrbTemplateID ), Pos, Rot, GetZone1(), GetZone2() );
        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
        if (pObj)
        {        
            alien_orb& Orb = alien_orb::GetSafeType( *pObj );  
            Orb.SetSpawner( GetGuid() );

            matrix4 L2W( vector3(1,1,1), Rot, Pos );
            Orb.OnTransform( L2W );  

            Dir = Pos - GetPosition();
            Dir.NormalizeAndScale(100.0f);

            vector3 Rand;
            f32     x = x_frand(-100,100);
            f32     y = x_frand(-100,100);
            f32     z = x_frand(-100,100);
            Rand.Set( x, y, z );

            Orb.Launch( Pos, Pos + Dir + Rand );
        }
    }
}

//=========================================================================

//=========================================================================

//=========================================================================
