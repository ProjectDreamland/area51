//==============================================================================
//
//  Pain.cpp
//
//==============================================================================

#define DO_LOG_SETUP 1
#define DO_LOG_COMPUTE_DAMAGE 1

//==============================================================================
//  INCLUDES
//==============================================================================
#include "Pain.hpp"
#include "Obj_Mgr/Obj_Mgr.hpp"

// Need this to handle factions.  
// Too bad base class object can't handle it.
#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"
#include "NetworkMgr/GameMgr.hpp"

//==============================================================================
//==============================================================================
//==============================================================================
//  PAIN FUNCTIONS
//==============================================================================
//==============================================================================
//==============================================================================

pain::pain( void )
{
    Clear();
}

//==============================================================================

pain::~pain( void )
{
    Clear();
}

//==============================================================================

void pain::Clear( void )
{
    m_bSetupCalled = FALSE;
    m_bComputeDamageAndForceCalled = FALSE;
    
    SetupDefaults();
}

//==============================================================================

void pain::SetupDefaults( void )
{
    m_OriginGuid            = 0;
    m_DirectHitGuid         = 0;
    m_CustomScalar          = 1.0f;
    m_AnimEventID           = -1;
    m_pOriginRTTI           = NULL;
    m_Position.Zero();
    m_Direction.Zero();

    m_Force                 = 0.0f;
    m_Damage                = 0.0f;
    m_ForceDirection.Set    ( 0.0f, 1.0f, 0.0f );
    m_VictimGuid            = 0;
    m_VictimPainPosition.Zero();
    m_HitType               = -1;
    m_pVictimRTTI           = NULL;
    m_LOSCoverage           = 0.0f;

    m_bCollisionAvailable   = FALSE;
    m_bDirectHit            = FALSE;
}

//==============================================================================

void pain::Setup( const char* PainDesc, guid OriginGuid, const vector3& Position )
{
    pain_handle PainHandle( PainDesc );
    Setup( PainHandle, OriginGuid, Position );
}

//==============================================================================

void pain::Setup( pain_handle PainHandle, guid OriginGuid, const vector3& Position )
{
    ASSERT( PainHandle.IsValid() );

    #if defined(DATA_VAULT_KEEP_NAMES) && defined(X_LOGGING)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( OriginGuid );
        CLOG_MESSAGE(DO_LOG_SETUP,
                    "PAIN","Setup (%s) from (%s)",
                    PainHandle.GetName(),
                    (pObj) ? (pObj->GetLogicalName()) : ("UNKNOWN")
                    );
        ASSERT( PainHandle.GetName()[0] != 0 );
    }
    #endif

    m_bSetupCalled  = TRUE;
    m_bComputeDamageAndForceCalled = FALSE;

    SetupDefaults();

    m_PainHandle    = PainHandle;
    m_OriginGuid    = OriginGuid;
    m_Position      = Position;
    m_ImpactPoint   = Position;
    m_ImpactNormal(0,0,0);

    // Compute area of effect.  If PainProfile is not available
    // then it is reported through the logging system.
    const pain_profile* pPainProfile = PainHandle.GetPainProfile();
    if( pPainProfile  ) m_PainBBox = pPainProfile->m_BBox;
    else                m_PainBBox = bbox(vector3(0,0,0),10.0f);
    m_PainBBox.Translate( m_Position );

    // Grab a ptr to the rtti of the origin object
    object* pObj = g_ObjMgr.GetObjectByGuid(OriginGuid);
    if( pObj )
        m_pOriginRTTI = &pObj->GetRTTI();
}

//==============================================================================

xbool pain::SetupCalled( void ) const
{
    return m_bSetupCalled;
}

//==============================================================================

xbool pain::ComputeDamageAndForceCalled( void ) const
{
    return m_bComputeDamageAndForceCalled;
}

//==============================================================================

void pain::SetDirection( const vector3& Direction ) 
{
    ASSERT( m_bSetupCalled );
    m_Direction = Direction;
    m_Direction.Normalize();
    if( HasCollision() == FALSE )
        m_ImpactNormal = -m_Direction;
}

//==============================================================================

