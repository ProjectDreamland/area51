#include "Character_Attack_State.hpp"
#include "Character_Cover_State.hpp"
#include "Characters\Character.hpp"
#include "Characters\god.hpp"
#include "navigation\coverNode.hpp"
#include "navigation\alarmNode.hpp"

//=========================================================================
// CONSTs
//=========================================================================

const f32 k_TimeBetweenShouts   = 5.0f;
const f32 k_MinTimeMoving       = 4.0f;
const f32 k_MinTimeInIdle       = 1.0f;
const f32 k_MinTimeBetweenEvades = 4.0f;
const f32 k_MinTimeSeekCoverWhenShotAt = 2.0f;
typedef enum_pair<character_attack_state::eCoverDesire> cover_desire_enum_pair;

//****s_ActionsAllEnumTable **** this table should not use the BuildString() as it exceeds the 255 char limit...
static cover_desire_enum_pair s_SeekCoverEnumTable[] = 
{   
    cover_desire_enum_pair( "Always",               character_attack_state::SEEK_COVER_ALWAYS   ),
    cover_desire_enum_pair( "When Being Shot At",   character_attack_state::SEEK_COVER_SHOTAT   ),
    cover_desire_enum_pair( "When Injured",         character_attack_state::SEEK_COVER_DAMAGED  ),
    cover_desire_enum_pair( "Never",                character_attack_state::SEEK_COVER_NEVER    ),
    cover_desire_enum_pair( k_EnumEndStringConst,   character_attack_state::SEEK_COVER_INVALID  ) //**MUST BE LAST**//
};

enum_table<character_attack_state::eCoverDesire>  character_attack_state::m_SeekCoverEnum ( s_SeekCoverEnumTable ); 

//=========================================================================
// Character ATTACK STATE
//=========================================================================

character_attack_state::character_attack_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_MeleeAttackDelay      = 2.0f;
    m_SinceLastMeleeAttack  = m_MeleeAttackDelay;
    m_SinceLastEvade        = 0.0f;   

    m_OptimalDistance       = 1000.0f;
    m_JumpAttackPercent     = 75;
    m_MoveStyle             = loco::MOVE_STYLE_RUNAIM;
    m_OurCoverDesire        = SEEK_COVER_NEVER;

    m_JumpAttackMinDistance = 0.0f;
    m_JumpAttackMaxDistance = 0.0f;
    m_MinTimeBetweenJumpAttacks = 5.0f;
    m_MaxTimeClosingToAttack = 2.0f;
    m_SinceLastJumpAttack   = m_MinTimeBetweenJumpAttacks;
    m_SinceLastShotAt       = k_MinTimeSeekCoverWhenShotAt;
}

//=========================================================================

character_attack_state::~character_attack_state()
{
}

//=========================================================================

void character_attack_state::OnInit( void )
{    
    character_state::OnInit();
}

//=========================================================================

void character_attack_state::OnEnter( void )
{
    m_ScanTime = g_ObjMgr.GetGameTime();

    m_SinceLastMeleeAttack = m_MeleeAttackDelay;
    m_SinceLastEvade = k_MinTimeBetweenEvades;

    // If the jump attack distances
    // haven't been setup, extract them from the animation
    m_CanMelee = ( m_CharacterBase.HasAnim(loco::ANIM_MELEE_LONG ) || m_CharacterBase.HasAnim(loco::ANIM_MELEE_SHORT ));
    if (m_JumpAttackMinDistance == 0)
    {
        s32 animIndex = m_CharacterBase.GetLocoPointer()->GetAnimIndex(loco::ANIM_MELEE_LEAP);
        if( animIndex >= 0 )
        {        
            vector3 jumpDistance = m_CharacterBase.GetLocoPointer()->GetAnimGroupHandle().GetPointer()->GetAnimInfo(animIndex).GetTotalTranslation();
            jumpDistance.GetY() = 0.0f;

            m_JumpAttackMinDistance = jumpDistance.Length() * x_frand(0.7f,0.9f);
            m_JumpAttackMaxDistance = jumpDistance.Length() * x_frand(1.1f,1.3f);
        }
    }

    character_state::OnEnter();
}

//=========================================================================
xbool character_attack_state::OnExit()
{
    return character_state::OnExit();
}

//=========================================================================

