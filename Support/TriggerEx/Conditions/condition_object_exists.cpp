///////////////////////////////////////////////////////////////////////////////
//
//  condition_object_exists.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_object_exists.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_object_exists::condition_object_exists( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_CheckCode(CODE_DOES_OBJECT_EXIST)
{
}

//=============================================================================

xbool condition_object_exists::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    object* pObject = m_Affecter.GetObjectPtr();
    switch (m_CheckCode)
    {
    case CODE_DOES_OBJECT_EXIST:
        if ( pObject != NULL )
            return TRUE;
        return FALSE;
        break;
    case CODE_DOES_OBJECT_NOT_EXIST:
        if ( pObject == NULL )
            return TRUE;
        break;
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void condition_object_exists::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_Affecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumInt ( "CheckCode" ,        "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum( "Operation",         "Does Exist\0Does NOT exist\0", "Are we checking whether the object exists or does not exist.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_object_exists::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_Affecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "CheckCode"  , m_CheckCode ) )
        return TRUE;
    
    if ( rPropQuery.IsVar  ( "Operation" ) )
    {
        
        if( rPropQuery.IsRead() )
        {
            switch ( m_CheckCode )
            {
            case CODE_DOES_OBJECT_EXIST:     rPropQuery.SetVarEnum( "Does Exist" );      break;
            case CODE_DOES_OBJECT_NOT_EXIST: rPropQuery.SetVarEnum( "Does NOT exist" );  break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Does Exist" )==0)       { m_CheckCode = CODE_DOES_OBJECT_EXIST;}
            if( x_stricmp( pString, "Does NOT exist" )==0)   { m_CheckCode = CODE_DOES_OBJECT_NOT_EXIST;}
            
            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

#ifndef X_RETAIL
void condition_object_exists::OnDebugRender ( s32 Index )
{
    object* pObject = m_Affecter.GetObjectPtr();
    if (pObject)
    {
        sml_string Info;
        switch (m_CheckCode)
        {
        case CODE_DOES_OBJECT_EXIST:
            Info.Set("Exists");
            break;
        case CODE_DOES_OBJECT_NOT_EXIST:
            Info.Set("Doesn't Exist");
            break;
        default:
            ASSERT(0);
            break;
        }

        draw_BBox( pObject->GetBBox(), XCOLOR_PURPLE );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[If %d]%s", Index, Info.Get()) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[Else If %d]%s", Index, Info.Get()) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

const char* condition_object_exists::GetDescription( void )
{
    static big_string   Info;

    switch ( m_CheckCode )
    {
    case CODE_DOES_OBJECT_EXIST:     
        Info.Set(xfs("If %s Exists", m_Affecter.GetObjectInfo()));          
        break;
    case CODE_DOES_OBJECT_NOT_EXIST: 
        Info.Set(xfs("If %s Does Not Exist", m_Affecter.GetObjectInfo())); 
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
