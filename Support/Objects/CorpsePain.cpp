//==============================================================================
//
//  CorpsePain.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "CorpsePain.hpp"
#include "Objects\Actor\Actor.hpp"
#include "Corpse.hpp"
#include "Objects\ParticleEmiter.hpp"


//==============================================================================
// FUNCTIONS
//==============================================================================

// Constructor
corpse_pain::corpse_pain()
{
    Clear();
}

//==============================================================================

// Clears defaults to a suicide
void corpse_pain::Clear( void )
{
    m_Type       = TYPE_SUICIDE;
    m_iRigidBody = -1;
    m_bOnDeath = TRUE;
    m_bDirectHit = FALSE;
    m_OriginNetSlot = -1;
    m_Position.Set( 0.0f, 1.8f * 100.0f, 0.0f );
    m_Direction.Set( 0.0f, 0.0f, 1.0f );
    m_Force = 0.25f;
    m_ForceFarDist = 100.0f;
}

//==============================================================================

// Read/Write to bitstream
void corpse_pain::Read( const bitstream& BS )
{
    s32 V;

    BS.ReadRangedS32( V, 0, TYPE_COUNT );
    m_Type = V;

    BS.ReadRangedS32( V, -1, 15 );
    m_iRigidBody = V;

    m_bOnDeath   = BS.ReadFlag();
    m_bDirectHit = BS.ReadFlag();
    
    BS.ReadRangedS32( V, -1, 31 );
    m_OriginNetSlot = V;
    
    BS.ReadRangedVector( m_Position, 10, -100*50.0f, 100*50.0f );
    BS.ReadUnitVector( m_Direction, 20 );
    BS.ReadRangedF32( m_Force, 20, 0.0f, 10000.0f );
    BS.ReadRangedF32( m_ForceFarDist, 20, 0.0f, 100*100.0f );
}

//==============================================================================

void corpse_pain::Write( bitstream& BS )
{
    // Make sure direction is valid
    if( m_Direction.LengthSquared() < x_sqr( 0.95f ) )
        m_Direction.Set( 0.0f, 0.0f, 1.0f );

    BS.WriteRangedS32( m_Type, 0, TYPE_COUNT );
    BS.WriteRangedS32( m_iRigidBody, -1, 15 );
    BS.WriteFlag( m_bOnDeath );
    BS.WriteFlag( m_bDirectHit );
    BS.WriteRangedS32( m_OriginNetSlot, -1, 31 );
    BS.WriteRangedVector( m_Position, 10, -100*50.0f, 100*50.0f );
    BS.WriteUnitVector( m_Direction, 20 );
    BS.WriteRangedF32( m_Force, 20, 0.0f, 10000.0f );
    BS.WriteRangedF32( m_ForceFarDist, 20, 0.0f, 10001.0f );
}

//==============================================================================

