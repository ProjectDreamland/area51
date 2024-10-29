#include "mutanttank_attack_state.hpp"
#include "mutant_tank.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Objects\WeaponShotgun.hpp"
#include "Objects\SuperDestructible.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
// DEFINES
//=========================================================================


#ifndef X_RETAIL
// Use to test in debugger
static s32 MUTANT_TANK_TEST_STAGE = -1;
static mutanttank_attack_state::stage* P_MUTANT_TANK_TEST_STAGE = NULL;
#endif

#define MUTANT_TANK_MAX_PACE_TIME   4.0f

//=========================================================================
// DATA
//=========================================================================

typedef enum_pair<mutanttank_attack_state::type> type_enum_pair;
static type_enum_pair s_TypeEnumPair[] = 
{
    type_enum_pair( "A",    mutanttank_attack_state::TYPE_A ),
    type_enum_pair( "E3",    mutanttank_attack_state::TYPE_E3 ),
    type_enum_pair( "AB45", mutanttank_attack_state::TYPE_AB45 ),
    
    type_enum_pair( k_EnumEndStringConst,   mutanttank_attack_state::TYPE_COUNT ) //**MUST BE LAST**//
};
enum_table<mutanttank_attack_state::type>  s_TypeList( s_TypeEnumPair );


//=========================================================================
// STAGE FUNCTIONS
//=========================================================================

mutanttank_attack_state::stage::stage()
{
    // Zero everything out
    x_memset( this, 0, sizeof( this ) );
}

//=========================================================================

void mutanttank_attack_state::stage::LoadTweaks( const xstring& Prefix )
{
    // Read tweaks
    m_Health                      = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "HealthPercent"        ),  0.0f );    
    m_MoveSpeed                   = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "MoveSpeed"            ),  0.0f );        
    m_ChaseMinTime                = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "MinChaseTime"         ),  0.5f );        
    m_EvadeInterval               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "EvadeInterval"        ),  2.0f );    
    m_ChargeInterval              = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ChargeInterval"       ), -1.0f );    
    m_ChargeToMaxSpeedTime        = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ChargeToMaxSpeedTime" ),  0.0f );    
    m_ChargeMaxSpeed              = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ChargeMaxSpeed"       ),  0.0f );    
    m_ChargeMinDist               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ChargeMinDist"        ),  0.0f );    
    m_ChargeMaxDist               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ChargeMaxDist"        ),  0.0f );    
    m_RangedInterval              = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "RangedInterval"       ), -1.0f );    
    m_RangedMinDist               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "RangedMinDist"        ),  0.0f );    
    m_RangedMaxDist               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "RangedMaxDist"        ),  0.0f );    
    m_LeapInterval                = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "LeapInterval"         ), -1.0f );    
    m_LeapMinDist                 = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "LeapMinDist"          ),  0.0f );    
    m_LeapMaxDist                 = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "LeapMaxDist"          ),  0.0f );    
    m_MeleeInterval               = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "MeleeInterval"        ),  0.0f );    
    m_BubbleHealthInterval        = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "BubbleHealthPercentInterval" ), -1.0f );    
    m_CanisterHealthInterval      = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "CanisterHealthPercentInterval" ), -1.0f );    
    m_CanisterIdleTime            = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "CanisterIdleTime" ), -1.0f );    
    m_ParasiteShieldRegenInterval = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ParasiteShieldRegenInterval" ), -1.0f );    
    m_ContagionHealthInterval     = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "ContagionHealthPercentInterval" ), -1.0f );
    m_JumpToPerch                 = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "JumpToPerch" ), -1.0f );    
    m_JumpToGrate                 = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "JumpToGrate" ), -1.0f );
}

//=========================================================================
// ATTACK STATE FUNCTIONS
//=========================================================================

mutanttank_attack_state::mutanttank_attack_state( character& Mutant, states State ) :
    character_attack_state(Mutant, State)
{
    // Tank uses mix between walk and run move styles
    m_MoveStyle = loco::MOVE_STYLE_WALK;
    
    // Misc
    m_LastPaceLeft       = x_irand(0,1);
    m_bSwitchOutOfAttack = FALSE;
    m_bWantToEvade       = FALSE;

    // Reset timers    
    m_ChaseTimer        = 0.0f;
    m_EvadeTimer        = 0.0f;
    m_LeapTimer         = 0.0f;
    m_MeleeTimer        = 0.0f;
    m_RangedTimer       = 0.0f;
    m_ChargeTimer       = 0.0f;
    m_RegenShieldTimer  = 0.0f;

    // Misc
    m_ChargeSpeed    = 0.0f;
    m_ChargeLineStart.Zero();
    m_ChargeLineEnd.Zero();

    // Reset health trackers
    m_BubbleHealth      = -1;
    m_StageRageHealth   = -1;
    m_CanisterHealth    = -1;
    m_iDestCanister     = -1;
    m_GroundYPos        = 0.0f;
    m_ContagionHealth   = -1.0f;
    
    m_PerchJumped       = FALSE;
    m_GrateJumped       = FALSE;
    
    // Reset current stage
    m_CurrentStage = 0;

    // Setup type
    m_Type = TYPE_A;
}

//=========================================================================

mutanttank_attack_state::~mutanttank_attack_state ( void )
{
}

//=========================================================================

