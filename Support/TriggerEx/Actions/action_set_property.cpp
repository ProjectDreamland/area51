///////////////////////////////////////////////////////////////////////////
//
//  action_set_property.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_set_property.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Entropy.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Objects\Group.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_set_property::action_set_property ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_RawString(-1),
m_Code(PMOD_CODE_SET),
m_PropertyName(-1),
m_PropertyType(PROP_TYPE_NULL),
m_ObjectType(-1)
{
    x_memset(&m_VarRaw, 0, sizeof(m_VarRaw)) ;
}

//=============================================================================

const char* action_set_property::GetPropertyName ( void )
{
    if( m_PropertyName == -1 )
        return "";
    else
        return g_StringMgr.GetString( m_PropertyName );        
}

//=============================================================================

xbool action_set_property::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if (m_PropertyName != -1)
    {
        object** pObjectArray;
        s32 nObjects = BuildObjectList( pObjectArray );
        s32 iCurObject = 0;        
        xbool bRet = FALSE;

//TODO: HANDLE RETURN, BREAK, and ERROR CASES

        while (iCurObject < nObjects)
        {
            object* pObject = pObjectArray[iCurObject];
            iCurObject++;

            if (NULL == pObject)
                continue;

            switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
            {
            case PROP_TYPE_ANGLE: 
                {
                    prop_query pqRead;
                    radian RadianVal = 0.0f;
                    pqRead.RQueryAngle( g_StringMgr.GetString(m_PropertyName), RadianVal);
                    if (pObject->OnProperty( pqRead ))
                    {
                        radian& RadianMod = m_VarRaw.m_Float ;

                        switch (m_Code)
                        {
                        case PMOD_CODE_ADD:           RadianVal += RadianMod; break;
                        case PMOD_CODE_SUBTRACT:      RadianVal -= RadianMod; break;
                        case PMOD_CODE_SET:           RadianVal =  RadianMod; break;
                        default:                    
                            m_bErrorInExecute = TRUE;
                            return (!RetryOnError());
                            break;
                        }

                        prop_query pqWrite;
                        pqWrite.WQueryAngle(g_StringMgr.GetString(m_PropertyName), RadianVal);
                        if (pObject->OnProperty( pqWrite ))
                        {
                            return TRUE;
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
                        f32& FloatMod = m_VarRaw.m_Float ;
    
                        switch (m_Code)
                        {
                        case PMOD_CODE_ADD:           FloatVal += FloatMod; break;
                        case PMOD_CODE_SUBTRACT:      FloatVal -= FloatMod; break;
                        case PMOD_CODE_SET:           FloatVal =  FloatMod; break;
                        default:                    
                            m_bErrorInExecute = TRUE;
                            return (!RetryOnError());
                            break;
                        }

                        prop_query pqWrite;
                        pqWrite.WQueryFloat(g_StringMgr.GetString(m_PropertyName), FloatVal);
                        if (pObject->OnProperty( pqWrite ))
                        {
                            bRet = TRUE;
                            continue;
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
                        s32& IntMod = m_VarRaw.m_Integer ;
    
                        switch (m_Code)
                        {
                        case PMOD_CODE_ADD:           IntVal += IntMod; break;
                        case PMOD_CODE_SUBTRACT:      IntVal -= IntMod; break;
                        case PMOD_CODE_SET:           IntVal =  IntMod; break;
                        default:                    
                            m_bErrorInExecute = TRUE;
                            return (!RetryOnError());
                            break;
                        }

                        prop_query pqWrite;
                        pqWrite.WQueryInt(g_StringMgr.GetString(m_PropertyName), IntVal);
                        if (pObject->OnProperty( pqWrite ))
                        {
                            bRet = TRUE;
                            continue;
                        }
                    }
                }
                break;
            case PROP_TYPE_BOOL: 
                {
                    xbool& BoolValue = m_VarRaw.m_Bool ;
                    prop_query pqWrite;
                    pqWrite.WQueryBool(g_StringMgr.GetString(m_PropertyName), BoolValue);
                    if (pObject->OnProperty( pqWrite ))
                    {
                        bRet = TRUE;
                        continue;
                    }
                }
                break;
            case PROP_TYPE_GUID: 
                {
                    guid GuidValue = m_GuidVar.GetGuid();
                    prop_query pqWrite;
                    pqWrite.WQueryGUID(g_StringMgr.GetString(m_PropertyName), GuidValue);
                    if (pObject->OnProperty( pqWrite ))
                    {
                        bRet = TRUE;
                        continue;
                    }
                }
                break;
            case PROP_TYPE_ENUM: 
                {
                    if (m_RawString != -1)
                    {
                        prop_query pqWrite;
                        pqWrite.WQueryEnum(g_StringMgr.GetString(m_PropertyName), g_StringMgr.GetString(m_RawString));
                        if (pObject->OnProperty( pqWrite ))
                        {
                            bRet = TRUE;
                            continue;
                        }
                    }
                }
                break;
            case PROP_TYPE_BUTTON:
                {
                    prop_query pqWrite;
                    pqWrite.WQueryButton( g_StringMgr.GetString(m_PropertyName), g_StringMgr.GetString(m_PropertyName) );
                    if (pObject->OnProperty( pqWrite ))
                    {
                        bRet = TRUE;
                        continue;
                    }
                }
                break;
            }  
        }

        if (bRet)
            return TRUE;
    }

    //failure
    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

#ifndef X_RETAIL
void action_set_property::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        xcolor Color = xcolor(128,0,255);

        draw_Line( GetPositionOwner(), pObject->GetPosition(), Color );
        draw_BBox( pObject->GetBBox(), Color );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), Color, xfs("[%d]Change Property", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), Color, xfs("[Else %d]Change Property", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_set_property::OnEnumProp	( prop_enum& rPropList )
{
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );
//    rPropList.AddInt        ( "PropType" , "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumString( "PropTypeS", "", PROP_TYPE_DONT_SHOW  );

    xbool bShowProp = FALSE;
    if (RequiresObjectType())
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
        rPropList.PropEnumEnum   ( "Operation",  "Add\0Subtract\0Set To\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumAngle  ( "Angle Op",   "Angle value.", 0 );
        break;
    case PROP_TYPE_FLOAT: 
        rPropList.PropEnumEnum   ( "Operation",  "Add\0Subtract\0Set To\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumFloat  ( "Float Op",   "Floating point value.", 0 );
        break;
    case PROP_TYPE_INT:     
        rPropList.PropEnumEnum   ( "Operation",  "Add\0Subtract\0Set To\0", "Options available to this action.", PROP_TYPE_MUST_ENUM  );
        rPropList.PropEnumInt    ( "Int Op" ,    "Integer value.", 0 );
        break;
    case PROP_TYPE_BOOL:   
        rPropList.PropEnumBool   ( "Set Bool",   "Set this property to this Boolean value.", 0  );
        break;
    case PROP_TYPE_GUID:   
        // Since this prop was never exposed via PROP_TYPE_EXPOSE, we don't have to 
        // worry about breaking any scripting.
        //rPropList.AddGuid   ( "Set Guid",   "Set this property to this Guid value."  );
        m_GuidVar.OnEnumProp( rPropList, "Source" );        
        break;
    case PROP_TYPE_ENUM:
        GetEnumForProperty(strEnum);
        rPropList.PropEnumEnum   ( "Set Enum",   strEnum, "Set this property to this Enumeration.", 0  );
        break;
    case PROP_TYPE_BUTTON:
        break;
    }
    
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_set_property::OnProperty	( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
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
                m_Code = PMOD_CODE_SET;
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
            case PROP_TYPE_BUTTON: 
                rPropQuery.SetVarString("BTN",256);
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
            else if (x_strcmp(rPropQuery.GetVarString(),"BTN")==0)
            {
                m_PropertyType = PROP_TYPE_BUTTON;
            }

        }
        return TRUE;
    }

    if ( rPropQuery.IsVar( "Operation" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case PMOD_CODE_ADD:          rPropQuery.SetVarEnum( "Add" );            break;
            case PMOD_CODE_SUBTRACT:     rPropQuery.SetVarEnum( "Subtract" );       break;
            case PMOD_CODE_SET:          rPropQuery.SetVarEnum( "Set To" );         break;
            default:                     rPropQuery.SetVarEnum( "INVALID" );        break;
            }
            
            return TRUE;
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();

            if( x_stricmp( pString, "Add"              )==0)    { m_Code = PMOD_CODE_ADD;}
            else if( x_stricmp( pString, "Subtract"    )==0)    { m_Code = PMOD_CODE_SUBTRACT;}
            else if( x_stricmp( pString, "Set To"      )==0)    { m_Code = PMOD_CODE_SET;}
            
            return TRUE;
        }
    }
    
    if (rPropQuery.VarAngle ( "Angle Op" , m_VarRaw.m_Float) )
        return TRUE;

    if (rPropQuery.VarFloat ( "Float Op", m_VarRaw.m_Float) )
        return TRUE; 
    
    if (rPropQuery.VarInt ( "Int Op" , m_VarRaw.m_Integer ) )
        return TRUE;

    if (rPropQuery.VarBool ( "Set Bool" , m_VarRaw.m_Bool) )
        return TRUE;
    
    //if (rPropQuery.VarGUID ( "Set Guid" , *(guid*)&m_VarRaw.m_Guid ))
    //        return TRUE;

    if (m_GuidVar.OnProperty( rPropQuery, "Source" ))
    {
        return TRUE;
    }
    if (rPropQuery.IsVar( "Set Guid" ))
    {
        // Handle old guid var
        if (rPropQuery.IsRead())
        {
            rPropQuery.SetVarGUID( m_GuidVar.GetGuid() );
        }
        else
        {
            m_GuidVar.SetStaticGuid( rPropQuery.GetVarGUID() );
        }
        return TRUE;
    }


    if ( rPropQuery.IsVar( "Set Enum" ) )
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

void action_set_property::GetExternalPropTag( char* pTag )
{
    s32     CurrentIndex    = 0;

    x_strcpy( (char*) &pTag[CurrentIndex], "prop_mod" );
    CurrentIndex += x_strlen("prop_mod");
    pTag[CurrentIndex] = 0;
    CurrentIndex++;

    x_strcpy( (char*) &pTag[CurrentIndex], "prop_mod\\" );
    CurrentIndex += x_strlen("prop_mod\\");

    if (RequiresObjectType())
    {
        //variable guid or group object
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

void action_set_property::GetEnumForProperty( xstring& strEnum )
{
    if (m_PropertyName >= 0)
    {
        if (RequiresObjectType())
        {
            if (m_ObjectType != -1)
            {
                guid ObjGuid = g_ObjMgr.CreateObject(g_StringMgr.GetString(m_ObjectType));
                object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);

                // Count the number of properties
                prop_enum_counter Counter;
                pObj->OnEnumProp( Counter);

                // Now enumerate the properties
                prop_enum List;
                List.SetCapacity( Counter.GetCount() );
                pObj->OnEnumProp( List );

                // Iterate over the properties
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
                g_ObjMgr.DestroyObjectEx( ObjGuid, TRUE );
            }
        }
        else if (m_ObjectAffecter.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
        {
            if (m_ObjectAffecter.GetObjectPtr())
            {
                // Count the number of properties
                prop_enum_counter Counter;
                m_ObjectAffecter.GetObjectPtr()->OnEnumProp( Counter );

                // Now enumerate the properties
                prop_enum List;
                List.SetCapacity( Counter.GetCount() );
                m_ObjectAffecter.GetObjectPtr()->OnEnumProp(List);

                // Iterate over the properties
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

void action_set_property::SetPropertyType( void )
{
    if (m_PropertyName == -1)
        return;

    if (RequiresObjectType())
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
                g_ObjMgr.DestroyObjectEx(ObjGuid, TRUE);
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

const char* action_set_property::GetDescription( void )
{
    static big_string   Info;
    static med_string   DataVal;
    
    switch ( m_PropertyType & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_ANGLE:
        DataVal.Set(xfs("%g", RAD_TO_DEG(m_VarRaw.m_Float)));
        break;
    case PROP_TYPE_FLOAT: 
        DataVal.Set(xfs("%g", m_VarRaw.m_Float));
        break;
    case PROP_TYPE_INT:     
        DataVal.Set(xfs("%d", m_VarRaw.m_Integer));
        break;
    case PROP_TYPE_BOOL:   
        if (m_VarRaw.m_Bool)
            DataVal.Set("true");
        else
            DataVal.Set("false");
        break;
    case PROP_TYPE_GUID:     
        //DataVal.Set(guid_ToString(m_VarRaw.m_Guid)) ;
        DataVal.Set(guid_ToString(m_GuidVar.GetGuid()));
        break;
    case PROP_TYPE_ENUM:
        if ( m_RawString != -1 )
            DataVal.Set( g_StringMgr.GetString(m_RawString) );
        else
            DataVal.Set( "???" );
        break;
    case PROP_TYPE_BUTTON:     
        DataVal.Set("Clicked");
        break;
    }

    if ((m_PropertyName != -1) && !DataVal.IsEmpty())
    {
        switch ( m_Code )
        {
        case PMOD_CODE_ADD:          
            Info.Set(xfs("Add %s to \"%s\" on %s", DataVal.Get(), g_StringMgr.GetString(m_PropertyName), m_ObjectAffecter.GetObjectInfo()));          
            break;
        case PMOD_CODE_SUBTRACT:     
            Info.Set(xfs("Subtract %s from \"%s\" on %s", DataVal.Get(), g_StringMgr.GetString(m_PropertyName), m_ObjectAffecter.GetObjectInfo()));   
            break;
        case PMOD_CODE_SET:          
            Info.Set(xfs("Set \"%s\" to %s on %s", g_StringMgr.GetString(m_PropertyName), DataVal.Get(), m_ObjectAffecter.GetObjectInfo()));          
            break;
        case INVALID_PMOD_CODES:     
            Info.Set("Set ??? Property");                                          
            break;
        }
    }
    else
    {
        Info.Set("Set Unknown Property");                                          
    }
    
    return Info.Get();
}

//=============================================================================

xbool action_set_property::RequiresObjectType( void )
{
    xbool bRet = FALSE;

    if (m_ObjectAffecter.GetObjectCode()  == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID)
    {
        bRet = TRUE;
    }
    
    guid gObj = m_ObjectAffecter.GetGuid();
    if (NULL_GUID != gObj)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( group::GetRTTI() ))
            {
                bRet = TRUE;
            }
        }
    }

    return bRet;
}

//=============================================================================

s32 action_set_property::BuildObjectList( object**& ppObjectArray )
{
    xbool bGatherFromGroup = FALSE;

    // We will gather object pointers from a group iff:
    // 1) the source object is indeed a group object
    // 2) the object type is not a group object (indicating a specific object type contained with the group)

    guid gObj = m_ObjectAffecter.GetGuid();
    if (NULL_GUID != gObj)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( group::GetRTTI() ))
            {
                // Source object is a group object.
                // Check the manually specified type now.
                if (m_ObjectType != -1)
                {
                    const char* pTypeName = g_StringMgr.GetString(m_ObjectType);
                    if (x_stricmp(pTypeName,"Group")!=0)
                    {
                        // We are referring to the objects within a group
                        bGatherFromGroup = TRUE;

                        group& Group = group::GetSafeType( *pObj );

                        // Rather than iterate twice to count and build, we will
                        // allocate the array large enough for all the children in the group
                        s32 nSourceObjects = Group.GetNumChildren();

                        ppObjectArray = (object**)smem_BufferAlloc( sizeof(object*) * nSourceObjects );
                        ASSERT( ppObjectArray );
                        if (NULL == ppObjectArray)
                            return 0;                

                        s32 nOutputObjects = 0;                       
                        s32 i;
                        for (i=0;i<nSourceObjects;i++)
                        {
                            guid gChild = Group.GetChild( i );
                            if (NULL_GUID == gChild)
                                continue;

                            object* pObj = g_ObjMgr.GetObjectByGuid( gChild );
                            if (NULL == pObj)
                                continue;

                            ppObjectArray[ nOutputObjects ] = pObj;
                            nOutputObjects++;
                        }

                        return nOutputObjects;
                    }
                }
            }
        }

        ppObjectArray = (object**)smem_BufferAlloc( sizeof(object*) * 1 );
        ASSERT( ppObjectArray );
        if (NULL == ppObjectArray)
            return 0;

        ppObjectArray[0] = pObj;
        return 1;
    }

    return 0;
}

//=============================================================================

#ifdef X_EDITOR

object_affecter* action_set_property::GetObjectRef0   ( xstring& Desc )
{
    object_ptr<group> pGroup( m_ObjectAffecter.GetGuid() );

    if (pGroup)
    {
        Desc = "Object error: "; 
        return NULL;        
    }
     
    Desc = "Object error: "; 
    return &m_ObjectAffecter;
}

//=============================================================================

s32* action_set_property::GetPropertyRef  ( xstring& Desc, s32& PropertyType ) 
{ 
    Desc = "Property error: "; PropertyType = m_PropertyType; return &m_PropertyName;  
}

#endif // X_EDITOR

//=============================================================================

//=============================================================================