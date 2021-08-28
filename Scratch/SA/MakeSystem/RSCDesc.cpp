
#include <io.h>
#include "RscDesc.hpp"

//=========================================================================
// LOCAL VARIABLES
//=========================================================================
rsc_desc_type*   rsc_desc_type::s_pHead = NULL;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

const rsc_desc_type*  rsc_desc_mgr::GetFirstType( void ) const
{
    return rsc_desc_type::s_pHead;
}

//=========================================================================

const rsc_desc_type*  rsc_desc_mgr::GetNextType( const rsc_desc_type* PrevType ) const
{
    return PrevType->m_pNext;
}

//=========================================================================

rsc_desc_mgr::node& rsc_desc_mgr::GetRscDesc( const char* pName )
{
    s32 n = m_lRscDesc.GetCount();
    for( s32 i=0; i<n; i++ )
    {
        if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetRscName() ) == 0 )
        {
            return m_lRscDesc[i];
        }
    }

    e_throw( xfs("unable to find [%s] RscDesc", pName ) );

    return m_lRscDesc[i];
}

//=========================================================================

const rsc_desc_type& rsc_desc_mgr::GetType( const char* pType ) const
{
    ASSERT( pType );

    for( const rsc_desc_type* pLoop = GetFirstType(); pLoop; pLoop = GetNextType( pLoop ) )
    {
        if ( x_stricmp( pType, pLoop->GetName() ) == 0 )
            return *pLoop;
    }

    e_throw( xfs("unable to find [%s] RscDesc", pType ) );

    return *pLoop;
}


//=========================================================================

rsc_desc_mgr::rsc_desc_mgr( void )
{
    m_IsCompiling = FALSE;
}

//=========================================================================

rsc_desc& rsc_desc_mgr::CreateRscDesc( const char* pType )
{
    ASSERT( pType );

    if( m_IsCompiling )
        e_throw( "You can't build any more resources while compiling" );

    rsc_desc* pRscDsc = GetType( pType ).CreateRscDesc();

    if( pRscDsc == NULL ) 
        e_throw( xfs("Out of memory while creating a RscDesc of type [%s]", pType));

    // Add the node into the list
    node& Node = m_lRscDesc.Add();
    Node.pDesc = pRscDsc;
    Node.Flags = FLAGS_UPDATE_DEP;
    Node.nDeps = 0;

    return *pRscDsc;
}

//=========================================================================

s32 rsc_desc_mgr::FindRscDesc( const char* pName )
{
    ASSERT( pName );
    for( s32 i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetRscName() ) == 0 )
            return i;
    }

    e_throw( xfs("unable to find [%s] RrcDesc", pName ));
    return -1;
}

//=========================================================================

void rsc_desc_mgr::DeleteRscDesc( const char* pName )
{    
    if( m_IsCompiling )
        e_throw( "You can't delete any resources while compiling" );

    s32 Index = FindRscDesc( pName );

    // Remove all the dependencies
    for( s32 i=0; i<m_lRscDesc[Index].nDeps; i++ )
    {
        DelDep( m_lRscDesc[i].Dep[i] );
    }

    // Delete the actual Description
    delete m_lRscDesc[i].pDesc;

    // Now we can delete the node
    m_lRscDesc.DeleteByIndex( Index );
}


//=========================================================================

xhandle rsc_desc_mgr::FindDep( const char* pFileName )
{
    xhandle hHandle;
    hHandle.Handle = -1;
    
    ASSERT( pFileName );
    s32 L = x_strlen( pFileName );
    if( L == 0 ) return hHandle;

    for( s32 i=0; m_lDependency.GetCount(); i++ )
    {
        dependency& Dep = m_lDependency[i];

        if( x_stricmp( Dep.FileName, pFileName )== 0 ) 
        {
            hHandle = m_lDependency.GetHandleByIndex( i );
            break;
        }
    }

    return hHandle;
}

//=========================================================================

xhandle rsc_desc_mgr::AddDep( const char* pFileName )
{
    xhandle hHandle = FindDep( pFileName );

    // We already have that dependency
    if( hHandle.IsNull() == FALSE )
    {
        m_lDependency(hHandle).RefCount++;
        return hHandle;
    }

    // Add the dependency
    dependency& Dep = m_lDependency.Add();

    Dep.Flags    = 0;
    Dep.RefCount = 1;
    x_strcpy( Dep.FileName, pFileName );

    return hHandle;
}

//=========================================================================

void rsc_desc_mgr::DelDep( xhandle hHandle )
{
    ASSERT( hHandle.IsNull() == FALSE );

    // Nuke the dependency if we have to
    m_lDependency(hHandle).RefCount--;
    if( m_lDependency(hHandle).RefCount == 0 )
        m_lDependency.DeleteByHandle( hHandle );
}

