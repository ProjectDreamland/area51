///////////////////////////////////////////////////////////////////////////
//
//  action_affect_global.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_affect_global.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_affect_global::action_affect_global ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_VarRaw(0),
m_VarGuid(0),
m_Code(INVALID_VAR_CODES),
m_GlobalIndex(-1)
{
}

//=============================================================================

xbool action_affect_global::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    xhandle rHandle;

    if (m_GlobalIndex >= 0)
    {
        if (g_VarMgr.GetVarHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
        {
            switch ( g_VarMgr.GetType( rHandle ) )
            {
            case var_mngr::TYPE_FLOAT: 
                {
                    f32  GlobalValue = g_VarMgr.GetFloat( rHandle );
                    f32& FloatValue = *((f32*) &m_VarRaw);
        
                    switch (m_Code)
                    {
                    case VAR_CODE_ADD:           g_VarMgr.SetFloat( rHandle, GlobalValue + FloatValue );
                        return TRUE;
                    case VAR_CODE_SUBTRACT:      g_VarMgr.SetFloat( rHandle, GlobalValue - FloatValue );
                        return TRUE;
                    case VAR_CODE_SET:           g_VarMgr.SetFloat( rHandle, FloatValue );
                        return TRUE;
                    }
                }
                break;
            case var_mngr::TYPE_INT: 
                {
                    s32  GlobalValue = g_VarMgr.GetInt( rHandle );
                    s32& IntValue = *((s32*) &m_VarRaw);
        
                    switch (m_Code)
                    {
                    case VAR_CODE_ADD:           g_VarMgr.SetInt( rHandle, GlobalValue + IntValue );
                        return TRUE;
                    case VAR_CODE_SUBTRACT:      g_VarMgr.SetInt( rHandle, GlobalValue - IntValue );
                        return TRUE;
                    case VAR_CODE_SET:           g_VarMgr.SetInt( rHandle, IntValue );
                        return TRUE;
                    }
                }
                break;
            case var_mngr::TYPE_BOOL: 
                {
                    xbool& BoolValue = *((xbool*) &m_VarRaw);
                    g_VarMgr.SetBool( rHandle, BoolValue );
                    return TRUE;
                }
                break;
            }   
        }
    
        if (g_VarMgr.GetGuidHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
        {
            g_VarMgr.SetGuid( rHandle, m_VarGuid );
            return TRUE;
        }

        if (g_VarMgr.GetTimerHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
        {
            switch ( m_Code )
            {
            case VAR_CODE_RESET_TIMER:
               g_VarMgr.ResetTimer( rHandle );
               return TRUE;
            case VAR_CODE_START_TIMER:
               g_VarMgr.StartTimer( rHandle );
               return TRUE;
            case VAR_CODE_STOP_TIMER:
               g_VarMgr.StopTimer( rHandle );
               return TRUE;
            };
        }
    }

    //failure
    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

#ifndef X_RETAIL
void action_affect_global::OnDebugRender ( s32 Index )
{
    (void) Index;
}
#endif // X_RETAIL

//=============================================================================

s32 action_affect_global::GetVariableType( void )
{
    if (m_GlobalIndex < 0)
        return var_mngr::GLOBAL_NULL;

    xhandle rHandle;
    if (g_VarMgr.GetVarHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
    {
        var_mngr::variable_types Type = g_VarMgr.GetType(rHandle);
        switch(Type)
        {
        case var_mngr::TYPE_FLOAT:
            return var_mngr::GLOBAL_FLOAT;
            break;
        case var_mngr::TYPE_INT:
            return var_mngr::GLOBAL_INT;
            break;
        case var_mngr::TYPE_BOOL:
            return var_mngr::GLOBAL_BOOL;
            break;
        }
    }

    if (g_VarMgr.GetGuidHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
    {
        return var_mngr::GLOBAL_GUID;
    }

    if (g_VarMgr.GetTimerHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
    {
        return var_mngr::GLOBAL_TIMER;
    }

    return var_mngr::GLOBAL_NULL;
}

//=============================================================================

void action_affect_global::OnEnumProp	( prop_enum& rPropList )
{
    actions_ex_base::OnEnumProp( rPropList );

    rPropList.PropEnumExternal( "Variable",  "global\0global_all", 
                                "Which global Variable to affect.", PROP_TYPE_MUST_ENUM );
    
    rPropList.PropEnumInt     ( "Code" ,     "",  PROP_TYPE_DONT_SHOW  );
    
    var_mngr::global_types Type = (var_mngr::global_types)GetVariableType();
    switch ( Type )
    {
    case var_mngr::GLOBAL_FLOAT: 
        rPropList.PropEnumEnum   ( "Operation",  "Add\0Subtract\0Set To\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumFloat  ( "Float Op",   "Floating point value.", 0 );
        break;
    case var_mngr::GLOBAL_INT:     
        rPropList.PropEnumEnum   ( "Operation",  "Add\0Subtract\0Set To\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumInt    ( "Int Op" ,    "Interger value.", 0 );
        break;
    case var_mngr::GLOBAL_BOOL:   
        rPropList.PropEnumBool   ( "Set Bool",   "Set this global to this Boolean value.", 0  );
        break;
    case var_mngr::GLOBAL_TIMER:
        rPropList.PropEnumEnum   ( "Operation",  "Reset Timer\0Start Timer\0Stop Timer\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        break;
    case var_mngr::GLOBAL_GUID:     
        rPropList.PropEnumGuid   ( "Set Guid",   "Set this global to this guid value.", 0  );
        break;
    }
        
}

//=============================================================================

xbool action_affect_global::OnProperty	( prop_query& rPropQuery )
{
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if (rPropQuery.VarFloat ( "Float Op", *((f32*) &m_VarRaw) ))
        return TRUE; 
    
    if (rPropQuery.VarInt ( "Int Op" , m_VarRaw ))
        return TRUE;

    if (rPropQuery.VarBool ( "Set Bool" , *((xbool*) &m_VarRaw) ))
        return TRUE;

    if (rPropQuery.VarGUID ( "Set Guid" , m_VarGuid ))
        return TRUE;

    if ( rPropQuery.IsVar( "Operation" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case VAR_CODE_ADD:          rPropQuery.SetVarEnum( "Add" );            break;
            case VAR_CODE_SUBTRACT:     rPropQuery.SetVarEnum( "Subtract" );       break;
            case VAR_CODE_SET:          rPropQuery.SetVarEnum( "Set To" );         break;
            case VAR_CODE_RESET_TIMER:  rPropQuery.SetVarEnum( "Reset Timer" );    break;
            case VAR_CODE_START_TIMER:  rPropQuery.SetVarEnum( "Start Timer" );    break;
            case VAR_CODE_STOP_TIMER:   rPropQuery.SetVarEnum( "Stop Timer" );     break;
            default:                    rPropQuery.SetVarEnum( "INVALID" );        break;
            }
            
            return TRUE;
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();

            if( x_stricmp( pString, "Add"              )==0)    { m_Code = VAR_CODE_ADD;}
            else if( x_stricmp( pString, "Subtract"    )==0)    { m_Code = VAR_CODE_SUBTRACT;}
            else if( x_stricmp( pString, "Set To"      )==0)    { m_Code = VAR_CODE_SET;}
            else if( x_stricmp( pString, "Reset Timer" )==0)    { m_Code = VAR_CODE_RESET_TIMER;}
            else if( x_stricmp( pString, "Start Timer" )==0)    { m_Code = VAR_CODE_START_TIMER;}
            else if( x_stricmp( pString, "Stop Timer"  )==0)    { m_Code = VAR_CODE_STOP_TIMER;}
            
            return TRUE;
        }
    }

    if ( rPropQuery.IsVar  ( "Variable" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_GlobalIndex >= 0 )
                rPropQuery.SetVarExternal( g_StringMgr.GetString(m_GlobalIndex), 256 );
            else
                rPropQuery.SetVarExternal("", 256);
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarExternal()) > 0)
            {
                m_GlobalIndex = g_StringMgr.Add( rPropQuery.GetVarExternal() );

                var_mngr::global_types Type = (var_mngr::global_types)GetVariableType();
                switch ( Type )
                {
                case var_mngr::GLOBAL_FLOAT: 
                case var_mngr::GLOBAL_INT:     
                case var_mngr::GLOBAL_BOOL:   
                case var_mngr::GLOBAL_GUID:     
                    m_Code = VAR_CODE_SET;
                    break;
                case var_mngr::GLOBAL_TIMER:
                    m_Code = VAR_CODE_RESET_TIMER;
                    break;
                }
            
                m_VarGuid = 0;
                m_VarRaw  = 0;

                return TRUE;
            }
        }
    }  

    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_affect_global::GetDescription( void )
{
    static big_string   Info;
    
    if (m_GlobalIndex == -1)
    {
        Info.Set("Affect Unknown Global");
    }
    else
    {
        switch ( m_Code )
        {
        case VAR_CODE_ADD:          Info.Set(xfs("Add to %s", g_StringMgr.GetString(m_GlobalIndex)));          break;
        case VAR_CODE_SUBTRACT:     Info.Set(xfs("Subtract from %s", g_StringMgr.GetString(m_GlobalIndex)));   break;
        case VAR_CODE_SET:          Info.Set(xfs("Set %s", g_StringMgr.GetString(m_GlobalIndex)));             break;
        case VAR_CODE_RESET_TIMER:  Info.Set(xfs("Reset %s", g_StringMgr.GetString(m_GlobalIndex)));           break;
        case VAR_CODE_START_TIMER:  Info.Set(xfs("Start %s", g_StringMgr.GetString(m_GlobalIndex)));           break;
        case VAR_CODE_STOP_TIMER:   Info.Set(xfs("Stop %s", g_StringMgr.GetString(m_GlobalIndex)));            break;
        case INVALID_VAR_CODES:     Info.Set("Affect Unknown Global");                                         break;
        }
    }

    return Info.Get();
}


