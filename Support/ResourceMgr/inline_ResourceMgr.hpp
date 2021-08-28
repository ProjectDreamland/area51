
#ifndef INLINE_RESOURCE_MANAGER_HPP
#define INLINE_RESOURCE_MANAGER_HPP

//==============================================================================
//==============================================================================
// RSC_LOADER
//==============================================================================
//==============================================================================

inline rsc_loader::rsc_loader( const char* pType, const char* pExt )
{
    // Confirm that extension begins with a dot
    ASSERT( pExt[0] == '.' );

    m_pType = pType;
    m_pExt  = pExt;
    m_pNext = rsc_mgr::m_pLoader;
    rsc_mgr::m_pLoader = this;
    rsc_mgr::m_NumLoaders++;
}


//==============================================================================
//==============================================================================
//  RHANDLE FUNCTIONS
//==============================================================================
//==============================================================================

//==============================================================================

template< class T > inline
T* rhandle<T>::GetPointer( void ) const
{ 
    return (T*)rhandle_base::GetPointer(); 
}

//==============================================================================
//==============================================================================
//  RHANDLE BASE FUNCTIONS
//==============================================================================
//==============================================================================

//==============================================================================

inline
rhandle_base::rhandle_base( void )
{
    SetIndex( -1 );
}

//==============================================================================
inline
rhandle_base::rhandle_base( const char* pResourceName )
{
    SetIndex( -1 );
    g_RscMgr.AddRHandle( *this, pResourceName );
}

//==============================================================================
inline
rhandle_base::~rhandle_base( void )
{
}

//==============================================================================
inline
void rhandle_base::Destroy( void )
{
    SetIndex( -1 );
}

//==============================================================================
inline
void rhandle_base::SetName( const char* pResourceName )
{
    if( pResourceName != NULL )
        g_RscMgr.AddRHandle( *this, pResourceName );
}

//==============================================================================
inline
const char* rhandle_base::GetName( void ) const
{
    return g_RscMgr.GetRHandleName( *this );
}

//==============================================================================
inline
void* rhandle_base::GetPointer( void ) const
{
    return g_RscMgr.GetPointer( *this );
}

//==============================================================================
inline
xbool rhandle_base::IsLoaded( void ) const
{
    return g_RscMgr.IsRHandleLoaded( *this );
}

//==============================================================================
inline
s16 rhandle_base::GetIndex( void ) const
{
    return (s16)m_Data;
}

//==============================================================================
inline
void rhandle_base::SetIndex( s16 I )
{
    m_Data = I;
}

//==============================================================================

inline
void* rsc_mgr::GetPointer( const rhandle_base& RHandle )
{
    s16 I = RHandle.GetIndex();

    // Handle null handle
    if( I == -1 )
        return NULL;

    // Confirm range 
    ASSERT( (I>=0) && (I<m_nResourcesAllocated) );
    ASSERT( m_pResource[I].State != NOT_USED );

    if( m_pResource[I].pData )
    {
        return m_pResource[I].pData;
    }

    return GetPointerSlow(RHandle);
}

//==============================================================================
// END
//==============================================================================
#endif