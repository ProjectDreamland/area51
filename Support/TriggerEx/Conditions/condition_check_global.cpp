///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_global.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_check_global.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "miscutils\Guid.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

typedef enum_pair<condition_check_global::codes> code_pair;

//=============================================================================

static code_pair s_CodeTableFull[] = 
{
        code_pair("=",      condition_check_global::CVAR_CODE_EQUAL),
        code_pair("!=",     condition_check_global::CVAR_CODE_NOT_EQUAL),
        code_pair(">=",     condition_check_global::CVAR_CODE_GREATER_INCLUSIVE),
        code_pair("<=",     condition_check_global::CVAR_CODE_LESSER_INCLUSIVE),
        code_pair(">",      condition_check_global::CVAR_CODE_GREATER),
        code_pair("<",      condition_check_global::CVAR_CODE_LESSER),
        code_pair( k_EnumEndStringConst,condition_check_global::INVALID_CVAR_CODES),  //**MUST BE LAST**//
};

static code_pair s_CodeTableSmall[] = 
{
        code_pair("=",      condition_check_global::CVAR_CODE_EQUAL),
        code_pair("!=",     condition_check_global::CVAR_CODE_NOT_EQUAL),
        code_pair( k_EnumEndStringConst,condition_check_global::INVALID_CVAR_CODES),  //**MUST BE LAST**//
};

enum_table<condition_check_global::codes> condition_check_global::m_CodeTableFull( s_CodeTableFull   );      
enum_table<condition_check_global::codes> condition_check_global::m_CodeTableSmall( s_CodeTableSmall );      

//=========================================================================

condition_check_global::condition_check_global( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_Code(CVAR_CODE_EQUAL),
m_GlobalIndex(-1),
m_VarRaw(0),
m_VarGuid(0)
{
}

//=============================================================================

xbool condition_check_global::Execute( guid TriggerGuid )
{   
    (void) TriggerGuid;

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
                    case CVAR_CODE_GREATER_INCLUSIVE:    return GlobalValue >= FloatValue;
                    case CVAR_CODE_LESSER_INCLUSIVE:     return GlobalValue <= FloatValue;
                    case CVAR_CODE_GREATER:              return GlobalValue >  FloatValue;
                    case CVAR_CODE_LESSER:               return GlobalValue <  FloatValue;
                    case CVAR_CODE_EQUAL:                return GlobalValue == FloatValue;
                    case CVAR_CODE_NOT_EQUAL:            return GlobalValue != FloatValue;    
                    }
                }
                break;
            case var_mngr::TYPE_INT: 
                {
                    s32  GlobalValue = g_VarMgr.GetInt( rHandle );
                    s32& IntValue = *((s32*) &m_VarRaw);
        
                    switch (m_Code)
                    {
                    case CVAR_CODE_GREATER_INCLUSIVE:    return GlobalValue >= IntValue;
                    case CVAR_CODE_LESSER_INCLUSIVE:     return GlobalValue <= IntValue;
                    case CVAR_CODE_GREATER:              return GlobalValue >  IntValue;
                    case CVAR_CODE_LESSER:               return GlobalValue <  IntValue;
                    case CVAR_CODE_EQUAL:                return GlobalValue == IntValue;
                    case CVAR_CODE_NOT_EQUAL:            return GlobalValue != IntValue;    
                    }
                }
                break;
            case var_mngr::TYPE_BOOL: 
                {
                    xbool& BoolValue = *((xbool*) &m_VarRaw);
                    xbool GlobalValue = g_VarMgr.GetBool(rHandle);
                
                    switch (m_Code)
                    {
                    case CVAR_CODE_EQUAL:                return GlobalValue == BoolValue;
                    case CVAR_CODE_NOT_EQUAL:            return GlobalValue != BoolValue;    
                    }
                }
                break;
            }   
        }
    
        if (g_VarMgr.GetGuidHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
        {
            guid GlobalGuid = g_VarMgr.GetGuid(rHandle);
            switch (m_Code)
            {
            case CVAR_CODE_EQUAL:                return m_VarGuid == GlobalGuid;
            case CVAR_CODE_NOT_EQUAL:            return m_VarGuid != GlobalGuid;    
            }
        }

        if (g_VarMgr.GetTimerHandle(g_StringMgr.GetString(m_GlobalIndex), &rHandle) )
        {
            f32 CurrentTime = g_VarMgr.ReadTimer( rHandle );;
            f32& FloatValue = *((f32*) &m_VarRaw);

            switch ( m_Code )
            {
            case CVAR_CODE_GREATER_INCLUSIVE:    return CurrentTime >= FloatValue;
            case CVAR_CODE_LESSER_INCLUSIVE:     return CurrentTime <= FloatValue;
            case CVAR_CODE_GREATER:              return CurrentTime >  FloatValue;
            case CVAR_CODE_LESSER:               return CurrentTime <  FloatValue;
            case CVAR_CODE_EQUAL:                return CurrentTime == FloatValue;
            case CVAR_CODE_NOT_EQUAL:            return CurrentTime != FloatValue;
            }
        }
    }
    
    return FALSE;
}    

//=============================================================================

