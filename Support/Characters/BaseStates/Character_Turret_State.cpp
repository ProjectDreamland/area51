#include "Character_Turret_State.hpp"
#include "Character_Cover_State.hpp"
#include "..\Character.hpp"
#include "Navigation\ng_node2.hpp"
#include "navigation\coverNode.hpp"
#include "objects\turret.hpp"
#include "objects\coupler.hpp"

//=========================================================================
// GRAY TURRET STATE
//=========================================================================

character_turret_state::character_turret_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_MoveStyle = loco::MOVE_STYLE_RUN;
}

//=========================================================================

character_turret_state::~character_turret_state()
{
}

//=========================================================================

void character_turret_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

xbool character_turret_state::OnExit( void )
{
    object *turretObject = g_ObjMgr.GetObjectByGuid(m_CurrentTurret );
    if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
    {        
        turret &ourTurret = turret::GetSafeType( *turretObject );
        ourTurret.SetGunner(0);
        object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
        if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
        {
            coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
            ourCoupler.RemoveChild( m_CharacterBase.GetGuid() );
        }
    }
    m_CurrentTurret = 0;
    return TRUE;
}

//=================================================================================================

xbool character_turret_state::OnPain( const pain& Pain )
{
    // get the source of the pain
    guid sourceGuid = Pain.GetOriginGuid();    
    // if enemy
    if( m_CharacterBase.IsEnemyFaction( m_CharacterBase.GetFactionForGuid( sourceGuid ) ) )
    {
        // get our turret and set the source as the target
        object *turretObject = g_ObjMgr.GetObjectByGuid(m_CurrentTurret );
        if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );
            ourTurret.SetTargetGuid(sourceGuid);
        }
    }
    return character_state::OnPain(Pain);
}

//=========================================================================

void character_turret_state::SetCurrentTurret( guid ourTurretGuid )
{
    m_CurrentTurret = ourTurretGuid;
    object *turretObject = g_ObjMgr.GetObjectByGuid(m_CurrentTurret);
    if ( !turretObject || turretObject->IsKindOf( turret::GetRTTI() ) != TRUE )
    {   
        ASSERT(FALSE);
        return;
    }
    turret &ourTurret = turret::GetSafeType( *turretObject );           
    const char* turretAnimGroupName = ourTurret.GetAnimGroupName( m_CharacterBase.GetType() );
    if( turretAnimGroupName == NULL )
    {
        ASSERT(FALSE);
        return;
    }
    m_TurretAnimGroupHandle.SetName( turretAnimGroupName );
}

//=========================================================================

void character_turret_state::OnEnter( void )
{
    SetCurrentTurret(m_CharacterBase.GetStickyTurret());
    character_state::OnEnter();
    m_EndState = FALSE;
}

//=========================================================================

s32 character_turret_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;
    
    object *turretObject = g_ObjMgr.GetObjectByGuid( m_CurrentTurret );

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_TURRET_GOTO_TURRET;
        break;
    case PHASE_TURRET_GOTO_TURRET:
        m_CharacterBase.CheckShooting();
        if ( !m_CurrentTurret || m_CurrentTurret != m_CharacterBase.GetStickyTurret() )
        {
            // remove us from the turret
            if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
            {        
                turret &ourTurret = turret::GetSafeType( *turretObject );
                ourTurret.SetGunner(0);
                object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
                if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
                {
                    coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
                    ourCoupler.RemoveChild( m_CharacterBase.GetGuid() );
                }
            }
            newPhase = PHASE_TURRET_EXIT_TURRET;
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.GetGoalSucceeded() )
            {
                newPhase = PHASE_TURRET_ALIGN_TO_MARKER;
            }
            else
            {
                newPhase = PHASE_TURRET_GOTO_TURRET;
            }
        }
        break;
    case PHASE_TURRET_ALIGN_TO_MARKER:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_TURRET_ENTER_TURRET;
        }
        break;
    case PHASE_TURRET_ENTER_TURRET:
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            // lock us into the turret
            if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
            {        
                turret &ourTurret = turret::GetSafeType( *turretObject );
                ourTurret.SetGunner(m_CharacterBase.GetGuid());
                object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
                if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
                {
                    coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
                    ourCoupler.AddChild( m_CharacterBase.GetGuid() );
                    ourCoupler.SnapAttachPoints( m_CharacterBase.GetGuid() );
                }
            }            
            newPhase = PHASE_TURRET_IDLE;
        }
        break;
    case PHASE_TURRET_IDLE:
        if ( !m_CurrentTurret || m_CurrentTurret != m_CharacterBase.GetStickyTurret() )
        {
            // remove us from the turret
            if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
            {        
                turret &ourTurret = turret::GetSafeType( *turretObject );
                ourTurret.SetGunner(0);
                object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
                if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
                {
                    coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
                    ourCoupler.RemoveChild( m_CharacterBase.GetGuid() );
                }
            }
            newPhase = PHASE_TURRET_EXIT_TURRET;
        }
        else if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );
            if( ourTurret.GetCurrentState() == turret::STATE_FIRING &&
                m_TurretAnimGroupHandle.GetPointer()->GetAnimIndex("TURRET_SHOOT") >= 0 )
            {
                newPhase = PHASE_TURRET_FIRE;
            }
            else if (ourTurret.GetCurrentState() == turret::STATE_RELOADING &&
                m_TurretAnimGroupHandle.GetPointer()->GetAnimIndex("TURRET_RELOAD") >= 0 )
            {
                newPhase = PHASE_TURRET_RELOAD;
            }
        }
        else
        {
            newPhase = PHASE_TURRET_EXIT_TURRET;
        }
        break;
    case PHASE_TURRET_FIRE:
        if ( !m_CurrentTurret || m_CurrentTurret != m_CharacterBase.GetStickyTurret() )
        {
            // remove us from the turret
            if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
            {        
                turret &ourTurret = turret::GetSafeType( *turretObject );
                ourTurret.SetGunner(0);
                object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
                if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
                {
                    coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
                    ourCoupler.RemoveChild( m_CharacterBase.GetGuid() );
                }
            }
            newPhase = PHASE_TURRET_EXIT_TURRET;
        }
        else if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );
            if( ourTurret.GetCurrentState() != turret::STATE_FIRING )
            {
                newPhase = PHASE_TURRET_IDLE;
            }
        }
        break;
    case PHASE_TURRET_RELOAD:
        if ( !m_CurrentTurret || m_CurrentTurret != m_CharacterBase.GetStickyTurret() )
        {
            // remove us from the turret
            if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
            {        
                turret &ourTurret = turret::GetSafeType( *turretObject );
                ourTurret.SetGunner(0);
                object *couplerObject = g_ObjMgr.GetObjectByGuid( ourTurret.GetController() );
                if( couplerObject && couplerObject->IsKindOf( coupler::GetRTTI() ) == TRUE )
                {
                    coupler &ourCoupler = coupler::GetSafeType( *couplerObject );
                    ourCoupler.RemoveChild( m_CharacterBase.GetGuid() );
                }
            }
            newPhase = PHASE_TURRET_EXIT_TURRET;
        }
        else if( turretObject && turretObject->IsKindOf( turret::GetRTTI() ) == TRUE )
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );
            if( ourTurret.GetCurrentState() != turret::STATE_RELOADING )
            {
                newPhase = PHASE_TURRET_IDLE;
            }
        }
        break;
    case PHASE_TURRET_EXIT_TURRET:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_EndState = TRUE;
            m_CurrentTurret = 0;
        }        
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase in Character Turret State" );
        }
    };

    return newPhase;
}