// Sets up values from pain hitting a corpse
void corpse_pain::Setup( const pain& Pain, corpse& Corpse )
{
    // If the origin guid is 0, then this came from suicide
    if( Pain.GetOriginGuid() == 0 )
    {
        Clear();
        return;
    }

    // Force direct hit guid to be corpse 
    // (incase pain came from direct hit of an npc being killed)
    pain CorpsePain = Pain;
    if( Pain.GetDirectHitGuid() != 0 )
        CorpsePain.SetDirectHitGuid( Corpse.GetGuid() );

    // Compute damage and force that this pain does    
    health_handle HealthHandle( Corpse.GetLogicalName() );
    CorpsePain.ComputeDamageAndForce( HealthHandle, Corpse.GetGuid(), Corpse.GetPosition() );

    // Lookup max force
    f32 MaxForce  = 1.0f;
    pain_health_handle hPainHealthHandle = CorpsePain.GetPainHealthHandle();
    const pain_health_profile* pPainHealthProfile = hPainHealthHandle.GetPainHealthProfile();
    if( pPainHealthProfile )
    {
        MaxForce  = pPainHealthProfile->m_Force;
    }

    // Lookup pain info
    f32   ForceFarDist = 100.0f * 10.0f;
    xbool bSplash  = FALSE;
    pain_handle hPain = CorpsePain.GetPainHandle();
    const pain_profile* pPainProfile = hPain.GetPainProfile();
    if( pPainProfile )
    {
        ForceFarDist = pPainProfile->m_ForceFarDist;
        bSplash      = pPainProfile->m_bSplash;
    }

    // Lookup rigid body that was hit due to a collision?
    s32   iHitRigidBody = -1;
    xbool bOnDeath = FALSE;
    if( CorpsePain.HasCollision() )
    {
        // Lookup collision
        const collision_mgr::collision& Collision = CorpsePain.GetCollision();

        // If corpse was hit, just use prim key as is (see "physics_inst::OnColCheck" )
        if( Collision.ObjectHitGuid == Corpse.GetGuid() )
        {
            iHitRigidBody = Collision.PrimitiveKey;
        }
        else
        {
            // If npc actor geometry was hit, then map from bone index to rigid body
            object_ptr<actor> pActor( Collision.ObjectHitGuid );
            if( pActor )
            {
                bOnDeath = TRUE;

                // Get geometry
                geom* pGeom = pActor->GetGeomPtr();
                if( pGeom )
                {
                    // Lookup bone, and map to rigid body if valid
                    s32 iBone = Collision.PrimitiveKey & 0xFF;
                    if( ( iBone >= 0 ) && ( iBone < pGeom->m_nBones ) )
                        iHitRigidBody = pGeom->m_pBone[ iBone ].iRigidBody;
                }
            }
        }            
    }
    
    // Clear to suicide
    Clear();
    
    // Set defaults
    m_Position      = CorpsePain.GetPosition() - Corpse.GetPosition();
    m_Direction     = CorpsePain.GetDirection();
    m_ForceFarDist  = ForceFarDist;            
    m_bOnDeath      = bOnDeath;
    m_bDirectHit    = CorpsePain.IsDirectHit();
    
    // Setup origin net slot
    m_OriginNetSlot = -1;
    object_ptr<actor> pOriginActor( CorpsePain.GetOriginGuid() );
    if( pOriginActor )
        m_OriginNetSlot = pOriginActor->net_GetSlot();
    
    // Use the hit_type to determine how to react
    s32 HitType = CorpsePain.GetHitType();
    if( HitType == 4 )  // Melee?
    {
        // Setup melee pain
        m_Type  = TYPE_MELEE;
        m_Force = CorpsePain.GetForce();
    }
    else
    {
        // Come from an explosion?
        if( bSplash )
        {
            // Setup splash pain
            m_Type         = TYPE_SPLASH;
            m_Force        = MaxForce;
            m_ForceFarDist = ForceFarDist;            
        }
        else
        {
            // Apply directly to rigid body?
            if( ( iHitRigidBody >= 0 ) && ( iHitRigidBody < Corpse.GetPhysicsInst().GetNRigidBodies() ) )
            {
                // Setup rigid body pain
                m_Type       = TYPE_RIGID_BODY;
                m_Force      = MaxForce;
                m_iRigidBody = iHitRigidBody;
            }                
            else
            {
                // Setup blast pain
                m_Type  = TYPE_BLAST;
                m_Force = MaxForce;
            }
        }
    }
    
    // Create impact effect
    Corpse.CreateImpactEffect( CorpsePain );
}

//==============================================================================

volatile f32 DEATH_FORCE_SCALE = 3.0f;