s32 character_attack_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;   
    
    m_SinceLastMeleeAttack  += DeltaTime;
    m_SinceLastEvade        += DeltaTime;
    m_SinceLastJumpAttack   += DeltaTime;
    m_SinceLastShotAt       += DeltaTime;
    if( m_ShotAtThisTick )
    {
        m_SinceLastShotAt = 0.0f;
    }

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_ATTACK_IDLE;
        break;
    case PHASE_ATTACK_SURPRISED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_ATTACK_IDLE;
        }
        break;
    case PHASE_ATTACK_CLOSE_TO_OPTIMAL:
    case PHASE_ATTACK_RETREAT_TO_OPTIMAL:
        if( m_CharacterBase.GetGoalCompleted() || m_TimeInPhase > k_MinTimeMoving )
        {
            newPhase = ChooseNextPhase();
        }
        else if ( m_ShotAtThisTick && m_SinceLastEvade > k_MinTimeBetweenEvades )
        {
            newPhase = ChooseEvade();
        }
        break;
    case PHASE_ATTACK_IDLE:
        // what should we do?
        if( m_TimeInPhase > k_MinTimeInIdle )
        {        
            newPhase = ChooseNextPhase();
        }
        else if ( m_ShotAtThisTick && m_SinceLastEvade > k_MinTimeBetweenEvades )
        {
            newPhase = ChooseEvade();
        }
        break;
    case PHASE_ATTACK_CLOSE_FOR_MELEE:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
            {
                //are we close enough for melee
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));

                if( m_DiffToTargetYaw > k_MinAngleToMeleeFace )
                {            
                    return PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE;
                }
                else
                {
                    return PHASE_ATTACK_SHORT_MELEE;
                }
            }
            else
                newPhase = ChooseNextPhase();
        }
        else if( m_CharacterBase.GetToTarget().LengthSquared() < m_CharacterBase.GetLongMeleeRange() * m_CharacterBase.GetLongMeleeRange() )
        {
            //are we close enough for melee
            radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));

            if( m_DiffToTargetYaw > k_MinAngleToMeleeFace )
            {            
                return PHASE_ATTACK_ALIGN_FOR_LONG_MELEE;
            }
            else
            {
                return PHASE_ATTACK_LONG_MELEE;
            }
        }
        else if ( m_TimeInPhase > m_MaxTimeClosingToAttack )
        {
            newPhase = ChooseNextPhase();
        }
        else if ( m_ShotAtThisTick && m_SinceLastEvade > k_MinTimeBetweenEvades )
        {
            newPhase = ChooseEvade();
        }

        break;
    case PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
                newPhase = PHASE_ATTACK_SHORT_MELEE;
            else
                newPhase = ChooseNextPhase();
        }
        else if ( m_TimeInPhase > k_MinTimeMoving )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_ATTACK_ALIGN_FOR_LONG_MELEE:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
                newPhase = PHASE_ATTACK_LONG_MELEE;
            else
                newPhase = ChooseNextPhase();
        }
        else if ( m_TimeInPhase > k_MinTimeMoving )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_ATTACK_SHORT_MELEE:
    case PHASE_ATTACK_LONG_MELEE:
        if( m_CharacterBase.GetGoalCompleted() )
        {        
            newPhase = PHASE_ATTACK_RETREAT_TO_OPTIMAL;
        }
        break;
    case PHASE_ATTACK_EVADE_LEFT:
    case PHASE_ATTACK_EVADE_RIGHT:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
            {
                //are we close enough for melee
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));

                if( m_DiffToTargetYaw > R_5 )
                {            
                    newPhase = PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK;
                }
                else
                {
                    newPhase = PHASE_ATTACK_JUMP_ATTACK;
                }
            }
            else
                newPhase = ChooseNextPhase();
        }
        //else if ( m_TimeInPhase > k_MinTimeMoving )
        else if ( m_TimeInPhase > m_MaxTimeClosingToAttack )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if ( m_CharacterBase.GetGoalSucceeded() )
                newPhase = PHASE_ATTACK_JUMP_ATTACK;
            else
                newPhase = ChooseNextPhase();
        }
        else if ( m_TimeInPhase > k_MinTimeMoving )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    case PHASE_ATTACK_JUMP_ATTACK:
        // what should we do?
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = ChooseNextPhase();
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }
    return newPhase;
}

//=========================================================================

