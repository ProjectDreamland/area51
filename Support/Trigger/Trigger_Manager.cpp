///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Manager.cpp
//
//
///////////////////////////////////////////////////////////////////////////////


//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "gamelib\StatsMgr.hpp"
//=========================================================================
// GLOBALS
//=========================================================================

trigger_mngr g_TriggerMgr;

//=========================================================================
// TRIGGER_MNGR
//=========================================================================

trigger_mngr::trigger_mngr( ) 
{
    m_ClassList[TRIGGER_NO_UPDATE]      = NULL;
    m_ClassList[TRIGGER_UPDATE]         = NULL;
} 

//=============================================================================

trigger_object* trigger_mngr::GetTriggerPtr(  guid TriggerGuid )
{
    object* pObject = NULL;
    
    if ( SMP_UTIL_IsGuidOfType ( &pObject, TriggerGuid , trigger_object::GetRTTI() ) == FALSE )
    {
        return NULL;
    }      

    return (trigger_object*) pObject;
}

//=============================================================================

void trigger_mngr::OnUpdate ( f32 DeltaTime )
{
    stat_logger temp(k_stats_TriggerSystem);

    TRIGGER_CONTEXT( "trigger_mngr::OnUpdate" );

    //note this must be const reference, as its possible in some situations for the list to become
    //empty due to all active triggers being removed...
    const guid& rHeadGuid = m_ClassList[TRIGGER_UPDATE];
 
    if (rHeadGuid == NULL)
        return;
    
    trigger_object* pCurrentTrigger = GetTriggerPtr ( rHeadGuid );
    
    ASSERT( pCurrentTrigger );
    
    while(1)
    {
        guid Next = pCurrentTrigger->m_Next;
       
        //Note within ExecuteLogic the pCurrentTrigger can be destroyed and unregistered, so 
        //that is why we store away the guids Next and not access pCurrentTrigger after calling
        //ExecuteLogic...

        pCurrentTrigger->ExecuteLogic( DeltaTime );
    
        pCurrentTrigger = NULL;

        //HeadGuid can change after ExecuteLogic because triggers can be 
        //removed, or destroyed from the active list..
        
        if ( rHeadGuid == Next || rHeadGuid == NULL )
            break;
        
        pCurrentTrigger = GetTriggerPtr ( Next );

        if (pCurrentTrigger == NULL)
            break;
    }
}

//=============================================================================

void  trigger_mngr::TriggerSleep ( trigger_object &  rTrigger )
{
    if (rTrigger.m_TriggerSlot == TRIGGER_NO_UPDATE)
        return;
    
    UnregisterTrigger( rTrigger );
    RegisterTrigger  ( rTrigger );
}

//=============================================================================

void  trigger_mngr::TriggerAwake ( trigger_object &  rTrigger )
{
    if (rTrigger.m_TriggerSlot == TRIGGER_UPDATE)
        return;
    
    UnregisterTrigger( rTrigger );
    RegisterTrigger  ( rTrigger );
}

//=============================================================================

void  trigger_mngr::RegisterTrigger ( trigger_object &  rTrigger )
{
    //PDL::Insert the new trigger into the proper update list, depending upon
    //  their update need. Sleeping triggers go into the non-update list.

    //TODO: implement a priority queue for the updating triggers much more effiecnt
    //  than iterating through all updateable triggers.... -ddn

    TRIGGER_CONTEXT( "trigger_mngr::RegisterTrigger" );

    //The list is cyclic.....

    guid* pHeadGuid = NULL;

    if ( rTrigger.IsAwake() )
    {
        pHeadGuid = &m_ClassList[TRIGGER_UPDATE];
        rTrigger.m_TriggerSlot = TRIGGER_UPDATE;
    }
    else
    {
        pHeadGuid = &m_ClassList[TRIGGER_NO_UPDATE];
        rTrigger.m_TriggerSlot = TRIGGER_NO_UPDATE;
    }

    guid NewTriggerGuid = rTrigger.GetGuid();

    if (*pHeadGuid == NULL)
    {
        rTrigger.m_Next     = NewTriggerGuid;
        rTrigger.m_Prev     = NewTriggerGuid;
        *pHeadGuid           = NewTriggerGuid;

        return;
    }

    trigger_object* pTriggerHead        = GetTriggerPtr ( *pHeadGuid );
    trigger_object* pTriggerHeadPrev    = NULL;
    
    ASSERT( pTriggerHead );
    
    pTriggerHeadPrev = GetTriggerPtr ( pTriggerHead->m_Prev );

    ASSERT( pTriggerHeadPrev );

    //check if we have a single element cyclic list
    if ( pTriggerHead == pTriggerHeadPrev )
    {
        pTriggerHead->m_Next = NewTriggerGuid;
        pTriggerHead->m_Prev = NewTriggerGuid;
        
        rTrigger.m_Next  = *pHeadGuid;
        rTrigger.m_Prev  = *pHeadGuid;

        return;
    }

    //Insert the new trigger into the tail end of the cyclic list
    rTrigger.m_Next  = *pHeadGuid;
    rTrigger.m_Prev  = pTriggerHeadPrev->GetGuid();
    
    pTriggerHeadPrev->m_Next    = NewTriggerGuid;
    pTriggerHead->m_Prev        = NewTriggerGuid;
    
    return;
}

//=============================================================================

void  trigger_mngr::UnregisterTrigger ( trigger_object &  rTrigger )
{
    TRIGGER_CONTEXT( "trigger_mngr::UnregisterTrigger" );

    //PDL::Delete the trigger from the list
    //The list is cyclic...

    guid  DeleteTriggerGuid = rTrigger.GetGuid();
    guid* pHeadGuid = NULL;

    if ( rTrigger.m_TriggerSlot >=0 && rTrigger.m_TriggerSlot < TRIGGER_CLASS_END )
    {
        pHeadGuid = &m_ClassList[rTrigger.m_TriggerSlot];
    }
    else
    {
        x_DebugMsg( "trigger_mngr::UnregisterTrigger, Cannot find slot for given trigger." );
        ASSERT(0);
        return;
    }
   
    if ( *pHeadGuid == NULL )
        return;

    trigger_object* pDeleteTrigger        = GetTriggerPtr ( DeleteTriggerGuid );
    trigger_object* pDeleteTriggerPrev    = NULL;
    trigger_object* pDeleteTriggerNext    = NULL;
    
    ASSERT( pDeleteTrigger );
    
    pDeleteTriggerPrev = GetTriggerPtr ( pDeleteTrigger->m_Prev );
    pDeleteTriggerNext = GetTriggerPtr ( pDeleteTrigger->m_Next );

    //Check for single element cyclic link list
    if ( pDeleteTrigger == pDeleteTriggerPrev )
    {
        *pHeadGuid = NULL;
        return;
    }

    //HACK - until we get someone to figure these triggers out, don't want them crashes Level Black
    
    //Check for 2nd to last element
    if (pDeleteTriggerNext == NULL)
    {
        pDeleteTriggerNext = pDeleteTriggerPrev;
    }

    if ( pDeleteTriggerPrev && pDeleteTriggerNext)
    {
        //Remove the delete trigger from the cyclic list...
        pDeleteTriggerPrev->m_Next      = pDeleteTriggerNext->GetGuid();
        pDeleteTriggerNext->m_Prev      = pDeleteTriggerPrev->GetGuid();
     
        if ( *pHeadGuid == DeleteTriggerGuid )
        {
            *pHeadGuid = pDeleteTriggerNext->GetGuid(); 
        }
    }
    else
    {
        *pHeadGuid = NULL;
    }

    return;
}