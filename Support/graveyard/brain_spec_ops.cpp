///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain_spec_ops.cpp
//
//      - brain specialization for a special ops AI
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "brain_spec_ops.hpp"
#include "objects\npc.hpp"
#include "objects\playerstagefive.hpp"


#include "objects\DeadBody.hpp"
#include "objects\ParticleEmiter.hpp"

static const f32  PAIN_PARTICLE_DISPLACE_AMT = 20.0f;

static f32 s_GunOffsets[brain_spec_ops::GUN_TYPES_END] = 
{
    20.0f
};

//=========================================================================

static spec_ops_loco* GetSpecOpsLoco( npc* pOwner )
{
    ASSERT(pOwner);

    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    spec_ops_loco* pSpecOpsLoco = NULL;

    ASSERT(pLoco);
    
    if ( pOwner->GetCharacterType() == npc::NPC_TYPE_SPEC_OPS )
        pSpecOpsLoco = (spec_ops_loco*)pLoco;
    else
       return NULL;
   
    ASSERT(pSpecOpsLoco);

    return pSpecOpsLoco;
}

//=========================================================================

brain_spec_ops::brain_spec_ops( npc* newOwner) :
    brain(newOwner),
    m_MuzzleFlashGuid(0),
    m_MuzzleBone(-1),
    m_BoneIint(FALSE)
{}

brain_spec_ops::~brain_spec_ops()
{}

void brain_spec_ops::Init(void)
{
    weapon* tempWeapon = new weapon(m_Owner->GetGuid(), WEAPON_TYPE_SPEC_OPS_GUN );

    m_Weapons[0] = tempWeapon;

    brain::Init();
}

