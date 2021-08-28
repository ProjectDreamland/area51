///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Manager.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "gamelib\StatsMgr.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

trigger_ex_mngr g_TriggerExMgr;
s32             g_TriggerAdvLogicCount = 0;

#ifndef CONFIG_RETAIL
f32             fLastTriggerUpdateDeltaTime = 0.0f;
xtick           xtLastTriggerUpdateTime     = 0;
#endif

//=========================================================================
// TRIGGER_MNGR
//=========================================================================

trigger_ex_mngr::trigger_ex_mngr( ) 
{
    m_ClassList[TRIGGER_EX_NO_UPDATE]      = NULL;
    m_ClassList[TRIGGER_EX_UPDATE]         = NULL;
} 

//=============================================================================

trigger_ex_object* trigger_ex_mngr::GetTriggerPtr(  guid TriggerGuid )
{
    object* pObject = NULL;
    
    if ( SMP_UTIL_IsGuidOfType ( &pObject, TriggerGuid , trigger_ex_object::GetRTTI() ) == FALSE )
    {
        return NULL;
    }      

    return (trigger_ex_object*) pObject;
}

//=============================================================================

void trigger_ex_mngr::OnUpdate ( f32 DeltaTime )
{
    STAT_LOGGER( temp, k_stats_TriggerSystem );

#ifndef CONFIG_RETAIL
    xtLastTriggerUpdateTime     = g_ObjMgr.GetGameTime();
    fLastTriggerUpdateDeltaTime = DeltaTime;
#endif

    //note this must be const reference, as its possible in some situations for the list to become
    //empty due to all active triggers being removed...
    const guid& rHeadGuid = m_ClassList[TRIGGER_EX_UPDATE];
 
    if (rHeadGuid == NULL)
        return;
    
    trigger_ex_object* pCurrentTrigger = GetTriggerPtr ( rHeadGuid );
    
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

void  trigger_ex_mngr::TriggerSleep ( trigger_ex_object &  rTrigger )
{
    if (rTrigger.m_TriggerSlot == TRIGGER_EX_NO_UPDATE)
        return;
    
    UnregisterTrigger( rTrigger );
    RegisterTrigger  ( rTrigger );
}

//=============================================================================

void  trigger_ex_mngr::TriggerAwake ( trigger_ex_object &  rTrigger )
{
    if (rTrigger.m_TriggerSlot == TRIGGER_EX_UPDATE)
        return;
    
    UnregisterTrigger( rTrigger );
    RegisterTrigger  ( rTrigger );
}

//=============================================================================

void  trigger_ex_mngr::RegisterTrigger ( trigger_ex_object &  rTrigger )
{
    //PDL::Insert the new trigger into the proper update list, depending upon
    //  their update need. Sleeping triggers go into the non-update list.

    //TODO: implement a priority queue for the updating triggers much more effiecnt
    //  than iterating through all updateable triggers.... -ddn

    //The list is cyclic.....

    guid* pHeadGuid = NULL;

    if ( rTrigger.IsAwake() )
    {
        pHeadGuid = &m_ClassList[TRIGGER_EX_UPDATE];
        rTrigger.m_TriggerSlot = TRIGGER_EX_UPDATE;
    }
    else
    {
        pHeadGuid = &m_ClassList[TRIGGER_EX_NO_UPDATE];
        rTrigger.m_TriggerSlot = TRIGGER_EX_NO_UPDATE;
    }

    guid NewTriggerGuid = rTrigger.GetGuid();

    if (*pHeadGuid == NULL)
    {
        rTrigger.m_Next     = NewTriggerGuid;
        rTrigger.m_Prev     = NewTriggerGuid;
        *pHeadGuid           = NewTriggerGuid;

        return;
    }

    trigger_ex_object* pTriggerHead        = GetTriggerPtr ( *pHeadGuid );
    trigger_ex_object* pTriggerHeadPrev    = NULL;
    
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

void  trigger_ex_mngr::UnregisterTrigger ( trigger_ex_object &  rTrigger )
{
    //PDL::Delete the trigger from the list
    //The list is cyclic...

    guid  DeleteTriggerGuid = rTrigger.GetGuid();
    guid* pHeadGuid = NULL;

    if ( rTrigger.m_TriggerSlot >=0 && rTrigger.m_TriggerSlot < TRIGGER_EX_CLASS_END )
    {
        pHeadGuid = &m_ClassList[rTrigger.m_TriggerSlot];
    }
    else
    {
        x_DebugMsg( "trigger_ex_mngr::UnregisterTrigger, Cannot find slot for given trigger." );
        ASSERT(0);
        return;
    }
   
    if ( *pHeadGuid == NULL )
        return;

    trigger_ex_object* pDeleteTrigger        = GetTriggerPtr ( DeleteTriggerGuid );
    trigger_ex_object* pDeleteTriggerPrev    = NULL;
    trigger_ex_object* pDeleteTriggerNext    = NULL;
    
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

//=============================================================================

#ifndef CONFIG_RETAIL 

void trigger_ex_mngr::DumpData( const char* pFileName )
{
    X_FILE* pFile = x_fopen( pFileName, "wt" );
    ASSERT( pFile );

    x_DebugMsg("Starting trigger dump.....\n");

    x_fprintf( pFile, "GBL_ADV_LGC_CNT = %d\n",   g_TriggerAdvLogicCount);
    x_fprintf( pFile, "Last_Delta_Time = %g\n",   fLastTriggerUpdateDeltaTime);
    x_fprintf( pFile, "Last_Game_Time  = %g\n\n", x_TicksToMs(xtLastTriggerUpdateTime));

    x_fprintf( pFile, "A TYPE TYPDF GUID              GG ADVLGC CNT CST CAS  CAI NUT      EXE TIME \n" );
    x_fprintf( pFile, "= ==== ===== ========:======== == ====== === === ==== === ======== ======== \n" );
    slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_TRIGGER_EX);
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        if( pObject != NULL )
        {
            trigger_ex_object& TriggerObject = trigger_ex_object::GetSafeType( *pObject );
            TriggerObject.DumpData(pFile);
        }

        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    x_fprintf( pFile, "\n=================================================================\n" );
    x_fprintf( pFile, "=================================================================\n\n" );

    const guid& rHeadGuid = m_ClassList[TRIGGER_EX_UPDATE];
    if (rHeadGuid != NULL)
    {
        trigger_ex_object* pCurrentTrigger = GetTriggerPtr ( rHeadGuid );
        ASSERT( pCurrentTrigger );
        while(1)
        {
            guid Next = pCurrentTrigger->m_Next;
            x_fprintf( pFile, "%08X:%08X\n", 
                (u32)((pCurrentTrigger->GetGuid()>>32)&0xFFFFFFFF),
                (u32)((pCurrentTrigger->GetGuid()>>0 )&0xFFFFFFFF));
            pCurrentTrigger = NULL;
            if ( rHeadGuid == Next || rHeadGuid == NULL )
                break;
            pCurrentTrigger = GetTriggerPtr ( Next );

            if (pCurrentTrigger == NULL)
                break;
        }    
    }

    x_fclose(pFile);

/*
    text_out DataFile;
    DataFile.OpenFile( pFileName );

    DataFile.AddHeader("TriggerData",g_ObjMgr.GetNumInstances(object::TYPE_TRIGGER_EX));
    slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_TRIGGER_EX);
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        if( pObject != NULL )
        {
            trigger_ex_object& TriggerObject = trigger_ex_object::GetSafeType( *pObject );
            TriggerObject.DumpData(DataFile);
        }

        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    const guid& rHeadGuid = m_ClassList[TRIGGER_EX_UPDATE];
    if (rHeadGuid != NULL)
    {
        trigger_ex_object* pCurrentTrigger = GetTriggerPtr ( rHeadGuid );
        ASSERT( pCurrentTrigger );
        while(1)
        {
            guid Next = pCurrentTrigger->m_Next;
            DataFile.AddHeader("TgrMgrLstObj",1);
            DataFile.AddGuid  ("Guid", pCurrentTrigger->GetGuid() );
            pCurrentTrigger = NULL;
            if ( rHeadGuid == Next || rHeadGuid == NULL )
                break;
            pCurrentTrigger = GetTriggerPtr ( Next );

            if (pCurrentTrigger == NULL)
                break;
        }    
    }

    DataFile.CloseFile();
*/
}

#endif //CONFIG_RETAIL