void mutanttank_attack_state::LoadTweaks( void )
{
    s32 i;
    
    // Lookup tank reference
    mutant_tank& Tank = *(mutant_tank*)&m_CharacterBase;
    
    // Construct prefix in the form:  THETA_C_
    xstring Prefix;
    Prefix.Format( "THETA_%s_", s_TypeList.GetString( m_Type ) );

    // Lookup stage count
    m_nStages = GetTweakS32( xfs( "%s%s", (const char*)Prefix, "StageCount" ), -1 );
    ASSERTS( m_nStages != -1, xfs( "THETA_%s tweaks are missing - make sure you have pulled from perforce!", s_TypeList.GetString( m_Type ) ) );
    ASSERTS( m_nStages <= MAX_STAGE_COUNT, "See SteveB so more stages can be added to theta" );

    // Lookup health and reset if tweaks have been updated
    f32 InitHealth = GetTweakF32( xfs( "%s%s", (const char*)Prefix, "Health" ), m_CharacterBase.GetMaxHealth() );
    if( InitHealth != m_CharacterBase.GetMaxHealth() )
    {
        m_CharacterBase.ResetHealth( InitHealth );
        m_CharacterBase.SetMaxHealth( InitHealth );
    }
    
    // Set spore health
    f32 SporeInitHealth = GetTweakF32( "THETA_BubbleHealth", 200.0f );
    for( i = 0; i < mutant_tank::SPORE_COUNT; i++ )
    {
        // Lookup spore (if there is one) and reset health
        super_destructible_obj* pSpore = Tank.GetSporeObject( i );
        if( pSpore )
        {
            // Only reset if tweaks have been updated...
            if( pSpore->GetMaxHealth() != SporeInitHealth )
                pSpore->SetInitHealth( SporeInitHealth );            
        }            
    }
    
    // Now load the stages
    f32 Health = InitHealth;
    for( i = 0; i < m_nStages ; i++ )
    {
        // Lookup stage
        stage& Stage = m_Stages[ i ];
        
        // Construct prefix in the form:  THETA_C_Stage1_???
        Prefix.Format( "THETA_%s_Stage%d_", s_TypeList.GetString( m_Type ), i );

        // Load tweaks for stage    
        Stage.LoadTweaks( Prefix );
        
        // Convert stage health percentage into health value before next stage
        f32 StageHealth = InitHealth * Stage.m_Health / 100.0f;
        Health -= StageHealth;
        Stage.m_Health = Health;
        
        // Convert bubble health percentage into health
        if( Stage.m_BubbleHealthInterval != -1 )
            Stage.m_BubbleHealthInterval = InitHealth * Stage.m_BubbleHealthInterval / 100.0f;
        
        // Convert canister health percentage into health
        if( Stage.m_CanisterHealthInterval != -1 )
            Stage.m_CanisterHealthInterval = InitHealth * Stage.m_CanisterHealthInterval / 100.0f;
        
        // Convert contagion health percentage into health
        if( Stage.m_ContagionHealthInterval != -1 )
            Stage.m_ContagionHealthInterval = InitHealth * Stage.m_ContagionHealthInterval / 100.0f;
    }
    
    // Load loco tweaks
    mutant_tank_loco* pLoco = (mutant_tank_loco*)Tank.GetLocoPointer();
    if( pLoco )
        pLoco->LoadTweaks();
}

//=========================================================================

void mutanttank_attack_state::OnEnter( void )
{
    // Call base class
    character_attack_state::OnEnter();
    
    ASSERTS( m_CharacterBase.GetRootWhenIdle() == FALSE, "Set \"Character\\Root to position and idle\" to FALSE!" );

    // Reset the stage rage health to -1 so that the theta performs a rage animation    
    m_StageRageHealth = -1;
}

//=========================================================================

vector3 GetClosestPointOnInfiniteLine( const vector3& P, const vector3& A, const vector3& B )
{
    FORCE_ALIGNED_16( &P );
    FORCE_ALIGNED_16( &A );
    FORCE_ALIGNED_16( &B );

    // Get direction vector and length squared of line
    vector3 AB = B - A;
    f32     L  = AB.LengthSquared();

    // Is line a point?
    if ( L < 0.0000001f )
        return P;

    // Get vector from start of line to point
    vector3 AP = P - A;

    // Calculate: Cos(theta)*|AP|*|AB|
    f32 Dot = AP.Dot(AB);

    // Let distance along AB = X = (|AP| dot |AB|) / |AB|
    // Ratio T is X / |AB| = (|AP| dot |AB|) / (|AB| * |AB|)
    f32 T = Dot / L;

    // Interpolate between A and B
    //  T = 0, Point = A
    //  T = 1, Point = B
    return A + ( T * ( B - A ) );
}

//=========================================================================

xbool mutanttank_attack_state::OnPain( const pain& Pain )
{
    (void)Pain;
    if( IsCurrentPhaseOverride() )
    {
        return FALSE;
    }
    else
    {
        return character_attack_state::OnPain(Pain);
    }
}

//=========================================================================

xbool mutanttank_attack_state::IgnorePain( const pain& Pain )
{
    (void)Pain;
    if( IsCurrentPhaseOverride() )
    {
        return TRUE;
    }
    else
    {
        return character_attack_state::IgnorePain(Pain);
    }
}

//=========================================================================

xbool mutanttank_attack_state::IsCurrentPhaseOverride( )
{
    switch (m_CurrentPhase)
    {
    case PHASE_MUTANTTANK_ATTACK_CANISTER_TO:
    case PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE:
    case PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH:
    case PHASE_MUTANTTANK_ATTACK_CANISTER_FROM:
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT:
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT:
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH:
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH:
    case PHASE_MUTANTTANK_ATTACK_PERCH_ROAR:
    case PHASE_MUTANTTANK_ATTACK_FROM_PERCH:
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT:
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT:
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE:
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE:
    case PHASE_MUTANTTANK_ATTACK_GRATE_SMASH:
    case PHASE_MUTANTTANK_ATTACK_FROM_GRATE:
        return TRUE;
        break;
    default:
        return FALSE;
        break;
    }
}

//=========================================================================

