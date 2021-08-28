
#include <io.h>
#include "RscDesc.hpp"
#include <time.h>

//=========================================================================
// TYPES
//=========================================================================
class external_rsc_desc : public rsc_desc
{
public:
    CREATE_RTTI( external_rsc_desc, rsc_desc, rsc_desc );

    virtual void    EnumProp            ( xarray<prop_enum>& List ) {}
    virtual xbool   Property            ( prop_query& I ) {return TRUE; }
    virtual void    GetDependencies     ( xarray<prop_enum>& List    ) {}; 
    virtual void    GetCompilerRules    ( xstring& CompilerRules     ) { CompilerRules = m_CompilerRules; };

    external_rsc_desc( void );

    xstring  m_CompilerRules;
};

//
// lets the compiler know about the resource desc type
//
DEFINE_RSC_TYPE( s_Desc, external_rsc_desc, "???", "EXTERNAL", "External Resource" );

//=========================================================================
// LOCAL VARIABLES
//=========================================================================
rsc_desc_type*   rsc_desc_type::s_pHead = NULL;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

external_rsc_desc::external_rsc_desc( void ) : rsc_desc( s_Desc ) {}

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
        if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetName() ) == 0 )
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
    x_strcpy( m_WorkingDir, "C:\\Projects\\A51\\Scratch\\TA\\MakeSystem" );
}

//=========================================================================

rsc_desc& rsc_desc_mgr::CreateRscDesc( const char* pName )
{
    ASSERT( pName );

    if( m_IsCompiling )
        e_throw( "You can't build any more resources while compiling" );

    // Get extension
    char Extension[256];
    Extension[0]=0;
    x_splitpath( pName, NULL,NULL,NULL, Extension );
    if( Extension[0] == 0 || Extension[0] != '.' )
        e_throw( xfs("When creating a resource name it must have an extension [%s]", pName ));

    // Create resource descriptor
    rsc_desc* pRscDsc = GetType( &Extension[1] ).CreateRscDesc();

    if( pRscDsc == NULL ) 
        e_throw( xfs("Out of memory while creating a RscDesc of type [%s]", pName));

    pRscDsc->SetName( pName );

    // Add the node into the list
    node& Node      = m_lRscDesc.Add();
    Node.pDesc      = pRscDsc;
    Node.Flags      = 0;
    Node.nDeps      = 0;
    Node.nExternals = 0;
    Node.TimeStamp  = 0;
    Node.RefCount   = 1;

    return *pRscDsc;
}

//=========================================================================

s32 rsc_desc_mgr::FindRscDesc( const char* pName )
{
    ASSERT( pName );
    for( s32 i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetName() ) == 0 )
            return i;
    }

    e_throw( xfs("unable to find [%s] RrcDesc", pName ));
    return -1;
}

//=========================================================================

void rsc_desc_mgr::DeleteRscDesc( xhandle hRsc )
{    
    DeleteRscDesc( m_lRscDesc(hRsc).pDesc->GetName() );
}

//=========================================================================

void rsc_desc_mgr::DeleteRscDesc( const char* pName )
{    
    if( m_IsCompiling )
        e_throw( "You can't delete any resources while compiling" );

    s32 Index = FindRscDesc( pName );
    s32 i;

    // Remove all the dependencies
    for( i=0; i<m_lRscDesc[Index].nDeps; i++ )
    {
        DelDep( m_lRscDesc[Index].Dep[i] );
    }

    // Delete all the external dep
    for( i=0; i<m_lRscDesc[Index].nExternals; i++ )
    {
        DeleteRscDesc( m_lRscDesc[Index].External[i] );
    }

    // Delete the actual Description
    m_lRscDesc[Index].RefCount--;
    if( m_lRscDesc[Index].RefCount <= 0 )
    {
        if( m_lRscDesc[Index].pDesc )
            delete m_lRscDesc[Index].pDesc;

        // Now we can delete the node
        m_lRscDesc.DeleteByIndex( Index );
    }
}


//=========================================================================

