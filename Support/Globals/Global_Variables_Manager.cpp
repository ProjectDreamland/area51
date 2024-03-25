///////////////////////////////////////////////////////////////////////////
//
//  Global_Variables_Manager.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Globals\Global_Variables_Manager.hpp"

#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

#include "MiscUtils\SimpleUtils.hpp"

#ifdef X_EDITOR
#include <windows.h>
#endif // X_EDITOR

#include "..\Support\Obj_mgr\obj_mgr.hpp"
/*
//=========================================================================
// GLOVALS
//=========================================================================

global_var_mgr g_GVarMgr;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================
// TIMER FUNCTIONS
//=========================================================================

global_var_mgr::timer::timer( void )
{ 
    ResetTimer();
}

//=========================================================================

void global_var_mgr::timer::Pause( xbool bStop )
{
    if( bStop && bPause )
    {
        // Nothing to do
    }
    else if( bPause )
    {
        // Subtract delta from the main timer
        GameTimer = g_ObjMgr.GetGameTime() - GameTimer;
    }
    else if( bStop )
    {
        // Store the delta
        GameTimer = g_ObjMgr.GetGameTime() - GameTimer;
    }
    else
    {
        // Nothing to do
    }

    // Set the pause state
    bPause = bStop;
}

//=========================================================================

void global_var_mgr::timer::ResetTimer( void )
{
    GameTimer = g_ObjMgr.GetGameTime();
    bPause   = FALSE; 
}

//=========================================================================

f32  global_var_mgr::timer::GetTime( void )
{
    if( bPause )
    {
        return (f32)x_TicksToSec(GameTimer);
    }
    
    return g_ObjMgr.GetGameDeltaTime( GameTimer );
}

//=========================================================================
// EDITOR ONLY
//=========================================================================
#ifdef TARGET_PC

//=========================================================================
static global_var_mgr s_Backup;

//=========================================================================
void global_var_mgr::StoreState( void )
{
    s_Backup.m_lVars      = m_lVars;
    s_Backup.m_lVarFIBs   = m_lVarFIBs;
    s_Backup.m_lVarTimers = m_lVarTimers;
    s_Backup.m_lVarGuids  = m_lVarGuids;
}

//=========================================================================
void global_var_mgr::RestoreState( void )
{
    m_lVars      = s_Backup.m_lVars;
    m_lVarFIBs   = s_Backup.m_lVarFIBs;
    m_lVarTimers = s_Backup.m_lVarTimers;
    m_lVarGuids  = s_Backup.m_lVarGuids;
}

#else

void global_var_mgr::StoreState  ( void ) {}
void global_var_mgr::RestoreState( void ) {}

#endif

//=========================================================================
//=========================================================================

//=========================================================================

void global_var_mgr::OnEnumProp( prop_enum&  Enum )
{
    Enum.AddInt( "VariableCount", "Total Number of variables" );        

    for( s32 i=0; i<m_lVars.GetCount(); i++ )
    {
        var& Var  = m_lVars[i];

        Enum.AddHeader( xfs("Variable[%d]", i),        "A Global Variable" );        
        Enum.AddString( xfs("Variable[%d]\\Name", i),  "Name of the global variableA Global Variable" );        

        switch( Var.Type )
        {
        case TYPE_FLOAT:
                Enum.AddFloat ( xfs("Variable[%d]\\Float", i), "Floating point value." );
            break;
        
        case TYPE_INT:
                Enum.AddInt   ( xfs("Variable[%d]\\Int", i) ,  "Interger point value." );
            break;
        
        case TYPE_BOOL:
                Enum.AddBool  ( xfs("Variable[%d]\\Bool", i) , "Boolean point value." );
            break;

        case TYPE_TIMER:
                Enum.AddGuid  ( xfs("Variable[%d]\\TimerValue", i) , "Time in ticks." );
                Enum.AddBool  ( xfs("Variable[%d]\\TimerPause", i) , "Boolean tells whether the time is pause." );
            break;

        case TYPE_GUID:
                Enum.AddGuid  ( xfs("Variable[%d]\\Guid", i) , "Boolean point value." );
            break;        

        default:
            ASSERT(0);  //Cannot be of indeterminite type..
            break;
        }

        Enum.AddString( xfs("Variable[%d]\\Notes", i),  "Notes from the user" );        
    }
}

//=========================================================================

xbool global_var_mgr::OnProperty( prop_query& I )
{
    if( I.IsVar( "VariableCount" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_lVars.GetCount() );
        }
        else
        {
            m_lVars.Clear();

            for( s32 i=0; i<I.GetVarInt(); i++ )
            {
                m_lVars.Add();
            }
        }
    }
    else if( I.VarString( "Variable[]\\Name", m_lVars[I.GetIndex(0)].Name, var::MAX_NAME_LENGTH) )
    {        
    }
    else if( I.IsVar( "Variable[]\\Notes" ) ) 
    {        
        if( I.IsRead() )
        {
            I.SetVarString( (const char*)GetVarNotes( m_lVars.GetHandleByIndex(I.GetIndex(0)) ), 256 );
        }
        else
        {
            m_lVars[I.GetIndex(0)].Notes = I.GetVarString();            
        }
    }
    else if( I.IsVar( "Variable[]\\Float" ) ) 
    {        
        if( I.IsRead() )
        {
            I.SetVarFloat( GetVarFloat( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) );
        }
        else
        {
            CreateVar( m_lVars[I.GetIndex(0)], TYPE_FLOAT, m_lVarFIBs );
            GetVarFloat( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) = I.GetVarFloat();            
        }
    }
    else if( I.IsVar( "Variable[]\\Int" ) ) 
    {        
        if( I.IsRead() )
        {
            I.SetVarInt( GetVarInt( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) );
        }
        else
        {
            CreateVar( m_lVars[I.GetIndex(0)], TYPE_INT, m_lVarFIBs );
            GetVarInt( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) = I.GetVarInt();            
        }
    }
    else if( I.IsVar( "Variable[]\\Bool" ) ) 
    {        
        if( I.IsRead() )
        {
            I.SetVarInt( GetVarBool( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) );
        }
        else
        {
            CreateVar( m_lVars[I.GetIndex(0)], TYPE_BOOL, m_lVarFIBs );
            GetVarBool( m_lVars.GetHandleByIndex(I.GetIndex(0)) ) = I.GetVarBool();            
        }
    }
    else if( I.IsVar( "Variable[]\\TimerValue" ) ) 
    {        
        if( I.IsRead() )
        {
            timer& Timer = GetVarTimer( m_lVars.GetHandleByIndex(I.GetIndex(0)) );
            I.SetVarGUID( guid(Timer.GameTimer) );
        }
        else
        {
            CreateVar( m_lVars[I.GetIndex(0)], TYPE_GUID, m_lVarGuids );

            GetVarTimer( m_lVars.GetHandleByIndex(I.GetIndex(0)) ).GameTimer = I.GetVarGUID().Guid;
        }
    }
    else if( I.IsVar( "Variable[]\\TimerPause" ) ) 
    {        
        if( I.IsRead() )
        {
            timer& Timer = GetVarTimer( m_lVars.GetHandleByIndex(I.GetIndex(0)) );
            I.SetVarBool( Timer.bPause );
        }
        else
        {
            GetVarTimer( m_lVars.GetHandleByIndex(I.GetIndex(0)) ).bPause = I.GetVarBool();            
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
*/

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
var_mngr g_VarMgr;