void brain_spec_ops::OnPain( const pain& Pain )
{

    if(m_CurrentState)
    {
        m_CurrentState->OnPain(Pain);
    }

    // Cheap sphere v axis aligned cylinder collision

    // Get owner
    npc* pOwner = GetOwner() ;
    ASSERT(pOwner) ;

    // Get loco
    base_loco* pLoco = (base_loco*)pOwner->GetLocomotion() ;
    ASSERT(pLoco) ;

    vector3 Pos       = pLoco->GetPosition();
    f32     ColHeight = pLoco->m_Physics.GetColHeight();
    f32     ColRadius = pLoco->m_Physics.GetColRadius();

    // Lookup info about self
    f32     Health    = pOwner->GetHealth();

    // Below player?
    if ( (Pain.Center.Y + Pain.RadiusR1) < Pos.Y)
        return ;

    // Above player?
    if ( (Pain.Center.Y - Pain.RadiusR1) > (Pos.Y + ColHeight) )
        return ;

    // Get distance to axis aligned cylinder
    f32 Dist = SQR(Pos.X - Pain.Center.X) + SQR(Pos.Z - Pain.Center.Z) ;
    if (Dist > 0.00001f)
    {
        Dist = x_sqrt(Dist) ;
        Dist = MAX(0, Dist - ColRadius) ;
    }

	if ( Pain.AnimEvent == ANIM_EVENT_NULL )
	{
		OnNonAnimPain( Pain , Pos );
		return;
	}
/*
    if( Pain.Type == pain::TYPE_PROJECTILE_BULLET || Pain.Type == pain::TYPE_PROJECTILE_SPIKE )
    {
        Health = MAX(0,Health - Pain.DamageR0);

        //  SULTAN Play hit by spike sound here    Maybe SpecOps1_ImpactGrunt
        if(Health == 0 )
        {
        
            if( pOwner->IsAlive() )
            {
                ai_state* tempState = this->GetStateByName("Dead");
       
                if( tempState )
                {
                    RequestStateChange( tempState );
                }
                else
                {
                    x_DebugMsg( "No Dead state for this AI!\n" );
                }
            }

            {
                // Create the Blood Particle Effect only on Damage result..
                particle_emitter::CreateOnPainEffect(
                    Pain, PAIN_PARTICLE_DISPLACE_AMT, 
                    particle_emitter::BLOODY_MESS);
                
            }  
            
            {
                // Create the Blood Particle Effect only on Damage result..
                particle_emitter::CreateOnPainEffect(
                    Pain, PAIN_PARTICLE_DISPLACE_AMT, 
                    particle_emitter::BLOODY_SPIKE_HIT);
            }

        }
        else
        {
            pOwner->SetLife((s32)Health);

            // If any of the voices are playing, turn them off before playing the impact grunt.
            if( g_AudioManager.IsPlaying( m_AlertVoiceID ) || g_AudioManager.IsStarting( m_AlertVoiceID ) )
                g_AudioManager.Stop( m_AlertVoiceID );

            if( g_AudioManager.IsPlaying( m_ManDownVoiceID ) || g_AudioManager.IsStarting( m_ManDownVoiceID ) )
                g_AudioManager.Stop( m_ManDownVoiceID );

            if( g_AudioManager.IsPlaying( m_SpotStage5VoiceID ) || g_AudioManager.IsStarting( m_SpotStage5VoiceID ) )
                g_AudioManager.Stop( m_SpotStage5VoiceID );
            
            if( g_AudioManager.IsPlaying( m_IdleVoiceID ) || g_AudioManager.IsStarting( m_IdleVoiceID ) )
                g_AudioManager.Stop( m_IdleVoiceID );
            
            // Play the impact grunt.
            s8 DescName[32];
            x_sprintf( (char*)DescName, "SpecOps%d_ImpactGrunt",m_SpecOpType );
            g_AudioManager.Play( (const char*)DescName, Pos );

            
            g_AudioManager.Play( "ClawSliceFlesh", Pos );


            radian myAngle          = pLoco->GetFacingYaw();
            radian painDirection    = Pain.Direction.GetYaw();
            
            radian differenceAngle = myAngle - painDirection ;
            
            if( differenceAngle > PI + PI )
            {
                differenceAngle -= PI + PI;
            }
            else if( differenceAngle < 0.0 )
            {
                differenceAngle += PI + PI;
            }
            
            if( differenceAngle > PI * 0.25f && differenceAngle < PI * 0.75f )
            {
                pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_LEFT);    

            }
            else if ( differenceAngle > PI * 0.75f && differenceAngle < PI * 1.25f )
            {
                pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK);    

            }
            else if ( differenceAngle > PI * 1.25f && differenceAngle < PI * 1.75f )
            {
                pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_RIGHT);    

            }
            else 
            {
                ASSERT ( differenceAngle < PI * 0.25f || differenceAngle > PI * 1.75f );
                pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_FORWARD);                                
            }

            {
                // Create the Blood Particle Effect only on Damage result..
                particle_emitter::CreateOnPainEffect(
                    Pain, PAIN_PARTICLE_DISPLACE_AMT, 
                    particle_emitter::BLOODY_SPIKE_HIT);
            }
        }

    }
*/

    // Hit?
    else if (    Dist <= Pain.RadiusR1 || 
            Pain.AnimEvent == ANIM_EVENT_GRAB_ATTACH ||
            Pain.AnimEvent == ANIM_EVENT_IMPALE_BEGIN ||
            Pain.AnimEvent == ANIM_EVENT_IMPALE_ATTACH ||
            Pain.AnimEvent == ANIM_EVENT_IMPALE_DISMOUNT  )
    {
        // Get T value from 0->1, where T=0 at R0, and T=1 at R1
        f32 T = 0 ;
        if (Dist > Pain.RadiusR0)
            T = (Dist - Pain.RadiusR0) / (Pain.RadiusR1 - Pain.RadiusR0) ;

        // Calculate damage and force
        f32 Force  = Pain.ForceR0  + (T * (Pain.ForceR1  - Pain.ForceR0 )) ;
        f32 Damage = Pain.DamageR0 + (T * (Pain.DamageR1 - Pain.DamageR0)) ;

        // Take that pain
        Health = MAX(0, Health - Damage);
        
        {
            // Create the Blood Particle Effect only on Damage result..
            particle_emitter::CreateOnPainEffect(
                Pain, PAIN_PARTICLE_DISPLACE_AMT );
        }
        
        // Die?
        if (Health == 0 && 
            !(  Pain.AnimEvent == ANIM_EVENT_GRAB_BEGIN ||
                Pain.AnimEvent == ANIM_EVENT_GRAB_ATTACH ||
                Pain.AnimEvent == ANIM_EVENT_IMPALE_BEGIN ||
                Pain.AnimEvent == ANIM_EVENT_IMPALE_ATTACH ||
                Pain.AnimEvent == ANIM_EVENT_IMPALE_DISMOUNT  ) )
        {
            if( pOwner->IsAlive() )
            {
                ai_state* tempState = this->GetStateByName("Dead");
        
                if( tempState )
                {
                    RequestStateChange( tempState );
                }
                else
                {
                    x_DebugMsg( "No Dead state for this AI!\n" );
                }
            }
        }
        else
        {
            // If coming from a claw swipe - play a random flinch for now
            switch(Pain.AnimEvent)
            {
                case ANIM_EVENT_PAIN_CLAW_FORWARD:
                case ANIM_EVENT_PAIN_CLAW_LEFT_2_RIGHT:
                case ANIM_EVENT_PAIN_CLAW_RIGHT_2_LEFT:
                {
                    // TO DO - Code the math to play the correct impact when I've got all the anims
                    /*
                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK) ;
                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_FORWARD) ;
                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_LEFT) ;
                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_RIGHT) ;
                    */

                    radian myAngle          = pLoco->GetFacingYaw();
                    radian painDirection    = Pain.Direction.GetYaw();
                    
                    radian differenceAngle = myAngle + painDirection;
                    
                    if( differenceAngle > PI + PI )
                    {
                        differenceAngle -= PI + PI;
                    }
                    else if( differenceAngle < 0.0 )
                    {
                        differenceAngle += PI + PI;
                    }
                    
                    if( differenceAngle > PI * 0.25f && differenceAngle < PI * 0.75f )
                    {
                        pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_LEFT);    

                    }
                    else if ( differenceAngle > PI * 0.75f && differenceAngle < PI * 1.25f )
                    {
                        pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK);    

                    }
                    else if ( differenceAngle > PI * 1.25f && differenceAngle < PI * 1.75f )
                    {
                        pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_RIGHT);    

                    }
                    else 
                    {
                        ASSERT ( differenceAngle < PI * 0.25f || differenceAngle > PI * 1.75f );
                        pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_FORWARD);                                
                    }
                    
                    g_AudioManager.Play( "ClawSliceFlesh", Pos );

                    // If any of the voices are playing, turn them off before playing the impact grunt.
                    if( g_AudioManager.IsPlaying( m_AlertVoiceID ) || g_AudioManager.IsStarting( m_AlertVoiceID ) )
                        g_AudioManager.Stop( m_AlertVoiceID );

                    if( g_AudioManager.IsPlaying( m_ManDownVoiceID ) || g_AudioManager.IsStarting( m_ManDownVoiceID ) )
                        g_AudioManager.Stop( m_ManDownVoiceID );

                    if( g_AudioManager.IsPlaying( m_SpotStage5VoiceID ) || g_AudioManager.IsStarting( m_SpotStage5VoiceID ) )
                        g_AudioManager.Stop( m_SpotStage5VoiceID );
            
                    if( g_AudioManager.IsPlaying( m_IdleVoiceID ) || g_AudioManager.IsStarting( m_IdleVoiceID ) )
                        g_AudioManager.Stop( m_IdleVoiceID );
            
                    // Play the impact grunt.
                    s8 DescName[32];
                    x_sprintf( (char*)DescName, "SpecOps%d_ImpactGrunt", m_SpecOpType );
                    g_AudioManager.Play( (const char*)DescName, Pos );


                    // For now just play step back since I don't have a step right anim yet!
//                    pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK);    
                }
                break ;


                case ANIM_EVENT_GRAB_BEGIN:
                    {
                        // There is not more Stage5
                        ASSERT( 0 );
                        /*
                        object* pObject = g_ObjMgr.GetObjectByGuid( Pain.Origin );
                        if ( pObject->IsKindOf( player_stage5::GetRTTI() ) )
                        {
							player_stage5& rPlayer = player_stage5::GetSafeType( *pObject );

							//if the player is not currently syphoning someone.
							if ( !rPlayer.GetSyphonAttackSuccess() )
							{
								m_Owner->Disable();
								//create a new dead_body
								guid bodyGuid = g_ObjMgr.CreateObject( object::TYPE_DEAD_BODY );
								object* pDeadBody = g_ObjMgr.GetObjectByGuid( bodyGuid );
								( ( dead_body* )pDeadBody )->InitializeDeadBody( m_Owner->GetSkinInst().GetSkinGeomName() , m_Owner->GetLocomotion()->m_Player , m_Owner->GetLocomotion()->m_Physics );

								//store the guid of the dead body inside the player
								rPlayer.StoreBodyGuid( bodyGuid );
								rPlayer.SetSyphonAttackSuccess( TRUE , bodyGuid );
		                        RequestStateChange( "Grabbed" );
							}
                        }
                        */
/*
                        m_LastAnimSet = base_loco::AT_GRABBED_START;

                        pLoco->PlayAnimState( base_loco::AT_GRABBED_START );
                        object* tempObject = g_ObjMgr.GetObjectByGuid( Pain.Origin );
                        if ( tempObject->IsKindOf( player_stage5::GetRTTI() ) )
                        {
                            player_stage5& tempPlayer = player_stage5::GetSafeType( *tempObject );
                            tempPlayer.SetSyphonAttackSuccess( TRUE , m_Owner->GetGuid() );
                        }
//                        m_Owner->SetAttrBits(m_Owner->GetAttrBits() & !object::ATTR_COLLIDABLE );
//                        pLoco->SetPitch(Pain.Direction.GetPitch() );
                        pLoco->SetYaw(  Pain.Direction.GetYaw() );
//                        vector3 tempVec = pLoco->GetBonePosition("Bip01 R Clavicle");
*/
 
                    }
                    break;
                case ANIM_EVENT_GRAB_ATTACH:
                    {
                        if(m_LastAnimSet != base_loco::AT_GRABBED_SUFFERING )
                        {
                            m_LastAnimSet = base_loco::AT_GRABBED_SUFFERING;
                            pLoco->PlayAnimState( base_loco::AT_GRABBED_SUFFERING );
                        }

                    }
                    break;
                case ANIM_EVENT_IMPALE_BEGIN:
                    {
                        if(m_LastAnimSet != base_loco::AT_GRABBED_GOES_LIMP )
                        {
                            m_LastAnimSet = base_loco::AT_GRABBED_GOES_LIMP;
                            pLoco->PlayAnimState( base_loco::AT_GRABBED_GOES_LIMP );
                        }

                        g_AudioManager.Play( "ClawImpaleFlesh", Pos );
    
                    }
                    break;
                case ANIM_EVENT_IMPALE_ATTACH:
                    {
                        if(m_LastAnimSet != base_loco::AT_GRABBED_PUSH_OFF )
                        {
                            m_LastAnimSet = base_loco::AT_GRABBED_PUSH_OFF;
                            pLoco->PlayAnimState( base_loco::AT_GRABBED_PUSH_OFF );
                        }
                        
                        g_AudioManager.Play( "ClawImpactFlesh", Pos );
                    }
                    break;
                case ANIM_EVENT_IMPALE_DISMOUNT:
                    {
                        if(m_LastAnimSet != base_loco::AT_GRABBED_FLYING )
                        {
                            m_LastAnimSet = base_loco::AT_GRABBED_FLYING;
                            pLoco->PlayAnimState( base_loco::AT_GRABBED_FLYING );
                            ai_state* tempState = GetStateByName("Dead");
            
                            if( tempState )
                            {
//                                RequestStateChange( tempState );
                            }
                            else
                            {
                                x_DebugMsg( "No Dead state for this AI!\n" );
                            }
                        }

                    }
                    break;
            }

        }
        
        pOwner->SetLife((s32)Health);

        // Calculate impulse - TO DO - Use: Force/Mass 
        vector3 Impulse = Pain.Direction * Force;
        
        // Apply impulse
        pLoco->m_Physics.AddVelocity(Impulse);
    }
}

