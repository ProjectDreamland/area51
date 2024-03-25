///////////////////////////////////////////////////////////////////////////////
//
//  condition_within_range.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_within_range.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_within_range::condition_within_range( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_CheckCode(CODE_WITHIN_RANGE),
m_fDistance(400.0f)
{
}

//=============================================================================

xbool condition_within_range::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    object* pObject1 = m_Affecter1.GetObjectPtr();
    object* pObject2 = m_Affecter2.GetObjectPtr();
    
    if (!pObject1 || !pObject2)
        return FALSE;

    vector3 vDistance = pObject1->GetBBox().GetCenter() - pObject2->GetBBox().GetCenter();

    switch (m_CheckCode)
    {
    case CODE_WITHIN_RANGE:
        if( vDistance.LengthSquared() < (m_fDistance*m_fDistance) ) 
            return TRUE;
        break;
    case CODE_OUT_OF_RANGE:
        if( vDistance.LengthSquared() > (m_fDistance*m_fDistance) ) 
            return TRUE;
        break;
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void condition_within_range::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_Affecter1.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumInt  ( "CheckCode" ,        "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum ( "Operation",         "In Range\0Out of Range\0", "Are we checking whether the object is in range of another object, or out of range.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumFloat( "Distance",          "What range to compare? (based on bbox centers)", 0 );

    m_Affecter2.OnEnumProp( rPropList, "Target" );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_within_range::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_Affecter1.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( m_Affecter2.OnProperty( rPropQuery, "Target" ) )
        return TRUE;

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "CheckCode"  , m_CheckCode ) )
        return TRUE;
    
    if ( rPropQuery.VarFloat ( "Distance"  , m_fDistance ) )
        return TRUE;

    if ( rPropQuery.IsVar  ( "Operation" ) )
    {
        
        if( rPropQuery.IsRead() )
        {
            switch ( m_CheckCode )
            {
            case CODE_WITHIN_RANGE:     rPropQuery.SetVarEnum( "In Range" );      break;
            case CODE_OUT_OF_RANGE:     rPropQuery.SetVarEnum( "Out of Range" );  break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "In Range" )==0)       { m_CheckCode = CODE_WITHIN_RANGE;}
            if( x_stricmp( pString, "Out of Range" )==0)   { m_CheckCode = CODE_OUT_OF_RANGE;}
            
            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

#ifndef X_RETAIL
void condition_within_range::OnDebugRender ( s32 Index )
{
    object* pObject1 = m_Affecter1.GetObjectPtr();
    object* pObject2 = m_Affecter2.GetObjectPtr();
    if (pObject2)
    {
        sml_string Info;
        switch (m_CheckCode)
        {
        case CODE_WITHIN_RANGE:
            Info.Set("Within Range");
            break;
        case CODE_OUT_OF_RANGE:
            Info.Set("Out of Range");
            break;
        default:
            ASSERT(0);
            break;
        }

        if (pObject1)
        {
            draw_Line( pObject1->GetBBox().GetCenter(), pObject2->GetBBox().GetCenter(), XCOLOR_PURPLE );
            draw_BBox( pObject1->GetBBox(), XCOLOR_PURPLE );
        }

        draw_Sphere(pObject2->GetBBox().GetCenter(), m_fDistance, XCOLOR_PURPLE);

        if (!GetElse())
        {
            draw_Label( pObject2->GetPosition(), XCOLOR_PURPLE, xfs("[If %d]%s", Index, Info.Get()) );
        }
        else
        {
            draw_Label( pObject2->GetPosition(), XCOLOR_PURPLE, xfs("[Else If %d]%s", Index, Info.Get()) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

const char* condition_within_range::GetDescription( void )
{
    static big_string   Info;
    static med_string   TargetData;
    TargetData.Set(m_Affecter2.GetObjectInfo());

    switch ( m_CheckCode )
    {
    case CODE_WITHIN_RANGE:     
        Info.Set(xfs("If %s within %g cm of %s", m_Affecter1.GetObjectInfo(), m_fDistance, TargetData.Get()));          
        break;
    case CODE_OUT_OF_RANGE: 
        Info.Set(xfs("If %s is %g cm away from %s", m_Affecter1.GetObjectInfo(), m_fDistance, TargetData.Get())); 
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