s32 condition_check_global::GetVariableType( void )
{
    if (m_GlobalIndex == -1)
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

void condition_check_global::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumExternal( "Variable",  "global\0global_all", "Which global Variable to check.", PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumInt     ( "Code" ,     "",  PROP_TYPE_DONT_SHOW  );

    var_mngr::global_types Type = (var_mngr::global_types)GetVariableType();
    switch ( Type )
    {
    case var_mngr::GLOBAL_FLOAT: 
        rPropList.PropEnumEnum   ( "Operation",  m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumFloat  ( "Check Float","Floating point value to compare against.", PROP_TYPE_MUST_ENUM );
        break;
    case var_mngr::GLOBAL_INT:     
        rPropList.PropEnumEnum   ( "Operation",  m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumInt    ( "Check Int" , "Interger value to compare against.", PROP_TYPE_MUST_ENUM );
        break;
    case var_mngr::GLOBAL_BOOL:   
        rPropList.PropEnumEnum   ( "Operation",  m_CodeTableSmall.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumBool   ( "Check Bool", "Boolean value to compare against.", PROP_TYPE_MUST_ENUM  );
        break;
    case var_mngr::GLOBAL_TIMER:
        rPropList.PropEnumEnum   ( "Operation",  m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumFloat  ( "Check Timer","Number of Seconds to compare against.", PROP_TYPE_MUST_ENUM  );
        break;
    case var_mngr::GLOBAL_GUID:     
        rPropList.PropEnumEnum   ( "Operation",  m_CodeTableSmall.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumGuid   ( "Check Guid", "Guid value to compare against.", PROP_TYPE_MUST_ENUM  );
        break;
    }
    
    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_check_global::OnProperty ( prop_query& rPropQuery )
{
    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if (rPropQuery.VarFloat ( "Check Float", *((f32*) &m_VarRaw) ))
        return TRUE; 
    
    if (rPropQuery.VarInt   ( "Check Int" , m_VarRaw ))
        return TRUE;

    if (rPropQuery.VarBool  ( "Check Bool" , *((xbool*) &m_VarRaw) ))
        return TRUE;

    if (rPropQuery.VarGUID  ( "Check Guid" , m_VarGuid ))
        return TRUE; 

    if (rPropQuery.VarFloat ( "Check Timer", *((f32*) &m_VarRaw) ))
        return TRUE; 

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

                m_Code = CVAR_CODE_EQUAL;
                m_VarGuid = 0;
                m_VarRaw  = 0;

                return TRUE;
            }
        }
    }  

    var_mngr::global_types Type = (var_mngr::global_types)GetVariableType();
    switch (Type)
    {
    case var_mngr::GLOBAL_FLOAT: 
    case var_mngr::GLOBAL_INT:
    case var_mngr::GLOBAL_TIMER: 
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Operation", m_Code, m_CodeTableFull ) )
            return TRUE;
        break;
    case var_mngr::GLOBAL_BOOL: 
    case var_mngr::GLOBAL_GUID: 
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Operation", m_Code, m_CodeTableSmall ) )
            return TRUE; break;
    default:
        break;   
    }
    
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* condition_check_global::GetDescription( void )
{
    static big_string   Info;
    static sml_string   VarData;
    static med_string   GlbData;
    
    var_mngr::global_types Type = (var_mngr::global_types)GetVariableType();
    switch (Type)
    {
    case var_mngr::GLOBAL_FLOAT:    VarData.Set(xfs("%g", *((f32*) &m_VarRaw))); break;
    case var_mngr::GLOBAL_INT:      VarData.Set(xfs("%d", m_VarRaw )); break;
    case var_mngr::GLOBAL_TIMER:    VarData.Set(xfs("%g", *((f32*) &m_VarRaw))); break;
    case var_mngr::GLOBAL_BOOL:     
        if (*((xbool*) &m_VarRaw))
            VarData.Set("true"); 
        else
            VarData.Set("false"); 
        break;
    case var_mngr::GLOBAL_GUID:     VarData.Set(guid_ToString(m_VarGuid)); break;
    default:                        VarData.Set(xfs("???")); break;   
    }

    if (m_GlobalIndex == -1)
        GlbData.Set("???");
    else
        GlbData.Set(g_StringMgr.GetString(m_GlobalIndex));
    
    switch ( m_Code )
    {
    case CVAR_CODE_EQUAL:               Info.Set(xfs("Is %s = %s", GlbData.Get(), VarData.Get() ));          
        break;
    case CVAR_CODE_NOT_EQUAL:           Info.Set(xfs("Is %s != %s", GlbData.Get(), VarData.Get() ));   
        break;
    case CVAR_CODE_GREATER_INCLUSIVE:   Info.Set(xfs("Is %s >= %s", GlbData.Get(), VarData.Get() ));             
        break;
    case CVAR_CODE_LESSER_INCLUSIVE:    Info.Set(xfs("Is %s <= %s", GlbData.Get(), VarData.Get() ));           
        break;
    case CVAR_CODE_GREATER:             Info.Set(xfs("Is %s > %s", GlbData.Get(), VarData.Get() ));           
        break;
    case CVAR_CODE_LESSER:              Info.Set(xfs("Is %s < %s", GlbData.Get(), VarData.Get() ));            
        break;
    case INVALID_CVAR_CODES:            Info.Set("Check Unknown Global");                      
        break;
    }
    
    return Info.Get();
}