//=========================================================================

void brain_spec_ops::OnNonAnimPain( const pain& Pain , const vector3& Pos )
{
	f32 fNewHealth = fMax( 0.f , m_Owner->GetHealth() - Pain.DamageR0 );

	//decrement the NPC's health by the pain value
	m_Owner->SetLife( (s32)fNewHealth );

	//if the owner is dead, request a state change.
    if( ! ( m_Owner->IsAlive() ) )
    {
        ai_state* tempState = this->GetStateByName("Dead");

        if( tempState )
        {
            RequestStateChange( tempState );
        }
        else
        {
            x_DebugMsg( "No Dead state for this AI!\n" );
        }

		//play the effect
        // Create the Blood Particle Effect only on Damage result..
        particle_emitter::CreateOnPainEffect( Pain, PAIN_PARTICLE_DISPLACE_AMT , particle_emitter::BLOODY_MESS );
    }
	else
	{
		base_loco* pLoco = m_Owner->GetLocomotion() ;

        // If any of the voices are playing, turn them off before playing the impact grunt.
        if( g_AudioManager.IsPlaying( m_AlertVoiceID ) || g_AudioManager.IsStarting( m_AlertVoiceID ) )
            g_AudioManager.Release( m_AlertVoiceID, 0.0f );

        if( g_AudioManager.IsPlaying( m_ManDownVoiceID ) || g_AudioManager.IsStarting( m_ManDownVoiceID ) )
            g_AudioManager.Release( m_ManDownVoiceID, 0.0f );

        if( g_AudioManager.IsPlaying( m_SpotStage5VoiceID ) || g_AudioManager.IsStarting( m_SpotStage5VoiceID ) )
            g_AudioManager.Release( m_SpotStage5VoiceID, 0.0f );
        
        if( g_AudioManager.IsPlaying( m_IdleVoiceID ) || g_AudioManager.IsStarting( m_IdleVoiceID ) )
            g_AudioManager.Release( m_IdleVoiceID, 0.0f );
        
        // Play the impact grunt.
        s8 DescName[32];
        x_sprintf( (char*)DescName, "SpecOps%d_ImpactGrunt",m_SpecOpType );
        g_AudioManager.Play( (const char*)DescName, Pos );

        
        g_AudioManager.Play( "ClawSliceFlesh", Pos );


        radian myAngle          = pLoco->GetFacingYaw();
        radian painDirection    = Pain.Direction.GetYaw();
        
        radian differenceAngle = myAngle - painDirection ;
        
        if( differenceAngle > PI + PI )
        {
            differenceAngle -= PI + PI;
        }
        else if( differenceAngle < 0.0 )
        {
            differenceAngle += PI + PI;
        }
        
        if( differenceAngle > PI * 0.25f && differenceAngle < PI * 0.75f )
        {
            pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_LEFT);    

        }
        else if ( differenceAngle > PI * 0.75f && differenceAngle < PI * 1.25f )
        {
            pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_BACK);    

        }
        else if ( differenceAngle > PI * 1.25f && differenceAngle < PI * 1.75f )
        {
            pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_RIGHT);    

        }
        else 
        {
            ASSERT ( differenceAngle < PI * 0.25f || differenceAngle > PI * 1.75f );
            pLoco->PlayAnimState(base_loco::AT_IMPACT_STEP_FORWARD);                                
        }

        // Create the Blood Particle Effect only on Damage result..
        particle_emitter::CreateOnPainEffect( Pain, PAIN_PARTICLE_DISPLACE_AMT, particle_emitter::BLOODY_SPIKE_HIT );
	}
}
//=========================================================================