//=========================================================================

void var_mngr::var_key::Copy( const var_key& rSource )
{
    m_Name.Set( rSource.m_Name.Get() );

    m_Type = rSource.m_Type;                 
    m_Data = rSource.m_Data;                 
}

//=========================================================================

void var_mngr::guid_key::Copy( const guid_key& rSource )
{
    m_Name.Set( rSource.m_Name.Get() );
    m_Guid = rSource.m_Guid;                 
}

//=========================================================================

void var_mngr::timer_key::Copy( const timer_key& rSource )
{
    m_Name.Set( rSource.m_Name.Get() );
    m_Timer = rSource.m_Timer;                 
}

//=========================================================================

xhandle var_mngr::RegisterFloat ( const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterFloat" );

    return RegisterVariable( TYPE_FLOAT, pName );
}

//=========================================================================

xhandle var_mngr::RegisterInt ( const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterInt" );

    return RegisterVariable( TYPE_INT, pName );
}

//=========================================================================

xhandle var_mngr::RegisterBool ( const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterBool" );

    return RegisterVariable( TYPE_BOOL, pName );
}

//=========================================================================

xhandle var_mngr::RegisterVariable    ( variable_types Type, const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterVariable" );

    xhandle new_handle;
    
#ifdef X_EDITOR
    if ( GetVarHandle(pName, &new_handle) == TRUE )
    {
        if (GetType(new_handle) == Type)
        {
            MessageBox( NULL, "Tried to add prexisting variable.", "Warning",MB_ICONWARNING );
            return new_handle;
        }
        else
        {
            MessageBox( NULL, "Tried to add prexisting variable but of differnt type..", "Warning",MB_ICONWARNING );
            return new_handle;
        }
    }
#endif // X_EDITOR

    var_key &new_key = m_VarKeys.Add( new_handle );
    
    ASSERT( new_handle.IsNonNull() );
    
    f32     f_value = 0.0f;
    s32     s_value = 0;
    xbool   b_value = FALSE;
    
    new_key.m_Name.Set( pName );
    new_key.m_Type = Type;
    
    switch ( Type )
    {
    case TYPE_FLOAT:
        new_key.m_Data = *((u32*)(&f_value));
        break;
    case TYPE_INT:
        new_key.m_Data = *((u32*)(&s_value));
        break;
    case TYPE_BOOL:
        new_key.m_Data = *((u32*)(&b_value));
        break;
    default:
        ASSERT(0);  //Cannot be of indeterminite type..
        break;
    }
    
    return new_handle;
}

//=========================================================================

xhandle var_mngr::RegisterTimer( const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterTimer" );

    xhandle new_handle;
    
#ifdef X_EDITOR
    if ( GetTimerHandle(pName, &new_handle) == TRUE )
    {
        MessageBox( NULL, "Tried to add prexisting timer.", "Warning",MB_ICONWARNING );
        return new_handle;
    }
#endif // X_EDITOR

    timer_key &new_key = m_TimerKeys.Add( new_handle );
    
    ASSERT( new_handle.IsNonNull() );
  
    new_key.m_Name.Set( pName );

    return new_handle;
}

