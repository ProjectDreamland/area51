///////////////////////////////////////////////////////////////////////////////
//
//  Global_Variables_Manager.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _GLOBAL_VARIABLE_MANAGER_
#define _GLOBAL_VARIABLE_MANAGER_

//=========================================================================
// INCLUDES
//=========================================================================

#include <x_types.hpp>
#include <x_array.hpp>
#include "Auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "x_bitstream.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

//#define _ENABLE_GLOBAL_VARIABLE_PROFILING_   
                         
#ifdef _ENABLE_GLOBAL_VARIABLE_PROFILING_
    #define VAR_MNGR_CONTEXT( exp )  CONTEXT( exp )                 
#else
    #define VAR_MNGR_CONTEXT( exp ) /*no-op*/
#endif  

//=========================================================================
// CLASS
//=========================================================================

// NOTE: timers state are not persistant they dont get saved...
/*
class global_var_mgr : public prop_interface
{
public:

    struct timer
    {
                timer       ( void );
        void    Pause       ( xbool bStop );
        void    ResetTimer  ( void );
        f32     GetTime     ( void );
        xtick   GameTimer;
        xbool   bPause;
    };
    
public:

    xhandle         CreateVarInt        ( const char* pName );
    xhandle         CreateVarFloat      ( const char* pName );
    xhandle         CreateVarBool       ( const char* pName );
    xhandle         CreateVarTimer      ( const char* pName );
    xhandle         CreateVarGuid       ( const char* pName );

    void            DestroyVariable     ( xhandle Handle );

    xhandle         GetVarHandle        ( const char* pName );
    const char*     GetVarName          ( xhandle Handle );

    s32&            GetVarInt           ( xhandle Handle );
    f32&            GetVarFloat         ( xhandle Handle );
    xbool&          GetVarBool          ( xhandle Handle );
    timer&          GetVarTimer         ( xhandle Handle );
    guid&           GetVarGuid          ( xhandle Handle );
    xstring&        GetVarNotes         ( xhandle Handle );

    virtual void    OnEnumProp          ( prop_enum&   rPropList  );
    virtual xbool   OnProperty          ( prop_query&  rPropQuery );

    //
    // Editor only functions
    //
public:

    void        StoreState          ( void );
    void        RestoreState        ( void );

protected:

    enum type
	{
        TYPE_NULL = 0,

        TYPE_FLOAT,
        TYPE_INT,
        TYPE_BOOL,
        TYPE_TIMER,
        TYPE_GUID,

        // TYPE_STRING, // ???

        TYPE_END
    };

    struct var
    {
        enum
        {
            MAX_NAME_LENGTH = 128
        };

        char        Name[MAX_NAME_LENGTH];  // Name of the variable
        type        Type;                   // Type of the Variable
        xhandle     hData;                  // Handle to the data
        xstring     Notes;                  // User extra info
    };

protected:

    template< class T> inline 
    xhandle CreateVar( const char* pName, type Type, T& List )
    {
        ASSERT( pName );        

        if( x_strlen( pName ) >= var::MAX_NAME_LENGTH )
            x_throw( xfs( "ERROR: Variable name (%s) too long", pName) );

        if( GetVarHandle( pName ).Handle != HNULL )
            x_throw( xfs( "ERROR: Variable (%s) already exits", pName) );

        xhandle Handle;

        var& Var  = m_lVars.Add( Handle );

        x_strcpy( Var.Name, pName );

        List.Add( Var.hData );
        Var.Type = Type;

        return Handle;
    }

    template< class T> inline 
    void CreateVar( var& Var, type Type, T& List )
    {
        List.Add( Var.hData );
        Var.Type = Type;
    }

protected:

    xharray<var>            m_lVars;          // Variables
    xharray<u32>            m_lVarFIBs;       // (float, int, bool) var
    xharray<timer>          m_lVarTimers;     // Timer var
    xharray<guid>           m_lVarGuids;      // Guid var
};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline
xhandle global_var_mgr::GetVarHandle( const char* pName )
{
    ASSERT( pName );
    xhandle Handle( HNULL );

    for( s32 i=0; i<m_lVars.GetCount(); i++ )
    {
        var& Var = m_lVars[i];

        if( x_strcmp( Var.Name, pName ) == 0 )
        {
            Handle = m_lVars.GetHandleByIndex( i );
            break;
        }
    }

    return Handle;
}

//=========================================================================

inline
const char* global_var_mgr::GetVarName( xhandle Handle )
{
    return m_lVars(Handle).Name;
}

//=========================================================================

inline
xstring& global_var_mgr::GetVarNotes( xhandle Handle )
{
    return m_lVars(Handle).Notes;
}
                 
//=========================================================================

inline
void global_var_mgr::DestroyVariable( xhandle Handle )
{
    var&    Var   = m_lVars(Handle);
    
    switch( Var.Type )
    {
    case TYPE_FLOAT:
    case TYPE_INT:
    case TYPE_BOOL:
        m_lVarFIBs.DeleteByHandle( Var.hData );
        break;
    case TYPE_TIMER:
        m_lVarTimers.DeleteByHandle( Var.hData );
        break;
    case TYPE_GUID:
        m_lVarGuids.DeleteByHandle( Var.hData );
        break;
    default:
        ASSERT( 0 );
        break;
    }

    m_lVars.DeleteByHandle( Handle );
}

//=========================================================================

inline
xhandle global_var_mgr::CreateVarInt( const char* pName )
{
    xhandle Handle( CreateVar( pName, TYPE_INT, m_lVarFIBs ) );
    m_lVarFIBs( Handle ) = 0;
    return Handle;
}

//=========================================================================

inline
xhandle global_var_mgr::CreateVarFloat( const char* pName )
{
    return CreateVar( pName, TYPE_FLOAT, m_lVarFIBs );
}

//=========================================================================

inline
xhandle global_var_mgr::CreateVarBool ( const char* pName )
{
    return CreateVar( pName, TYPE_BOOL, m_lVarFIBs );
}

//=========================================================================

inline
xhandle global_var_mgr::CreateVarTimer( const char* pName )
{
    return CreateVar( pName, TYPE_TIMER, m_lVarTimers );
}

//=========================================================================

inline
xhandle global_var_mgr::CreateVarGuid ( const char* pName )
{
    return CreateVar( pName, TYPE_GUID, m_lVarGuids );
}

//=========================================================================

inline
s32& global_var_mgr::GetVarInt( xhandle Handle )
{
    ASSERT( m_lVarFIBs(Handle) == TYPE_INT );
    return *((s32*)&m_lVarFIBs(Handle));
}

//=========================================================================

inline
f32& global_var_mgr::GetVarFloat( xhandle Handle )
{
    ASSERT( m_lVarFIBs(Handle) == TYPE_FLOAT );
    return *((f32*)&m_lVarFIBs(Handle));
}

//=========================================================================

inline
xbool& global_var_mgr::GetVarBool( xhandle Handle )
{
    ASSERT( m_lVarFIBs(Handle) == TYPE_BOOL );
    return *((xbool*)&m_lVarFIBs(Handle));
}

//=========================================================================

inline
global_var_mgr::timer& global_var_mgr::GetVarTimer( xhandle Handle )
{
    return m_lVarTimers(Handle);
}

//=========================================================================

inline
guid& global_var_mgr::GetVarGuid( xhandle Handle )
{
    return m_lVarGuids(Handle);
}


extern global_var_mgr g_GVarMgr;
*/