xbool brain_spec_ops::FireWeaponAt( guid thisTarget, s32 thisWeapon )
{
    xbool ReturnVal = brain::FireWeaponAt( thisTarget, thisWeapon );
    
    //already created a muzzle flash....
    if (m_MuzzleFlashGuid != NULL)
        return ReturnVal;

    spec_ops_loco* pSpecOpsLoco = GetSpecOpsLoco( GetOwner() );

    if (pSpecOpsLoco == NULL)
       return ReturnVal;
 
    guid gFlashGuid = particle_emitter::CreatePresetParticle( particle_emitter::SPEC_OPS_MUZZLE_FLASH );

    m_MuzzleFlashGuid = gFlashGuid;
    
    PositionMuzzleFlash();

    return ReturnVal;
} 

//=========================================================================

void brain_spec_ops::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "brain_spec_ops::OnAdvanceLogic" );

    brain::OnAdvanceLogic( deltaTime );

    PositionMuzzleFlash();
} 

//=========================================================================

void brain_spec_ops::PositionMuzzleFlash()
{
    //MUZZLE FLASH!

    if ( m_BoneIint == FALSE )
    {
        //Will only try this once ....
        spec_ops_loco* pSpecOpsLoco = GetSpecOpsLoco( GetOwner() );
        
        if ( pSpecOpsLoco != NULL )
            m_MuzzleBone = pSpecOpsLoco->GetBoneIndexByName("Bip01 L Hand");

        m_BoneIint = TRUE;
    }

    if ( m_MuzzleFlashGuid == NULL || m_MuzzleBone == -1 )
        return;
    
    npc* pOwner = GetOwner();

    spec_ops_loco* pSpecOpsLoco = GetSpecOpsLoco( pOwner );
    
    if (pSpecOpsLoco == NULL)
        return;
    
    object* pBloodObject = g_ObjMgr.GetObjectByGuid( m_MuzzleFlashGuid );
    
    if (pBloodObject == NULL)
    {
        m_MuzzleFlashGuid = NULL;
        return;
    }

    vector3 vBonPos = pSpecOpsLoco->GetBonePosition( m_MuzzleBone );
    vector3 vLookAt = vBonPos - pOwner->GetLocomotion()->GetLookAt();

    vLookAt.Normalize();

    vector3 MuzzlePos = vBonPos - s_GunOffsets[GetGunType()]*vLookAt;
    particle_emitter::PointObjectAt( pBloodObject, vLookAt, MuzzlePos );

    return;
}