s32 mutanttank_attack_state::UpdatePhase( f32 DeltaTime )
{
#ifndef X_RETAIL
    // Re-load tweaks
    LoadTweaks();
#endif

    // Lookup tank reference
    mutant_tank& Tank = *(mutant_tank*)&m_CharacterBase;
    
    #ifndef X_RETAIL
        // Use to test in debugger
        if( MUTANT_TANK_TEST_STAGE != -1 )
        {
            m_CurrentStage = MUTANT_TANK_TEST_STAGE;
            P_MUTANT_TANK_TEST_STAGE = &m_Stages[ m_CurrentStage ];
        }                
    #endif
    
    // Lookup stage
    ASSERT( m_CurrentStage >= 0 );
    ASSERT( m_CurrentStage < m_nStages );
    stage& Stage = m_Stages[ m_CurrentStage ];

    // Update chase timer
    if(     ( m_CurrentPhase == PHASE_MUTANTTANK_ATTACK_CHASE ) 
         || ( m_CurrentPhase == PHASE_MUTANTTANK_ATTACK_PACE ) )
    {
        m_ChaseTimer += DeltaTime;
    }
    else
    {        
        m_ChaseTimer = 0.0f;
    }
    
    // Update timers
    m_EvadeTimer        += DeltaTime;
    m_SinceLastEvade    += DeltaTime;
    m_LeapTimer         += DeltaTime;
    m_MeleeTimer        += DeltaTime;
    m_RangedTimer       += DeltaTime;
    m_ChargeTimer       += DeltaTime;
    m_RegenShieldTimer  += DeltaTime;

    // Clear new phase
    s32 newPhase = PHASE_NONE;    

    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = ChooseNextPhase();
        break;
    case PHASE_MUTANTTANK_ATTACK_SURPRISED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_STAGE_RAGE:            
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;        
    
    case PHASE_MUTANTTANK_ATTACK_CHASE:   
        // Evade or chase?
        if( m_bWantToEvade )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE_EVADE;
        }            
        else
        {        
            newPhase = ChooseNextPhase();
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_CHASE_EVADE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE;
        }            
        break;
    
    case PHASE_MUTANTTANK_ATTACK_CHARGE_ALIGN:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = PHASE_MUTANTTANK_ATTACK_CHARGE;
            }
            else
            {
                newPhase = ChooseNextPhase();
            }
        }
        break;
    
    case PHASE_MUTANTTANK_ATTACK_MELEE_ALIGN:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
            {
                newPhase = PHASE_MUTANTTANK_ATTACK_MELEE;
            }
            else
            {
                newPhase = ChooseNextPhase();
            }
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CHARGE:
        
        // Increase the speed        
        if( Stage.m_ChargeToMaxSpeedTime > 0 )
        {
            // Increase over time
            m_ChargeSpeed += DeltaTime * Stage.m_ChargeMaxSpeed / Stage.m_ChargeToMaxSpeedTime;
            if( m_ChargeSpeed > Stage.m_ChargeMaxSpeed )
                m_ChargeSpeed = Stage.m_ChargeMaxSpeed;
        }            
        else
        {        
            // Go straight to top speed
            m_ChargeSpeed = Stage.m_ChargeMaxSpeed;
        }

        // Gone past player?
        if( !m_CharacterBase.IsFacingTarget( GetTweakRadian( "THETA_ChargeExitAngle", R_90 ) ) )
        {
            // We missed the player - so play missed anim
            newPhase = PHASE_MUTANTTANK_ATTACK_CHARGE_MISS;
        }
        // Close enough to swing?
        else if ( distToTargetSqrd < x_sqr( GetTweakF32( "THETA_MeleeRange", 300.0f ) ) )
        {
            // Go ahead and swing...
            newPhase = PHASE_MUTANTTANK_ATTACK_CHARGE_SWING;
        }
        // At end of charge?        
        else if(        ( m_CharacterBase.GetGoalCompleted() )
                    ||  (!m_CharacterBase.GetCanMoveForward() ) ) 
        {
            // End of charge - go back to attacking
            newPhase = ChooseNextPhase();
        }
        // Update target location when player is infront of theta        
        else if(        ( m_CharacterBase.IsFacingTarget( GetTweakRadian( "THETA_ChargeUpdateAngle", R_15 ) ) )
                    &&  ( m_CharacterBase.CanPathToTarget() ) )
        {
            // Keep going for the player - the move style delta yaw limit keeps him goinf almost straight
            m_ChargeLineStart = m_CharacterBase.GetPosition();
            m_ChargeLineEnd   = m_CharacterBase.GetTargetPosWithOffset( m_CharacterBase.GetTargetGuid() );
            m_CharacterBase.UpdateGoalLocation( m_ChargeLineEnd );
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_CHARGE_SWING:
    case PHASE_MUTANTTANK_ATTACK_CHARGE_MISS:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_ChargeTimer = 0.0f;
            if(     ( Stage.m_MeleeInterval >= 0.0f )
                &&  ( m_MeleeTimer >= Stage.m_MeleeInterval ) 
                &&  ( distToTargetSqrd < x_sqr( GetTweakF32( "THETA_MeleeRange", 300.0f ) ) ) )
            {
                //are we close enough for melee
                newPhase = PHASE_MUTANTTANK_ATTACK_MELEE;
            }
            else
            {            
                newPhase = ChooseNextPhase();
            }
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_MELEE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_MeleeTimer = 0.0f;
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_LEAP_ALIGN:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_LEAP;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_LEAP:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_LeapTimer = 0.0f;
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_RangedTimer = 0.0f;
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_STEP_BACK:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_PACE:
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            if( m_CharacterBase.CanPathToTarget() ) // Can attach player?
            {            
                newPhase = ChooseNextPhase();
            }
            else
            {
                newPhase = PHASE_MUTANTTANK_ATTACK_PACE;
            }
        }
        else if(    ( Stage.m_RangedInterval >= 0.0f )          // Can fire at player?
                 && ( m_RangedTimer >= Stage.m_RangedInterval )
                 && ( m_CharacterBase.GetHasClearLOSEyes() ) )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN;
        }
        else if(    ( m_CharacterBase.GetHasClearLOSEyes() )    // Can attack player?
                 && ( m_CharacterBase.CanPathToTarget() ) )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE;
        }
        else if( m_TimeInPhase >= MUTANT_TANK_MAX_PACE_TIME ) // Unable to get to target?
        {
            // Choose new target
            newPhase = PHASE_MUTANTTANK_ATTACK_PACE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_BORED:
        if( m_CharacterBase.GetTargetGuid() && m_CharacterBase.IsTargetEnemy() && m_CharacterBase.CanSeeTarget())
        {
            //resume attack
            newPhase = ChooseNextPhase();
        }           
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            //leave attack
            m_bSwitchOutOfAttack = TRUE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            //resume attack
            newPhase = PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK;
        }           
        break;
    case PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN:
        // If all spores have been killed, go back to attack
        if( Tank.GetAllSporesHealth() == 0 )
        {
            newPhase = ChooseNextPhase();
        }            
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_BUBBLE;
        }           
        break;
    case PHASE_MUTANTTANK_ATTACK_BUBBLE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }           
        break;

    case PHASE_MUTANTTANK_ATTACK_CANISTER_TO:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CANISTER_FROM;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_FROM:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_GRATE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_GRATE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_GRATE_SMASH;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_GRATE_SMASH:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_FROM_GRATE;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_FROM_GRATE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_PERCH;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_JUMP_PERCH;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_PERCH_ROAR;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_PERCH_ROAR:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_FROM_PERCH;
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_FROM_PERCH:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_REGEN_SHIELD:        
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_RegenShieldTimer = 0.0f;  // Reset time before next regen
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE;
        }
        break;

    case PHASE_MUTANTTANK_ATTACK_CONTAGION:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_MUTANTTANK_ATTACK_CHASE;
        }
        break;

    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    }

    // if we leave the connections go to base state, unless we are doing an override phase
    if( !IsCurrentPhaseOverride() )
    {    
        s32 basePhase = character_state::UpdatePhase(DeltaTime);
        if( basePhase != PHASE_NONE )
        {
            newPhase = basePhase;
        }
    }

    return newPhase;
}

//=========================================================================

