//==============================================================================
//
//  Trigger.cpp
//
//      The Trigger object does not insert itself into the spatial dabase nor
//      does it use the collision manager.  Instead, it makes use of the object
//      managers get objects in bbox functionality and checks that every so
//      often.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Trigger.hpp"
#include "LuaLib\LuaMgr.hpp"


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct trigger_desc : public object_desc
{
//=========================================================================
    trigger_desc( void ) : object_desc( object::TYPE_TRIGGER, 
                                        "Trigger",
                                        object::ATTR_NEEDS_LOGIC_TIME ) 
    {

    }

//=========================================================================
    virtual object* Create          ( void )
    {
        return new trigger;
    }

//=========================================================================
    virtual void    Delete          ( object* pObj )
    {
        delete (trigger*)pObj;
    }

} s_Trigger_Desc;

//==============================================================================
trigger::trigger(void) : object( s_Trigger_Desc )
{

}


//==============================================================================
trigger::~trigger(void)
{

}
 

//==============================================================================
void trigger::OnInit(void)
{
    object::OnInit();
    m_TimeBetweenChecks     = 0.0f; 
    m_TimeSinceCheck        = 0.0f;      
    m_TimeBetweenActivation = 500.0f;
    m_LastActivationTime    = 99999.0f;
    m_PlayerOnly            = true;
    SetAttrBits( ATTR_NEEDS_LOGIC_TIME );
    m_ScriptName[0] = 0;

    
}

//==============================================================================
void trigger::SetTriggerZone  ( bbox  TriggerBBox )
{
   m_TriggerZone = TriggerBBox;

}


//==============================================================================
xbool trigger::OnActivate( slot_ID ID )
{
	(void)ID;    

//    if(m_ScriptName[0])
//    {
//        m_ScriptHandle.SetName(m_ScriptName);
//        lua_script* pScript = (lua_script*)(m_ScriptHandle.GetPointer());
//        g_LuaMgr.ExecuteScript(pScript);

//    }


    return true;
}


//==============================================================================
void trigger::OnAdvanceLogic  ( f32 DeltaTime )
{
    CONTEXT( "trigger::OnAdvanceLogic" );

    object::OnAdvanceLogic(DeltaTime);
    
    //  increase how much time since we activated
    m_LastActivationTime += DeltaTime;
    m_TimeSinceCheck += DeltaTime;

    if(m_LastActivationTime < m_TimeBetweenActivation)
        return;

    //  See if it's time for another check
    if( m_TimeBetweenChecks > m_TimeSinceCheck )
    {
        return;
    }
    m_TimeSinceCheck = 0.0f;



    if( m_PlayerOnly )
    {
        g_ObjMgr.SelectBBox(    ATTR_ALL, m_TriggerZone, object::TYPE_PLAYER );
    }
    else        
    {
        g_ObjMgr.SelectBBox(    ATTR_ALL, m_TriggerZone );
    }
    
    slot_ID ID = g_ObjMgr.StartLoop();
//    xbool Handled = false;

    //  While we aren't at the end of the list and it has not been Handled
//    while( ID != obj_mgr::INVALID_OBJECT && !Handled )
    if(ID != obj_mgr::INVALID_OBJECT )
    {
        m_LastActivationTime = 0.0f;
        OnEnter(  );
    }

    //  End the loop.
    g_ObjMgr.EndLoop();
    

        
}
 

//=============================================================================
//
//		SetTriggerScript
//
//
//=============================================================================
/*
void trigger::SetTriggerScript( const char *scriptName )
{
    ASSERT(scriptName);
    ASSERT(x_strlen(scriptName) < MAX_NAME_LENGTH);

    x_strncpy(m_ScriptName,scriptName, MAX_NAME_LENGTH );

    

}
*/
void    trigger::OnEnter ( void )
{

    char ScriptName[128];
    if(GetObjectName() != NULL)
    {
        x_sprintf(ScriptName,"%s.OnEnter.lc",GetObjectName() );
        rhandle_base aHandle(ScriptName);
        lua_script* pScript = (lua_script*)(aHandle.GetPointer());
        if(pScript)        
        {
            g_LuaMgr.ExecuteScript(pScript);

        }

    }

}


void    trigger::OnExit  ( void )
{
    char ScriptName[128];
    if(GetObjectName() != NULL)
    {
        x_sprintf(ScriptName,"%s.OnExit.lc",GetObjectName() );
        rhandle_base aHandle(ScriptName);
        lua_script* pScript = (lua_script*)(aHandle.GetPointer());
        if(pScript)        
        {
            g_LuaMgr.ExecuteScript(pScript);

        }

    }


}


void    trigger::OnInside( void )
{

    char ScriptName[128];
    if(GetObjectName() != NULL)
    {
        x_sprintf(ScriptName,"%s.OnInside.lc",GetObjectName() );
        rhandle_base aHandle(ScriptName);
        lua_script* pScript = (lua_script*)(aHandle.GetPointer());
        if(pScript)        
        {
            g_LuaMgr.ExecuteScript(pScript);

        }

    }


}





//=============================================================================
//
//		SetTimeBetweenActivations
//
//
//=============================================================================
void trigger::SetTimeBetweenActivations( f32 Time )
{
    m_TimeBetweenActivation = Time;

}