void character_attack_state::ChangePhase( s32 newPhase )
{
    switch( newPhase ) 
    {
    case PHASE_ATTACK_SURPRISED:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
        break;
    case PHASE_ATTACK_IDLE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetIdleGoal();
    	break;
    case PHASE_ATTACK_CLOSE_TO_OPTIMAL:
        m_CharacterBase.SetWantsToAim(TRUE);
        if( m_CurrentPhase != PHASE_ATTACK_CLOSE_TO_OPTIMAL )
        {        
            m_CharacterBase.PlayDialog( character::DIALOG_RUSH );        
        }
        m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_OptimalDistance);
    	break;
    case PHASE_ATTACK_RETREAT_TO_OPTIMAL:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetRetreatFromTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_OptimalDistance);
    	break;
    case PHASE_ATTACK_EVADE_LEFT:
        m_SinceLastEvade = 0.0f;
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_LEFT);
        break;
    case PHASE_ATTACK_EVADE_RIGHT:
        m_SinceLastEvade = 0.0f;
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_RIGHT);
        break;
    case PHASE_ATTACK_CLOSE_FOR_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_CharacterBase.GetShortMeleeRange() );
        break;
    case PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE:
        m_CharacterBase.SetTurnToTargetGoal(m_CharacterBase.GetTargetGuid());
    	break;
    case PHASE_ATTACK_ALIGN_FOR_LONG_MELEE:
        m_CharacterBase.SetTurnToTargetGoal(m_CharacterBase.GetTargetGuid());
        break;
    case PHASE_ATTACK_SHORT_MELEE:
        {
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
            m_SinceLastMeleeAttack = 0.0f;
        }
    	break;
    case PHASE_ATTACK_LONG_MELEE:
        {
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_LONG);
            m_SinceLastMeleeAttack = 0.0f;
        }
        break;
    case PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_JumpAttackMaxDistance );
    	break;
    case PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK:
        m_CharacterBase.SetTurnToTargetGoal(m_CharacterBase.GetTargetGuid());
    	break;
    case PHASE_ATTACK_JUMP_ATTACK:
        m_SinceLastMeleeAttack = 0.0f;
        m_SinceLastJumpAttack  = 0.0f;      // track this seperately
        m_CharacterBase.SetScaledPlayAnimationGoal(loco::ANIM_MELEE_LEAP);        
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

s32 character_attack_state::ChooseNextPhase( )
{
    if( !m_CharacterBase.GetTargetGuid() ||
        !m_CharacterBase.CanPathToTarget() )
    {
        return PHASE_ATTACK_IDLE;
    }
    else if( m_CharacterBase.CanSeeTarget() )
    {
        // I can see you! 
        return ChooseTargetVisiblePhase();
    }
    else if (m_CharacterBase.IsTargetInSightCone() )
    {
        // target is in our sight cone but not visible... the hidin bastard.
        return ChooseTargetHiddenPhase();
    }
    else
    {
        return PHASE_ATTACK_CLOSE_TO_OPTIMAL;
    }
}
//=========================================================================

s32 character_attack_state::ChooseTargetVisiblePhase( )
{
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();
    f32 optimalDistanceSqr = m_OptimalDistance * m_OptimalDistance;

    // first.. what can we do? this will decide what we choose
    if( m_CharacterBase.WeaponReady() )
    {
        // if we can shoot, then things are a little tougher...

        // first let's see if we want to jump
        if( m_CharacterBase.GetHasClearJumpAttack() && 
            (m_SinceLastJumpAttack >= m_MinTimeBetweenJumpAttacks) &&
            (x_irand(1,100) < m_JumpAttackPercent) &&
            (distToTargetSqr >= m_JumpAttackMinDistance * m_JumpAttackMinDistance) &&
            (distToTargetSqr <= m_JumpAttackMaxDistance * m_JumpAttackMaxDistance) )
        {
            return PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK;
        }

        // shall we close range?
        if( distToTargetSqr > optimalDistanceSqr * 1.44f )
        {
            // at 2x optimal we definately close, otherwise it's a % chance.
            f32 twiceOptimalSqr = optimalDistanceSqr * 4.0f;
            f32 closeCutoff = x_frand( optimalDistanceSqr, twiceOptimalSqr );
            if( closeCutoff > distToTargetSqr )
            {
                return PHASE_ATTACK_CLOSE_TO_OPTIMAL;
            }
        }
    
        // if target is very close we may melee
        if( (distToTargetSqr < (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) * 4.0f) && m_CanMelee )
        {
            f32 meleeAttackRadiusSqr = m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange();
            f32 twiceMeleeAttackRadius = meleeAttackRadiusSqr * 4.0f;
            // first should we melee?
            f32 closeCutoff = x_frand( meleeAttackRadiusSqr, twiceMeleeAttackRadius );
            if( distToTargetSqr < closeCutoff )
            {            
                return PHASE_ATTACK_CLOSE_FOR_MELEE;
            }
        }
    
        // if closer than optimal we may backup.
        if( distToTargetSqr < optimalDistanceSqr * 0.64f )
        {        
            f32 meleeAttackRadiusSqr = m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange();
            // do we want to backup instead?
            f32 closeCutoff = x_frand( meleeAttackRadiusSqr, optimalDistanceSqr );
            if( distToTargetSqr < closeCutoff )
            {
                return PHASE_ATTACK_RETREAT_TO_OPTIMAL;
            }       
        }
    }
    else
    {
        // if we can't shoot, there is really never a good reason to back up.
        // either close for melee, idle, or close to jump attack.
        
        // if we are ready to attack try to do so.
        if( m_SinceLastMeleeAttack >= m_MeleeAttackDelay )
        {
            if( m_CharacterBase.GetHasClearJumpAttack() &&
                m_SinceLastJumpAttack >= m_MinTimeBetweenJumpAttacks &&
                x_irand(1,100) < m_JumpAttackPercent &&
                distToTargetSqr >= m_JumpAttackMinDistance * m_JumpAttackMinDistance && 
                distToTargetSqr <= m_JumpAttackMaxDistance * m_JumpAttackMaxDistance )
            {
                return PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK;
            }   
            else 
            {
                return PHASE_ATTACK_CLOSE_FOR_MELEE;
            }
        }

        // otherwise we aren't ready to attack yet. If further than optimal close, otherwise just hang
        if( distToTargetSqr > optimalDistanceSqr * 1.44f )
        {
            return PHASE_ATTACK_CLOSE_TO_OPTIMAL;
        }
    }

    // if all else fails, do idle.
    return PHASE_ATTACK_IDLE;
}

