#include "grunt_cover_state.hpp"
#include "..\Character.hpp"
#include "navigation\coverNode.hpp"
#include "objects\NewWeapon.hpp"
#include "audiomgr\audiomgr.hpp"

//=========================================================================
// constants
//=========================================================================

//=========================================================================
// GRAY COVER STATE
//=========================================================================

grunt_cover_state::grunt_cover_state( character& ourCharacter, character_state::states State ) :
    character_cover_state(ourCharacter, State)
{
}

//=========================================================================

grunt_cover_state::~grunt_cover_state()
{
}

//=========================================================================

xbool grunt_cover_state::GetAnimNameFromPhase( s32 nextPhase, char* animName )
{
    xbool bFoundName = FALSE;
    char weaponPrefix[4];

    weaponPrefix[0] = 0;
    
    new_weapon* pWeapon = m_CharacterBase.GetCurrentWeaponPtr();
    if( pWeapon )
    {
        x_strcpy( weaponPrefix,new_weapon::GetWeaponPrefixFromInvType2( pWeapon->GetInvenItem()) );
    }    

    switch( nextPhase )
    {
    case PHASE_GRUNT_COVER_ENTER_COVER:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_ENTER");
        }
        else
        {
            x_strcpy(animName,"COVER_ENTER");
        }
        break;
    case PHASE_GRUNT_COVER_IDLE:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_IDLE");
        }
        else
        {
            x_strcpy(animName,"COVER_IDLE");
        }
        break;
    case PHASE_GRUNT_COVER_EXIT_COVER:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_EXIT");
        }
        else
        {
            x_strcpy(animName,"COVER_EXIT");
        }
        break;
    };  
    return bFoundName;
}

//=========================================================================

//=========================================================================

s32 grunt_cover_state::GetFacePhase()
{
    // if we have an enter cover, we will blend roll in instead of facing initially.
    if( HasAnimForCoverPhase(PHASE_GRUNT_COVER_ENTER_COVER) )
    {
        return PHASE_GRUNT_COVER_ENTER_COVER;
    }
    else
    {
        return PHASE_GRUNT_COVER_FACE_EXACT;
    }
}

//=========================================================================

xbool grunt_cover_state::ProvideAutofire()
{   
    return FALSE;
}

//=========================================================================

xbool grunt_cover_state::IgnoreFullBodyFlinches()
{
    if( m_CurrentPhase == PHASE_GRUNT_COVER_ENTER_COVER || 
        m_CurrentPhase == PHASE_GRUNT_COVER_EXIT_COVER )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

xbool grunt_cover_state::UseRelativeMode( void )
{
    return FALSE;
    // Update loco relative mode base on final phase that was chosen
    switch( m_CurrentPhase )
    {
        // If arriving at cover node, turn on relative mode and set the position
    case PHASE_GRUNT_COVER_IDLE:        
        return TRUE;

        // If npc is moving to cover or doing a base phase, turn off relative mode
    default:

        // These "OUT" anims are at the origin so they can't use relative mode

        // These phases are either going to or coming from cover so no relative mode
    case PHASE_GRUNT_COVER_FACE_EXACT:
    case PHASE_GRUNT_COVER_ENTER_COVER:
    case PHASE_GRUNT_COVER_GOTO_COVER:
    case PHASE_GRUNT_COVER_EXIT_COVER:
        return FALSE;
    }
}

//=========================================================================

s32 grunt_cover_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;

    if( m_InCover )
    {
        m_TimeInCover += DeltaTime;
    }
    else
    {
        m_TimeInCover = 0.0f;
    }

    if( m_RolledOut )
    {
        m_TimeRolledOut += DeltaTime;
    }
    else
    {
        m_TimeRolledOut = 0.0f;
    }

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_GRUNT_COVER_GOTO_COVER;
        break;
    case PHASE_GRUNT_COVER_GOTO_COVER:
        {       
            SetInCover(FALSE);
            m_CharacterBase.CheckShooting();
            // if sucessfully completed
            if( m_CharacterBase.GetGoalCompleted() &&
                m_CharacterBase.GetGoalSucceeded() )
            {
                // Record that we are at this cover       
                SetCurrentCover( m_CharacterBase.GetCurrentCover() );            
                newPhase = GetFacePhase();
            }
            // else if failed or target changes.
            else if( m_CharacterBase.GetCurrentGoalInfo().m_TargetGuid != m_CharacterBase.GetCurrentCover() ||
                     m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_GRUNT_COVER_GOTO_COVER;
            }
        }
        break;
    case PHASE_GRUNT_COVER_FACE_EXACT:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {               
            newPhase = PHASE_GRUNT_COVER_ENTER_COVER;
        }
        break;
    case PHASE_GRUNT_COVER_ENTER_COVER:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(TRUE);
            newPhase = PHASE_GRUNT_COVER_IDLE;
        }
        break;
    case PHASE_GRUNT_COVER_IDLE:
        {        
            SetInCover(TRUE);
            SetRolledOut(FALSE);
            m_DelayTillNextAction -= DeltaTime;

            // ok, here's the skinny... 
            // stay in idle until either 
            // cover not valid or time up and not sticky.

            if( (m_DelayTillNextAction <= 0 &&
                 m_CurrentCover != m_CharacterBase.GetStickyCoverNode()) ||
                m_CurrentCover != m_CharacterBase.GetCurrentCover() )
            {
                newPhase = PHASE_GRUNT_COVER_EXIT_COVER;
            }
            else if( m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_GRUNT_COVER_IDLE;
            }
        }
        break;
    case PHASE_GRUNT_COVER_EXIT_COVER:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(FALSE);
            m_DesiredState = m_CharacterBase.GetStateFromAwareness();
            if( m_DesiredState == STATE_COVER )
            {            
                newPhase = PHASE_GRUNT_COVER_GOTO_COVER;
            }
            // we are done!
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

    // Setup relative mode
    UpdateRelativeMode();

    // ignore the base state.
    //??? why ignore the base state? Then they can't evade grenades or get back into 
    // the nav path when running and they get out.
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }
    return newPhase;
}