//=========================================================================

xhandle var_mngr::RegisterGuid( const char* pName )
{
    VAR_MNGR_CONTEXT( "var_mngr::RegisterGuid" );

    xhandle new_handle;
    
#ifdef X_EDITOR
    if ( GetGuidHandle(pName, &new_handle) == TRUE )
    {
        MessageBox( NULL, "Tried to add prexisting guid.", "Warning",MB_ICONWARNING );
        return new_handle;
    }
#endif // X_EDITOR

    guid_key &new_key = m_GuidKeys.Add( new_handle );
    
    ASSERT( new_handle.IsNonNull() );
  
    new_key.m_Name.Set( pName );

    return new_handle;
}

//=========================================================================

xbool var_mngr::DestroyVariable ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::DestroyVariable" );

    ASSERT( rVarHandle.IsNonNull() );
    
    m_VarKeys.DeleteByHandle ( rVarHandle );
    
    return TRUE;
}

//=========================================================================

xbool var_mngr::DestroyGuid ( xhandle& rGuidHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::DestroyGuid" );

    ASSERT( rGuidHandle.IsNonNull() );
    
    m_GuidKeys.DeleteByHandle ( rGuidHandle );
    
    return TRUE;
}

//=========================================================================

xbool var_mngr::DestroyTimer ( xhandle& rTimerHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::DestroyTimer" );

    ASSERT( rTimerHandle.IsNonNull() );
    
    m_TimerKeys.DeleteByHandle ( rTimerHandle );
    
    return TRUE;
}

//=========================================================================

f32 var_mngr::GetFloat ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetFloat" );

    ASSERT( rVarHandle.IsNonNull() );
    
    var_key &key = m_VarKeys( rVarHandle );
    return *((f32*) &key.m_Data);   
}

//=========================================================================

s32 var_mngr::GetInt ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetInt" );

    ASSERT( rVarHandle.IsNonNull() );
    
    var_key &key = m_VarKeys( rVarHandle );
    return *((s32*) &key.m_Data);   
}

//=========================================================================

xbool var_mngr::GetBool ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetBool" );

    ASSERT( rVarHandle.IsNonNull() );
    
    var_key &key = m_VarKeys( rVarHandle );
    return *((xbool*) &key.m_Data);   
}

//=========================================================================

xbool var_mngr::GetVarHandle ( const char* pName, xhandle* rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetVarHandle" );

    ASSERT( pName != NULL );
    
    s32 count = m_VarKeys.GetCount();
    
    for (s32 i = 0; i < count; i++)
    {
        var_key &key = m_VarKeys[i];
        
        if ( key.m_Name.IsSame( pName ) )
        {
            *rVarHandle = m_VarKeys.GetHandleByIndex(i);
            return TRUE;
        }
    }
    
    return FALSE;
}

//=========================================================================

xbool var_mngr::GetTimerHandle ( const char* pName, xhandle* rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetTimerHandle" );

    ASSERT( pName != NULL );
    
    s32 count = m_TimerKeys.GetCount();
    
    for (s32 i = 0; i < count; i++)
    {
        timer_key &key = m_TimerKeys[i];
        
        if ( key.m_Name.IsSame(pName) )
        {
            *rVarHandle = m_TimerKeys.GetHandleByIndex(i);
            return TRUE;
        }
    }
    
    return FALSE;
}

//=========================================================================

xbool var_mngr::GetGuidHandle ( const char* pName, xhandle* rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetGuidHandle" );

    ASSERT( pName != NULL );
    
    s32 count = m_GuidKeys.GetCount();
    
    for (s32 i = 0; i < count; i++)
    {
        guid_key &key = m_GuidKeys[i];
        
        if ( key.m_Name.IsSame(pName) )
        {
            *rVarHandle = m_GuidKeys.GetHandleByIndex(i);
            return TRUE;
        }
    }
    
    return FALSE;
}

//=========================================================================

var_mngr::variable_types  var_mngr::GetType( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetType" );

    ASSERT( rVarHandle.IsNonNull() );

    if ( rVarHandle.IsNull() )
        return TYPE_NULL;

    var_key &key = m_VarKeys( rVarHandle );
    
    return (variable_types) key.m_Type;
}

//=========================================================================

void var_mngr::SetFloat ( xhandle& rVarHandle, f32 Value )
{
    VAR_MNGR_CONTEXT( "var_mngr::SetFloat" );

    ASSERT( rVarHandle.IsNonNull() );
    
    if ( rVarHandle.IsNull() )
        return;

    var_key &key = m_VarKeys( rVarHandle );
    key.m_Data   = *((u32*)&Value);
}

//=========================================================================

void var_mngr::SetInt ( xhandle& rVarHandle, s32 Value  )
{
    VAR_MNGR_CONTEXT( "var_mngr::SetInt" );

    ASSERT( rVarHandle.IsNonNull() );
     
    if ( rVarHandle.IsNull() )
        return;
    
    var_key &key = m_VarKeys( rVarHandle );
    key.m_Data   = *((u32*)&Value);
}

//=========================================================================

void var_mngr::SetBool ( xhandle& rVarHandle, xbool Value  )
{
    VAR_MNGR_CONTEXT( "var_mngr::SetInt" );
    
    ASSERT( rVarHandle.IsNonNull() );
    
    if ( rVarHandle.IsNull() )
        return;

    var_key &key = m_VarKeys( rVarHandle );
    key.m_Data   = *((u32*)&Value);
}  

