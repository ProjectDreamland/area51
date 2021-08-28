///////////////////////////////////////////////////////////////////////////////
//
//  action_fade_geometry.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_fade_geometry.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\Render\RenderInst.hpp"

static const xcolor s_FadeInColor   (0,255,0);
static const xcolor s_FadeOutColor  (255,0,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_fade_geometry::action_fade_geometry( guid ParentGuid ):  actions_ex_base(  ParentGuid ),
m_FadeCode(CODE_FADE_IN),
m_FadeTime(3.0f)
{
}

//=============================================================================

xbool action_fade_geometry::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        render_inst* pRenderInst = pObject->GetRenderInstPtr();
        switch (m_FadeCode)
        {
        case CODE_FADE_IN:
            if ( pRenderInst )
                pRenderInst->StartFade( 1, m_FadeTime );
            break;

        case CODE_FADE_OUT:
            if ( pRenderInst )
                pRenderInst->StartFade( -1, m_FadeTime );
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
void action_fade_geometry::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        xcolor Color;
        sml_string Info;
        switch (m_FadeCode)
        {
        case CODE_FADE_IN:
            Color = s_FadeInColor;
            Info.Set("Fade in");
            break;
        case CODE_FADE_OUT:
            Color = s_FadeOutColor;
            Info.Set("Fade out");
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

void action_fade_geometry::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumInt  ( "FadeCode" , "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumFloat( "FadeTime",  "Time it takes for the object to fade in or out.", 0 );
    rPropList.PropEnumEnum ( "Operation", "FadeIn\0FadeOut\0", "Shall we fade in or fade out this object?", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_fade_geometry::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "FadeCode"  , m_FadeCode ) )
        return TRUE;

    if ( rPropQuery.VarFloat( "FadeTime" , m_FadeTime ) )
        return TRUE;

    if ( rPropQuery.IsVar  ( "Operation" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_FadeCode )
            {
            case CODE_FADE_IN:  rPropQuery.SetVarEnum( "FadeIn" );     break;
            case CODE_FADE_OUT: rPropQuery.SetVarEnum( "FadeOut" );    break;
            default:
                ASSERT(0);
                break;
            }

            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "FadeIn" )==0)      { m_FadeCode = CODE_FADE_IN;}
            if( x_stricmp( pString, "FadeOut" )==0)     { m_FadeCode = CODE_FADE_OUT;}

            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

const char* action_fade_geometry::GetDescription( void )
{
    static big_string   Info;

    switch (m_FadeCode)
    {
    case CODE_FADE_IN:
        Info.Set(xfs("Fade In %s", m_ObjectAffecter.GetObjectInfo()));          
        break;
    case CODE_FADE_OUT:
        Info.Set(xfs("Fade Out %s", m_ObjectAffecter.GetObjectInfo()));          
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