s32 mutanttank_attack_state::ChooseNextPhase()
{
    // Lookup tank reference
    mutant_tank& Tank = *(mutant_tank*)&m_CharacterBase;
 
    // Lookup stage
    ASSERT( m_CurrentStage >= 0 );
    ASSERT( m_CurrentStage < m_nStages );
    stage& Stage = m_Stages[ m_CurrentStage ];
    
    // Lookup useful info
    f32 distToTargetSqrd = Tank.GetToTarget().LengthSquared();
    f32 Health           = Tank.GetHealth();
    f32 SporesHealth     = Tank.GetAllSporesHealth();
    
    // Compute if theta can charge
    xbool bCanCharge =    ( m_ChaseTimer >= Stage.m_ChaseMinTime )
                       && ( Stage.m_ChargeInterval >= 0.0f )
                       && ( m_ChargeTimer >= Stage.m_ChargeInterval )
                       && ( distToTargetSqrd > x_sqr( Stage.m_ChargeMinDist ) )
                       && ( distToTargetSqrd < x_sqr( Stage.m_ChargeMaxDist ) )
                       && ( m_CharacterBase.GetLocoPointer()->m_Player.IsBlending() == FALSE )
                       && ( m_CharacterBase.GetLocoPointer()->GetMotion() == loco::MOTION_FRONT )
                       && ( m_CharacterBase.CanPathToTarget() )     
                       && ( m_CharacterBase.GetHasClearLOSEyes() )  
                       && ( m_CharacterBase.GetCanMoveForward() )   
                       && ( m_CharacterBase.IsFacingTarget( GetTweakRadian( "THETA_ChargeStartAngle", R_15 ) ) );

    // Compute if theta can leap        
    xbool bCanLeap =    ( m_ChaseTimer >= Stage.m_ChaseMinTime )
                     && ( Stage.m_LeapInterval >= 0.0f )
                     && ( m_LeapTimer >= Stage.m_LeapInterval )
                     && ( distToTargetSqrd > x_sqr( Stage.m_LeapMinDist ) )
                     && ( distToTargetSqrd < x_sqr( Stage.m_LeapMaxDist ) )
                     && ( m_CharacterBase.GetLocoPointer()->m_Player.IsBlending() == FALSE )
                     && ( m_CharacterBase.CanSeeTarget() )
                     && ( m_CharacterBase.CanPathToTarget() )
                     && ( m_CharacterBase.IsFacingTarget( GetTweakRadian( "THETA_LeapStartAngle", R_15 ) ) );
    
    // Compute if theta can perform a range attack
    xbool bCanRangeAttack =    ( m_ChaseTimer >= Stage.m_ChaseMinTime )
                            && ( Stage.m_RangedInterval >= 0.0f )
                            && ( m_RangedTimer >= Stage.m_RangedInterval )
                            && ( distToTargetSqrd > x_sqr( Stage.m_RangedMinDist ) )
                            && ( distToTargetSqrd < x_sqr( Stage.m_RangedMaxDist ) ) 
                            && ( m_CharacterBase.GetHasClearLOSEyes() );
    
    // Can theta charge and leap?
    if( ( bCanCharge ) && ( bCanLeap ) )
    {
        // Randomly choose between leap and charge instead of being biased by code order
        s32 R = x_irand( 0, 100 );
        if ( R > 50 )
            bCanCharge = FALSE;
        else
            bCanLeap = FALSE;
    }              

    xbool bCanPerchJump = ( m_CurrentPhase == PHASE_MUTANTTANK_ATTACK_CHASE ) &&
        ( Stage.m_JumpToPerch != -1 ) &&
        ( !m_PerchJumped );
    
    xbool bCanGrateJump = ( m_CurrentPhase == PHASE_MUTANTTANK_ATTACK_CHASE ) &&
        ( Stage.m_JumpToGrate != -1 ) &&
        ( !m_GrateJumped );

    // Can theta jump to a canister?
    s32 iDestCanister = Tank.GetClosestCanister( Tank.GetPosition(), 0.0f, x_sqr( 50*100.0f ) );
    xbool bCanCanister = ( m_CurrentPhase == PHASE_MUTANTTANK_ATTACK_CHASE ) &&
                         ( iDestCanister != -1 ) &&
                         ( m_CanisterHealth <= 0 ) &&
                         ( Stage.m_CanisterHealthInterval != -1 );
    
    // Time for the theta to regenerate the parasite shield?
    f32 ShieldPercent     = Tank.GetParasiteShieldPercent();
    xbool bCanRegenShield =     ( m_RegenShieldTimer >= Stage.m_ParasiteShieldRegenInterval )
                             && ( ShieldPercent >= 0.0f ) 
                             && ( ShieldPercent < 25.0f );
    
    // Time for a contagion attack?
    xbool bCanContagion = ( ShieldPercent > 0.0f ) && 
                          ( Stage.m_ContagionHealthInterval != -1 ) &&
                          ( m_ContagionHealth <= 0.0f );
        
    // Perform canister attack?
    if( bCanCanister )
    {
        // Record canister to jump to and update health
        m_iDestCanister = iDestCanister;
        m_CanisterHealth += Stage.m_CanisterHealthInterval;

        return PHASE_MUTANTTANK_ATTACK_CANISTER_TO;             
    }
    // Jump to Grate?
    else if( bCanGrateJump )
    {
        // only one question, do I jump straight there or jump to the intermediate point?
        if( Tank.GetWithinGrateHoppointRadius() )
        {
            return PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE;             
        }
        else
        {        
            return PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT;             
        }
    }
    // Jump to Perch?
    else if( bCanPerchJump )
    {
        // only one question, do I jump straight there or jump to the intermediate point?
        if( Tank.GetWithinPerchHoppointRadius() )
        {
            return PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH;             
        }
        else
        {        
            return PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT;             
        }
    }
    // Perform bubble attack?
    else if( ( Stage.m_BubbleHealthInterval != -1 ) && 
             ( m_BubbleHealth <= 0.0f ) &&
             ( SporesHealth > 0.0f ) )
    {
        // Update health
        m_BubbleHealth += Stage.m_BubbleHealthInterval;
    
        // If almost facing the player, go straight to bubble attach, otherwise, align before
        if( m_CharacterBase.IsFacingTarget( R_15 ) )
        {
            return PHASE_MUTANTTANK_ATTACK_BUBBLE;
        }            
        else
        {
            return PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN;
        }            
    }
    // Goto next stage?
    else if (       ( m_StageRageHealth == -1 )  // Just entered arena?
                     ||  (       ( Health <= m_StageRageHealth ) // Next stage, but not the last
                             &&  ( m_CurrentStage < ( m_nStages - 1 ) ) ) )
    {
        // If not just entered the arena, goto the next stage
        if( m_StageRageHealth != -1 )
            m_CurrentStage++;
            
        // Setup health before next stage
        ASSERT( m_CurrentStage >= 0 );
        ASSERT( m_CurrentStage < m_nStages );
        const stage& NextStage = m_Stages[ m_CurrentStage ];
        
        // Reset stage healths
        m_StageRageHealth  = NextStage.m_Health;
        
        if( m_BubbleHealth == -1 )
            m_BubbleHealth = NextStage.m_BubbleHealthInterval;
        
        if( m_CanisterHealth == -1 )
            m_CanisterHealth   = NextStage.m_CanisterHealthInterval;
        
        if( m_ContagionHealth == -1 )    
            m_ContagionHealth  = NextStage.m_ContagionHealthInterval;

        m_RegenShieldTimer = 0.0f;

        return PHASE_MUTANTTANK_STAGE_RAGE;            
    }
    // Regenerate parasite shield?
    else if( bCanRegenShield )
    {
        return PHASE_MUTANTTANK_ATTACK_REGEN_SHIELD;            
    }
    else if ( !m_CharacterBase.CanPathToTarget() )
    {
        if( m_CharacterBase.GetHasClearLOSEyes() )
        {
            return PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN;            
        }
        else
        { 
            return PHASE_MUTANTTANK_ATTACK_PACE;
        }
    }
    // Perform contagion attack?
    else if( bCanContagion )
    {
        m_ContagionHealth += Stage.m_ContagionHealthInterval;
        return PHASE_MUTANTTANK_ATTACK_CONTAGION;
    }
    // Melee?
    else if( distToTargetSqrd < x_sqr( GetTweakF32( "THETA_MeleeRange", 300.0f ) ) )
    {
        radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
        radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
        radian m_DiffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);
        radian m_DiffToTargetYaw = x_abs(m_DiffToTargetYawRelative);
        //are we close enough for melee
        if( m_DiffToTargetYaw > GetTweakRadian( "THETA_MeleeMinAngle", R_45 ) )
        {            
            return PHASE_MUTANTTANK_ATTACK_MELEE_ALIGN;
        }
        else
        {
            return PHASE_MUTANTTANK_ATTACK_MELEE;
        }
    }
    else if( bCanCharge )
    {
        return PHASE_MUTANTTANK_ATTACK_CHARGE;
    }
    else if( bCanLeap )
    {
        // If almost facing the player, go straight to leap, otherwise, align before
        if( m_CharacterBase.IsFacingTarget( R_5 ) )
        {
            return PHASE_MUTANTTANK_ATTACK_LEAP_ALIGN;
        }
        else
        {            
            return PHASE_MUTANTTANK_ATTACK_LEAP;
        }            
    }
    else if( m_CharacterBase.GetGoalCompleted() && !m_CharacterBase.GetGoalSucceeded() )
    {
        return PHASE_MUTANTTANK_ATTACK_PACE;
    }
    // Perform a ranged attack?
    else if( bCanRangeAttack )
    {
        // If almost facing the player, go straight to firing, otherwise, align before
        if( m_CharacterBase.IsFacingTarget( R_5 ) )
        {
            return PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK;
        }
        else            
        {
            return PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN;
        }            
    }
    // If lost target and not performing a stage rage, then be bored...
    else if(    ( m_CurrentPhase != PHASE_MUTANTTANK_ATTACK_BORED )
             && ( m_CurrentPhase != PHASE_MUTANTTANK_STAGE_RAGE )
             && ( m_CurrentPhase != PHASE_MUTANTTANK_ATTACK_BUBBLE )
             && ( m_CharacterBase.GetAwarenessLevel() < character::AWARENESS_ACQUIRING_TARGET ) )
    {
        return PHASE_MUTANTTANK_ATTACK_BORED;
    }
    else if ( m_CurrentPhase != PHASE_MUTANTTANK_ATTACK_CHASE )
    {    
        return PHASE_MUTANTTANK_ATTACK_CHASE;
    }
    
    return PHASE_NONE;
}

