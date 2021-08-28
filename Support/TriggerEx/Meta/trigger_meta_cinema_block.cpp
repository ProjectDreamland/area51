///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_cinema_block.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_cinema_block.hpp"
#include "..\TriggerEx_Object.hpp"
#include "..\..\Objects\Cinema.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_cinema_block::trigger_meta_cinema_block ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_CinemaGUID(0)
{
    m_BlockingMarker[0] = 0;
}

//=============================================================================

xbool trigger_meta_cinema_block::Execute ( f32 DeltaTime )
{
    (void)DeltaTime;
    object_ptr<cinema_object> CinemaObj(m_CinemaGUID) ;

    // Is it a cineam object?
    if (CinemaObj.IsValid())
    {
        return CinemaObj.m_pObject->IsPast( m_BlockingMarker );
    }
    // Opps, not a cinema object, so don't block.
    else
    {
        // Done blocking!
        return TRUE;
    }
}

//=============================================================================

void trigger_meta_cinema_block::OnEnumProp( prop_enum& I )
{
    char *pEnumString = "\0";

    I.PropEnumGuid( "Cinema GUID", "GUID of the cinema to block on", PROP_TYPE_MUST_ENUM );

#ifdef X_EDITOR
    object_ptr<cinema_object> CinemaObj(m_CinemaGUID) ;

    // Is it a cinema object?
    if (CinemaObj.IsValid())
    {
        pEnumString = *(CinemaObj.m_pObject->GetEnumString());
        if( pEnumString == NULL )
            pEnumString = "\0";
    }            
#endif

    I.PropEnumEnum( "Blocking Marker", pEnumString, "Name of blocking marker", PROP_TYPE_MUST_ENUM );

    trigger_meta_base::OnEnumProp( I );
}

//=============================================================================

xbool trigger_meta_cinema_block::OnProperty( prop_query& I )
{

    if( trigger_meta_base::OnProperty( I ) )
        return TRUE;

    if( I.VarGUID( "Cinema GUID", m_CinemaGUID ) )
    {
        return TRUE;
    }

    if( I.VarEnum( "Blocking Marker", m_BlockingMarker ) )
    {
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

const char* trigger_meta_cinema_block::GetDescription( void )
{
    static big_string Info;
    if( m_BlockingMarker[0] )
    {
        Info.Set( xfs("* BLOCK on Cinema(%s)", m_BlockingMarker ) );
        return Info.Get();
    }
    else
    {
        return "* BLOCK on Cinema(Undefined)";
    }
}