void pain::SetDirectHitGuid( guid DirectHitGuid )
{
    ASSERT( m_bSetupCalled );
    m_DirectHitGuid = DirectHitGuid;
    m_bDirectHit    = ( DirectHitGuid != 0 );
}

//==============================================================================

void pain::SetCustomScalar( f32 CustomScalar )
{
    ASSERT( m_bSetupCalled );
    ASSERT( CustomScalar>=0 );
    m_CustomScalar = CustomScalar;
}

//==============================================================================

void pain::SetCollisionInfo( const collision_mgr::collision& CollisionInfo )
{
    ASSERT( m_bSetupCalled );
    m_Collision = CollisionInfo;
    m_ImpactPoint = CollisionInfo.Point;
    m_ImpactNormal = CollisionInfo.Plane.Normal;
    m_bCollisionAvailable = TRUE;
}

//==============================================================================

void pain::SetAnimEventID( s32 AnimEventID )
{
    ASSERT( m_bSetupCalled );
    ASSERT( AnimEventID >= 0 );
    m_AnimEventID = AnimEventID;
}

//==============================================================================

pain_handle pain::GetPainHandle( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_PainHandle;
}

//==============================================================================

pain_health_handle pain::GetPainHealthHandle ( void ) const
{
    ASSERT( m_bSetupCalled );
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_PainHealthHandle;
}

//==============================================================================

guid pain::GetOriginGuid( void ) const
{
    return m_OriginGuid;
}

//==============================================================================

const vector3& pain::GetPosition( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_Position;
}

//==============================================================================

const vector3& pain::GetImpactPoint( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_ImpactPoint;
}

//==============================================================================

const vector3& pain::GetImpactNormal( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_ImpactNormal;
}

//==============================================================================

const rtti& pain::GetOriginRTTI( void ) const
{
    if( m_pOriginRTTI )
        return *m_pOriginRTTI;
    else
        return object::GetRTTI();
}

//==============================================================================

const vector3& pain::GetDirection( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_Direction;
}

//==============================================================================

f32 pain::GetCustomScalar( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_CustomScalar;
}

//==============================================================================

guid pain::GetDirectHitGuid( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_DirectHitGuid;
}

//==============================================================================

s32 pain::GetAnimEventID( void ) const
{
    ASSERT( m_bSetupCalled );
    return m_AnimEventID;
}

//==============================================================================

xbool pain::IsDamageComputed( void ) const
{
    return m_bComputeDamageAndForceCalled;
}

//==============================================================================

xbool pain::HasCollision( void ) const
{
    return m_bCollisionAvailable;
}

//==============================================================================

const collision_mgr::collision& pain::GetCollision( void ) const
{
    ASSERT( m_bSetupCalled );
    ASSERT( m_bCollisionAvailable );
    return m_Collision;
}

//==============================================================================

f32 pain::GetDamage( void ) const
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_Damage;
}

//==============================================================================

f32 pain::GetForce ( void ) const
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_Force;
}

//==============================================================================

const vector3& pain::GetForceDirection( void ) const 
{
    return m_ForceDirection;
}

//==============================================================================

vector3 pain::GetForceVelocity( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_ForceDirection * m_Force;
}

//==============================================================================

xbool pain::IsDirectHit( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_bDirectHit;
}

//==============================================================================

xbool pain::IsFriendlyFire( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_bIsFriendlyFire;
}

//==============================================================================

guid pain::GetVictimGuid( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_VictimGuid;
}

//==============================================================================

health_handle pain::GetHealthHandle( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_HealthHandle;
}

//==============================================================================

const vector3& pain::GetVictimPainPosition( void ) const 
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_VictimPainPosition;
}

//==============================================================================

s32 pain::GetHitType( void ) const
{
    // SB: 2/23/05
    // NOTE: "m_HitType" is always valid - it defaults to -1 if 
    //       "ComputeDamageAndForceCalled" has not been called
    return m_HitType;
}

//==============================================================================

f32 pain::GetLOSCoverage( void ) const
{
    ASSERT( m_bComputeDamageAndForceCalled );
    return m_LOSCoverage;
}

//==============================================================================

