///////////////////////////////////////////////////////////////////////////
//
//  change_state_vars.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\change_state_vars.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

//=========================================================================
// CHANGE_STATE_VARS : change a global variables vaule by an x amount..
//=========================================================================

change_state_vars::change_state_vars(  guid ParentGuid ): actions_base( ParentGuid ),
m_Code(CODE_SET),
m_VarHandle(HNULL),     
m_Type(var_mngr::TYPE_NULL),          
m_VarRaw(0)
{
}

//=============================================================================

void change_state_vars::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION *change_state_vars::Execute" );

    (void) pParent;

    if (m_VarHandle.IsNull())
        return;

    switch ( g_VarMgr.GetType( m_VarHandle ) )
    {
    case var_mngr::TYPE_FLOAT: 
        {
            f32  GlobalValue = g_VarMgr.GetFloat( m_VarHandle );
            f32& FloatValue = *((f32*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_ADD:           g_VarMgr.SetFloat( m_VarHandle, GlobalValue + FloatValue );
                return;
            case CODE_SUBTRACT:      g_VarMgr.SetFloat( m_VarHandle, GlobalValue - FloatValue );
                return;
            case CODE_SET:           g_VarMgr.SetFloat( m_VarHandle, FloatValue );
                return;
            default:
                ASSERT(0);
                break;
            }
        }
        break;
        
    case var_mngr::TYPE_INT: 
        {
            s32  GlobalValue = g_VarMgr.GetInt( m_VarHandle );
            s32& IntValue = *((s32*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_ADD:           g_VarMgr.SetInt( m_VarHandle, GlobalValue + IntValue ); 
                return;
            case CODE_SUBTRACT:      g_VarMgr.SetInt( m_VarHandle, GlobalValue - IntValue );
                return;
            case CODE_SET:           g_VarMgr.SetInt( m_VarHandle, IntValue );
                return;
            default:
                ASSERT(0);
                break;
            }
        }
        break;
        
    case var_mngr::TYPE_BOOL: 
        {
            xbool& BoolValue = *((xbool*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_ADD:          //unsupported code for this type
                return;
            case CODE_SUBTRACT:     //unsupported code for this type
                return;
            case CODE_SET:          g_VarMgr.SetBool( m_VarHandle, BoolValue );
                return;
            default:
                ASSERT(0);
                break;
            }
        }
        break;

    default:
        //no-op
        break;
    }
    
    return; 
}    

//=============================================================================

void change_state_vars::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddString	( "Variable Name", "Name of the Global Variable.", PROP_TYPE_MUST_ENUM );
    
    rPropList.AddEnum( "Type","FLOAT\0INT\0BOOL\0UNDETERMINED\0", "Variable Type." , PROP_TYPE_READ_ONLY );
 
    switch ( m_Type )
    { 
    case var_mngr::TYPE_FLOAT: 
        {
            rPropList.AddFloat ( "Float" , "Floating point value." );
        }
        break; 
        
    case var_mngr::TYPE_INT: 
        {
            rPropList.AddInt ( "Int" , "Interger value." );
        }
        break;
    case var_mngr::TYPE_BOOL: 
        {
            rPropList.AddBool( "Bool", "Boolean value."  );
        }
        break;
    default:
        
        break;
    }
    
    switch (m_Type)
    {
    case var_mngr::TYPE_FLOAT: case  var_mngr::TYPE_INT:
        rPropList.AddEnum( "Logic", "ADD\0SUBTRACT\0SET_TO\0", "Logic available to this action."  );
        break;
    case var_mngr::TYPE_BOOL: 
        rPropList.AddEnum( "Logic", "SET_TO\0", "Logic available to this action."  );
        break;
    default:
        break;
    }
    
    actions_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool change_state_vars::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Variable Name" ))
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarString( m_VariableName.Get(), m_VariableName.MaxLen() );
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarString();
           
            m_VariableName.Set( pString );

            if ( g_VarMgr.GetVarHandle( m_VariableName.Get(), &m_VarHandle ) == TRUE )
            {
                m_Type      = g_VarMgr.GetType( m_VarHandle );
            }
            else
            {
                m_Type      = var_mngr::TYPE_NULL;
                m_VarRaw    = 0;
                m_VarHandle = HNULL;
            }

            return( TRUE );
        }
    }
    
    if (rPropQuery.VarFloat ( "Float", *((f32*) &m_VarRaw) ))
    { 
        return TRUE; 
    }
    
    if (rPropQuery.VarInt ( "Int" , m_VarRaw ))
    {     
        return TRUE;
    }

    if (rPropQuery.VarBool ( "Bool" , *((xbool*) &m_VarRaw) ))
    { 
        return TRUE;
    }

    if ( rPropQuery.IsVar( "Type"))
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Type )
            { 
            case var_mngr::TYPE_FLOAT:  rPropQuery.SetVarEnum( "FLOAT" );           break; 
            case var_mngr::TYPE_INT:    rPropQuery.SetVarEnum( "INT" );             break;  
            case var_mngr::TYPE_BOOL:   rPropQuery.SetVarEnum( "BOOL" );            break;   
            case var_mngr::TYPE_NULL:   rPropQuery.SetVarEnum( "UNDETERMINED" );    break;   
            default:                    ASSERT(0);    break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "FLOAT" )==0) {             m_Type = var_mngr::TYPE_FLOAT;}
            if( x_stricmp( pString, "INT" )==0) {               m_Type = var_mngr::TYPE_INT;}
            if( x_stricmp( pString, "BOOL" )==0) {              m_Type = var_mngr::TYPE_BOOL;}
            if( x_stricmp( pString, "UNDETERMINED" )==0) {      m_Type = var_mngr::TYPE_NULL;}
            
            return( TRUE );
        }
    }
    
    if ( rPropQuery.IsVar( "Logic" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_ADD:          rPropQuery.SetVarEnum( "ADD" );            break;
            case CODE_SUBTRACT:     rPropQuery.SetVarEnum( "SUBTRACT" );       break;
            case CODE_SET:          rPropQuery.SetVarEnum( "SET_TO" );         break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "ADD" )==0)         { m_Code = CODE_ADD;}
            if( x_stricmp( pString, "SUBTRACT" )==0)    { m_Code = CODE_SUBTRACT;}
            if( x_stricmp( pString, "SET_TO" )==0)      { m_Code = CODE_SET;}
            
            return( TRUE );
        }
    }

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}