//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
// GLOBALS
//=========================================================================

class var_mngr : public prop_interface
{
public:

	enum global_types
	{
        GLOBAL_NULL = 0,

        GLOBAL_FLOAT,
        GLOBAL_INT,
        GLOBAL_BOOL,
        GLOBAL_TIMER,
        GLOBAL_GUID,

        GLOBAL_END
    };

	enum variable_types
	{
        TYPE_NULL  = 0 ,

        TYPE_FLOAT  ,
        TYPE_INT    ,
        TYPE_BOOL   ,

        VARIABLE_TYPES_END
	};

    xhandle                     RegisterFloat       ( const char* pName );
    xhandle                     RegisterInt         ( const char* pName );
    xhandle                     RegisterBool        ( const char* pName );
    xhandle                     RegisterTimer       ( const char* pName );
    xhandle                     RegisterGuid        ( const char* pName );

    xbool                       DestroyVariable     ( xhandle& rVarHandle );
    xbool                       DestroyGuid         ( xhandle& rGuidHandle );
    xbool                       DestroyTimer        ( xhandle& rTimerHandle );

    f32                         GetFloat            ( xhandle& rVarHandle );
    s32                         GetInt              ( xhandle& rVarHandle );
    xbool                       GetBool             ( xhandle& rVarHandle );