//=========================================================================

void rsc_desc_mgr::DelDep( const char* pFileName )
{
    xhandle Handle = FindDep( pFileName );
    if( Handle.IsNull() )
        e_throw( "Unable to find dependency" );

    DelDep( Handle );
}

//=========================================================================

s32 rsc_desc_mgr::BeginCompiling( void )
{
    s32 j, i;
    ASSERT( m_IsCompiling == FALSE );

    //
    // First go throw all the objec desc and update their dependencies
    //
    xarray<prop_enum> PropEnum;
    for( i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        node& Node = m_lRscDesc[i];

        // All seem okay from here move to the next one
        if( (Node.Flags & FLAGS_UPDATE_DEP) == 0 )
            continue;

        // Get all the dependencies out of the resource descriptor
        PropEnum.Clear();
        Node.pDesc->GetDependencies( PropEnum );

        // Nuke all the previous dependencies
        for( j=0; j<Node.nDeps; j++ )
        {
            DelDep( Node.Dep[j] );
        }

        // Add all the new ones
        for( j=0; j<PropEnum.GetCount(); j++ )
        {
            AddDep( PropEnum[j].String );
        }

        // Nuke all the external resources
        //...

        // Update the node
        Node.Flags &= ~FLAGS_UPDATE_DEP;
    }

    //
    // Find the first node that needs to be compile
    //
    for( m_CompilingIndex=0; m_CompilingIndex<m_lRscDesc.GetCount(); m_CompilingIndex++ )
    {
        if( NeedCompiling( m_lRscDesc[m_CompilingIndex] ) == TRUE )
        break;        
    }

    if( m_CompilingIndex == m_lRscDesc.GetCount() )
    {
        return -1;
    }

    // Set the 
    m_IsCompiling = TRUE;

    return m_CompilingIndex;
}

//=========================================================================

s32 rsc_desc_mgr::NextCompiling    ( void )
{
    ASSERT( m_IsCompiling == TRUE );

    //
    // Start from where we left of in the BeginCompile.
    //
    for( ; m_CompilingIndex<m_lRscDesc.GetCount(); m_CompilingIndex++ )
    {
        if( NeedCompiling( m_lRscDesc[m_CompilingIndex] ) == TRUE )
        {
            m_CompilingIndex++;
            break;        
        }
    }

    if( m_CompilingIndex == m_lRscDesc.GetCount() )
    {
        return -1;
    }

    return m_CompilingIndex;
}

//=========================================================================

void rsc_desc_mgr::EndCompiling( void )
{
    ASSERT( m_IsCompiling == TRUE );
    m_IsCompiling = FALSE;
}

//=========================================================================

bool rsc_desc_mgr::NeedCompiling ( node& RscDesc )
{
    long            hFile;
    long            hDepFile;
    _finddata_t     c_file;
    _finddata_t     c_depfile;
    char            Path[]= {"C:\\Projects\\A51\\Scratch\\TA\\MakeSystem"};

    // Get the resource file and check if it needs compiling.
    if( (hFile = _findfirst( xfs( "%s\\%s.%s", Path, RscDesc.pDesc->GetRscName(), RscDesc.pDesc->GetRscType() ), &c_file )) == -1L )
        ASSERTS( FALSE, "Resource Desc File Missing" );

    // Check the last time write on the resours if it is getter than the last compile time then
    //  just compile the resource and don't worry about checking the dependencies.

    if( c_file.time_write > RscDesc.TimeStamp )
    {
        // Update the time stamp
//        RscDesc.TimeStamp = time();

        _findclose( hFile );
        return TRUE;
    }

    // Update the time stamp
//    RscDesc.TimeStamp = time();

    xarray<prop_enum> DependencyList;
    RscDesc.pDesc->GetDependencies( DependencyList );

    // Check all the dependecies for the resource.
    for( s32 j = 0; j < DependencyList.GetCount(); j++ )
    {
        if( (hDepFile = _findfirst( xfs( "%s\\%s", Path, DependencyList[j].String ), &c_depfile )) == -1L )
            ASSERTS( FALSE, "Dependency File Missing" );

        if( c_depfile.time_write > RscDesc.TimeStamp )
        {
            _findclose( hDepFile );
            return TRUE;
        }

        _findclose( hDepFile );
    }

    return FALSE;
}

//=========================================================================

void rsc_desc_mgr::AddExternalRsc ( const char* pExternalInfo, s32 Index )
{
    (void)pExternalInfo;
    (void)Index;

}

//=========================================================================