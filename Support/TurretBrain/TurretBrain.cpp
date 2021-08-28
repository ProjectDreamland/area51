#include "TurretBrain.hpp"

static f32 k_TimeBetweenLOSChecks       = 2.0f;

//=========================================================================

turret_brain::turret_brain()
{
    m_TurretGuid = 0;
    m_TargetGuid = 0;

    m_W2L.Identity();

    m_SensorPos.Set(0,0,0);
    m_LocalTargetDir(0,0,1);

    m_SenseRadius = 0;
    m_FireRadius  = 0;

    m_LOSCheckTimer = 0;

    m_ObjSlotForLastLOSCheck = SLOT_NULL;

    m_bLastLOSPassed = FALSE;
}

//=========================================================================

void turret_brain::SetTargetGuid   ( guid Guid )
{
    m_TargetGuid = Guid;
}

//=========================================================================

void turret_brain::SetTurretGuid( guid Guid )
{
    m_TurretGuid = Guid;
}

//=========================================================================

void turret_brain::SetL2W( const matrix4& L2W )
{
    m_W2L = L2W;
    m_W2L.Invert();
}

//=========================================================================

turret_brain::tracking_status turret_brain::UpdateTracking  ( f32 DeltaTime )
{
    (void)DeltaTime;

    if (m_TargetGuid == 0)
        return TRACK_NO_TARGET;

    if (!CanSenseTarget() && !CanFireAtTarget())
    {
        SetTargetGuid(0);
        return TRACK_NO_TARGET;
    }

    if (!IsTargetValid())
    {
        SetTargetGuid(0);
        return TRACK_NO_TARGET;
    }
/*
    // Make sure target is in FOF
    vector3 Pt = GetObjectAimAt( m_TargetGuid );
    if (GetAimScoreForPoint( Pt ) < 0)
    {
        SetTargetGuid(0);
        return TRACK_OUTSIDE_FOF;
    }
*/
    vector3 TargetPos = GetObjectAimAt( m_TargetGuid );

    // We want to work with position and angles in local turret space
    TargetPos = m_W2L.Transform( TargetPos );
    m_SensorPos = m_W2L.Transform( m_SensorPos );

    vector3 Delta = TargetPos - m_SensorPos;
    m_LocalTargetDir = Delta;
    m_LocalTargetDir.Normalize();

    radian  Yaw,Pitch;
    Delta.GetPitchYaw(Pitch,Yaw);

    // Look for out of range condition
    Delta.Y = 0;
    f32 Dist = Delta.Length();
    if (Dist > MAX( m_FireRadius, m_SenseRadius ))
        return TRACK_NO_TARGET;

    Yaw += R_180;
    
    return TRACK_AIMING;
}

//=========================================================================

radian3 turret_brain::GetRotations( void ) const
{
    radian3 Ret(0,0,0);

    return Ret;
}

//=========================================================================

void turret_brain::SetSensorPos( const vector3& SensorPos )
{
    m_SensorPos = SensorPos;
}

//=========================================================================

vector3 turret_brain::GetObjectAimAt( guid Guid, actor::eOffsetPos OfsPos) const
{
    if (Guid == 0)
        return vector3(0,0,0);

    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );

    if (NULL == pObj)
        return vector3(0,0,0);    
    
    vector3 TargetPos = pObj->GetBBox().GetCenter();

    if (pObj->IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( *pObj );

        TargetPos = Actor.GetPositionWithOffset( OfsPos );
    }

    return TargetPos;
}


//=========================================================================

f32 turret_brain::DistToObjectAimAt( guid Guid ) const
{
    vector3     Target = GetObjectAimAt( Guid );
    
    return (Target - m_SensorPos).Length();
}

//=========================================================================

xbool turret_brain::CanSenseTarget( void )
{
    if ( DistToObjectAimAt( m_TargetGuid ) > m_SenseRadius )
        return FALSE;

    if ( !CheckLOS( m_TargetGuid ) )
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool turret_brain::CanFireAtTarget( void )
{
    if ( DistToObjectAimAt( m_TargetGuid ) > m_FireRadius )
        return FALSE;

    if ( !CheckLOS( m_TargetGuid ) )
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool turret_brain::IsTargetInRange( void ) const
{
    f32 Dist = DistToObjectAimAt( m_TargetGuid );

    if ( (Dist > m_FireRadius) && (Dist > m_SenseRadius) )
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool turret_brain:: CheckLOS( const vector3& Pt ) const
{
    g_CollisionMgr.LineOfSightSetup( m_TurretGuid, m_SensorPos, Pt );
    g_CollisionMgr.AddToIgnoreList( m_TurretGuid );
    g_CollisionMgr.AddToIgnoreList( m_TargetGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING) );

    //no collisions, move the object.
    if( g_CollisionMgr.m_nCollisions == 0 )
    {
        return TRUE;
    } 

    return FALSE;
}

//=========================================================================

xbool turret_brain::CheckLOS( guid Guid )
{
    slot_id SID = g_ObjMgr.GetSlotFromGuid( Guid );
    if ( SID == SLOT_NULL )
        return FALSE;

    if (m_LOSCheckTimer > k_TimeBetweenLOSChecks)
    {
        m_LOSCheckTimer          = 0;
        m_ObjSlotForLastLOSCheck = SLOT_NULL;
    }

    if ( SID == m_ObjSlotForLastLOSCheck )
        return m_bLastLOSPassed;

    vector3 TargetPos = GetObjectAimAt( Guid );

    m_bLastLOSPassed         = CheckLOS( TargetPos );    
    m_ObjSlotForLastLOSCheck = SID;

    return m_bLastLOSPassed;
}

//=========================================================================

xbool turret_brain::IsTargetValid( void ) const
{
    if (m_TargetGuid == 0)
        return FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return FALSE;

    ASSERT( pObj ); 
    
    if (pObj->IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( *pObj );

        return !!(Actor.GetParametricHealth() > 0);
    }
   
    return TRUE;
}

//=========================================================================

void turret_brain::OnEnumProp( prop_enum& List )
{
    List.AddFloat   ( "Alert Distance", "How far can the turret sense targets", PROP_TYPE_EXPOSE );
    List.AddFloat   ( "Firing Distance", "How far can the turret shoot", PROP_TYPE_EXPOSE );
}

//=============================================================================

xbool turret_brain::OnProperty( prop_query& I )
{
    (void)I;
    
    /*
    if( I.VarAngle( "Pitch Speed", m_PitchSpeed ) )
        return TRUE;
    else
    if( I.VarAngle( "Yaw Speed", m_YawSpeed ) )
        return TRUE;
    else
    if( I.VarAngle( "Pitch Up Limit", m_PitchUpLimit) )
    {
        m_PitchUpLimit = MIN(R_90,MAX(R_0,m_PitchUpLimit));
        
        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Pitch Down Limit", m_PitchDownLimit ) )
    {
        m_PitchDownLimit = MIN(R_90,MAX(R_0,m_PitchDownLimit));

        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Yaw Right Limit", m_YawRightLimit) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawLeftLimit = R_360 - m_YawRightLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Yaw Left Limit", m_YawLeftLimit ) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawRightLimit = R_360 - m_YawLeftLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    */
    return( FALSE );
}