    xbool                       GetVarHandle        ( const char* pName, xhandle* rVarHandle );
    xbool                       GetTimerHandle      ( const char* pName, xhandle* rTimerHandle );
    xbool                       GetGuidHandle       ( const char* pName, xhandle* rGuidHandle );

    variable_types              GetType             ( xhandle& rVarHandle );
    
    void                        SetFloat            ( xhandle& rVarHandle, f32   Value  );
    void                        SetInt              ( xhandle& rVarHandle, s32   Value  );
    void                        SetBool             ( xhandle& rVarHandle, xbool Value  );

    guid                        GetGuid             ( xhandle& rGuidHandle );
    void                        SetGuid             ( xhandle& rGuidHandle, guid  Value  );
    
    void                        ResetTimer          ( xhandle& rVarHandle );
    void                        StartTimer          ( xhandle& rVarHandle );
    void                        StopTimer           ( xhandle& rVarHandle );
    f32                         ReadTimer           ( xhandle& rVarHandle );

    xbool                       SaveRuntimeData     ( void );
    xbool                       LoadRuntimeData     ( void );
    
    void                        StoreRuntimeData    ( bitstream& BitStream );
    void                        RestoreRuntimeData  ( bitstream& BitStream );
    void                        DumpRuntimeData     ( bitstream& BitStream );

    void                        ClearData           ( void );

#ifdef X_EDITOR

    //Editor only Functions and data
    struct global_def 
    {
        global_types    Type;
        med_string      Name;
    };
    
    xbool                       GetGlobalsList      ( xharray<global_def>& Globals );
    xbool                       DoesGlobalExist     ( const char* pName );

#endif // X_EDITOR

public:

    //Editor functionality for restorting states

    void                        LoadGlobals         ( const char* pFileName );
#if defined( X_EDITOR )
    void                        SaveGlobals         ( const char* pFileName );
#endif // defined( X_EDITOR )
    void                        ImportGlobals       ( const char* pFileName );
    void                        StoreState          ( void );
    void                        ResetState          ( void );
    void                        ResetAllTimers      ( void );

public:
    
    //Property system implementation for saving and loading ...
    
    virtual			void	    OnEnumProp			( prop_enum&   rList );
    virtual			xbool	    OnProperty			( prop_query&  rPropQuery );
          
    virtual			void	    OnEnumPropInternal	( prop_enum&   rList );
    virtual			xbool	    OnPropertyInternal  ( prop_query&  rPropQuery );

    virtual			void	    OnEnumPropVariables ( prop_enum&   rList, u32 Flags = 0 );
    virtual			xbool	    OnPropertyVariables	( prop_query&  rPropQuery );

protected:
        
    struct var_key 
    {
        void      Copy( const var_key& rSource );
       
        med_string  m_Name;                     // Name of variable..
        u16         m_Type;                     // Type of the variable used to access the correct array
        u32         m_Data;                     // Data itself
    };
 
    struct timer_key 
    {
        void      Copy( const timer_key& rSource );
       
        med_string  m_Name;                     // Name of variable..
        xtimer      m_Timer;                    // The timer associated with this variable.
    };
    
    struct guid_key 
    {
        void      Copy( const guid_key& rSource );
       
        med_string  m_Name;                     // Name of variable..
        guid        m_Guid;                     // The guid associated with this variable.
    };

protected:

    xhandle                     RegisterVariable    ( variable_types Type, const char* pName );
    void                        ResetKeys           ( void );
    
protected:

    xharray<var_key>        m_VarKeys;          // Variables Key Array..
    xharray<timer_key>      m_TimerKeys;        // Timer Key Array..
    xharray<guid_key>       m_GuidKeys;         // Guid Key Array..
    xharray<var_key>        m_StoreVarKeys;     // Variables Key Array for state restore..
    xharray<guid_key>       m_StoreGuidKeys;    // Variables Key Array for state restore..
    xharray<var_key>        m_RuntimeVarKeys;   // Variables Key Array for state restore..
    xharray<guid_key>       m_RuntimeGuidKeys;  // Variables Key Array for state restore..
};

extern var_mngr g_VarMgr;


#endif//_GLOBAL_VARIABLE_MANAGER_