//=========================================================================

void mutanttank_attack_state::ChangePhase( s32 newPhase )
{
    // Lookup tank reference
    mutant_tank& Tank = *(mutant_tank*)&m_CharacterBase;

    // Lookup stage
    ASSERT( m_CurrentStage >= 0 );
    ASSERT( m_CurrentStage < m_nStages );
    const stage& Stage = m_Stages[ m_CurrentStage ];

    // Choose new phase
    switch( newPhase ) 
    {
    case PHASE_MUTANTTANK_ATTACK_SURPRISED:
        Tank.SetPlayAnimationGoal( loco::ANIM_SPOT_TARGET, 0.2f );
        break;
    case PHASE_MUTANTTANK_ATTACK_BORED:
        Tank.SetPlayAnimationGoal( loco::ANIM_LOST_TARGET, 0.2f );        
        break;
    case PHASE_MUTANTTANK_STAGE_RAGE:
        // Set player as target so that character state machine doesn't go to idle state
        Tank.SetTargetGuid( SMP_UTIL_GetActivePlayerGuid(), FALSE );
        Tank.SetAwarenessLevel( character::AWARENESS_ACQUIRING_TARGET );
        
        // Generate parasite shield?
        if( Stage.m_ParasiteShieldRegenInterval != -1 )
            Tank.SetPlayAnimationGoal( loco::ANIM_SHIELD_ON, 0.2f );        
        else            
            Tank.SetPlayAnimationGoal( (loco::anim_type)( loco::ANIM_STAGE0_RAGE + m_CurrentStage ), 0.2f );        
        break;
    
    case PHASE_MUTANTTANK_ATTACK_CHASE:
        // Turn on the aimer   
        ASSERT( Tank.GetLocoPointer() ); 
        Tank.GetLocoPointer()->SetAimerWeight( 1.0f, 0.5f );
        
        // Goto player
        Tank.SetGotoTargetGoal(Tank.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_NULL, GetTweakF32( "THETA_MeleeRange", 300.0f ) );
        m_GroundYPos = Tank.GetPosition().GetY();
        break;
    case PHASE_MUTANTTANK_ATTACK_CHASE_EVADE:
        {    
            // Reset evade flags and timer
            m_bWantToEvade = FALSE;
            m_EvadeTimer   = 0.0f;
            
            // Evade left or right?
            if( x_irand( 0, 100 ) > 50 )
                Tank.SetPlayAnimationGoal( loco::ANIM_EVADE_LEFT, 0.2f );
            else            
                Tank.SetPlayAnimationGoal( loco::ANIM_EVADE_RIGHT, 0.2f );
        }            
        break;
    
    case PHASE_MUTANTTANK_ATTACK_CHARGE_ALIGN:
        Tank.SetTurnToTargetGoal( Tank.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), 5.0f, TRUE );
    	break;
    case PHASE_MUTANTTANK_ATTACK_CHARGE:
        m_ChargeLineStart = Tank.GetPosition();
        m_ChargeLineEnd   = Tank.GetTargetPosWithOffset( Tank.GetTargetGuid() );
        Tank.SetGotoLocationGoal( m_ChargeLineEnd, loco::MOVE_STYLE_CHARGE, 5.0f );
        m_ChargeSpeed = 0.0f;
    	break;
    case PHASE_MUTANTTANK_ATTACK_CHARGE_SWING:
        Tank.SetPlayAnimationGoal( loco::ANIM_ATTACK_CHARGE_SWING, 0.2f );
        break;
    case PHASE_MUTANTTANK_ATTACK_CHARGE_MISS:
        Tank.SetPlayAnimationGoal( loco::ANIM_ATTACK_CHARGE_MISS, 0.2f );
        break;
    
    case PHASE_MUTANTTANK_ATTACK_MELEE_ALIGN:
        Tank.SetTurnToTargetGoal( Tank.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), 10.0f );
        break;
    case PHASE_MUTANTTANK_ATTACK_MELEE:
        {
            radian toTargetYaw = Tank.GetToTarget().GetYaw();
            radian ourYaw = Tank.GetLocoPointer()->GetYaw();
            Tank.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            Tank.SetPlayAnimationGoal( loco::ANIM_MELEE_SHORT, 0.2f );
        }
        break;    
    case PHASE_MUTANTTANK_ATTACK_LEAP_ALIGN:
        Tank.SetTurnToTargetGoal( Tank.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), 5.0f );
        break;
    case PHASE_MUTANTTANK_ATTACK_LEAP:
        Tank.SetScaledPlayAnimationGoal( loco::ANIM_MELEE_LEAP, 0.2f );
        break;    
    case PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK:
        Tank.SetPlayAnimationGoal( loco::ANIM_ATTACK_RANGED_ATTACK, 0.2f );
    	break;
    case PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN:
        Tank.SetTurnToTargetGoal( Tank.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), 10.0f );
    	break;   
    case PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN:
        Tank.SetTurnToTargetGoal( Tank.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), 20.0f );
        break;
    case PHASE_MUTANTTANK_ATTACK_BUBBLE:
        Tank.SetPlayAnimationGoal( loco::ANIM_ATTACK_BUBBLE, 0.2f );
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT:
        {
            radian toTargetYaw = Tank.GetToTarget( Tank.GetPerchHoppoint() ).GetYaw();
            radian ourYaw = Tank.GetLocoPointer()->GetYaw();
            Tank.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_CROUCH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER  );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT:
        {
            // Lookup destination hop point
            object* pHopPointMarker = g_ObjMgr.GetObjectByGuid( Tank.GetPerchHoppoint() );
            ASSERT( pHopPointMarker );

            // Jump to hop point
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_JUMP, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_THETA_JUMP, pHopPointMarker->GetPosition(), pHopPointMarker->GetL2W().GetRotation().Yaw , TRUE, FALSE );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH:
        {
            radian toTargetYaw = Tank.GetToTarget( Tank.GetPerchGuid() ).GetYaw();
            radian ourYaw = Tank.GetLocoPointer()->GetYaw();
            Tank.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_CROUCH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER   );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_PERCH:
        {
            // Lookup destination hop point
            object* pHopPointMarker = g_ObjMgr.GetObjectByGuid( Tank.GetPerchGuid() );
            ASSERT( pHopPointMarker );

            // Jump to hop point
            Tank.SetPlayAnimationGoal( loco::ANIM_PERCH_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_PERCH_TO, pHopPointMarker->GetPosition(), pHopPointMarker->GetL2W().GetRotation().Yaw , TRUE, TRUE );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT:
        {
            radian toTargetYaw = Tank.GetToTarget( Tank.GetGrateHoppoint() ).GetYaw();
            radian ourYaw = Tank.GetLocoPointer()->GetYaw();
            Tank.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_CROUCH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER   );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT:
        {
            // Lookup destination hop point
            object* pHopPointMarker = g_ObjMgr.GetObjectByGuid( Tank.GetGrateHoppoint() );
            ASSERT( pHopPointMarker );

            // Jump to hop point
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_JUMP, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_THETA_JUMP, pHopPointMarker->GetPosition(), pHopPointMarker->GetL2W().GetRotation().Yaw , TRUE, FALSE);
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE:
        {
            radian toTargetYaw = Tank.GetToTarget( Tank.GetGrateGuid() ).GetYaw();
            radian ourYaw = Tank.GetLocoPointer()->GetYaw();
            Tank.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            Tank.SetPlayAnimationGoal( loco::ANIM_THETA_CROUCH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER   );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_JUMP_GRATE:
        {
            // Lookup destination hop point
            object* pHopPointMarker = g_ObjMgr.GetObjectByGuid( Tank.GetGrateGuid() );
            ASSERT( pHopPointMarker );

            // Jump to hop point
            Tank.SetPlayAnimationGoal( loco::ANIM_GRATE_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_GRATE_TO, pHopPointMarker->GetPosition(), pHopPointMarker->GetL2W().GetRotation().Yaw , TRUE, TRUE );
        }
        break;
/*    case PHASE_MUTANTTANK_ATTACK_HOP_STARGATE_GRATE:
    case PHASE_MUTANTTANK_ATTACK_HOP_STARGATE_PERCH:
        {
            // Lookup destination hop point
            object* pHopPointMarker = g_ObjMgr.GetObjectByGuid( m_CurrentHopPoint );
            ASSERT( pHopPointMarker );

            // Jump to hop point
            Tank.SetPlayAnimationGoal( loco::ANIM_GRATE_FROM, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_GRATE_FROM, pHopPointMarker->GetPosition(), pHopPointMarker->GetL2W().GetRotation().Yaw , TRUE, TRUE );
        }
        break;
    case PHASE_MUTANTTANK_ATTACK_TO_STARGATE_GRATE:
        {
            // Lookup destination hook
            ASSERT( m_iDestGrateHook != -1 );
            mutant_tank::stargate_hooks& GrateHook = Tank.GetGrateHook( m_iDestGrateHook );

            // Lookup hook info
            object* pGrateHookMarker = g_ObjMgr.GetObjectByGuid( GrateHook.m_AttachGuid );
            ASSERT( pGrateHookMarker );

            // Jump to grate hook
            Tank.SetPlayAnimationGoal( loco::ANIM_STARGATEHOOK_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_STARGATEHOOK_TO, pGrateHookMarker->GetPosition(), pGrateHookMarker->GetL2W().GetRotation().Yaw , TRUE );
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_STARGATE_GRATE_IDLE:
    case PHASE_MUTANTTANK_ATTACK_STARGATE_PERCH_IDLE:
        Tank.SetPlayAnimationGoal( loco::ANIM_STARGATEHOOK_IDLE, 0.2f );
        break;
    case PHASE_MUTANTTANK_ATTACK_FROM_STARGATE_TO_GRATE:
        {
            // Lookup grate info
            object* pGrateMarker = g_ObjMgr.GetObjectByGuid( Tank.GetGrateGuid() );
            ASSERT( pGrateMarker );

            // Jump to grate
            Tank.SetPlayAnimationGoal( loco::ANIM_GRATE_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_GRATE_TO, pGrateMarker->GetPosition(), pGrateMarker->GetL2W().GetRotation().Yaw , TRUE );
        }            
        break;*/
    case PHASE_MUTANTTANK_ATTACK_GRATE_SMASH:
        {
            // Lookup grate info
            Tank.SetPlayAnimationGoal( loco::ANIM_GRATE_SMASH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            object *grateScript = g_ObjMgr.GetObjectByGuid( Tank.GetGrateScript() );
            if( grateScript )
            {
                grateScript->OnActivate(TRUE);
            }
        }            
        break;

    case PHASE_MUTANTTANK_ATTACK_FROM_GRATE:
        {
            m_GrateJumped = TRUE;
            // Lookup grate info
            Tank.SetPlayAnimationGoal( loco::ANIM_GRATE_FROM, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
        }            
        break;
    
/*    case PHASE_MUTANTTANK_ATTACK_TO_STARGATE_PERCH:
    case PHASE_MUTANTTANK_ATTACK_TO_STARGATE_PERCH_FOR_DEATH:
        {
            // Lookup destination hook
            ASSERT( m_iDestPerchHook != -1 );
            mutant_tank::stargate_hooks& PerchHook = Tank.GetPerchHook( m_iDestPerchHook );

            // Lookup hook info
            object* pPerchHookMarker = g_ObjMgr.GetObjectByGuid( PerchHook.m_AttachGuid );
            ASSERT( pPerchHookMarker );

            // Jump to Perch hook
            Tank.SetPlayAnimationGoal( loco::ANIM_STARGATEHOOK_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_STARGATEHOOK_TO, pPerchHookMarker->GetPosition(), pPerchHookMarker->GetL2W().GetRotation().Yaw , TRUE );
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_FROM_STARGATE_TO_PERCH_FOR_DEATH:
        m_PerchDeathJumped = TRUE;
    case PHASE_MUTANTTANK_ATTACK_FROM_STARGATE_TO_PERCH:
        {
            // Lookup Perch info
            object* pPerchMarker = g_ObjMgr.GetObjectByGuid( Tank.GetPerchGuid() );
            ASSERT( pPerchMarker );

            // Jump to Perch
            Tank.SetPlayAnimationGoal( loco::ANIM_PERCH_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_PERCH_TO, pPerchMarker->GetPosition(), pPerchMarker->GetL2W().GetRotation().Yaw , TRUE );
        }            
        break;*/
    case PHASE_MUTANTTANK_ATTACK_PERCH_ROAR:
        {
            // Lookup grate info
            Tank.SetPlayAnimationGoal( "ATTACK_HOWL" );
            radian yawDiff = x_MinAngleDiff( m_CharacterBase.GetLocoPointer()->GetYaw(),Tank.GetToTarget().GetYaw() );
            m_CharacterBase.SetAnimYawDelta( -yawDiff );
            object *perchScript = g_ObjMgr.GetObjectByGuid( Tank.GetPerchScript() );
            if( perchScript )
            {
                perchScript->OnActivate(TRUE);
            }
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_FROM_PERCH:
        {
            m_PerchJumped = TRUE;
            
            // Lookup Perch info
            object* pPerchEndMarker = g_ObjMgr.GetObjectByGuid( Tank.GetPerchHoppoint() );
            ASSERT( pPerchEndMarker );
            // Lookup grate info
            Tank.SetPlayAnimationGoal( loco::ANIM_PERCH_FROM, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_PERCH_FROM, pPerchEndMarker->GetPosition(), pPerchEndMarker->GetL2W().GetRotation().Yaw + R_180, TRUE, TRUE );
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_TO:
        {
            // Lookup destination canister
            ASSERT( m_iDestCanister != -1 );
            mutant_tank::canister& Canister = Tank.GetCanister( m_iDestCanister );

            // Lookup canister info
            object* pCenter = g_ObjMgr.GetObjectByGuid( Canister.m_CenterGuid );
            object* pAttach = g_ObjMgr.GetObjectByGuid( Canister.m_AttachGuid );
            ASSERT( pCenter );
            ASSERT( pAttach );
            vector3 Center = pCenter->GetPosition();
            vector3 Attach = pAttach->GetPosition();
            
            // Compute attach radius
            vector3 Offset = Attach - Center;
            f32     Radius = x_sqrt( x_sqr( Offset.GetX() ) + x_sqr( Offset.GetZ() ) );
            
            // Compute jump target
            vector3 Delta  = Tank.GetPosition() - Center;
            Delta.GetY() = 0.0f;
            Delta.NormalizeAndScale( Radius );
            vector3 DestPos = Center + Delta;
            DestPos.GetY() = Attach.GetY();

            // Jump to canister
            Tank.SetPlayAnimationGoal( loco::ANIM_CANISTER_TO, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
            Tank.BeginJump( loco::ANIM_CANISTER_TO, DestPos, Delta.GetYaw() + R_180, TRUE, TRUE );
        }            
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE:
        Tank.SetPlayAnimationGoal( loco::ANIM_CANISTER_IDLE, 
                                   0.2f, 
                                   loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_PLAY_TYPE_TIMED, 
                                   Stage.m_CanisterIdleTime );
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH:
        Tank.SetPlayAnimationGoal( loco::ANIM_CANISTER_SMASH, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
        break;
    case PHASE_MUTANTTANK_ATTACK_CANISTER_FROM:
        {
            // Compute jump end position
            vector3 JumpEndPos = Tank.GetPosition();
            JumpEndPos.GetY() = m_GroundYPos;
            radian  JumpEndYaw = Tank.GetYaw();

            // Jump backwards from facing direction
            vector3 Offset( 0.0f, 0.0f, 400.0f );
            Offset.RotateY( Tank.GetYaw() + R_180 );
            JumpEndPos += Offset;
            
            // Set jump goal
            Tank.SetPlayAnimationGoal( loco::ANIM_CANISTER_FROM, 0.2f );
            Tank.BeginJump( loco::ANIM_CANISTER_FROM, JumpEndPos, JumpEndYaw, FALSE, TRUE );
        }            
        break;
    	
    case PHASE_MUTANTTANK_ATTACK_REGEN_SHIELD:
        Tank.SetPlayAnimationGoal( loco::ANIM_SHIELD_REGEN, 0.2f, loco::ANIM_FLAG_TURN_OFF_AIMER );
        break;
    
    case PHASE_MUTANTTANK_ATTACK_CONTAGION:
        Tank.SetPlayAnimationGoal( loco::ANIM_SHIELD_SHOOT, 0.2f );
        break;
    	
    case PHASE_MUTANTTANK_ATTACK_PACE:
        {
            vector3 locationToTarget = Tank.GetToTarget();            
            if( Tank.CanSeeTarget() )
            {
                // this one paces back and forth, because he can shoot from this basic position
                if( m_LastPaceLeft )
                {
                    m_LastPaceLeft = FALSE;
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(200.0f,500.0f) );
                    locationToTarget.RotateY(R_90);
            
                }
                else
                {
                    m_LastPaceLeft = TRUE;
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(200.0f,500.0f) );
                    locationToTarget.RotateY(R_270);
                }
            }
            else
            {
                //we can't reach our target, so try to get to a good shooting position,
                //if target above try to move away, if target same level or below try to get closer
                if( locationToTarget.GetY() > 200.0f )
                {
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(-R_90,R_90));
                }
                else
                {                
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(R_90,R_270));
                }
            }
            vector3 vDestination = Tank.GetPosition() + locationToTarget;
            nav_connection_slot_id iConn = g_NavMap.GetNearestConnectionInGrid( Tank.GetCurrentConnection(), vDestination);
            Tank.SetGotoLocationGoal( g_NavMap.GetNearestPointOnConnection( iConn, vDestination ) );
        }
    	break;
    	
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid New Phase " );
        }
    }
    character_state::ChangePhase( newPhase );
}

//=========================================================================

character_state::states mutanttank_attack_state::UpdateState( f32 DeltaTime )
{
    // we need to update our cover status
    if( !m_CharacterBase.HasWeaponEquiped() && !m_CharacterBase.CanPathToTarget() )
    {
        m_OurCoverDesire = SEEK_COVER_ALWAYS;        
    }
    else
    {
        m_OurCoverDesire = SEEK_COVER_NEVER;
    }

    (void)DeltaTime;
    if( m_bSwitchOutOfAttack )
    {
        states newState = m_CharacterBase.GetStateFromAwareness();
        if( newState != STATE_ATTACK )
        {        
            return newState;
        }
    }
    return character_attack_state::UpdateState( DeltaTime );
}

//=========================================================================

void mutanttank_attack_state::OnBeingShotAt( object::type Type , guid ShooterID )
{
    // Call base class
    character_attack_state::OnBeingShotAt( Type, ShooterID );

    // In a phase that allows evades?
    if( m_CurrentPhase != PHASE_MUTANTTANK_ATTACK_CHASE )
        return;
        
    // Lookup stage
    ASSERT( m_CurrentStage >= 0 );
    ASSERT( m_CurrentStage < m_nStages );
    stage& Stage = m_Stages[ m_CurrentStage ];

    // Evade disabled?
    if( Stage.m_EvadeInterval < 0 )
        return;
    
    // Has it been long enough since the last evade?    
    if( m_EvadeTimer < Stage.m_EvadeInterval )
        return;
        
    // Facing the player?
    if ( !m_CharacterBase.IsFacingTarget( m_CharacterBase.GetTargetPosWithOffset(), R_30 ) )
        return;
        
    // TO DO: Check for clear area to side of theta?
            
    // Evade!
    m_bWantToEvade = TRUE;
}

//=========================================================================

xbool mutanttank_attack_state::OnExit( void )
{
    return character_attack_state::OnExit();
}


//=========================================================================

const char* mutanttank_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

#define PHASE_NAME_CASE( __phase__ ) case __phase__: return #__phase__

    switch( namedPhase ) 
    {
    PHASE_NAME_CASE( PHASE_MUTANTTANK_STAGE_RAGE                );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHASE              );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHASE_EVADE        );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHARGE_ALIGN       );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHARGE             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHARGE_SWING       );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CHARGE_MISS        );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_MELEE_ALIGN        );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_MELEE              );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_LEAP_ALIGN         );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_LEAP               );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_STEP_BACK          );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_PACE               );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_BORED              );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK      );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN       );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_SURPRISED          );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN       );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_BUBBLE             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CANISTER_TO        );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE      );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH     );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CANISTER_FROM      );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_REGEN_SHIELD       );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CONTAGION          );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_GRATE_SMASH            );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_FROM_GRATE             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_PERCH_ROAR             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_FROM_PERCH             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_JUMP_PERCH             );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH           );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT    );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT  );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT  );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT    );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE           );
    PHASE_NAME_CASE( PHASE_MUTANTTANK_ATTACK_JUMP_GRATE             );
    }
    
    return character_state::GetPhaseName(thePhase);
}

