///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_property.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_check_property.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

typedef enum_pair<condition_check_property::property_compare_modes> code_pair;

//=============================================================================

static code_pair s_CodeTableFull[] = 
{
        code_pair("=",      condition_check_property::PCOMP_CODE_EQUAL),
        code_pair("!=",     condition_check_property::PCOMP_CODE_NOT_EQUAL),
        code_pair(">=",     condition_check_property::PCOMP_CODE_GREATER_INCLUSIVE),
        code_pair("<=",     condition_check_property::PCOMP_CODE_LESSER_INCLUSIVE),
        code_pair(">",      condition_check_property::PCOMP_CODE_GREATER),
        code_pair("<",      condition_check_property::PCOMP_CODE_LESSER),
        code_pair( k_EnumEndStringConst,condition_check_property::INVALID_PCOMP_CODES),  //**MUST BE LAST**//
};

static code_pair s_CodeTableSmall[] = 
{
        code_pair("=",      condition_check_property::PCOMP_CODE_EQUAL),
        code_pair("!=",     condition_check_property::PCOMP_CODE_NOT_EQUAL),
        code_pair( k_EnumEndStringConst,condition_check_property::INVALID_PCOMP_CODES),  //**MUST BE LAST**//
};

enum_table<condition_check_property::property_compare_modes> condition_check_property::m_CodeTableFull( s_CodeTableFull   );      
enum_table<condition_check_property::property_compare_modes> condition_check_property::m_CodeTableSmall( s_CodeTableSmall );      

//=============================================================================