const rtti& pain::GetVictimRTTI( void ) const
{
    if( m_pVictimRTTI )
        return *m_pVictimRTTI;
    else
        return object::GetRTTI();
}

//==============================================================================

xbool pain::ComputeDamageAndForce( const char* HealthDesc, guid VictimGuid, const vector3& VictimPainPosition ) const
{
    health_handle HealthHandle( HealthDesc );
    return ComputeDamageAndForce( HealthHandle, VictimGuid, VictimPainPosition );
}

//==============================================================================

xbool pain::ComputeDamageAndForce( health_handle HealthHandle, guid VictimGuid, const vector3& VictimPainPosition ) const
{
    // Copy parameters into pain class
    ASSERT( HealthHandle.IsValid() );
    m_HealthHandle       = HealthHandle;
    m_VictimGuid         = VictimGuid;
    m_VictimPainPosition = VictimPainPosition;

    // Get ptr to the killer and victim
    object* pKillerObj = g_ObjMgr.GetObjectByGuid(m_OriginGuid);
    object* pVictimObj = g_ObjMgr.GetObjectByGuid(m_VictimGuid);

    // Grab a ptr to the rtti of the victim object
    if( pVictimObj )
        m_pVictimRTTI = &pVictimObj->GetRTTI();

    // Clear return values
    m_Damage = 0;
    m_Force  = 0;
    m_ForceDirection.Zero();
    m_bComputeDamageAndForceCalled = TRUE;
    m_bIsFriendlyFire = TRUE;
    m_LOSCoverage = 0;

    // Lookup the different data structures from the data_vault.
    // If they are not available it will be reported via the 
    // logging system by the individual handles.  We just need to survive.

    // Lookup pain_profile
    const pain_profile* pPainProfile = m_PainHandle.GetPainProfile();
    if( !pPainProfile ) return FALSE;
    const pain_profile& PainProfile = *pPainProfile;

    // Do LOSCoverage check
    if( PainProfile.m_bCheckLOS )
    {
        ComputeLOSCoverage();

        // If we have full coverage then no damage or force can touch us.
        if( m_LOSCoverage==1.0f )
            return FALSE;
    }

    // Lookup health_profile
    const health_profile* pHealthProfile = m_HealthHandle.GetHealthProfile();
    if( !pHealthProfile ) return FALSE;

    // Lookup pain_health_profile
    m_PainHealthHandle = m_PainHandle.BuildPainHealthProfileHandle( m_HealthHandle );
    const pain_health_profile* pPainHealthProfile = m_PainHealthHandle.GetPainHealthProfile();
    if( !pPainHealthProfile ) return FALSE;
    const pain_health_profile& PainHealthProfile = *pPainHealthProfile;

    // Copy hit-type out of profile
    m_HitType = PainHealthProfile.m_HitType;

    // Compute Scalars.  These are used to scale the final table-based
    // Damage and Force values.
    f32 DamageScalar;
    f32 ForceScalar;

    // Check if pain does splash damage or not
    if( PainProfile.m_bSplash )
    {
        // Handle splash-based pain.
        ASSERT( PainProfile.m_bSplash==TRUE );

        // Compute falloff distance to decay pain by.
        vector3 PainVictimDelta = m_VictimPainPosition - m_Position;
        f32 FalloffDist = PainVictimDelta.Length();

        // Trivially reject pain if too far away.
        if( (FalloffDist > PainProfile.m_DamageFarDist) && (FalloffDist > PainProfile.m_ForceFarDist) )
            return FALSE;

        // Compute linear, parametric, clamped, falloff scalar
        DamageScalar = x_parametric( FalloffDist, PainProfile.m_DamageFarDist, PainProfile.m_DamageNearDist, TRUE );
        ForceScalar  = x_parametric( FalloffDist, PainProfile.m_ForceFarDist,  PainProfile.m_ForceNearDist, TRUE );

        // If a direct hit was provided then apply full pain
        if( m_DirectHitGuid && (m_DirectHitGuid == m_VictimGuid) )
        {
            DamageScalar = 1.0f;
            ForceScalar = 1.0f;
        }

        // Compute force direction
        m_ForceDirection = PainVictimDelta;
        m_ForceDirection.Normalize();
    }
    else
    {
        // Handle non-splash-based pain.
        ASSERT( PainProfile.m_bSplash==FALSE );

        // If a direct hit was provided then this must match
/*        if( m_DirectHitGuid && (m_DirectHitGuid != m_VictimGuid) )
        {
            return FALSE;
        }*/

        // Set scalars to exactly what's in the tables
        DamageScalar = 1.0f;
        ForceScalar  = 1.0f;

        // Use pain direction to build force direction
        m_ForceDirection = m_Direction;
    }
    
    // Apply team damage scalars
    m_bIsFriendlyFire = HandleFriendlyFire( pKillerObj, pVictimObj, DamageScalar, ForceScalar );

    // Apply LOS coverage
    DamageScalar *= (1-m_LOSCoverage);
    ForceScalar  *= (1-m_LOSCoverage);

    // Force final scalars within range
    DamageScalar = x_clamp( DamageScalar, 0.0f, 1.0f );
    ForceScalar  = x_clamp( ForceScalar,  0.0f, 1.0f );

    // Apply custom scalar.  It is applied here since it is allowed to be > 1.0f
    DamageScalar *= m_CustomScalar;
    ForceScalar  *= m_CustomScalar;

    // Apply global scalar.  It is applied here since it is allowed to be > 1.0f
    DamageScalar *= GetTweakF32( "Global_Damage_Scalar", 1.0f );

    // Apply scalars to table values
    m_Damage = PainHealthProfile.m_Damage * DamageScalar;
    m_Force  = PainHealthProfile.m_Force * ForceScalar;

    // Log the final info if anyone is interested.
    #if defined(DATA_VAULT_KEEP_NAMES) && defined(X_LOGGING)
    CLOG_MESSAGE(DO_LOG_COMPUTE_DAMAGE,
                "pain::ComputeDamageAndForce","(%s)->(%s) D(%5.1f) F(%5.1f) %s",
                m_PainHandle.GetName(),
                HealthHandle.GetName(),
                m_Damage,
                m_Force,
                (m_bIsFriendlyFire) ? ("friendy") : ("enemy"));
    #endif

    // All trivial rejects have already happened.  Even if there is no 
    // damage or force we still need to return true to objects can
    // react to being hit.
    return TRUE;
}