//=========================================================================
// Property functions
//=========================================================================

void mutanttank_attack_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader( "MutantAttackState",             "Mutant attacks state variables", 0 );
    List.PropEnumEnum  ( "MutantAttackState\\Type", s_TypeList.BuildString(), "A = Grunt-Melee-NPC, \nE3 = Excavation 3 Boss, \nAB45 = AlienBase4-5 Theta Boss", 0 );

    // Call base state
    character_attack_state::OnEnumProp( List );
}

//=========================================================================

xbool mutanttank_attack_state::OnProperty( prop_query& I )
{
    // Type?
    if ( SMP_UTIL_IsEnumVar<mutanttank_attack_state::type, mutanttank_attack_state::type>
            ( I, 
              "MutantAttackState\\Type", 
              m_Type, 
              s_TypeList ) )
    {             
        if( I.IsRead() == FALSE )                                                                
        {
            LoadTweaks();
        }
        return TRUE;
    }
    else
    {
        // Call base state
        return character_attack_state::OnProperty( I );
    }
}

//=========================================================================
// Debug functions
//=========================================================================
#ifndef X_RETAIL

void mutanttank_attack_state::OnDebugRender( void )
{
    // Render charge
    if( GetCurrentPhase() == PHASE_MUTANTTANK_ATTACK_CHARGE )
    {
        // Compute charge line pts
        vector3 Offset( 0.0f, 10.0f, 0.0f );
        vector3 Start( m_ChargeLineStart + Offset );
        vector3 End( m_ChargeLineEnd + Offset );
        
        // Render
        draw_ClearL2W();
        draw_Sphere( Start, 15.0f, XCOLOR_GREEN );
        draw_Sphere( End,   15.0f, XCOLOR_GREEN );
        draw_Line( Start, End, XCOLOR_GREEN );
    }
}

#endif // X_RETAIL

//=========================================================================