condition_check_property::condition_check_property( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_VarRaw(0),
m_RawString(-1),
m_Code(PCOMP_CODE_EQUAL),
m_PropertyName(-1),
m_PropertyType(PROP_TYPE_NULL),
m_ObjectType(-1)
{
}

//=============================================================================

xbool condition_check_property::Execute( guid TriggerGuid )
{   
    (void) TriggerGuid;

    if (m_PropertyName != -1)
    {
        object* pObject = m_ObjectAffecter.GetObjectPtr();
        if (pObject)
        {
            switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
            {
            case PROP_TYPE_ANGLE: 
                {
                    prop_query pqRead;
                    radian RadianVal = 0.0f;
                    pqRead.RQueryAngle( g_StringMgr.GetString(m_PropertyName), RadianVal);
                    if (pObject->OnProperty( pqRead ))
                    {
                        radian& RadianCheck = *((f32*) &m_VarRaw);

                        switch (m_Code)
                        {
                        case PCOMP_CODE_GREATER_INCLUSIVE:    return RadianVal >= RadianCheck;
                        case PCOMP_CODE_LESSER_INCLUSIVE:     return RadianVal <= RadianCheck;
                        case PCOMP_CODE_GREATER:              return RadianVal >  RadianCheck;
                        case PCOMP_CODE_LESSER:               return RadianVal <  RadianCheck;
                        case PCOMP_CODE_EQUAL:                return RadianVal == RadianCheck;
                        case PCOMP_CODE_NOT_EQUAL:            return RadianVal != RadianCheck;    
                        }
                    }
                }
                break;
            case PROP_TYPE_FLOAT: 
                {
                    prop_query pqRead;
                    f32 FloatVal = 0.0f;
                    pqRead.RQueryFloat( g_StringMgr.GetString(m_PropertyName), FloatVal);
                    if (pObject->OnProperty( pqRead ))
                    {
                        f32& FloatCheck = *((f32*) &m_VarRaw);

                        switch (m_Code)
                        {
                        case PCOMP_CODE_GREATER_INCLUSIVE:    return FloatVal >= FloatCheck;
                        case PCOMP_CODE_LESSER_INCLUSIVE:     return FloatVal <= FloatCheck;
                        case PCOMP_CODE_GREATER:              return FloatVal >  FloatCheck;
                        case PCOMP_CODE_LESSER:               return FloatVal <  FloatCheck;
                        case PCOMP_CODE_EQUAL:                return FloatVal == FloatCheck;
                        case PCOMP_CODE_NOT_EQUAL:            return FloatVal != FloatCheck;    
                        }
                    }
                }
                break;
            case PROP_TYPE_INT: 
                {
                    prop_query pqRead;
                    s32 IntVal = 0;
                    pqRead.RQueryInt( g_StringMgr.GetString(m_PropertyName), IntVal);
                    if (pObject->OnProperty( pqRead ))
                    {
                        s32& IntCheck = *((s32*) &m_VarRaw);

                        switch (m_Code)
                        {
                        case PCOMP_CODE_GREATER_INCLUSIVE:    return IntVal >= IntCheck;
                        case PCOMP_CODE_LESSER_INCLUSIVE:     return IntVal <= IntCheck;
                        case PCOMP_CODE_GREATER:              return IntVal >  IntCheck;
                        case PCOMP_CODE_LESSER:               return IntVal <  IntCheck;
                        case PCOMP_CODE_EQUAL:                return IntVal == IntCheck;
                        case PCOMP_CODE_NOT_EQUAL:            return IntVal != IntCheck;    
                        }
                    }
                }
                break;
            case PROP_TYPE_BOOL: 
                {
                    prop_query pqRead;
                    xbool BoolVal = FALSE;
                    pqRead.RQueryBool( g_StringMgr.GetString(m_PropertyName), BoolVal);
                    if (pObject->OnProperty( pqRead ))
                    {
                        xbool& BoolCheck = *((xbool*) &m_VarRaw);

                        switch (m_Code)
                        {
                        case PCOMP_CODE_EQUAL:                return BoolVal == BoolCheck;
                        case PCOMP_CODE_NOT_EQUAL:            return BoolVal != BoolCheck;    
                        }
                    }
                }
                break;
            case PROP_TYPE_ENUM: 
                {
                    if (m_RawString != -1)
                    {
                        prop_query pqRead;
                        big_string strData;
                        pqRead.RQueryEnum( g_StringMgr.GetString(m_PropertyName), strData.Get());
                        if (pObject->OnProperty( pqRead ))
                        {
                            switch (m_Code)
                            {
                            case PCOMP_CODE_EQUAL:                return (x_strcmp(g_StringMgr.GetString(m_RawString), strData.Get()) == 0);
                            case PCOMP_CODE_NOT_EQUAL:            return (x_strcmp(g_StringMgr.GetString(m_RawString), strData.Get()) != 0);
                            }
                        }
                    }
                }
                break;
            }   
        }
    }

    return FALSE; 
}    

//=============================================================================

void condition_check_property::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );
//    rPropList.AddInt        ( "PropType" , "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumString( "PropTypeS", "", PROP_TYPE_DONT_SHOW  );

    xbool bShowProp = FALSE;
    if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID)
    {
        rPropList.PropEnumExternal   ( "ObjType",    "object_picker\0object_picker", "What kind of object are you planning to modify. This is required for variable guid objects.", PROP_TYPE_MUST_ENUM );

        if (m_ObjectType != -1)
            bShowProp = TRUE;
    }
    else if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
    {
        if (m_ObjectAffecter.GetObjectPtr())
            bShowProp = TRUE;
    }

    if (bShowProp)
    {
        char PropTag[256];
        GetExternalPropTag(PropTag);
        rPropList.PropEnumExternal   ( "Property",   PropTag, "Select the property to modify", PROP_TYPE_MUST_ENUM);
    }

    rPropList.PropEnumInt        ( "Code" ,     "",  PROP_TYPE_DONT_SHOW  );

    xstring strEnum;
    switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_ANGLE: 
        rPropList.PropEnumEnum   ( "Operation",      m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumAngle  ( "Check Angle",    "Angle value.", 0 );
        break;
    case PROP_TYPE_FLOAT: 
        rPropList.PropEnumEnum   ( "Operation",      m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumFloat  ( "Check Float",    "Floating point value.", 0 );
        break;
    case PROP_TYPE_INT:     
        rPropList.PropEnumEnum   ( "Operation",      m_CodeTableFull.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumInt    ( "Check Int" ,     "Integer value.", 0 );
        break;
    case PROP_TYPE_BOOL:   
        rPropList.PropEnumEnum   ( "Operation",      m_CodeTableSmall.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumBool   ( "Check Bool",     "Check this property against this Boolean value.", 0 );
        break;
    case PROP_TYPE_ENUM:
        GetEnumForProperty(strEnum);
        rPropList.PropEnumEnum   ( "Operation",      m_CodeTableSmall.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumEnum   ( "Check Enum",     strEnum, "Check this property against this Enumeration.", 0 );
        break;
    }

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_check_property::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if (rPropQuery.IsVar("Property"))
    {
        if( rPropQuery.IsRead() )
        {
            if (m_PropertyName >= 0)
                rPropQuery.SetVarExternal( g_StringMgr.GetString(m_PropertyName), 256 );
            else
                rPropQuery.SetVarExternal( "", 256);
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarExternal()) > 0)
            {
                m_PropertyName = g_StringMgr.Add( rPropQuery.GetVarExternal() );
#ifdef X_EDITOR
                //only do this editor side
                SetPropertyType();
                m_Code = PCOMP_CODE_EQUAL;
#endif // X_EDITOR
            }
        }
        return TRUE;
    }

    if (rPropQuery.IsVar("ObjType"))
    {
        if( rPropQuery.IsRead() )
        {
            if (m_ObjectType >= 0)
                rPropQuery.SetVarExternal( g_StringMgr.GetString(m_ObjectType), 256 );
            else
                rPropQuery.SetVarExternal( "", 256);
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarExternal()) > 0)
            {
                m_ObjectType = g_StringMgr.Add( rPropQuery.GetVarExternal() );
            }
        }
        return TRUE;
    }
    
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;

    // for backwards compatibility
    if ( rPropQuery.VarInt ( "PropType" , m_PropertyType ) )
        return TRUE;

    if ( rPropQuery.IsVar ( "PropTypeS" ) )
    {
        if (rPropQuery.IsRead())
        {
            switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
            {
            case PROP_TYPE_ANGLE:
                rPropQuery.SetVarString("ANGLE",256);
                break;
            case PROP_TYPE_FLOAT: 
                rPropQuery.SetVarString("FLOAT",256);
                break;
            case PROP_TYPE_INT: 
                rPropQuery.SetVarString("INT",256);
                break;
            case PROP_TYPE_BOOL: 
                rPropQuery.SetVarString("BOOL",256);
                break;
            case PROP_TYPE_GUID: 
                rPropQuery.SetVarString("GUID",256);
                break;
            case PROP_TYPE_ENUM: 
                rPropQuery.SetVarString("ENUM",256);
                break;
            }   
        }
        else
        {
            if (x_strcmp(rPropQuery.GetVarString(),"ANGLE")==0)
            {
                m_PropertyType = PROP_TYPE_ANGLE;
            }
            else if (x_strcmp(rPropQuery.GetVarString(),"FLOAT")==0)
            {
                m_PropertyType = PROP_TYPE_FLOAT;
            }
            else if (x_strcmp(rPropQuery.GetVarString(),"INT")==0)
            {
                m_PropertyType = PROP_TYPE_INT;
            }
            else if (x_strcmp(rPropQuery.GetVarString(),"BOOL")==0)
            {
                m_PropertyType = PROP_TYPE_BOOL;
            }
            else if (x_strcmp(rPropQuery.GetVarString(),"GUID")==0)
            {
                m_PropertyType = PROP_TYPE_GUID;
            }
            else if (x_strcmp(rPropQuery.GetVarString(),"ENUM")==0)
            {
                m_PropertyType = PROP_TYPE_ENUM;
            }
        }
        return TRUE;
    }
    
    if ( rPropQuery.IsVar ( "Operation" ))
    {
        switch (m_PropertyType & PROP_TYPE_BASIC_MASK)
        {
        case PROP_TYPE_ANGLE:
        case PROP_TYPE_FLOAT: 
        case PROP_TYPE_INT:
            if ( SMP_UTIL_IsEnumVar<s32,property_compare_modes>(rPropQuery, "Operation", m_Code, m_CodeTableFull ) )
                return TRUE;
            break;
        case PROP_TYPE_BOOL: 
        case PROP_TYPE_ENUM: 
            if ( SMP_UTIL_IsEnumVar<s32,property_compare_modes>(rPropQuery, "Operation", m_Code, m_CodeTableSmall ) )
                return TRUE; break;
        default:
            break;   
        }
    }
    
    if (rPropQuery.VarAngle ( "Check Angle", *((f32*) &m_VarRaw) ))
        return TRUE; 

    if (rPropQuery.VarFloat ( "Check Float", *((f32*) &m_VarRaw) ))
        return TRUE; 
    
    if (rPropQuery.VarInt   ( "Check Int" , m_VarRaw ))
        return TRUE;

    if (rPropQuery.VarBool  ( "Check Bool" , *((xbool*) &m_VarRaw) ))
        return TRUE;

    if ( rPropQuery.IsVar( "Check Enum" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_RawString != -1 )
            {
                rPropQuery.SetVarEnum( g_StringMgr.GetString(m_RawString) );
            }
            else
            {
                rPropQuery.SetVarEnum( "" );
            }
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarEnum()) > 0)
            {
                m_RawString = g_StringMgr.Add( rPropQuery.GetVarEnum() );
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

//=============================================================================

void condition_check_property::GetExternalPropTag( char* pTag )
{
    s32     CurrentIndex    = 0;

    x_strcpy( (char*) &pTag[CurrentIndex], "prop_chk" );
    CurrentIndex += x_strlen("prop_chk");
    pTag[CurrentIndex] = 0;
    CurrentIndex++;

    x_strcpy( (char*) &pTag[CurrentIndex], "prop_chk\\" );
    CurrentIndex += x_strlen("prop_chk\\");

    if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID)
    {
        //variable guid
        if (m_ObjectType != -1)
        {
            s32 Len = x_strlen(g_StringMgr.GetString(m_ObjectType));
            if (Len > 0)
            {
                x_strcpy( (char*) &pTag[CurrentIndex], g_StringMgr.GetString(m_ObjectType) );
                CurrentIndex += Len;
                pTag[CurrentIndex] = 0;
                CurrentIndex++;
            }
        }
    }
    else if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
    {
        if (m_ObjectAffecter.GetObjectPtr())
        {
            //static guid
            const char* pTypeDesc = m_ObjectAffecter.GetObjectPtr()->GetTypeDesc().GetTypeName();
            x_strcpy( (char*) &pTag[CurrentIndex], pTypeDesc );
            CurrentIndex += x_strlen(pTypeDesc);
            pTag[CurrentIndex] = 0;
            CurrentIndex++;
        }
        else
        {
            ASSERT(FALSE);
        }
    }

    pTag[CurrentIndex] = 0;    
}

