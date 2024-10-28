///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_base.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Characters\Character.hpp"
#include "Objects\group.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_base::action_ai_base( guid ParentGuid ):  actions_ex_base(  ParentGuid ),
m_bBlockUntilComplete(FALSE),
m_bIsRunningAction(FALSE),
m_bIsBlockingAction(FALSE),
m_bMustSucceed(FALSE)
//m_ResponseFlags(RF_IGNORE_ATTACKS|RF_IGNORE_SIGHT|RF_IGNORE_SOUND|RF_IGNORE_ALERTS)
{
}

//=============================================================================

void action_ai_base::OnActivate ( xbool Flag )
{
    (void)Flag;

    m_bIsRunningAction  = FALSE;
    m_bIsBlockingAction = FALSE;
}

//=============================================================================

xbool action_ai_base::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    if (m_bIsRunningAction)
    {
        if (m_bIsBlockingAction)
        {
            return FALSE;
        }
        else
        {
            m_bIsRunningAction = FALSE;
            return TRUE;
        }
    }

    object* pObject = m_CharacterAffecter.GetObjectPtr();
    if ( pObject && pObject->IsKindOf( character::GetRTTI() ) )
    {
        character* pChar = (character*)pObject;
        if (pChar->TriggerState(this))
        {
            m_bIsRunningAction = TRUE;

            if (m_bBlockUntilComplete)
            {
                m_bIsBlockingAction = TRUE;
                return FALSE;
            }
            else
            {
                m_bIsRunningAction = FALSE;
                return TRUE;
            }
        }
    }
    else if( pObject && pObject->IsKindOf( group::GetRTTI() ) )
    {
        xbool foundChar = FALSE;
        for( s32 i = 0 ; i < ((group*)pObject)->GetNumChildren() ; i ++ )
        {
            object* pChildObject = g_ObjMgr.GetObjectByGuid (((group*)pObject)->GetChild(i));
            if( pChildObject && pChildObject->IsKindOf( character::GetRTTI() ) )
            {            
                character* pChar = (character*)pChildObject;
                if( pChar->TriggerState(this) )
                {
                    m_bIsRunningAction = FALSE;
                    foundChar = TRUE;   
                }
            }
        }
        if( foundChar )
        {
            return TRUE;
        }
    }

    m_bErrorInExecute = TRUE;
    if (RetryOnError())
    {
        return FALSE;
    }
    else
    {
        m_bIsRunningAction = FALSE;
        return TRUE;
    }
} 

//=============================================================================

#ifndef X_RETAIL
void action_ai_base::OnDebugRender ( s32 Index )
{
    object* pObject = m_CharacterAffecter.GetObjectPtr();
    if (pObject && pObject->IsKindOf( character::GetRTTI() ) )
    {
        draw_Line( GetPositionOwner(), pObject->GetPosition(), XCOLOR_PURPLE );
        draw_BBox( pObject->GetBBox(), XCOLOR_PURPLE );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[%d]Trigger AI", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[Else %d]Trigger AI", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_ai_base::OnEnumProp ( prop_enum& rPropList )
{ 
    if( m_TriggerGuid )
    {
        //guid specific fields
        m_CharacterAffecter.OnEnumProp( rPropList, "Character" );

        rPropList.PropEnumBool  ( "Block",         "Is this AI action a blocking action? If so this action will block until the ai leaves the trigger state.", 0 );
        rPropList.PropEnumBool  ( "Must Succeed",  "Does this action have to succeed for us to move on?", 0 );
/*
        rPropList.AddHeader( "ResponseList",                   "List Various criteria for ignoring external stimuli");
        rPropList.AddBool  ( "ResponseList\\IgnoreAttacks",    "Ignore all attacks.");
        rPropList.AddBool  ( "ResponseList\\IgnoreSight",      "Ignore all spotted enemies.");
        rPropList.AddBool  ( "ResponseList\\IgnoreSound",      "Ignore all threatening sounds.");
        rPropList.AddBool  ( "ResponseList\\IgnoreAlerts",     "Ignore all alerts.");
        rPropList.AddBool  ( "ResponseList\\Invincible",       "Ignore all pain and damage. (use with caution)");
        */
        m_ResponseFlags.OnEnumProp(rPropList);
    }
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_ai_base::OnProperty ( prop_query& rPropQuery )
{
    if( m_TriggerGuid )
    {    
        //guid specific fields
        if( m_CharacterAffecter.OnProperty( rPropQuery, "Character" ) )
        {        
            return TRUE;
        }

        if ( rPropQuery.VarBool( "Block" , m_bBlockUntilComplete ) )
        {        
            return TRUE;
        }
        if ( rPropQuery.VarBool( "Must Succeed" , m_bMustSucceed ) )
        {        
            return TRUE;
        }
        if( m_ResponseFlags.OnProperty(rPropQuery) )
        {
            return TRUE;
        }   
    }

    
    if( actions_ex_base::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

const char* action_ai_base::GetAIName( void )
{
    static med_string CharData;
    if( m_TriggerGuid )
    {
        if (m_bBlockUntilComplete)
            CharData.Set(xfs("* %s",m_CharacterAffecter.GetObjectInfo()));
        else
            CharData.Set(m_CharacterAffecter.GetObjectInfo());

        return CharData.Get();
    }
    else
    {
        return "AI";
    }
}
