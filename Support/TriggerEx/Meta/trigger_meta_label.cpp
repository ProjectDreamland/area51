///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_label.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_label.hpp"
#include "..\TriggerEx_Object.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_label::trigger_meta_label ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_Label(-1)
{
}

//=============================================================================

xbool trigger_meta_label::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    return TRUE;
}

//=============================================================================

void trigger_meta_label::OnEnumProp	( prop_enum& rPropList )
{
    rPropList.PropEnumString	 ( "Label", "Name of this label.", PROP_TYPE_MUST_ENUM );

    trigger_meta_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool trigger_meta_label::OnProperty	( prop_query& rPropQuery )
{
    if( trigger_meta_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.IsVar  ( "Label" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_Label >= 0 )
                rPropQuery.SetVarString( g_StringMgr.GetString(m_Label), 256 );
            else
                rPropQuery.SetVarString("", 256);
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarString()) > 0)
            {
                m_Label = g_StringMgr.Add( rPropQuery.GetVarString() );
                return TRUE;
            }
        }
    }

    return FALSE;
}

//=============================================================================

const char* trigger_meta_label::GetDescription( void )
{
    static big_string   Info;
    Info.Set(xfs("* LABEL::%s",GetLabel()));
    return Info.Get();
}