//=============================================================================

void condition_check_property::GetEnumForProperty( xstring& strEnum )
{
    if (m_PropertyName >= 0)
    {
        if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID)
        {
            if (m_ObjectType != -1)
            {
                guid ObjGuid = g_ObjMgr.CreateObject(g_StringMgr.GetString(m_ObjectType));
                object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
                prop_enum List;
                pObj->OnEnumProp(List);
                for (s32 i=0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];
                    if (x_strcmp(enData.GetName(), g_StringMgr.GetString(m_PropertyName)) ==0 )
                    {
                        //found it
                        for (s32 j=0; j < enData.GetEnumCount(); j++)
                        {
                            const char* pString = enData.GetEnumType(j);
                            strEnum += pString;
                            strEnum += '\0';
                        }
                        break;
                    }
                }
                g_ObjMgr.DestroyObject(ObjGuid);
            }
        }
        else if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
        {
            if (m_ObjectAffecter.GetObjectPtr())
            {
                prop_enum List;
                m_ObjectAffecter.GetObjectPtr()->OnEnumProp(List);
                for (s32 i=0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];
                    if (x_strcmp(enData.GetName(), g_StringMgr.GetString(m_PropertyName)) ==0 )
                    {
                        //found it
                        for (s32 j=0; j < enData.GetEnumCount(); j++)
                        {
                            const char* pString = enData.GetEnumType(j);
                            strEnum += pString;
                            strEnum += '\0';
                        }
                        break;
                    }
                }
            }
        }
    }

    // Make sure there are 2 NULLs at the end of the string if it's an empty string
    if (strEnum.GetLength() <= 0 )
    {
        strEnum += '\0';    
    }
    strEnum += '\0'; 
}