//=========================================================================
// we can't see our target, what do we do?

s32 character_attack_state::ChooseTargetHiddenPhase()
{
    return PHASE_ATTACK_CLOSE_FOR_MELEE;
}

//=========================================================================

s32 character_attack_state::ChooseEvade( )
{
    if( x_irand(0,1) )
    {            
        if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_ATTACK_EVADE_RIGHT;
        }
        else if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_ATTACK_EVADE_LEFT;
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_ATTACK_EVADE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_ATTACK_EVADE_RIGHT;
        }
    }
    return PHASE_NONE;
}

//=========================================================================

character_state::states character_attack_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;

    // if we have a valid alarm go for it!
    object *alarmObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentAlarm() );
    if( alarmObject && 
        alarmObject->IsKindOf(alarm_node::GetRTTI()) &&
        m_CharacterBase.HasState(STATE_ALARM) )
    {
        return STATE_ALARM;
    }

    // if our sticky cover is valid, we go to it.
    if( m_CharacterBase.GetCurrentCover() && 
        m_CharacterBase.GetStickyCoverNode() &&
        m_CharacterBase.HasState(STATE_COVER) )
    {
        return STATE_COVER;
    }

    // otherwise seek cover when we are told to
    if( m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_ACQUIRING_TARGET && 
        m_CharacterBase.HasState(STATE_COVER) )
    {
        xbool seekCover = FALSE;
        switch( m_OurCoverDesire )
        {
        case SEEK_COVER_ALWAYS:
            seekCover = TRUE;
            break;
        case SEEK_COVER_SHOTAT:
            if( m_SinceLastShotAt <= k_MinTimeSeekCoverWhenShotAt )
            {
                seekCover = TRUE;
            }
            break;
        case SEEK_COVER_DAMAGED:
            if( m_CharacterBase.GetParametricHealth() <= 0.5f )
            {
                seekCover = TRUE;
            }
            break;
        }
        if( seekCover && m_CharacterBase.GetCurrentCover() )
        {
            return STATE_COVER;
        }
    }
    else if( m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_SEARCHING )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return character_state::UpdateState( DeltaTime );
}

//=========================================================================

void character_attack_state::OnThink( void )
{
    // Send an alert to others about our target every so often.    
    if ( g_ObjMgr.GetGameDeltaTime( m_ScanTime ) > k_TimeBetweenShouts && m_CharacterBase.CanSeeTarget() )
    {
//        m_CharacterBase.Shout( m_CharacterBase.GetTargetGuid() ) ;
        m_CharacterBase.SendAlert( alert_package::ALERT_TYPE_NPC_SHOUT );
        m_ScanTime = g_ObjMgr.GetGameTime() ;
    }    
    character_state::OnThink();
}

//=========================================================================