//==============================================================================

void pain::ComputeLOSCoverage( void ) const
{
    // This could be more intelligent and handle partial coverage.

    // Decide victim position
    vector3 VictimPos = m_VictimPainPosition;
    {
        object *VictimObject = g_ObjMgr.GetObjectByGuid( m_VictimGuid );
        if( VictimObject && VictimObject->IsKindOf(actor::GetRTTI()) )
        {
            actor &actorVictim = actor::GetSafeType( *VictimObject );
            VictimPos = actorVictim.GetPositionWithOffset(actor::OFFSET_CENTER);
        }
    }

    vector3 Distance = (VictimPos - m_Position);

    // if the distance is very small, no LOS coverage
    if( Distance.LengthSquared() < 1.0f )
    {
        m_LOSCoverage = 0.0f;
    }
    else
    {
        // Test ray from pain position to victim pain position
        g_CollisionMgr.LineOfSightSetup( 0, m_Position, VictimPos );
        g_CollisionMgr.AddToIgnoreList( m_OriginGuid );
        g_CollisionMgr.AddToIgnoreList( m_VictimGuid );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PAIN_LOS, (object::object_attr)( object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING ) );

        // set LOS coverage
        if( g_CollisionMgr.m_nCollisions )
            m_LOSCoverage = 1.0f;
        else
            m_LOSCoverage = 0.0f;

    }
}

//==============================================================================