// Applies pain to corpse
void corpse_pain::Apply( corpse& Corpse )
{
    // Lookup useful info
    vector3       Position    = Corpse.GetPosition() + m_Position;
    physics_inst& PhysicsInst = Corpse.GetPhysicsInst();
    
    // Which type?
    switch( m_Type )
    {
    case TYPE_MELEE:
        {   
            // Lookup tweaks
            f32 ForceScale = GetTweakF32( "DEADBODY_MeleeForceScale", 2000.0f );

            // If occurring at death, crank it up
            if( m_bOnDeath )
                ForceScale *= DEATH_FORCE_SCALE;

            // Apply melee impact
            PhysicsInst.ApplyVectorForce( m_Direction, ForceScale * m_Force );
            
            // Set constraints to full amount to stop npc rolling up into a ball!
            PhysicsInst.SetConstraintWeight( 1.0f );
        }     
        break;    
    
    case TYPE_SPLASH:
        {   
            // Lookup tweaks
            f32 ForceScale = GetTweakF32( "DEADBODY_ExplosionForceScale", 2000.0f );
            f32 DirAmount  = GetTweakF32( "DEADBODY_ExplosionDirAmount", 0.25f );
            f32 UpAmount   = GetTweakF32( "DEADBODY_ExplosionUpAmount", 0.75f );

            // If occurring at death, crank it up
            if( m_bOnDeath )
                ForceScale *= DEATH_FORCE_SCALE;

            // Apply grenade positional blast
            PhysicsInst.ApplyBlast( Position, m_ForceFarDist, ForceScale * DirAmount * m_Force );

            // Add fake upwards blast to make ragdolls look cool
            PhysicsInst.ApplyBlast( Position, vector3(0,1,0), m_ForceFarDist, ForceScale * UpAmount * m_Force );
            
            // Set constraints to full amount to stop npc rolling up into a ball!
            PhysicsInst.SetConstraintWeight( 1.0f );
        }     
        break;    

    case TYPE_RIGID_BODY:
        {   
            // Lookup tweaks
            f32 ForceScale = GetTweakF32( "DEADBODY_ImpactForceScale", 2000.0f );

            // If occurring at death, crank it up
            if( m_bOnDeath )
                ForceScale *= DEATH_FORCE_SCALE;
        
            // Lookup collision shape that was hit and get rigid body owner
            rigid_body& Body = PhysicsInst.GetRigidBody( m_iRigidBody );

            // Apply impulse directly to body and make sure physics instance becomes active
            Body.ApplyWorldImpulse( m_Force * ForceScale * m_Direction, Position );
            PhysicsInst.Activate();
            
            // Set constraints to full amount to stop npc rolling up into a ball!
            PhysicsInst.SetConstraintWeight( 1.0f );
        }     
        break;    
    
    case TYPE_SUICIDE:
    case TYPE_BLAST:
        {   
            // Lookup tweaks
            f32 ForceScale = GetTweakF32( "DEADBODY_ImpactForceScale", 2000.0f );
            f32 Radius     = GetTweakF32( "DEADBODY_ImpactRadius",     600.0f );

            // If occurring at death, crank it up
            if( m_bOnDeath )
                ForceScale *= DEATH_FORCE_SCALE;
        
            // Apply small blast
            PhysicsInst.ApplyBlast( Position, m_Direction, Radius, m_Force * ForceScale );
            
            // Set constraints to full amount to stop npc rolling up into a ball!
            PhysicsInst.SetConstraintWeight( 1.0f );
        }     
        break;    
    }
}

//==============================================================================
// Query functions
//==============================================================================

guid corpse_pain::GetOriginGuid( void ) const
{
#ifndef X_EDITOR
    // No origin?
    if( m_OriginNetSlot == -1 )
        return 0;
        
    // Lookup origin guid
    netobj* pOrigin = NetObjMgr.GetObjFromSlot( m_OriginNetSlot );
    if( pOrigin )
        return pOrigin->GetGuid();
#endif
    
    return 0;
}

//==============================================================================

const vector3& corpse_pain::GetDirection( void ) const
{
    return m_Direction;
}

//==============================================================================

xbool corpse_pain::IsDirectHit( void ) const
{
    return m_bDirectHit;
}

//==============================================================================