void character_attack_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader  ( "attackState",  "Different variables that effect the way that the character behaves in attack state.", 0 );
    List.PropEnumFloat   ( "attackState\\Optimal Distance", "NPC likes to keep this much distance between himself and target when not attacking.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumInt     ( "attackState\\Jump Attack Percent", "1-100 will we jump attack when given the chance?", PROP_TYPE_EXPOSE ) ;
    List.PropEnumFloat   ( "attackState\\Jump Attack Min Distance", "Minimum distance at which the NPC will jump attack", 0 );
    List.PropEnumFloat   ( "attackState\\Jump Attack Max Distance", "Maximum distance at which the NPC will jump attack", 0 );
    List.PropEnumFloat   ( "attackState\\Jump Attack Delay", "Min time between jump attacks", 0 );
    List.PropEnumFloat   ( "attackState\\Melee Attack Delay", "Min time betweeen swings", PROP_TYPE_EXPOSE ) ;
    List.PropEnumEnum    ( "attackState\\Seek Cover",    character_attack_state::m_SeekCoverEnum.BuildString(),         "Types of Seeking Cover." , PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "attackState\\Attack Rethink Time", "Max time to spend chasing target before rethinking what to do", 0 );
    character_state::OnEnumProp(List);
}

//=========================================================================

xbool character_attack_state::OnProperty ( prop_query& rPropQuery )
{

    if( rPropQuery.VarFloat("attackState\\Optimal Distance", m_OptimalDistance) )
        return TRUE;

    if( rPropQuery.VarInt("attackState\\Jump Attack Percent", m_JumpAttackPercent) )
        return TRUE;

    if( rPropQuery.VarFloat("attackState\\Jump Attack Min Distance", m_JumpAttackMinDistance) )
        return TRUE;

    if( rPropQuery.VarFloat("attackState\\Jump Attack Max Distance", m_JumpAttackMaxDistance) )
        return TRUE;

    if( rPropQuery.VarFloat("attackState\\Jump Attack Delay", m_MinTimeBetweenJumpAttacks) )
        return TRUE;

    if( rPropQuery.VarFloat("attackState\\Attack Rethink Time", m_MaxTimeClosingToAttack) )
        return TRUE;

    if( rPropQuery.VarFloat("attackState\\Melee Attack Delay", m_MeleeAttackDelay ) )
    {
        m_SinceLastMeleeAttack = m_MeleeAttackDelay;
        return TRUE;
    }

    if( rPropQuery.IsVar( "attackState\\Seek Cover")  )
    {
        if( rPropQuery.IsRead() )
        {
            if ( character_attack_state::m_SeekCoverEnum.DoesValueExist( m_OurCoverDesire ) )
            {
                rPropQuery.SetVarEnum( character_attack_state::m_SeekCoverEnum.GetString(m_OurCoverDesire) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            character_attack_state::eCoverDesire CoverDesire;

            if( character_attack_state::m_SeekCoverEnum.GetValue( rPropQuery.GetVarEnum(), CoverDesire ) )
            {
                m_OurCoverDesire = CoverDesire;
            }
        }       
        return( TRUE );
    }            
    return character_state::OnProperty( rPropQuery );
}

//=========================================================================

const char*character_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_ATTACK_SURPRISED:
        return "PHASE_ATTACK_SURPRISED";
        break;
    case PHASE_ATTACK_CLOSE_TO_OPTIMAL:
        return "PHASE_ATTACK_CLOSE_TO_OPTIMAL";
    	break;
    case PHASE_ATTACK_IDLE:
        return "PHASE_ATTACK_IDLE";
    	break;
    case PHASE_ATTACK_CLOSE_FOR_MELEE:
        return "PHASE_ATTACK_CLOSE_FOR_MELEE";
    	break;
    case PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE:
        return "PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE";
    	break;
    case PHASE_ATTACK_ALIGN_FOR_LONG_MELEE:
        return "PHASE_ATTACK_ALIGN_FOR_LONG_MELEE";
        break;
    case PHASE_ATTACK_SHORT_MELEE:
        return "PHASE_ATTACK_SHORT_MELEE";
    	break;
    case PHASE_ATTACK_LONG_MELEE:
        return "PHASE_ATTACK_LONG_MELEE";
        break;
    case PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK:
        return "PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK";
    	break;
    case PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK:
        return "PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK";
    	break;
    case PHASE_ATTACK_JUMP_ATTACK:
        return "PHASE_ATTACK_JUMP_ATTACK";
    	break;
    case PHASE_ATTACK_RETREAT_TO_OPTIMAL:
        return "PHASE_ATTACK_RETREAT_TO_OPTIMAL";
    	break;
    case PHASE_ATTACK_EVADE_LEFT:
        return "PHASE_ATTACK_EVADE_LEFT";
    	break;
    case PHASE_ATTACK_EVADE_RIGHT:
        return "PHASE_ATTACK_EVADE_RIGHT";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