xhandle rsc_desc_mgr::FindDep( const char* pFileName )
{
    xhandle hHandle;
    hHandle.Handle = -1;
    
    ASSERT( pFileName );
    s32 L = x_strlen( pFileName );
    if( L == 0 ) return hHandle;

    for( s32 i=0; i<m_lSrcDep.GetCount(); i++ )
    {
        dependency& Dep = m_lSrcDep[i];

        if( x_stricmp( Dep.FileName, pFileName )== 0 ) 
        {
            hHandle = m_lSrcDep.GetHandleByIndex( i );
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
        m_lSrcDep(hHandle).RefCount++;
        return hHandle;
    }

    // Add the dependency
    dependency& Dep = m_lSrcDep.Add( hHandle );

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
    m_lSrcDep(hHandle).RefCount--;
    if( m_lSrcDep(hHandle).RefCount == 0 )
        m_lSrcDep.DeleteByHandle( hHandle );
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
// external rsc example: "dep: test.txt @ compiler -Input test.txt ... "
// But pExternalInfo = "test.txt @ compiler -Input test.txt ... "
//=========================================================================
void rsc_desc_mgr::ParseExternalRsc( const char* pExternalInfo, char* pRscName, xstring& CompilerRules )
{
    s32 i,j;

    // Skip any white spaces and " 
    for( i=0; pExternalInfo[i] && (pExternalInfo[i] == ' ' || pExternalInfo[i] == '"'); i++ );

    // Skip the fist part of the path
    for( j=0; pExternalInfo[i] && x_toupper(m_WorkingDir[j]) == x_toupper(pExternalInfo[i]); i++ );

    // copy rsc name
    for( j=0; pRscName[j] = pExternalInfo[i] && pExternalInfo[i] != '@' && pExternalInfo[i] != '"'; i++, j++ )
        if( j >= 128 )
            e_throw( xfs("External Resurce name was too long [%s]", pExternalInfo));

    // Then just terminate the string
    if( pExternalInfo[i] == '"' )
    {
        pRscName[j] = 0;

        // Find our terminator
        for( i=0; pExternalInfo[i] && pExternalInfo[i] != '@'; i++ );

        if( pExternalInfo[i] == 0 )
            e_throw( xfs("Error: The depencency string was not formated corectly.\n[%s]", pExternalInfo ));
    }
    else if( pExternalInfo[i] == '@' )
    {
        // Skip any white spaces for the name
        while( j>0 && (pRscName[j] == ' ' || pRscName[j] == '@') ) j--;
    
        if( j == 0 )
            e_throw( xfs("Missing external resource name\n[%s]", pExternalInfo ));
    }
    else
    {
        ASSERT( pExternalInfo[i] == 0 );
        e_throw( xfs("Error: Dependency string must have been formatted worng\n[%s]", pExternalInfo ));
    }

    // Skip the strange symbol
    ASSERT( pExternalInfo[i] == '@' );
    i++;

    // Skip any white spaces
    for( i=0; pExternalInfo[i] && pExternalInfo[i] == ' '; i++ );

    // Finally copy the compiler rules
    CompilerRules = pExternalInfo[i];
}

//=========================================================================

s32 rsc_desc_mgr::BeginCompiling( void )
{
    s32 j, i;
    ASSERT( m_IsCompiling == FALSE );
    m_IsCompiling = TRUE;

    //
    // First go throw all the objec desc and update their dependencies
    //
    xarray<prop_enum> PropEnum;
    for( i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        node& Node = m_lRscDesc[i];

        // Get all the dependencies out of the resource descriptor
        PropEnum.Clear();
        Node.pDesc->GetDependencies( PropEnum );

        //
        // Nuke all the previous dependencies
        //
        for( j=0; j<Node.nDeps; j++ )
        {
            DelDep( Node.Dep[j] );
        }

        // Add all the new ones
        if( PropEnum.GetCount() > 32 )
            e_throw( xfs("Internal error. We got too many dependencies for resource [%s]",Node.pDesc->GetName() ));

        Node.nDeps  = PropEnum.GetCount();       
        for( j=0; j<PropEnum.GetCount(); j++ )
        {
            Node.Dep[j] = AddDep( PropEnum[j].String );
        }

        //
        // Nuke all the external resources
        //
        for( j=0; j<Node.nExternals; j++ )
        {
            DeleteRscDesc( Node.External[j] );
        }
        Node.nExternals = 0;
    }

    //
    // Do a first quick time check pass for all dependencies
    //
    for( i = 0; i < m_lSrcDep.GetCount(); i++ )
    {
        long        hDepFile;
        _finddata_t c_depfile;

        hDepFile = _findfirst( xfs( "%s\\%s", m_WorkingDir, m_lSrcDep[j].FileName ), &c_depfile );

        // Could not find dependency so set the date to zero
        // To force recompilation of resources
        if( hDepFile == -1L )
        {
            xhandle hHandle = m_lSrcDep.GetHandleByIndex( i );
            for( s32 k=0; k<m_lRscDesc.GetCount(); k++ )
            {
                node& Rsc = m_lRscDesc[k];
                for(s32 t=0; t<Rsc.nDeps; t++ )
                {
                    if( Rsc.Dep[t] == hHandle )
                    {
                        Rsc.Flags |= FLAGS_SKIP;
                        break;
                    }
                }
            }

            m_lSrcDep[j].TimeStamp = 0;
            e_throw( xfs("A Dependency [%s] could not find in the hard-disk.\n So it mark all resources that use it to skip this compile.", m_lSrcDep[j].FileName ));
        }
        else
        {
            // Write good time
            m_lSrcDep[j].TimeStamp = c_depfile.time_write;
            _findclose( hDepFile );
        }
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

    return m_CompilingIndex;
}

//=========================================================================

void rsc_desc_mgr::AddExternalRsc( const char* pExternalInfo, s32 Index )
{
    char        RscName[128];
    xstring     CompilerRules;
    xhandle     hHandle;

    //
    // parse 
    //
    ParseExternalRsc( pExternalInfo, RscName, CompilerRules );

    // Find whether the external resource already exits 
    for( s32 i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        if( x_stricmp( RscName, m_lRscDesc[i].pDesc->GetName() ) == 0 )
        {
            break;
        }
    }

    if( i == m_lRscDesc.GetCount() )
    {
        node& Node      = m_lRscDesc.Add( hHandle );
        Node.pDesc      = new external_rsc_desc;
        if( Node.pDesc == NULL )
            e_throw( "Fatal error. Out of memory" );

        Node.Flags      = FLAGS_EXTERNAL;
        Node.nDeps      = 0;
        Node.nExternals = 0;
        Node.TimeStamp  = 0;
        Node.RefCount   = 1;

        // Initialize external resource
        Node.pDesc->SetName     ( RscName );   
//        Node.pDesc->SetSrcName  ( m_lRscDesc[i].pDesc->GetName() );
        ((external_rsc_desc*)Node.pDesc)->m_CompilerRules = CompilerRules;
    }
    else
    {
        node& Node = m_lRscDesc[Index];
        hHandle    = m_lRscDesc.GetHandleByIndex(i);

        // Check to make sure we don't add duplicates.
        for( i=0; i<Node.nExternals; i++) 
        {
            if( Node.External[i] == hHandle )
                break;
        }

        if( i == Node.nExternals )
        {
            if( i >= 32 ) 
                e_throw( xfs("Internal Error. The resource [%s] Had too many dependencies", m_lRscDesc[i].pDesc->GetName() ));

            m_lRscDesc(hHandle).RefCount++;
        }
    }

    //
    // Update the total number of dependencies
    //
    m_lRscDesc[Index].External[ m_lRscDesc[Index].nExternals ] = hHandle;
    m_lRscDesc[Index].nExternals++;
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

xbool rsc_desc_mgr::NeedCompiling( node& RscDesc )
{
    long            hFile;
    _finddata_t     c_final;

    // Check the skip flag first
    if( RscDesc.Flags & FLAGS_SKIP )
        return FALSE;

    // Get the source resource file and check if it needs compiling.
    hFile = _findfirst( xfs( "%s\\%s", m_WorkingDir, RscDesc.pDesc->GetName()), &c_final );
    if( hFile == -1L )
        return TRUE;

    _findclose( hFile );

    // Check all the dependecies for the resource are same or older.
    for( s32 j = 0; j < RscDesc.nDeps; j++ )
    {
        if( c_final.time_write < m_lSrcDep( RscDesc.Dep[j] ).TimeStamp )
            return TRUE;
    }

    // We don't need to compile the file, yea!
    return FALSE;
}

//=========================================================================