//=============================================================================

void condition_check_property::SetPropertyType( void )
{
    if (m_PropertyName == -1)
        return;

    if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID)
    {
        if (m_ObjectType != -1)
        {
            guid ObjGuid = g_ObjMgr.CreateObject(g_StringMgr.GetString(m_ObjectType));
            object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
            prop_enum List;
            if (pObj)
            {
                pObj->OnEnumProp(List);
                for (s32 i=0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];
                    if (x_strcmp(enData.GetName(), g_StringMgr.GetString(m_PropertyName)) ==0 )
                    {
                        //found it
                        m_PropertyType = enData.GetType();
                        break;
                    }
                }
                g_ObjMgr.DestroyObject(ObjGuid);
            }
        }
    }
    else if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
    {
        if (m_ObjectAffecter.GetObjectPtr())
        {
            prop_enum List;
            m_ObjectAffecter.GetObjectPtr()->OnEnumProp(List);
            for (s32 i=0; i < List.GetCount(); i++)
            {
                prop_enum::node& enData = List[i];
                if (x_strcmp(enData.GetName(), g_StringMgr.GetString(m_PropertyName)) ==0 )
                {
                    //found it
                    m_PropertyType = enData.GetType();
                    break;
                }
            }
        }
    }
}

//=============================================================================