//=========================================================================

void grunt_cover_state::ChangePhase( s32 newPhase )
{
    object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentCover());

    char AnimNameFromPhase[32];
    switch( newPhase)
    {    
    case PHASE_GRUNT_COVER_GOTO_COVER:                
        m_CharacterBase.SetWantsToAim(TRUE);
        if ( nodeObject && nodeObject->IsKindOf( cover_node::GetRTTI() ))
        {   
            cover_node &coverNode = cover_node::GetSafeType( *nodeObject );           
            m_CharacterBase.SetGotoTargetGoal(coverNode.GetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,0.0f,TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_GRUNT_COVER_FACE_EXACT:              
        m_CharacterBase.SetWantsToAim(TRUE);
        if ( nodeObject && nodeObject->IsKindOf( cover_node::GetRTTI() ))
        {   
            cover_node &coverNode = cover_node::GetSafeType( *nodeObject );           
            vector3 coverNodeFacing(0.0f,coverNode.GetNPCFacing());
            coverNodeFacing.NormalizeAndScale(1000.0f);
            m_CharacterBase.SetTurnToLocationGoal(coverNode.GetPosition() + coverNodeFacing, 0.0f, TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_GRUNT_COVER_IDLE:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD |*/ loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        break;
    case PHASE_GRUNT_COVER_ENTER_COVER:                   
        {        
            UpdateUsedDelays();
            if ( nodeObject && nodeObject->IsKindOf( cover_node::GetRTTI() ))
            {   
                cover_node &coverNode = cover_node::GetSafeType( *nodeObject );           
                radian yawDiff = x_MinAngleDiff( m_CharacterBase.GetLocoPointer()->GetYaw(),coverNode.GetNPCFacing() );
                m_CharacterBase.SetAnimYawDelta( -yawDiff );
            }
        }
    case PHASE_GRUNT_COVER_EXIT_COVER:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_INTERRUPT_BLEND | loco::ANIM_FLAG_TURN_OFF_AIMER );
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

character_state::states grunt_cover_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentCover());
    if ( !nodeObject || !nodeObject->IsKindOf(cover_node::GetRTTI()) )
    {   
        m_DesiredState = m_CharacterBase.GetStateFromAwareness();
        if( m_DesiredState == STATE_COVER )
        {
            m_DesiredState = STATE_NULL;
        }
    }
    // we stay here as long as we have sticky cover.
    else if( !m_CharacterBase.GetStickyCoverNode() )
    {   
        // if leave cover conditions met, leave cover
        cover_node &coverNode = cover_node::GetSafeType( *nodeObject );
        if( (m_LeaveCoverCondition == LEAVE_COVER_WHEN_DAMAGED && 
             m_TookPainThisTick)  ||
            (m_LeaveCoverCondition == LEAVE_COVER_WHEN_CAN_REACH_TARGET && 
             m_CharacterBase.CanPathToTarget())/* ||
            (m_LeaveCoverCondition == LEAVE_COVER_WHEN_BROKEN &&
             !m_CharacterBase.GetCoverIsValid()) */)
        {
            coverNode.InvalidateNode();
            m_DesiredState = m_CharacterBase.GetStateFromAwareness();
            if( m_DesiredState == STATE_COVER )
            {
                m_DesiredState = STATE_NULL;
            }
        }
    }
    
    if( !m_InCover && m_DesiredState != STATE_NULL )
    {
        return m_DesiredState;
    }
    return STATE_NULL;
}


//=========================================================================

void grunt_cover_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader (  "CoverState",  "Different variables that effect the way that the character behaves when standing still.", 0 );
    List.PropEnumFloat  ( "CoverState\\TimeTillBored", "Amount of Time to stay in state with no new sounds heard.", 0 ) ;
    List.PropEnumFloat  ( "CoverState\\MinTimeBetweenCombatPopup", "Min time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumFloat  ( "CoverState\\MaxTimeBetweenCombatPopup", "Max time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
}

//=========================================================================

xbool grunt_cover_state::OnProperty ( prop_query& rPropQuery )
{
    if (rPropQuery.VarFloat("CoverState\\TimeTillBored", m_TimeTillBored))
        return TRUE;
    if (rPropQuery.VarFloat("CoverState\\MinTimeBetweenCombatPopup", m_ActionDelayMin))
        return TRUE;
    if (rPropQuery.VarFloat("CoverState\\MaxTimeBetweenCombatPopup", m_ActionDelayMax))
        return TRUE;

    return FALSE ;
}

//=========================================================================

const char*grunt_cover_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_GRUNT_COVER_GOTO_COVER:
        return "PHASE_GRUNT_COVER_GOTO_COVER";
    	break;
    case PHASE_GRUNT_COVER_FACE_EXACT:
        return "PHASE_GRUNT_COVER_FACE_EXACT";
    	break;
    case PHASE_GRUNT_COVER_ENTER_COVER:
        return "PHASE_GRUNT_COVER_ENTER_COVER";
        break;
    case PHASE_GRUNT_COVER_IDLE:
        return "PHASE_GRUNT_COVER_IDLE";
    	break;
    case PHASE_GRUNT_COVER_EXIT_COVER:
        return "PHASE_GRUNT_COVER_EXIT_COVER";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