//=========================================================================

void var_mngr::SetGuid ( xhandle& rGuidHandle, guid Value  )
{
    VAR_MNGR_CONTEXT( "var_mngr::SetGuid" );
    
    ASSERT( rGuidHandle.IsNonNull() );
    
    if ( rGuidHandle.IsNull() )
        return;

    guid_key &key = m_GuidKeys( rGuidHandle );
    key.m_Guid    = Value;
}  

//=========================================================================

guid var_mngr::GetGuid ( xhandle& rGuidHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::GetGuid" );

    ASSERT( rGuidHandle.IsNonNull() );
    
    guid_key &key = m_GuidKeys( rGuidHandle );
    return key.m_Guid;   
}

//=========================================================================

void var_mngr::ResetTimer( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::ResetTimer" );
    
    ASSERT( rVarHandle.IsNonNull() );

    if ( rVarHandle.IsNull() )
        return;

    timer_key &key = m_TimerKeys( rVarHandle );
    key.m_Timer.Reset();
}

//=========================================================================

void var_mngr::StartTimer( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::StartTimer" );
    
    ASSERT( rVarHandle.IsNonNull() );
    
    if ( rVarHandle.IsNull() )
        return;

    timer_key &key = m_TimerKeys( rVarHandle );
    key.m_Timer.Start();
}

//=========================================================================

void var_mngr::StopTimer ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::StopTimer" );
    
    ASSERT( rVarHandle.IsNonNull() );
    
    if ( rVarHandle.IsNull() )
        return;

    timer_key &key = m_TimerKeys( rVarHandle );
    key.m_Timer.Stop();
}

//=========================================================================

f32 var_mngr::ReadTimer ( xhandle& rVarHandle )
{
    VAR_MNGR_CONTEXT( "var_mngr::StopTimer" );
    
    ASSERT( rVarHandle.IsNonNull() );
    
    if ( rVarHandle.IsNull() )
        return 0.0f;

    timer_key &key = m_TimerKeys( rVarHandle );

    return key.m_Timer.ReadSec();
}

//=========================================================================

void var_mngr::OnEnumProp ( prop_enum&   rPropList )
{
    s32 iHeader = rPropList.PushPath( "Global Variables\\Variables\\" );        
    
    OnEnumPropInternal( rPropList );

    rPropList.PopPath( iHeader );
}

//=========================================================================

xbool var_mngr::OnProperty ( prop_query&  rPropQuery )
{
    s32 iHeader = rPropQuery.PushPath( "Global Variables\\Variables\\" );        
    
    if (OnPropertyInternal(rPropQuery))
    {
        rPropQuery.PopPath( iHeader );
        return TRUE;
    }
    
    rPropQuery.PopPath( iHeader );
    return FALSE;
}

//=========================================================================

void var_mngr::OnEnumPropVariables ( prop_enum&   rPropList, u32 Flags )
{
    s32 count = m_VarKeys.GetCount();
    s32 i = 0;

    for ( i = 0; i < count; i++)
    {
        var_key &key = m_VarKeys[i];
        
        switch (key.m_Type)
        {
        case TYPE_FLOAT:
            {
                rPropList.PropEnumString( xfs("Variable[%d]", i) ,       "A single entry..", Flags );        
                rPropList.PropEnumFloat ( xfs("Variable[%d]\\Float", i), "Floating point value.", Flags );
                rPropList.PropEnumButton( xfs("Variable[%d]\\Delete",i), "Delete this Global Variable.", Flags | PROP_TYPE_MUST_ENUM );
            }
            break;
            
        case TYPE_INT:
            {
                rPropList.PropEnumString( xfs("Variable[%d]", i) ,       "A single entry..", Flags );        
                rPropList.PropEnumInt   ( xfs("Variable[%d]\\Int", i) ,  "Interger point value.", Flags );
                rPropList.PropEnumButton( xfs("Variable[%d]\\Int",i) ,   "Delete this Global Variable.", Flags | PROP_TYPE_MUST_ENUM );
            }
            break;
            
        case TYPE_BOOL:
            {
                rPropList.PropEnumString( xfs("Variable[%d]", i) ,       "A single entry..", Flags );        
                rPropList.PropEnumBool  ( xfs("Variable[%d]\\Bool", i) , "Boolean point value.", Flags );
                rPropList.PropEnumButton( xfs("Variable[%d]\\Bool",i) ,  "Delete this Global Variable.", Flags | PROP_TYPE_MUST_ENUM );
            }
            break;
            
        default:
            ASSERT(0);  //Cannot be of indeterminite type..
            break;
        }
    }

    //Enumerate the timers...
    
    count = m_TimerKeys.GetCount();
    i = 0;
    
    for ( i = 0; i < count; i++)
    {
        rPropList.PropEnumString( xfs("Timer[%d]", i) ,          "A single entry..", Flags );  
        rPropList.PropEnumFloat ( xfs("Timer[%d]\\Value", i) ,   "Current time value in secs.", Flags );
        rPropList.PropEnumButton( xfs("Timer[%d]\\Delete",i) ,   "Delete this Global Variable.", Flags | PROP_TYPE_MUST_ENUM );
    }

    //Enumerate the guids...
    
    count = m_GuidKeys.GetCount();
    i = 0;
    
    for ( i = 0; i < count; i++)
    {
        rPropList.PropEnumString( xfs("Guid[%d]", i) ,          "A single entry..", Flags );  
        rPropList.PropEnumGuid  ( xfs("Guid[%d]\\Value", i) ,   "Guid Value.", Flags );
        rPropList.PropEnumButton( xfs("Guid[%d]\\Delete",i) ,   "Delete this Global Variable.", Flags | PROP_TYPE_MUST_ENUM );
    }
}