const char* condition_check_property::GetDescription( void )
{
    static big_string   Info;
    static med_string   DataVal;
    
    switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_ANGLE:
        DataVal.Set(xfs("%g", RAD_TO_DEG(*((f32*) &m_VarRaw))));
        break;
    case PROP_TYPE_FLOAT: 
        DataVal.Set(xfs("%g", *((f32*) &m_VarRaw)));
        break;
    case PROP_TYPE_INT:     
        DataVal.Set(xfs("%d", m_VarRaw));
        break;
    case PROP_TYPE_BOOL:   
        if (*((xbool*) &m_VarRaw))
            DataVal.Set("true");
        else
            DataVal.Set("false");
        break;
    case PROP_TYPE_ENUM:
        if ( m_RawString != -1 )
            DataVal.Set( g_StringMgr.GetString(m_RawString) );
        else
            DataVal.Set( "???" );
        break;
    }

    if ((m_PropertyName != -1) && !DataVal.IsEmpty())
    {
        switch ( m_Code )
        {
        case PCOMP_CODE_EQUAL:               Info.Set(xfs("Is %s = %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));          
            break;
        case PCOMP_CODE_NOT_EQUAL:           Info.Set(xfs("Is %s != %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));   
            break;
        case PCOMP_CODE_GREATER_INCLUSIVE:   Info.Set(xfs("Is %s >= %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));             
            break;
        case PCOMP_CODE_LESSER_INCLUSIVE:    Info.Set(xfs("Is %s <= %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));           
            break;
        case PCOMP_CODE_GREATER:             Info.Set(xfs("Is %s > %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));           
            break;
        case PCOMP_CODE_LESSER:              Info.Set(xfs("Is %s < %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo() ));            
            break;
        case INVALID_PCOMP_CODES:            Info.Set("Check Unknown Property");                      
            break;
        }
    }
    else
    {
        Info.Set("Check Unknown Property");                                          
    }
    
    return Info.Get();
}

