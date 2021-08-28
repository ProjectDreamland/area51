///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Manager.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGEREX_MANAGER_
#define _TRIGGEREX_MANAGER_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "x_array.hpp"

#include "MiscUtils\Property.hpp"
#include "..\Support\TriggerEx\TriggerEx_Object.hpp"

//=========================================================================
// TRIGGER_EX_MNGR
//=========================================================================

class trigger_ex_mngr
{
public:
                        trigger_ex_mngr        ( );

    void                OnUpdate            ( f32 DeltaTime );

    void                RegisterTrigger     ( trigger_ex_object &  rTrigger );    //called by the trigger object on OnInit
    void                UnregisterTrigger   ( trigger_ex_object &  rTrigger );    //called to remove triggers from the mngr..
    void                TriggerSleep        ( trigger_ex_object &  rTrigger );
    void                TriggerAwake        ( trigger_ex_object &  rTrigger );

#ifndef CONFIG_RETAIL
    void                DumpData            ( const char* pFileName );
#endif

protected:
  
    enum trigger_ex_class
    {
        INVALID_TRIGGER_EX_CLASS = -1,
        TRIGGER_EX_NO_UPDATE,
        TRIGGER_EX_UPDATE,
        TRIGGER_EX_CLASS_END
    };

    trigger_ex_object*     GetTriggerPtr       (  guid TriggerGuid );
    guid                     m_ClassList[TRIGGER_EX_CLASS_END];        // List of Triggers
};

extern trigger_ex_mngr g_TriggerExMgr;

#endif  //_TRIGGER_EX_MANAGER_