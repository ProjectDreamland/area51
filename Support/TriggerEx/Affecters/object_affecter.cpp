///////////////////////////////////////////////////////////////////////////////
//
//  object_affecter.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "object_affecter.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "..\TriggerEx_Object.hpp"


big_string object_affecter::m_Description;

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

object_affecter::object_affecter( void ) :
m_GuidVarName(-1),
m_GuidVarHandle(HNULL),     
m_ObjectGuid(NULL), 
m_GuidCode(OBJECT_CODE_CHECK_BY_STATIC_GUID)
{
}

//=============================================================================

void object_affecter::OnEnumProp ( prop_enum& rPropList, const char* pPropName, u32 Flags )
{ 
    rPropList.PropEnumInt ( xfs("%sGCode", pPropName) ,     "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.PropEnumEnum( xfs("%s Selector", pPropName) ,     "Static Guid\0Variable Guid\0", "Are we checking a set guid or a global variable guid.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    if (m_GuidCode == OBJECT_CODE_CHECK_BY_STATIC_GUID )
        rPropList.PropEnumGuid(  xfs("%s Guid", pPropName), "The GUID of the object to act upon.", PROP_TYPE_MUST_ENUM | Flags );
    
    if (m_GuidCode == OBJECT_CODE_CHECK_BY_GLOBAL_GUID )
        rPropList.PropEnumExternal( xfs("%s Var", pPropName), "global\0global_guid", 
                                    "Name of the Global Guid Variable that you wish to affect.", PROP_TYPE_MUST_ENUM | Flags );
}

//=============================================================================

xbool object_affecter::OnProperty ( prop_query& rPropQuery, const char* pPropName )
{
    if ( rPropQuery.VarInt ( xfs("%sGCode", pPropName),  m_GuidCode ) )
        return TRUE;

    if ( rPropQuery.VarGUID( xfs("%s Guid", pPropName), m_ObjectGuid ) )
    {
        if( !rPropQuery.IsRead() )
        {        
            m_GuidCode = OBJECT_CODE_CHECK_BY_STATIC_GUID;
        }
        return TRUE;
    }
    
    if ( rPropQuery.IsVar  ( xfs("%s Var", pPropName) ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_GuidVarName >= 0 )
                rPropQuery.SetVarExternal( g_StringMgr.GetString(m_GuidVarName), 256 );
            else
                rPropQuery.SetVarExternal("", 256);
            return TRUE;
        }
        else
        {
            m_GuidCode = OBJECT_CODE_CHECK_BY_GLOBAL_GUID;
            if (x_strlen(rPropQuery.GetVarExternal()) > 0)
            {
                m_GuidVarName = g_StringMgr.Add( rPropQuery.GetVarExternal() );
                if ( m_GuidVarName == -1 )
                {
                    m_GuidVarHandle = HNULL;
                }
                else if ( !g_VarMgr.GetGuidHandle( g_StringMgr.GetString(m_GuidVarName), &m_GuidVarHandle ) )
                {
                    m_GuidVarHandle = HNULL;
                }
                return TRUE;
            }
        }
    }

    if ( rPropQuery.IsVar  ( xfs("%s Selector", pPropName) ) )
    {
        
        if( rPropQuery.IsRead() )
        {
            switch ( m_GuidCode )
            {
            case OBJECT_CODE_CHECK_BY_STATIC_GUID:     rPropQuery.SetVarEnum( "Static Guid" );   break;
            case OBJECT_CODE_CHECK_BY_GLOBAL_GUID:     rPropQuery.SetVarEnum( "Variable Guid" ); break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Static Guid" )==0)       { m_GuidCode = OBJECT_CODE_CHECK_BY_STATIC_GUID;}
            if( x_stricmp( pString, "Variable Guid" )==0)     { m_GuidCode = OBJECT_CODE_CHECK_BY_GLOBAL_GUID;}
            
            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

void object_affecter::SetStaticGuid( guid Guid )
{
    m_GuidCode = OBJECT_CODE_CHECK_BY_STATIC_GUID;
    m_ObjectGuid = Guid;
}

//=============================================================================

guid object_affecter::GetGuid( void )
{
    switch ( m_GuidCode )
    {
    case OBJECT_CODE_CHECK_BY_STATIC_GUID: 
        return m_ObjectGuid;
        break;
    case OBJECT_CODE_CHECK_BY_GLOBAL_GUID: 
        if ( m_GuidVarHandle != HNULL )
        {
            return g_VarMgr.GetGuid(m_GuidVarHandle);
        }           
        break;
    default:
        ASSERT(0);
        break;
    }    

    return 0;
}

//=============================================================================

object* object_affecter::GetObjectPtr( void )
{
    object* pObject = NULL;
    guid ObjectGuid = GetGuid();
    if ( ObjectGuid != 0 )
    {
        pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
    }

    return pObject;
}

//=============================================================================

const char* object_affecter::GetObjectInfo( void )
{
    m_Description.Set("???");

    switch ( m_GuidCode )
    {
    case OBJECT_CODE_CHECK_BY_STATIC_GUID: 
        if (m_ObjectGuid != 0)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_ObjectGuid );
            if (pObject)
            {
#ifdef X_EDITOR
                if (pObject->IsKindOf( trigger_ex_object::GetRTTI()))
                {
                    //special naming for triggers
                    trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                    m_Description.Set(pTrigger->GetTriggerName());
                    if (m_Description.IsEmpty())
                    {
                        m_Description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
                }
                else
#endif // X_EDITOR
                {
#ifdef X_EDITOR
                    m_Description.Set(pObject->GetName());
                    if (m_Description.IsEmpty())
                    {
                        m_Description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
#else
                    m_Description.Set(pObject->GetTypeDesc().GetTypeName());
#endif
                }
            }
        }
        break;
    case OBJECT_CODE_CHECK_BY_GLOBAL_GUID: 
        if ((m_GuidVarHandle != HNULL) && (m_GuidVarName != -1))
            m_Description.Set(xfs("[%s]", g_StringMgr.GetString(m_GuidVarName)));
        else
            m_Description.Set("Variable Object");           
        break;
    default:
        ASSERT(0);
        break;
    }

    return m_Description.Get();
}