///////////////////////////////////////////////////////////////////////////////
//
//  action_object_activation.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_object_activation.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Zonemgr\ZoneMgr.hpp"


static const xcolor s_ActivateColor          (0,255,0);
static const xcolor s_DeactivateColor        (255,0,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_object_activation::action_object_activation( guid ParentGuid ):  actions_ex_base(  ParentGuid ),
m_ActivateCode(CODE_ACTIVATE)
{
}

//=============================================================================

xbool action_object_activation::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        switch (m_ActivateCode)
        {
        case CODE_ACTIVATE:
            if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
            {
                g_ZoneMgr.TurnPortalOn( pObject->GetGuid() );
            }
            else
            {
                pObject->OnActivate(TRUE);
            }
            break;
        case CODE_DEACTIVATE:
            if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
            {
                g_ZoneMgr.TurnPortalOff( pObject->GetGuid() );
            }
            else
            {
                pObject->OnActivate(FALSE);
            }
            break;
        default:
            ASSERT(0);
            break;
        }
    }

    return TRUE;
}    
    

//=============================================================================

#ifndef X_RETAIL
void action_object_activation::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        xcolor Color;
        sml_string Info;
        switch (m_ActivateCode)
        {
        case CODE_ACTIVATE:
            Color = s_ActivateColor;
            Info.Set("Activate");
            break;
        case CODE_DEACTIVATE:
            Color = s_DeactivateColor;
            Info.Set("Deactivate");
            break;
        default:
            ASSERT(0);
            break;
        }

        draw_Line( GetPositionOwner(), pObject->GetPosition(), Color );
        draw_BBox( pObject->GetBBox(), Color );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), Color, xfs("[%d]%s", Index, Info.Get()) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), Color, xfs("[Else %d]%s", Index, Info.Get()) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_object_activation::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumInt ( "ActivateCode" ,     "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum( "Operation",         "Activate\0Deactivate\0", "Shall we activate or deactivate this object.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_object_activation::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "ActivateCode"  , m_ActivateCode ) )
        return TRUE;
    
    if ( rPropQuery.IsVar  ( "Operation" ) )
    {
        
        if( rPropQuery.IsRead() )
        {
            switch ( m_ActivateCode )
            {
            case CODE_ACTIVATE:     rPropQuery.SetVarEnum( "Activate" );      break;
            case CODE_DEACTIVATE:   rPropQuery.SetVarEnum( "Deactivate" );    break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Activate" )==0)        { m_ActivateCode = CODE_ACTIVATE;}
            if( x_stricmp( pString, "Deactivate" )==0)      { m_ActivateCode = CODE_DEACTIVATE;}
            
            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

const char* action_object_activation::GetDescription( void )
{
    static big_string   Info;

    switch (m_ActivateCode)
    {
    case CODE_ACTIVATE:
        Info.Set(xfs("Activate %s", m_ObjectAffecter.GetObjectInfo()));          
        break;
    case CODE_DEACTIVATE:
        Info.Set(xfs("Deactivate %s", m_ObjectAffecter.GetObjectInfo()));          
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
