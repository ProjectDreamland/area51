///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_health.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_check_health.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"


//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_check_health::condition_check_health( conditional_affecter* pParent ):
    conditional_ex_base(  pParent ),
    m_CheckCode( CODE_IS_OBJECT_DEAD )
{
}

//=============================================================================

xbool condition_check_health::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;
    
    object* pObject = m_Affecter.GetObjectPtr();
    switch( m_CheckCode )
    {
    case CODE_IS_OBJECT_DEAD:

        // If the object no longer exists then it must be dead
        if( pObject == NULL )
        {
            return TRUE;
        }            
        else if( pObject->IsAlive() )
        {        
            return FALSE;
        }            
        else
        {
            return TRUE;
        }            
        break;            
    
    case CODE_IS_OBJECT_ALIVE:
        
        // If the object no longer exists then it must be dead
        if( pObject == NULL )
        {
            return FALSE;
        }            
        else if( pObject->IsAlive() )
        {        
            return TRUE;
        }            
        else
        {
            return FALSE;
        }            
        break;            
        
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void condition_check_health::OnEnumProp ( prop_enum& List )
{ 
    //guid specific fields
    m_Affecter.OnEnumProp( List, "Object" );

    List.PropEnumInt ( "CheckCode" , "",  PROP_TYPE_DONT_SHOW  );
    List.PropEnumEnum( "Operation",  "Is Dead\0Is Alive\0", "Should we check if the object is dead or alive?", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    conditional_ex_base::OnEnumProp( List );
}

//=============================================================================

xbool condition_check_health::OnProperty ( prop_query& I )
{
    //guid specific fields
    if( m_Affecter.OnProperty( I, "Object" ) )
        return TRUE;

    if( conditional_ex_base::OnProperty( I ) )
        return TRUE;

    if ( I.VarInt ( "CheckCode"  , m_CheckCode ) )
        return TRUE;
    
    if ( I.IsVar  ( "Operation" ) )
    {
        if( I.IsRead() )
        {
            switch ( m_CheckCode )
            {
            case CODE_IS_OBJECT_DEAD:   I.SetVarEnum( "Is Dead"  ); break;
            case CODE_IS_OBJECT_ALIVE:  I.SetVarEnum( "Is Alive" ); break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = I.GetVarEnum();
            if( x_stricmp( pString, "Is Dead"  ) == 0 ) { m_CheckCode = CODE_IS_OBJECT_DEAD;  }
            if( x_stricmp( pString, "Is Alive" ) == 0 ) { m_CheckCode = CODE_IS_OBJECT_ALIVE; }
            
            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

#ifndef X_RETAIL
void condition_check_health::OnDebugRender ( s32 Index )
{
    object* pObject = m_Affecter.GetObjectPtr();
    if (pObject)
    {
        sml_string Info;
        switch (m_CheckCode)
        {
        case CODE_IS_OBJECT_DEAD:   Info.Set( "Is Dead" );  break;
        case CODE_IS_OBJECT_ALIVE:  Info.Set( "Is Alive" ); break;
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

const char* condition_check_health::GetDescription( void )
{
    static big_string   Info;

    switch ( m_CheckCode )
    {
    case CODE_IS_OBJECT_DEAD:     
        Info.Set( xfs( "If %s Is Dead", m_Affecter.GetObjectInfo() ) );          
        break;
    case CODE_IS_OBJECT_ALIVE: 
        Info.Set( xfs( "If %s Is Alive", m_Affecter.GetObjectInfo() ) ); 
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}

//=============================================================================