//=========================================================================

void var_mngr::OnEnumPropInternal	( prop_enum&   rPropList )
{
    rPropList.PropEnumInt( "Array\\Size", "", 0 );    

    s32 count = m_VarKeys.GetCount();
    s32 i = 0;

    for ( i = 0; i < count; i++ )
    {
        rPropList.PropEnumInt( xfs("Array\\Type[%d]", i) , "", 0 );    
    }

    rPropList.PropEnumInt( "Array\\Timer Size", "", 0 );

    rPropList.PropEnumInt( "Array\\Guid Size", "", 0 );

    OnEnumPropVariables(rPropList);
}

//=========================================================================

xbool var_mngr::OnPropertyInternal  ( prop_query&  rPropQuery )
{
    if (rPropQuery.IsVar( "Array\\Size" ))
    {
        if (rPropQuery.IsRead())
        {
            s32 Size = m_VarKeys.GetCount();
            
            rPropQuery.SetVarInt(Size);
        }
        else
        {
            s32 Size = rPropQuery.GetVarInt();
            
            for (s32 i = 0; i < Size; i++)
            {
                xhandle new_handle;
                var_key &EmptyKey  = m_VarKeys.Add( new_handle );
				(void)EmptyKey;
            }
        }
        
        return TRUE;
    }
    
    if (rPropQuery.IsVar( "Array\\Type[]" ))
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT ( iIndex < m_VarKeys.GetCount() );
        
        var_key &key = m_VarKeys[iIndex];
        
        if (rPropQuery.IsRead())
        {
            rPropQuery.SetVarInt((s32)key.m_Type);
        }
        else
        {
            key.m_Type = (variable_types)rPropQuery.GetVarInt();
        }
        
        return TRUE;
    }
    
    if (rPropQuery.IsVar( "Array\\Timer Size" ))
    {
        if (rPropQuery.IsRead())
        {
            s32 Size = m_TimerKeys.GetCount();
            
            rPropQuery.SetVarInt(Size);
        }
        else
        {
            s32 Size = rPropQuery.GetVarInt();
            
            for (s32 i = 0; i < Size; i++)
            {
                xhandle new_handle;
                timer_key &EmptyKey  = m_TimerKeys.Add( new_handle );
                (void)EmptyKey;
            }
        }
        
        return TRUE;
    }

    
    if (rPropQuery.IsVar( "Array\\Guid Size" ))
    {
        if (rPropQuery.IsRead())
        {
            s32 Size = m_GuidKeys.GetCount();
            
            rPropQuery.SetVarInt(Size);
        }
        else
        {
            s32 Size = rPropQuery.GetVarInt();
            
            for (s32 i = 0; i < Size; i++)
            {
                xhandle new_handle;
                guid_key &EmptyKey  = m_GuidKeys.Add( new_handle );
                (void)EmptyKey;
            }
        }
        
        return TRUE;
    }

    if (OnPropertyVariables(rPropQuery))
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool var_mngr::OnPropertyVariables	( prop_query&  rPropQuery )
{ 
    s32 iIndex = rPropQuery.GetIndex(0);
    
    if ( rPropQuery.IsSimilarPath("Variable[]") )
    {
        if ( iIndex >= m_VarKeys.GetCount() )
            return FALSE;
        
        var_key &key = m_VarKeys[iIndex];
        
        if ( rPropQuery.IsVar( "Variable[]" ) )
        { 
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString( key.m_Name.Get(), key.m_Name.MaxLen() );
            }
            else
            {
                key.m_Name.Set( rPropQuery.GetVarString() );
            }
            
            return TRUE;
        }
        
        if ( rPropQuery.VarFloat( "Variable[]\\Float" ,     *((f32*) &key.m_Data) ) )
        {
            return TRUE;
        }
        
        if ( rPropQuery.VarInt( "Variable[]\\Int" ,         *((s32*) &key.m_Data) ) )
        {
            return TRUE;
        }
        
        if ( rPropQuery.VarBool( "Variable[]\\Bool" ,       *((xbool*) &key.m_Data) ) )
        {
            return TRUE;
        }

        if( rPropQuery.IsVar( "Variable[]\\Delete" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarButton( "Delete" );
            }
            else
            {
                xhandle rHandle;
                if (GetVarHandle(key.m_Name.Get(), &rHandle))
                {
                    m_VarKeys.DeleteByHandle ( rHandle );
                }
            }
            return TRUE;
        }
    }
    
    if ( rPropQuery.IsSimilarPath("Timer[]") )
    {
        if ( iIndex >= m_TimerKeys.GetCount() )
            return FALSE;
        
        timer_key &key = m_TimerKeys[iIndex]; 
        
        if ( rPropQuery.IsVar( "Timer[]" ) )
        { 
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString( key.m_Name.Get(), key.m_Name.MaxLen() );
            }
            else
            {
                key.m_Name.Set( rPropQuery.GetVarString() );
            }
            
            return TRUE;
        }
        
        f32 CurrentTimeValue = key.m_Timer.ReadSec();

        if ( rPropQuery.VarFloat( "Timer[]\\Value" , CurrentTimeValue ) )
        {
            return TRUE;
        }

        if( rPropQuery.IsVar( "Timer[]\\Delete" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarButton( "Delete" );
            }
            else
            {
                xhandle rHandle;
                if (GetTimerHandle(key.m_Name.Get(), &rHandle))
                {
                    m_TimerKeys.DeleteByHandle ( rHandle );
                }
            }
            return TRUE;
        }
    }

    if ( rPropQuery.IsSimilarPath("Guid[]") )
    {
        if ( iIndex >= m_GuidKeys.GetCount() )
            return FALSE;
        
        guid_key &key = m_GuidKeys[iIndex]; 
        
        if ( rPropQuery.IsVar( "Guid[]" ) )
        { 
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString( key.m_Name.Get(), key.m_Name.MaxLen() );
            }
            else
            {
                key.m_Name.Set( rPropQuery.GetVarString() );
            }
            
            return TRUE;
        }
        
        if ( rPropQuery.VarGUID( "Guid[]\\Value" , key.m_Guid ) )
        {
            return TRUE;
        }

        if( rPropQuery.IsVar( "Guid[]\\Delete" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarButton( "Delete" );
            }
            else
            {
                xhandle rHandle;
                if (GetGuidHandle(key.m_Name.Get(), &rHandle))
                {
                    m_GuidKeys.DeleteByHandle ( rHandle );
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

void var_mngr::ResetKeys( void )
{
    m_VarKeys.Clear();
    m_TimerKeys.Clear();
    m_GuidKeys.Clear();
    m_StoreVarKeys.Clear();
    m_StoreGuidKeys.Clear();
}

//=========================================================================

void var_mngr::ClearData( void )
{
    ResetKeys();
    m_RuntimeGuidKeys.Clear();
    m_RuntimeVarKeys.Clear();
}

//=========================================================================

void var_mngr::ResetAllTimers ( void )
{
    s32 count = m_TimerKeys.GetCount();
    
    for (s32 i = 0; i < count; i++)
    { 
        timer_key &Key        = m_TimerKeys[i];
        Key.m_Timer.Reset();
    }
}

//=========================================================================

void var_mngr::LoadGlobals ( const char* pFileName )
{
    ResetKeys();
    if (SMP_UTIL_FileExist( pFileName ))
    {
        text_in GlobalFile;
        
        x_try;
        
        GlobalFile.OpenFile( pFileName );

        OnLoad( GlobalFile );
        
        GlobalFile.CloseFile();

        ResetAllTimers();
        
        x_catch_begin;
        
        x_catch_end;
    }
}

//=========================================================================

void var_mngr::ImportGlobals ( const char* pFileName )
{
    //first backup the current globals
    s32 i = 0;

    xharray<var_key> VarKeysBackup;
    for (i = 0; i < m_VarKeys.GetCount(); i++)
    {
        xhandle new_handle;
        var_key &Key        = m_VarKeys[i];
        var_key &CopyKey    = VarKeysBackup.Add( new_handle );
        CopyKey.Copy(Key);
    }

    xharray<guid_key> GuidKeysBackup;
    for (i = 0; i < m_GuidKeys.GetCount(); i++)
    {
        xhandle new_handle;
        guid_key &Key        = m_GuidKeys[i];
        guid_key &CopyKey    = GuidKeysBackup.Add( new_handle );
        CopyKey.Copy(Key);
    }

    xharray<timer_key> TimerKeysBackup;
    for (i = 0; i < m_TimerKeys.GetCount(); i++)
    {
        xhandle new_handle;
        timer_key &Key        = m_TimerKeys[i];
        timer_key &CopyKey    = TimerKeysBackup.Add( new_handle );
        CopyKey.Copy(Key);
    }
    
    //now load new globals
    LoadGlobals(pFileName);

    //next re-append our original globals
    for (i = 0; i < VarKeysBackup.GetCount(); i++)
    {
        xhandle new_handle;
        var_key &Key        = VarKeysBackup[i];
        var_key &CopyKey    = m_VarKeys.Add( new_handle );
        CopyKey.Copy(Key);
    }

    for (i = 0; i < GuidKeysBackup.GetCount(); i++)
    {
        xhandle new_handle;
        guid_key &Key        = GuidKeysBackup[i];
        guid_key &CopyKey    = m_GuidKeys.Add( new_handle );
        CopyKey.Copy(Key);
    }

    for (i = 0; i < TimerKeysBackup.GetCount(); i++)
    {
        xhandle new_handle;
        timer_key &Key        = TimerKeysBackup[i];
        timer_key &CopyKey    = m_TimerKeys.Add( new_handle );
        CopyKey.Copy(Key);
    }
}

//=========================================================================

#if defined( X_EDITOR )

void var_mngr::SaveGlobals ( const char* pFileName )
{
    text_out GlobalFile;
    
    x_try;
    
    GlobalFile.OpenFile( pFileName );
    
    OnSave( GlobalFile );
    
    GlobalFile.CloseFile();
    
    x_catch_begin;
    
    x_catch_end;
}

#endif // defined( X_EDITOR )

//=========================================================================

void var_mngr::StoreState ( void )
{
    s32 i = 0;
    m_StoreVarKeys.Clear();
    s32 count = m_VarKeys.GetCount();
    for (i = 0; i < count; i++)
    {
        xhandle new_handle;
        var_key &Key        = m_VarKeys[i];
        var_key &CopyKey    = m_StoreVarKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }

    m_StoreGuidKeys.Clear();
    count = m_GuidKeys.GetCount();
    for (i = 0; i < count; i++)
    {
        xhandle new_handle;
        guid_key &Key        = m_GuidKeys[i];
        guid_key &CopyKey    = m_StoreGuidKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }
}

//=========================================================================

void var_mngr::ResetState ( void )
{
    s32 i = 0;
    m_VarKeys.Clear();
    s32 count = m_StoreVarKeys.GetCount();
    for (i = 0; i < count; i++)
    { 
        xhandle new_handle;
        var_key &Key        = m_StoreVarKeys[i];
        var_key &CopyKey    = m_VarKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }

    m_GuidKeys.Clear();
    count = m_StoreGuidKeys.GetCount();
    for (i = 0; i < count; i++)
    { 
        xhandle new_handle;
        guid_key &Key        = m_StoreGuidKeys[i];
        guid_key &CopyKey    = m_GuidKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }
    
    ResetAllTimers();
}

//=========================================================================

xbool var_mngr::SaveRuntimeData( void )
{
    s32 i = 0;
    m_RuntimeVarKeys.Clear();
    s32 count = m_VarKeys.GetCount();
    for (i = 0; i < count; i++)
    {
        xhandle new_handle;
        var_key &Key        = m_VarKeys[i];
        var_key &CopyKey    = m_RuntimeVarKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }

    m_RuntimeGuidKeys.Clear();
    count = m_GuidKeys.GetCount();
    for (i = 0; i < count; i++)
    {
        xhandle new_handle;
        guid_key &Key        = m_GuidKeys[i];
        guid_key &CopyKey    = m_RuntimeGuidKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }

    return TRUE;
}

//=========================================================================

xbool var_mngr::LoadRuntimeData( void )
{
    s32 i = 0;
    m_VarKeys.Clear();
    s32 count = m_RuntimeVarKeys.GetCount();
    for (i = 0; i < count; i++)
    { 
        xhandle new_handle;
        var_key &Key        = m_RuntimeVarKeys[i];
        var_key &CopyKey    = m_VarKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }

    m_GuidKeys.Clear();
    count = m_RuntimeGuidKeys.GetCount();
    for (i = 0; i < count; i++)
    { 
        xhandle new_handle;
        guid_key &Key        = m_RuntimeGuidKeys[i];
        guid_key &CopyKey    = m_GuidKeys.Add( new_handle );

        CopyKey.Copy(Key);
    }
    
    ResetAllTimers();

    return TRUE;
}

//=========================================================================

void var_mngr::StoreRuntimeData( bitstream& BitStream )
{
    // write the variable count
    s32 Count = m_VarKeys.GetCount();
    BitStream.WriteS32( Count );

    // write the variable values
    for ( s32 i = 0; i < Count; i++ )
    {
        BitStream.WriteU32( m_VarKeys[i].m_Data );
    }

    // write the guid key count 
    Count = m_GuidKeys.GetCount();
    BitStream.WriteS32( Count );

    // write guid values
    for (s32 j = 0; j < Count; j++)
    {
        BitStream.WriteU64( m_GuidKeys[j].m_Guid );
    }
}

//=========================================================================

void var_mngr::RestoreRuntimeData( bitstream& BitStream )
{
    // read the variable count
    s32 Count;
    BitStream.ReadS32( Count );

    if( m_VarKeys.GetCount() != Count )
    {
        ASSERTS( FALSE, "Global variable count does not match save data!" );
        x_DebugMsg( 0, "Global variable count does not match save data!/n" );

        // Don't restore the globals!
        BitStream.SetCursor( BitStream.GetCursor() + ( Count * 8 ) );
    }
    else
    {
        // read the variable values
        for ( s32 i = 0; i < Count; i++ )
        {
            BitStream.ReadU32( m_VarKeys[i].m_Data );
        }
    }

    // read the guid key count 
    BitStream.ReadS32( Count );

    if( m_GuidKeys.GetCount() != Count )
    {
        ASSERTS( FALSE, "Guid count does not match save data!" );
        x_DebugMsg( 0, "Guid count does not match save data!/n" );

        // Don't restore the guids!
        BitStream.SetCursor( BitStream.GetCursor() + ( Count * 8 ) );
    }
    else
    {
        // read guid values
        for ( s32 i = 0; i < Count; i++ )
        {
            BitStream.ReadU64( m_GuidKeys[i].m_Guid.Guid );
        }
    }
}

//=========================================================================

void var_mngr::DumpRuntimeData( bitstream& BitStream )
{
    // output header info
    x_DebugMsg( 0, "------------------------\n" );
    x_DebugMsg( 0, "        Globals         \n" );
    x_DebugMsg( 0, "------------------------\n" );

    // read the variable count
    s32 Count;
    BitStream.ReadS32( Count );
    x_DebugMsg( 0, "Variable Count = %d\n", Count );

    if( m_VarKeys.GetCount() != Count )
    {
        x_DebugMsg( 0, "Global variable count does not match save data!/n" );

        // Don't restore the globals!
        BitStream.SetCursor( BitStream.GetCursor() + ( Count * 8 ) );
    }
    else
    {
        // read the variable values
        for ( s32 i = 0; i < Count; i++ )
        {           
            x_DebugMsg( 0, "------------------------\n" );
            x_DebugMsg( 0, "Variable Name : %s\n", m_VarKeys[i].m_Name );

            u32 Data;
            BitStream.ReadU32( Data );

            switch( m_VarKeys[i].m_Type )
            {
            case TYPE_NULL:
                x_DebugMsg( 0, "Variable Type : TYPE_NULL\n" );
                x_DebugMsg( 0, "Variable Data : %d\n", Data );
                break;
            case TYPE_FLOAT:
                x_DebugMsg( 0, "Variable Type : TYPE_FLOAT\n" );
                x_DebugMsg( 0, "Variable Data : %f\n", (f32)Data );
                break;
            case TYPE_INT:
                x_DebugMsg( 0, "Variable Type : TYPE_INT\n" );
                x_DebugMsg( 0, "Variable Data : %d\n", Data );
                break;
            case TYPE_BOOL:
                x_DebugMsg( 0, "Variable Type : TYPE_BOOL\n" );
                x_DebugMsg( 0, "Variable Data : %d\n", Data );
                break;
            default:
                x_DebugMsg( 0, "Variable Type : UNDEFINED!\n" );
                x_DebugMsg( 0, "Variable Data : %d\n", Data );
                break;
            }

        }
    }

    x_DebugMsg( 0, "------------------------\n" );
    x_DebugMsg( 0, "         GUIDS          \n" );
    x_DebugMsg( 0, "------------------------\n" );

    // read the guid key count 
    BitStream.ReadS32( Count );
    x_DebugMsg( 0, "Guid Count = %d\n", Count );

    if( m_GuidKeys.GetCount() != Count )
    {
        x_DebugMsg( 0, "Guid count does not match save data!/n" );

        // Don't restore the guids!
        BitStream.SetCursor( BitStream.GetCursor() + ( Count * 8 ) );
    }
    else
    {
        // read guid values
        for ( s32 i = 0; i < Count; i++ )
        {
            u64 GuidSequence;
            BitStream.ReadU64( GuidSequence );
            x_DebugMsg( 0, "------------------------\n" );
            x_DebugMsg( 0, "Guid Name     : %s\n", m_GuidKeys[i].m_Name );
            x_DebugMsg( 0, "Guid Sequence : %d\n", GuidSequence );            
        }
    }
}

//=========================================================================

#ifdef X_EDITOR

//=========================================================================
// EDITOR SIDE FUNCTION
//=========================================================================

xbool var_mngr::GetGlobalsList( xharray<global_def>& Globals )
{
    Globals.Clear();
    s32 i = 0;

    //Enumerate the variables...
    for ( i = 0; i < m_VarKeys.GetCount(); i++)
    {
        var_key &key = m_VarKeys[i];

        global_def& Def = Globals.Add();
        Def.Name.Set(key.m_Name.Get());
        
        switch (key.m_Type)
        {
        case TYPE_FLOAT:
            Def.Type = GLOBAL_FLOAT;
            break;
        case TYPE_INT:
            Def.Type = GLOBAL_INT;
            break;
        case TYPE_BOOL:
            Def.Type = GLOBAL_BOOL;
            break;
        default:
            ASSERT(0);  //Cannot be of indeterminite type..
            break;
        }
    }

    //Enumerate the timers...   
    for ( i = 0; i < m_TimerKeys.GetCount(); i++)
    {
        timer_key &key = m_TimerKeys[i];
        global_def& Def = Globals.Add();
        Def.Name.Set(key.m_Name.Get());
        Def.Type = GLOBAL_TIMER;
    }
    
    //Enumerate the guid...
    for ( i = 0; i < m_GuidKeys.GetCount(); i++)
    {
        guid_key &key = m_GuidKeys[i];
        global_def& Def = Globals.Add();
        Def.Name.Set(key.m_Name.Get());
        Def.Type = GLOBAL_GUID;
    }

    return TRUE;
}

//=========================================================================

xbool var_mngr::DoesGlobalExist( const char* pName )
{
    s32 i = 0;

    //Enumerate the variables...
    for ( i = 0; i < m_VarKeys.GetCount(); i++)
    {
        var_key &key = m_VarKeys[i];
        if (x_strcmp(key.m_Name.Get(), pName) == 0) 
            return TRUE;
    }

    //Enumerate the timers...   
    for ( i = 0; i < m_TimerKeys.GetCount(); i++)
    {
        timer_key &key = m_TimerKeys[i];
        if (x_strcmp(key.m_Name.Get(), pName) == 0) 
            return TRUE;
    }
    
    //Enumerate the guid...
    for ( i = 0; i < m_GuidKeys.GetCount(); i++)
    {
        guid_key &key = m_GuidKeys[i];
        if (x_strcmp(key.m_Name.Get(), pName) == 0) 
            return TRUE;
    }

    return FALSE;
}

#endif // X_EDITOR