//=========================================================================

void character_turret_state::ChangePhase( s32 newPhase )
{
    object *turretObject = g_ObjMgr.GetObjectByGuid(m_CurrentTurret );
    switch( newPhase ) 
    {
    case PHASE_TURRET_GOTO_TURRET:
        m_CharacterBase.SetWantsToAim(TRUE);
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );           
            m_CharacterBase.SetGotoTargetGoal(ourTurret.GetEntryMarker());
        }
    	break;
    case PHASE_TURRET_ALIGN_TO_MARKER:
        {
            turret &ourTurret = turret::GetSafeType( *turretObject );           
            object *ourMarker = g_ObjMgr.GetObjectByGuid(ourTurret.GetEntryMarker());
            vector3 positionFacing = vector3(0.0f, 0.0f, 2000.0f);
            positionFacing.Rotate( ourMarker->GetL2W().GetRotation() );
            positionFacing += ourMarker->GetPosition();
            m_CharacterBase.SetTurnToLocationGoal(positionFacing,0.1f,TRUE);
    	    break;
        }
    case PHASE_TURRET_ENTER_TURRET:
        {        
            turret &ourTurret = turret::GetSafeType( *turretObject );           
            m_CharacterBase.SetScaledPlayAnimationGoal( "TURRET_MOUNT",
                                                        m_TurretAnimGroupHandle.GetName(),
                                                        DEFAULT_BLEND_TIME,
                                                        loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_INTERRUPT_BLEND, 0.0f, ourTurret.GetStandBonePosition() );
        }
    	break;
    case PHASE_TURRET_IDLE:
        m_CharacterBase.SetPlayAnimationGoal( "TURRET_IDLE",
                                              m_TurretAnimGroupHandle.GetName(),
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_PLAY_TYPE_CYCLIC, -1.0f);
    	break;
    case PHASE_TURRET_FIRE:
        m_CharacterBase.SetPlayAnimationGoal( "TURRET_SHOOT",
                                              m_TurretAnimGroupHandle.GetName(),
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_END_STATE_HOLD );
    	break;
    case PHASE_TURRET_RELOAD:
        m_CharacterBase.SetPlayAnimationGoal( "TURRET_RELOAD",
                                              m_TurretAnimGroupHandle.GetName(),
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_END_STATE_HOLD );
    	break;
    case PHASE_TURRET_EXIT_TURRET:
        m_CharacterBase.SetCollisionIgnoreGuid(m_CurrentTurret);
        m_CharacterBase.SetPlayAnimationGoal( "TURRET_DISMOUNT",
                                              m_TurretAnimGroupHandle.GetName(),
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_INTERRUPT_BLEND);
    	break;
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    }
    character_state::ChangePhase( newPhase );
}

//=========================================================================

character_state::states character_turret_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_EndState )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return STATE_NULL;
}

//=========================================================================

void character_turret_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

void character_turret_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "TurretState",  "Different variables that effect the way that the character behaves in a turret.", 0 );
    character_state::OnEnumProp(List);
}

//=========================================================================

xbool character_turret_state::OnProperty ( prop_query& rPropQuery )
{
    return character_state::OnProperty( rPropQuery );
}

//=========================================================================

const char*character_turret_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_TURRET_GOTO_TURRET:
        return "PHASE_TURRET_GOTO_TURRET";
    	break;
    case PHASE_TURRET_ENTER_TURRET:
        return "PHASE_TURRET_ENTER_TURRET";
    	break;
    case PHASE_TURRET_IDLE:
        return "PHASE_TURRET_IDLE";
    	break;
    case PHASE_TURRET_FIRE:
        return "PHASE_TURRET_FIRE";
    	break;
    case PHASE_TURRET_RELOAD:
        return "PHASE_TURRET_RELOAD";
    	break;
    case PHASE_TURRET_EXIT_TURRET:
        return "PHASE_TURRET_EXIT_TURRET";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