xbool pain::HandleFriendlyFire( object* pKiller, object* pVictim, f32& DamageModifier, f32& ForceModifier ) const
{
    // If we don't have ptrs to the two parties we can't decide anything.
    if( !pKiller || !pVictim )
        return FALSE;

    // Both need to be descendents of actor or we can't determine factions.
    if( !pKiller->IsKindOf( actor::GetRTTI() ) ||
        !pVictim->IsKindOf( actor::GetRTTI() ) )
        return FALSE;

    // Alright, treat these as actors
    if( ((actor*)pVictim)->IsAlly( (actor*)pKiller ) )
    {
        //
        // They are friendly!
        //
        if( (pVictim == pKiller) && pKiller->IsKindOf( player::GetRTTI() ) )
        {
            // We are the player hurting ourselves, so we're not /really/ friendly
            return FALSE;
        }

#ifndef X_EDITOR
        if( GameMgr.GetGameType() != GAME_CAMPAIGN )
        {
            DamageModifier *= GameMgr.GetTeamDamage();
        }
        else
#endif
        {
            // Scale damage down by whatever scale tweak we have
            tweak_handle ScaleTweak("FriendlyFireScalar");
            DamageModifier *= ScaleTweak.GetF32();
        }

        // We're not going to do anything with force I guess.
        (void)ForceModifier;

        return TRUE;
    }
    else
    {
        //
        // They are enemies or neutral!
        //

        return FALSE;
    }
}

//==============================================================================

xbool pain::ApplyToWorld( xbool bIgnoreOrigin )
{
    // Clear the return result
    xbool bHitAnObject = FALSE;

    //
    // Get reference to pain profile or just return if not available.
    //
    const pain_profile* pPainProfile = m_PainHandle.GetPainProfile();
    if( !pPainProfile ) 
        return FALSE;
    const pain_profile& PainProfile = *pPainProfile;

    //
    // If ApplyToWorld() is called but this type of pain does not have splash
    // then apply the pain only to the DirectHitGuid if available.
    //
    if( PainProfile.m_bSplash==FALSE )
    {
        if( m_DirectHitGuid )
        {
            return ApplyToObject( m_DirectHitGuid );
        }
        return FALSE;
    }

    //
    // Setup a slot to ignore if we should ignore the origin object
    //
    slot_id IgnoreSlot = SLOT_NULL;
    if( bIgnoreOrigin )
    {
        IgnoreSlot = g_ObjMgr.GetSlotFromGuid( GetOriginGuid() );
    }

    //
    // Collect all damagable objects in range of pain.  We are putting them into
    // an array because we can't handle nested SelectBBox and the OnPain response
    // of the object might need it.
    //
    const s32 MAX_DAMAGEABLE_OBJECTS = 64;
    slot_id iSlot[MAX_DAMAGEABLE_OBJECTS];
    s32     nSlots = 0;
    {
        g_ObjMgr.SelectBBox( object::ATTR_DAMAGEABLE, m_PainBBox, object::TYPE_ALL_TYPES );    
        slot_id ID = g_ObjMgr.StartLoop();
        while( (ID != SLOT_NULL) && (nSlots < MAX_DAMAGEABLE_OBJECTS) )
        {
            if( ID != IgnoreSlot )
            {
                iSlot[nSlots] = ID;
                nSlots++;
            }
            ID = g_ObjMgr.GetNextResult( ID );
        }
        g_ObjMgr.EndLoop();
    }

    //
    // Loop through the objects and apply the pain
    //
    s32 i;
    for( i=0; i<nSlots; i++ )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( iSlot[i] );
        if( pObject )
        {
            bHitAnObject = ApplyToObject( pObject ) || bHitAnObject;
        }
    }

    return bHitAnObject;
}

//==============================================================================

xbool pain::ApplyToObject( guid VictimGuid )
{
    object* pObject = g_ObjMgr.GetObjectByGuid( VictimGuid );
    return ApplyToObject( pObject );
}

//==============================================================================

xbool pain::ApplyToObject( object* pObject )
{
    if( pObject )
    {
        // Does object have a parent?
        object* pParentObject = g_ObjMgr.GetObjectByGuid( pObject->GetParentGuid() );
        if( pParentObject )
        {
            // Give parent a chance to handle the pain
            if( pParentObject->OnChildPain( pObject->GetGuid(), *this ) )
                return TRUE;
        }
    
        // Parent didn't handle the pain, so apply to object
        pObject->OnPain( *this );
        return TRUE;
    }
    return FALSE;
}

//==============================================================================






