///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_nav_activation.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_nav_activation.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Navigation\Nav_Map.hpp"

#ifdef X_EDITOR
#include "..\..\Apps\WorldEditor\nav_connection2_editor.hpp"
#endif // X_EDITOR

static const xcolor s_ActivateColor          (0,255,0);
static const xcolor s_DeactivateColor        (255,0,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_nav_activation::action_ai_nav_activation( guid ParentGuid ):  actions_ex_base(  ParentGuid ),
m_ActivateCode(CODE_ACTIVATE)
{
}

//=============================================================================

xbool action_ai_nav_activation::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    if ( m_NavConnectionGuid == 0 )
        return TRUE;

    switch (m_ActivateCode)
    {
    case CODE_ACTIVATE:
        g_NavMap.SetConnectionStatus( m_NavConnectionGuid, TRUE );
        break;
    case CODE_DEACTIVATE:
        g_NavMap.SetConnectionStatus( m_NavConnectionGuid, FALSE );
        break;
    default:
        ASSERT(0);
        break;
    }
    return TRUE;
}    

//=============================================================================

#ifndef X_RETAIL
void action_ai_nav_activation::OnDebugRender ( s32 Index )
{
    (void)Index;
#ifdef X_EDITOR
    object* pObj = g_ObjMgr.GetObjectByGuid( m_NavConnectionGuid );
    if (pObj)
    {
        if (pObj->IsKindOf(nav_connection2_editor::GetRTTI() ) )
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

            draw_Line( GetPositionOwner(), pObj->GetPosition(), Color );
            draw_BBox( pObj->GetBBox(), Color );

            if (!GetElse())
            {
                draw_Label( pObj->GetPosition(), Color, xfs("[%d]%s", Index, Info.Get()) );
            }
            else
            {
                draw_Label( pObj->GetPosition(), Color, xfs("[Else %d]%s", Index, Info.Get()) );
            }
        }
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//=============================================================================

void action_ai_nav_activation::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumGuid( "Nav Connection",    "Guid of nav connection to be controller", 0 );
    rPropList.PropEnumInt ( "ActivateCode" ,     "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum( "Operation",         "Activate\0Deactivate\0", "Shall we activate or deactivate this object.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_ai_nav_activation::OnProperty ( prop_query& rPropQuery )
{
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.IsVar( "Nav Connection" ))
    {
        if (rPropQuery.IsRead())
        {
            rPropQuery.SetVarGUID( m_NavConnectionGuid );
        }
        else
        {
            guid Guid = rPropQuery.GetVarGUID();
            m_NavConnectionGuid = Guid;
            /*
            object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
            if (pObj)
            {
                if (pObj->IsKindOf(nav_connection2_editor::GetRTTI() ) )
                {   
                    m_NavConnectionGuid = Guid;
                }
            }*/
        }

        return TRUE;
    }

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

const char* action_ai_nav_activation::GetDescription( void )
{
    static big_string   Info;

    switch (m_ActivateCode)
    {
    case CODE_ACTIVATE:
        Info.Set(xfs("Activate nav connection"));
        break;
    case CODE_DEACTIVATE:
        Info.Set(xfs("Deactivate nav connection"));
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
