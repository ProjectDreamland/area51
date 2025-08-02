#include "x_files.hpp"
#if defined(X_EDITOR)
#include "StdAfx.h"
#endif
#include <io.h>
#include <stdio.h>
#include "RscDesc.hpp"
#include <time.h>
#include <errno.h>

#if defined(X_EDITOR)
#include "..\..\Support\ResourceMgr\ResourceMgr.hpp"
#include "..\WorldEditor\worldeditor.hpp"
#include "GenericDialog\GenericDialog.hpp"
#include "..\Editor\project.hpp"
#endif

//=========================================================================
// DATA
//=========================================================================
xbool rsc_desc::s_bVerbose = FALSE;


//=========================================================================
// TYPES
//=========================================================================
class external_rsc_desc : public rsc_desc
{
public:
    CREATE_RTTI( external_rsc_desc, rsc_desc, rsc_desc );

    virtual void    OnEnumProp                  ( prop_enum&  List ) {}
    virtual xbool   OnProperty                  ( prop_query& I )            {return TRUE; }
    virtual void    OnGetCompilerDependencies   ( xarray<xstring>& List    ) {}; 
    virtual void    OnGetCompilerRules          ( xstring& CompilerRules     ) { CompilerRules = m_CompilerRules; };
    virtual void    OnCheckIntegrity            ( void )                       {}
    virtual void    OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory ) {}
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
rsc_desc_mgr                    g_RescDescMGR;

//=========================================================================
// FUNCTIONS
//=========================================================================

static inline u32 ComputeUppercaseStringHash( const char* pString)
{
    u32 Hash = 5381;
    s32 C;
 
    // Process each character to generate the hash key
    while( (C = *pString++) )
    {
        if( (C >= 'a') && (C <= 'z') )
            C += ('A' - 'a');
        Hash = ((Hash<<5) + Hash) ^ C;
    }

    return Hash;
}

//=========================================================================

void rsc_desc_mgr::dependency::SetName( const char* pName )
{
    x_strncpy( Name, pName, 255 );
    NameHash = ComputeUppercaseStringHash(pName);
}

//=========================================================================

external_rsc_desc::external_rsc_desc( void ) : rsc_desc( s_Desc ) {}

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::RefreshDesc( void )
{
    CONTEXT( "rsc_desc_mgr::RefreshDesc" );

    ClearRscDesc();
    ScanForRscDesc();
    RefreshAllCompiledRsc();
}
#endif

//=========================================================================

void rsc_desc_mgr::ClearRscDesc( void )
{
    for( s32 i=0; i<GetRscDescCount(); i++ )
    {
        // TODO: Is it possible to have lock rsc desc because some other editors?
        node& Node = GetRscDescIndex( i );
        delete Node.pDesc;
    }

    m_lRscDesc.Clear();
    m_lSrcDep.Clear();
}

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::ScanForRscDesc( const char* DirName, long& hFile, _finddata_t& Data, const char* ThemeName )
{
    //
    // If it is a folder let it recurse because we are trying to find all data
    //
    while( Data.attrib & _A_SUBDIR )
    {
        struct _finddata_t  c_Subfile;
        long                hSubFile;
        char                SubDirName[256];

        if( Data.name[0] != '.' )
        {
            x_sprintf( SubDirName, "%s\\%s", DirName, Data.name );
    
            // handle folder
            if( (hSubFile = _findfirst( xfs("%s\\*", SubDirName), &c_Subfile )) != -1L )
            {
                ScanForRscDesc( SubDirName, hSubFile, c_Subfile, ThemeName );
                _findclose( hSubFile );
            }
        }

        // handle the next thing.
        if( _findnext( hFile, &Data ) != 0 )
            return;
    }

    //
    // okay so it is a file we need to do something about it
    //
    do
    {
        // handle folders
        if( Data.attrib & _A_SUBDIR )
            ScanForRscDesc( DirName, hFile, Data, ThemeName );

        // Create the rsc desc
        // Get extension
        char Extension[256];
        Extension[0]=0;
        x_splitpath( Data.name, NULL,NULL,NULL, Extension );
        if( Extension[0] == 0 || Extension[1] == '.' || Extension[0] != '.' || Extension[1] == 0 )
        {
            // No extension so nothing to do
        }
        else
        {
            //
            // Create resource descriptor
            //
            rsc_desc* pRscDsc = NULL;
            x_try;
                pRscDsc = GetType( &Extension[1] ).CreateRscDesc();

                if( pRscDsc == NULL ) 
                {
                    // Could not create the rsc desc.
                }
                else
                {
                    // Load the rsc desc
                    text_in TextIn;
                    TextIn.OpenFile( xfs("%s\\%s",DirName,Data.name) );
                    pRscDsc->OnLoad( TextIn );
                    TextIn.CloseFile();

                    // Set an actual node for this
                    node& Node      = m_lRscDesc.Add();
                    Node.pDesc      = pRscDsc;
                    Node.Flags      = 0;
                    Node.nExternals = 0;
                    Node.TimeStamp  = 0;
                    Node.RefCount   = 1;
                    Node.Theme      = ThemeName;
                }

            x_catch_begin;
            // TODO: Ignore exceptions for now...
            // We may need to handle exception that cames out of the loader expecial.
            x_catch_end;

            //
            // A bit of nanity check
            //
            if (pRscDsc) 
            {
                char Path[256];
                pRscDsc->GetFullName( Path );

                if( x_stricmp( Path, xfs("%s\\%s",DirName,Data.name) ) != 0 )
                {
                    ClearRscDesc();
                    x_throw( xfs("the resc name:\n[%s]\nDoesn't match with the actual file name:\n[%s]", Path, xfs("%s\\%s",DirName,Data.name)) );
                }    
            }            
        }

    } while( _findnext( hFile, &Data ) == 0 );
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::ScanForRscDesc( void )
{
    struct _finddata_t  c_file;
    long                hFile;

    // Do we have anything to do?
    if( g_Project.IsProjectOpen() == FALSE )
        return;

    char Path[256];
    for( s32 i = g_Project.GetFirstResourceDir( Path ); i != -1; i = g_Project.GetNextResourceDir( i, Path ) )
    {
        xstring ThemeName;
        if ( i == 0)  
        {
            ThemeName = xfs("Project::%s",g_Project.GetName());
        }
        else
        {
            ThemeName = g_Project.GetThemeName(i-1);
        }

        if( (hFile = _findfirst( xfs("%s\\*", Path), &c_file )) == -1L )
        {
            // Nothing to do
        }
        else
        {
            ScanForRscDesc( Path, hFile, c_file, ThemeName );

            // Done scanning drive
            _findclose( hFile );
        }
    }
}
#endif

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

rsc_desc_mgr::node* rsc_desc_mgr::GetRscDesc( const char* pName )
{
    u32 Hash = ComputeUppercaseStringHash( pName );
    s32 n = m_lRscDesc.GetCount();
    for( s32 i=0; i<n; i++ )
    {
        if( m_lRscDesc[i].pDesc->GetNameHash() == Hash )
        {
            if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetName() ) == 0 )
            {
                return &m_lRscDesc[i];
            }
        }
    }

    //x_throw( xfs("unable to find [%s] RscDesc", pName ) );

    return NULL;
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

    x_throw( xfs("unable to find [%s] RscDesc", pType ) );

    return *pLoop;
}


//=========================================================================

rsc_desc_mgr::rsc_desc_mgr( void )
{
    m_IsCompiling = FALSE;
    m_bStopBuild  = FALSE;

    m_Platform[0].Platform = PLATFORM_PC;
    m_Platform[0].bCompile = TRUE;
    x_sprintf( m_Platform[0].OutputDir, "PC"  );
    x_strcpy( m_Platform[0].Cmd, "PC" );

    m_Platform[1].Platform = PLATFORM_GCN;
    m_Platform[1].bCompile = FALSE;
    x_sprintf( m_Platform[1].OutputDir, "GCN" );
    x_strcpy( m_Platform[1].Cmd, "GCN" );

    m_Platform[2].Platform = PLATFORM_XBOX;
    m_Platform[2].bCompile = FALSE;
    x_sprintf( m_Platform[2].OutputDir, "XBOX" );
    x_strcpy( m_Platform[2].Cmd, "XBOX" );

    m_Platform[3].Platform = PLATFORM_PS2;
    m_Platform[3].bCompile = FALSE;
    x_sprintf( m_Platform[3].OutputDir, "PS2" );
    x_strcpy( m_Platform[3].Cmd, "PS2" );
}

//=========================================================================

rsc_desc& rsc_desc_mgr::CreateRscDesc( const char* pName )
{
    ASSERT( pName );

    if( m_IsCompiling )
        x_throw( "You can't build any more resources while compiling" );

    {
        s32 n = m_lRscDesc.GetCount();
        for( s32 i=0; i<n; i++ )
        {
            if( x_stricmp( pName, m_lRscDesc[i].pDesc->GetName() ) == 0 )
            {
                x_throw( xfs("Unable to create the new resource[%s]\nBecause it already exits", pName) );
            }
        }
    }

    // Get extension
    char Extension[256];
    Extension[0]=0;
    x_splitpath( pName, NULL,NULL,NULL, Extension );
    if( Extension[0] == 0 || Extension[0] != '.' )
        x_throw( xfs("When creating a resource name it must have an extension [%s]", pName ));

    //
    // Create resource descriptor
    //
    rsc_desc* pRscDsc = NULL;
    xhandle   Handle( HNULL );
    
    // Handle errors
    x_try;

    pRscDsc = GetType( &Extension[1] ).CreateRscDesc();

    if( pRscDsc == NULL ) 
        x_throw( xfs("Out of memory while creating a RscDesc of type [%s]", pName));

    pRscDsc->SetFullName( pName );

    // Add the node into the list
    node& Node      = m_lRscDesc.Add( Handle );
    Node.pDesc      = pRscDsc;
    Node.Flags      = 0;
    Node.nExternals = 0;
    Node.TimeStamp  = 0;
    Node.RefCount   = 1;

    // Clean up time
    x_catch_begin;

    if( pRscDsc ) delete pRscDsc;

    if( Handle.IsNonNull() )
    {
        m_lRscDesc.DeleteByHandle( Handle );
    }

    x_catch_end_ret;


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

    x_throw( xfs("unable to find [%s] RrcDesc", pName ));
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
        x_throw( "You can't delete any resources while compiling" );

    s32 Index = FindRscDesc( pName );
    s32 i;

    // Remove all the dependencies
    for( i=0; i<m_lRscDesc[Index].Dependency.GetCount(); i++ )
    {
        DelDep( m_lRscDesc[Index].Dependency[i] );
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

    u32 Hash = ComputeUppercaseStringHash(pFileName);

    s32 Count = m_lSrcDep.GetCount();
    for( s32 i=0; i<Count; i++ )
    {
        dependency& Dep = m_lSrcDep[i];
        if( Dep.NameHash == Hash )
        {
            if( x_stricmp( Dep.GetName(), pFileName )== 0 ) 
            {
                hHandle = m_lSrcDep.GetHandleByIndex( i );
                break;
            }
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
    Dep.SetName( pFileName );

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
        x_throw( "Unable to find dependency" );

    DelDep( Handle );
}


//=========================================================================
// external rsc example: "dep: test.txt @ compiler -Input test.txt ... "
// But pExternalInfo = "test.txt @ compiler -Input test.txt ... "
//=========================================================================
void rsc_desc_mgr::ParseExternalRsc( const char* pExternalInfo, char* pRscName, xstring& CompilerRules )
{
    ASSERT( 0 );
    /*
    s32 i,j;

    // Skip any white spaces and " 
    for( i=0; pExternalInfo[i] && (pExternalInfo[i] == ' ' || pExternalInfo[i] == '"'); i++ );

    // Skip the fist part of the path
    for( j=0; pExternalInfo[i] && x_toupper(m_WorkingDir[j]) == x_toupper(pExternalInfo[i]); i++ );

    // copy rsc name
    for( j=0; pRscName[j] = pExternalInfo[i] && pExternalInfo[i] != '@' && pExternalInfo[i] != '"'; i++, j++ )
        if( j >= 128 )
            x_throw( xfs("External Resurce name was too long [%s]", pExternalInfo));

    // Then just terminate the string
    if( pExternalInfo[i] == '"' )
    {
        pRscName[j] = 0;

        // Find our terminator
        for( i=0; pExternalInfo[i] && pExternalInfo[i] != '@'; i++ );

        if( pExternalInfo[i] == 0 )
            x_throw( xfs("Error: The depencency string was not formated corectly.\n[%s]", pExternalInfo ));
    }
    else if( pExternalInfo[i] == '@' )
    {
        // Skip any white spaces for the name
        while( j>0 && (pRscName[j] == ' ' || pRscName[j] == '@') ) j--;
    
        if( j == 0 )
            x_throw( xfs("Missing external resource name\n[%s]", pExternalInfo ));
    }
    else
    {
        ASSERT( pExternalInfo[i] == 0 );
        x_throw( xfs("Error: Dependency string must have been formatted worng\n[%s]", pExternalInfo ));
    }

    // Skip the strange symbol
    ASSERT( pExternalInfo[i] == '@' );
    i++;

    // Skip any white spaces
    for( i=0; pExternalInfo[i] && pExternalInfo[i] == ' '; i++ );

    // Finally copy the compiler rules
    CompilerRules = pExternalInfo[i];
    */
}


//=========================================================================

#if defined(X_EDITOR)
xbool rsc_desc_mgr::BeginCompiling( u32 Platform )
{
    s32 j, i;

    if( m_IsCompiling == TRUE )
        x_throw( "We are already compiling" );

    m_IsCompiling = TRUE;
    m_bStopBuild  = FALSE;

    //
    // Activate all the platforms what we need
    //
    for( i=0; i<PLATFORM_COUNT; i++ )
    {
        m_Platform[i].bCompile = (m_Platform[i].Platform & Platform) != 0;
    }

    //
    // First go throw all the objec desc and update their dependencies
    //
    xarray<xstring> Dependencies;
    for( i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        // Get the node
        node& Node = m_lRscDesc[i];

        // Clear any previously set skip flag
        Node.Flags &= ~FLAGS_SKIP;

        // Check if the compiled resource is read only - that means it came from perforce and doesn't need building locally
        bool IsReadOnly = true;
        for( s32 iPlatform=0; iPlatform<PLATFORM_COUNT; iPlatform++ )
        {
            // Are we compiling for this platform
            if( m_Platform[iPlatform].bCompile )
            {
                // Generate path to compiled resource
                char Path[X_MAX_PATH];
                x_sprintf( Path, "%s\\%s\\%s", g_Settings.GetReleasePath(),m_Platform[iPlatform].OutputDir, Node.pDesc->GetName() );

                // Check for READONLY attribute
                DWORD Attr = GetFileAttributes( Path );
                if( (Attr == INVALID_FILE_ATTRIBUTES) ||
                    !(Attr & FILE_ATTRIBUTE_READONLY) )
                {
                    // Not read only
                    IsReadOnly = false;
                    break;
                }
            }
        }

        // If ReadOnly on all platforms then skip this node
        if( IsReadOnly )
        {
            Node.Flags |= FLAGS_SKIP;
            continue;
        }

        // Get all the dependencies out of the resource descriptor
        Dependencies.Clear();
        Node.pDesc->OnGetCompilerDependencies( Dependencies );

        // We also include it self in the dependency list
        Dependencies.Append() = xfs( "%s%s", Node.pDesc->GetPath(), Node.pDesc->GetName() );

        //
        // Nuke all the previous dependencies
        //
        Node.Dependency.Clear();

        for( j=0; j<Dependencies.GetCount(); j++ )
        {
            Node.Dependency.Append() = AddDep( Dependencies[j] );
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
        dependency& Dep = m_lSrcDep[i];

        hDepFile = _findfirst( Dep.GetName(), &c_depfile );

        // Could not find dependency so set the date to zero
        // To force recompilation of resources
        if( hDepFile == -1L )
        {
            xhandle hHandle = m_lSrcDep.GetHandleByIndex( i );
            for( s32 k=0; k<m_lRscDesc.GetCount(); k++ )
            {
                node& Rsc = m_lRscDesc[k];
                for(s32 t=0; t<Rsc.Dependency.GetCount(); t++ )
                {
                    if( Rsc.Dependency[t] == hHandle )
                    {
                        // Get the resource name
                        const char* pResourceName = Rsc.pDesc->GetName();
                        s32 ResourceNameLen = x_strlen( pResourceName );

                        // Check for audiopkg resources, this is kind of a hack but the easiest way to do this for now
                        // TODO: CJ: Find a better way to do this, maybe an option in the configurations that enabled audio package errors
                        if( (ResourceNameLen > 9) &&
                            (strnicmp( &pResourceName[ResourceNameLen-9], ".audiopkg", 9 ) == 0) )
                        {
                            // Just add the error to the log
                            LOG_MESSAGE( "rsc_desc_mgr::BeingCompiling", xfs("Source File Missing: '%s' needed by RescDesc: '%s'", m_lSrcDep[i].GetName(),Rsc.pDesc->GetName() ));
                        }
                        else
                        {
                            // Display error dialog
                            x_try;
                            x_throw( xfs("Source File Missing:\n%s\n\nNeeded By RescDesc:\n%s\n", m_lSrcDep[i].GetName(),Rsc.pDesc->GetName() ));
                            x_catch_display_msg("Resource Description is missing a source file!");
                        }

                        // Set this resource to be skipped since we can't find some of it's dependants
                        // TODO: CJ: Evaluate if this is the correct behaviour, what if the dependant is then added, it still won't be rebuilt
                        Rsc.Flags |= FLAGS_SKIP;
                        break;
                    }
                }
            }

            Dep.TimeStamp = 0;

            //LOG_ERROR("Compile",xfs("A dependency [%s] was not found on the drive.\n", m_lSrcDep[i].FileName ));
            // This exception is reported via the LOG_ERROR above.
            //x_try;
            //x_throw( xfs("A Dependency [%s] was not found on the drive.\n All resources that use it will be skipped this compile.", m_lSrcDep[i].FileName ));
            //x_catch_display;
        }
        else
        {
            // Write good time
            Dep.TimeStamp = c_depfile.time_write;
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
        m_IsCompiling = FALSE;
        return FALSE;
    }

    return TRUE;
}
#endif

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
            x_throw( "Fatal error. Out of memory" );

        Node.Flags      = FLAGS_EXTERNAL;
        Node.nExternals = 0;
        Node.TimeStamp  = 0;
        Node.RefCount   = 1;

        // Initialize external resource
        Node.pDesc->SetFullName     ( RscName );   
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
                x_throw( xfs("Internal Error. The resource [%s] Had too many dependencies", m_lRscDesc[i].pDesc->GetName() ));

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

#if defined(X_EDITOR)
s32 rsc_desc_mgr::NextCompiling    ( void )
{
    s32 Last = m_CompilingIndex;
    if( Last == -1 ) return -1;

    ASSERT( m_IsCompiling == TRUE );

    //
    // Start from where we left of in the BeginCompile.
    //
    for( m_CompilingIndex++; m_CompilingIndex<m_lRscDesc.GetCount(); m_CompilingIndex++ )
    {
        if( NeedCompiling( m_lRscDesc[m_CompilingIndex] ) == TRUE )
            break;        
    }

    if( m_CompilingIndex == m_lRscDesc.GetCount() )
    {
        m_CompilingIndex = -1;
    }

    return Last;
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::EndCompiling( void )
{
    g_WorldEditor.ForceRscRefresh();

    ASSERT( m_IsCompiling == TRUE );
    m_IsCompiling = FALSE;
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::RefreshAllCompiledRsc( void )
{
    //
    // Go throw all the resource and reload them.
    //
    for( s32 i=0; i<GetRscDescCount(); i++ )
    {
        node& Node = GetRscDescIndex(i);

        if( (Node.Flags & FLAGS_RELOAD)==0 )
            continue;

        ASSERT( Node.pDesc );
        g_RscMgr.Refresh( Node.pDesc->GetName() );
        Node.Flags &= ~FLAGS_RELOAD;
    }
}
#endif

//=========================================================================

#if defined(X_EDITOR)
xbool rsc_desc_mgr::NeedCompiling( node& RscDesc )
{
    long            hFile;
    _finddata_t     c_final;
    long            BestTime = (long)(((u32)0xffffffff)>>1);

    // Check the skip flag first
    if( RscDesc.Flags & FLAGS_SKIP )
        return FALSE;

    // Check the final resource date ( if it exits )
    for( s32 i=0; i<PLATFORM_COUNT;i++)
    {
        if( m_Platform[i].bCompile == FALSE )
            continue;

        // Must use sprintf to bypass the x_theads.
        char Path[256];
        sprintf( Path, "%s\\%s\\%s", g_Settings.GetReleasePath(),m_Platform[i].OutputDir, RscDesc.pDesc->GetName() );

        hFile = _findfirst( Path, &c_final );
        if( hFile == -1L )
            return TRUE;

        // If the destination file is mark as read only we will assume is done via perforce.
        // Meaning some one compile it and checked it in in perforce.
        // WARNING: This can be missleading if the PC as checked in and the PS2 is not thought.
        if( c_final.attrib & _A_RDONLY )
            return FALSE;

        // done with file
        _findclose( hFile );

        // Choose the oldest time    
        if( c_final.time_write < BestTime )
            BestTime = c_final.time_write;
    }

    RscDesc.TimeStamp = BestTime;

    // Check all the dependecies for the resource are same or older.
    for( s32 j = 0; j < RscDesc.Dependency.GetCount(); j++ )
    {
        const dependency&   Dep = m_lSrcDep( RscDesc.Dependency[j]);

        if( BestTime <= Dep.TimeStamp )
            return TRUE;
    }

    // We don't need to compile the file, yea!
    return FALSE;
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::GetMakeRules( s32 Index, xstring& MakeRules, xstring& Output )
{
    if( Index < 0 )
        return;

    // Fist lets collect the make rules from the rsc desc.    
    GetRscDescIndex( Index ).pDesc->OnGetCompilerRules( MakeRules );

    // Then lets add where the compiles are
    xstring A;
    A  = g_Settings.GetCompilerPath();
    A += "\\";
    MakeRules = A + MakeRules + " ";

    // Now lets finally add the destination data
    Output = GetRscDescIndex( Index ).pDesc->GetName();

    for( s32 i=0; i<PLATFORM_COUNT; i++ )
    {
        if( m_Platform[i].bCompile == FALSE )
            continue;

        char FilePath[256];
        sprintf( FilePath, "-%s \"%s\\%s\\%s\" ", m_Platform[i].Cmd, g_Settings.GetReleasePath(), m_Platform[i].OutputDir, &Output[0] );
        MakeRules += FilePath;
    }

    Output += "\n";

    // We will assume that this resource is going to get compile so there for must be reloaded
    GetRscDescIndex( Index ).Flags |= FLAGS_RELOAD;
}
#endif

//=========================================================================

void rsc_desc_mgr::StopBuild( void )
{
    m_bStopBuild = TRUE;
}

//=========================================================================

xbool rsc_desc_mgr::IsStopBuild( void )
{
    return m_bStopBuild;
}

//=========================================================================

xbool rsc_desc_mgr::IsCompiling( void )
{
    return m_IsCompiling;
}

//=========================================================================

rsc_desc& rsc_desc_mgr::GetRscDescByString( const char* pName )
{
    s32 n = m_lRscDesc.GetCount();
    for( s32 i=0; i<n; i++ )
    {
        char Name[256];
        char Ext[256];
        x_splitpath( m_lRscDesc[i].pDesc->GetName(), NULL, NULL, Name, Ext );
        x_strcat( Name, Ext );

        if( x_stricmp( Name, pName ) == 0 )
        {
            return *m_lRscDesc[i].pDesc;
        }
    }

    x_throw( xfs("unable to find [%s] RscDesc", pName ) );

    return *m_lRscDesc[i].pDesc;
}

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::CleanResource( const rsc_desc& RscDesc )
{
    if( m_IsCompiling == TRUE )
        x_throw( "can't do this operation while compiling" );
    
    for( s32 i=0; i<PLATFORM_COUNT;i++)
    {
        // Must use sprintf to bypass the x_theads.
        char Path[256];
        sprintf( Path, "%s\\%s\\%s", g_Settings.GetReleasePath(), m_Platform[i].OutputDir, RscDesc.GetName() );

        if( _unlink( Path ) == -1 )
        {
            if( errno == EACCES )
            {
                LOG_WARNING( "rsc_desc_mgr::CleanResource", "Failed to delete '%s'", Path );
                //x_throw( xfs( "unable to clean up [%s]\nFile has read only atrributes", Path ) ); 
            }
        }
    }
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::DeleteRscDescFromDrive( const char* pName )
{
    if( m_IsCompiling == TRUE )
        x_throw( "can't do this operation while compiling" );

    s32 Index = FindRscDesc( pName );

    char FullName[256];
    m_lRscDesc[Index].pDesc->GetFullName(FullName);

    if( _unlink( FullName ) == -1 )
    {
        if( errno == EACCES )
            x_throw( xfs( "unable to delete [%s]\nFile has read only atrributes", FullName ) ); 
    }

    DeleteRscDesc( pName );
}
#endif

//=========================================================================

rsc_desc& rsc_desc_mgr::Load( const char* pFileName )
{
    ASSERT( pFileName );

    //
    // First try to find rsc.
    //

    char FileName[256];
    char Ext[256];

    x_splitpath( pFileName, NULL, NULL, FileName, Ext );
    node* pNode = GetRscDesc( xfs( "%s%s", FileName, Ext) );
    if( pNode )
    {
        return *pNode->pDesc;
    }
    else
    {
        // we could not find it, so try loading it
        rsc_desc&   Desc = CreateRscDesc( pFileName );
        text_in     TextIn;
        TextIn.OpenFile( pFileName );

        x_try;
        Desc.OnLoad( TextIn );
        x_catch_begin;
        TextIn.CloseFile();
        x_catch_end_ret;

        return Desc;
    }
}

//=========================================================================

#if defined( X_EDITOR )
void rsc_desc_mgr::Save( rsc_desc& Desc )
{
    text_out TextOut;
    char FileName[256];

    Desc.GetFullName( FileName );
    TextOut.OpenFile( FileName );

    x_try;    
        Desc.OnSave( TextOut );    
    x_catch_begin;
        TextOut.CloseFile();
    x_catch_end_ret;

    Desc.SetChanged( FALSE );
}
#endif


//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
// RSC DESC
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================

void rsc_desc::SetFullName( const char* pRscName )
{
    ASSERT( pRscName );

    char Ext[256];
    char FileName[256];
    char Path[256];
    char Drive[256];

    Drive[0] = 0;
    Path[0]  = 0;
    x_splitpath( pRscName, Drive,Path,FileName,Ext );

    if( Ext[0] == 0 )
    {
        Ext[0] = '.';
        x_strcpy( &Ext[1], m_Type.GetName() );
    }
    else
    {
        if( x_stricmp( &Ext[1], m_Type.GetName() ) != 0 )
            x_throw( "Wrong type in the name of the resource" );
    }

    x_sprintf( m_Path, "%s%s", Drive, Path );
    x_sprintf( m_Name, "%s%s", FileName, Ext );

    m_FullNameHash = ComputeUppercaseStringHash( pRscName );
    m_NameHash     = ComputeUppercaseStringHash( m_Name );

    SetChanged( TRUE );
}

//=========================================================================

void rsc_desc::GetFullName( char* pRscName ) const
{
    x_sprintf( pRscName, "%s%s", m_Path, m_Name );
}

//=========================================================================
void rsc_desc::OnEnumProp( prop_enum& List )
{
    List.PropEnumString  ( "ResDesc",       "Resource package Name", PROP_TYPE_HEADER );
    List.PropEnumFileName( "ResDesc\\Name", xfs("*.%s|*.%s||",m_Type.GetName(),m_Type.GetName()), "File name for the resource description", PROP_TYPE_MUST_ENUM );
}

//=========================================================================
void rsc_desc::OnCheckIntegrity( void )
{
    char Ext[128];
    char FileName[256];

    if( m_Path[0] == 0 ) 
        x_throw( "The name of the resource must contain a full path" );

    x_splitpath( m_Name, NULL, NULL, FileName, Ext );
    if( Ext[0] == 0 )
        x_throw( "You must include the extension in the resource name" );

    if( x_stricmp( &Ext[1], m_Type.GetName()) != 0 )
        x_throw( xfs("The extension should must the resource type[%s]",m_Type.GetName() )); 

    if( FileName[0] == 0 )
        x_throw( "The resource must have a file name" );
}

//=========================================================================
xbool rsc_desc::OnProperty( prop_query& I )
{
    if( I.IsRead() == FALSE )
    {
        SetChanged( TRUE );
    }

    if( I.IsVar( "ResDesc\\Name" ) )
    {
        if( I.IsRead() )
        {
            char Path[256]; 
            GetFullName( Path );
            I.SetVarFileName( Path, 256 );
        }
        else
        {
            SetFullName( I.GetVarFileName() );
        }
    }
    else if( I.IsVar( "ResDesc" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( GetName(), 256 );
        }
        else
        {
            ASSERT( 0 );
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================
void rsc_desc::OnStartEdit( void )
{
}

//=========================================================================
#if defined( X_EDITOR )
void rsc_desc::OnSave( text_out& TextOut )
{
    OnCheckIntegrity();

    prop_interface::OnSave( TextOut );
    SetChanged( FALSE );
}
#endif

//=========================================================================
void rsc_desc::OnLoad( text_in& TextIn )
{
    prop_interface::OnLoad( TextIn );
    SetChanged( FALSE );
}

//===========================================================================

#if defined(X_EDITOR)

xbool ExecuteCMD( const xstring& InputStr, xstring& OutputStr )
{
    OutputStr.Clear();

    HANDLE                  hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES     sa;

    sa.nLength                 = sizeof(sa);
    sa.bInheritHandle          = TRUE;
    sa.lpSecurityDescriptor    = NULL;

    if(!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) 
    {
        return FALSE;
    }

    // CReadPipe will auto-delete itself which includes closing the handle.
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb              = sizeof(STARTUPINFO);
    si.dwFlags         = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.hStdInput	   = INVALID_HANDLE_VALUE;
    si.hStdOutput      = hWritePipe;
    si.hStdError       = hWritePipe;
    si.wShowWindow     = SW_HIDE;


    // Since we've redirected the standard input, output and error handles
    // of the child process, we create it without a console of its own.
    // (That's the `DETACHED_PROCESS' part of the call.)  Other
    // possibilities include passing 0 so the child inherits our console,
    // or passing CREATE_NEW_CONSOLE so the child gets a console of its
    // own.

    char s_Buffer[1024*64];

    BOOL ret = CreateProcess( NULL,                  // LPCTSTR: Module name
                            (LPSTR)((const char*)InputStr),           // @script_256.txt",        // LPTSTR: cmd line
                            NULL,                  // LPSECURITY_ATTRIBUTES: process security
                            NULL,                  // thread security
                            TRUE,                  // BOOL: inherit handles
                            DETACHED_PROCESS | NORMAL_PRIORITY_CLASS, 
                            //CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS, 
                            NULL,                  // LPVOID: environment block
                            NULL,                  // LPCTSTR: current directory
                            &si, 
                            &pi );

    if( FALSE == ret ) 
    {
        // Clean the pipe
        CloseHandle( hWritePipe );
        CloseHandle( hReadPipe  ); 

        // TODO: may want to stop the compilation process
        //return ExitInstance();
        return FALSE;
    }

    // Block and collect output until the process is finished
    while( 1 )
    { 
        DWORD dwRead = 0;
        if( !PeekNamedPipe( hReadPipe, NULL, 0, NULL, &dwRead, NULL) )   
            break;

        if( dwRead )
        {   
            if( !ReadFile(hReadPipe, &s_Buffer, sizeof(s_Buffer)-1, &dwRead, NULL) )
                    break;
            s_Buffer[dwRead] = 0;
            OutputStr += s_Buffer;
        }
        else     
        {
            // maybe the program exited
            if(WaitForSingleObject(pi.hProcess,0) == WAIT_OBJECT_0)
                break;

            // Don't grab all the CPU cycles
            Sleep(2);
        }
    }

    // Output the data
    //x_DebugMsg("CMD: %s\n",(const char*)InputStr);
    //x_DebugMsg("RTN: %s\n",(const char*)OutputStr);

    // Close all the handles
    CloseHandle( pi.hThread  );
    CloseHandle( pi.hProcess );
    CloseHandle( hWritePipe );
    CloseHandle( hReadPipe  ); 

    Sleep(2);

    // Finished
    return TRUE;
}
#endif

//=========================================================================

struct p4_file
{
    char Path[256];
    u32  Hash;
};

xarray<p4_file> P4_File;
s32             P4_FileHashTableSize=0;
s32*            P4_FileHashTable;

//=========================================================================

#if defined(X_EDITOR)
void CleanPathForP4( const char* pSrc, char* pDst )
{
    s32 i;
    char Clean[256];

    // Copy into clean
    x_strcpy(Clean,pSrc);
    x_strtoupper(Clean);
    s32 Len = x_strlen(Clean);
    for( i=0; i<Len; i++ )
    {
        if( Clean[i] == '/')  Clean[i] = '\\';
    }

    char* pStart = x_stristr(Clean,"GAMEDATA");
    if( pStart==NULL )
        pStart = Clean;
    x_strcpy(pDst,pStart);
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void PERFORCE_SetupFileListing( void )
{
    // Generate file listing from perforce
    xstring P4Output;
    {
        xstring Input = xfs("p4 files c:\\gamedata\\a51\\...");
        ExecuteCMD( Input, P4Output );
        //P4Output.SaveFile("c:/temp/filelisting.txt");
        //P4Output.LoadFile("c:/temp/filelisting_missing.txt");
    }

    // Build parsed file output
    P4_File.Clear();
    char* pChar = (char*)((const char*)P4Output);
    s32 nChars = P4Output.GetLength();
    s32 i=0;
    while( i < nChars )
    {
        // Fast forward to end of this path
        s32 j=i;
        while( (pChar[j] != '#') && (j<nChars) )
            j++;
        if( j==nChars ) break;

        // Strip out and clean path
        pChar[j] = 0;
        p4_file NewFile;
        CleanPathForP4(pChar+i,NewFile.Path);
        NewFile.Hash = ComputeUppercaseStringHash(NewFile.Path);
        P4_File.Append(NewFile);

        // Fast forward to end of this line
        while( (pChar[j] != '\n') && (j<nChars) )
            j++;
        if( j==nChars ) break;

        i = j;
    }

    // Allocate hash table
    P4_FileHashTableSize = P4_File.GetCount() * 4;
    P4_FileHashTable = (s32*)x_malloc(sizeof(s32)*P4_FileHashTableSize);

    // Fill out hash table
    x_memset(P4_FileHashTable, 0xFFFFFFFF,sizeof(s32)*P4_FileHashTableSize);
    for( i=0; i<P4_File.GetCount(); i++ )
    {
        s32 I = P4_File[i].Hash % P4_FileHashTableSize;
        while( P4_FileHashTable[I] != -1 )
        {
            I++;
            if( I==P4_FileHashTableSize ) 
                I = 0;
        }

        P4_FileHashTable[I] = i;
    }
}
#endif

//=========================================================================

#if defined(X_EDITOR)
xbool PERFORCE_DoesFileExist( const char* pFileName )
{
    // Build clean path and hash
    char CleanPath[256];
    CleanPathForP4( pFileName,CleanPath);
    u32 Hash = ComputeUppercaseStringHash(CleanPath);

    // Lookup file in hash table
    s32 I = Hash % P4_FileHashTableSize;
    while( P4_FileHashTable[I] != -1 )
    {
        s32 i = P4_FileHashTable[I];
        if( P4_File[i].Hash == Hash )
        {
            if( x_stricmp(P4_File[i].Path,CleanPath) == 0 )
                return TRUE;
        }

        I++;
        if( I==P4_FileHashTableSize ) 
            I = 0;
    }

    return FALSE;
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void PERFORCE_GetInfo( const char* pFilePath, char* pLastCheckinName )
{

}
#endif

//=========================================================================

#if defined(X_EDITOR)
void FindSubdirectories( xarray<xstring>& DirListing, const char* DirName, long& hFile, _finddata_t& Data )
{
    //
    // If it is a folder let it recurse because we are trying to find all data
    //
    while( Data.attrib & _A_SUBDIR )
    {
        struct _finddata_t  c_Subfile;
        long                hSubFile;
        char                SubDirName[256];

        if( Data.name[0] != '.' )
        {
            x_sprintf( SubDirName, "%s\\%s", DirName, Data.name );
    
            // handle folder
            if( (hSubFile = _findfirst( xfs("%s\\*", SubDirName), &c_Subfile )) != -1L )
            {
                FindSubdirectories( DirListing, SubDirName, hSubFile, c_Subfile );
                _findclose( hSubFile );
            }
        }

        // handle the next thing.
        if( _findnext( hFile, &Data ) != 0 )
            return;
    }
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void AppendFileListing( xarray<xstring>& FileList, const char* pRootPath, const char* pWildcard )
{
    struct _finddata_t  Data;
    long                hFile;

    // Strip any trailing slashes
    char RootPath[256];
    x_strcpy(RootPath,pRootPath);
    s32 Len = x_strlen(RootPath);
    while( (Len>0) && ((RootPath[Len-1]=='\\') || (RootPath[Len-1]=='/')) )
    {
        RootPath[Len-1]=0;
        Len--;
    }

    hFile = _findfirst( RootPath, &Data );
    if( hFile == -1L )
        return;

    // Build list of subdirectories
    xarray<xstring>     DirListing;
    DirListing.Append(RootPath);
    FindSubdirectories( DirListing, RootPath, hFile, Data );

    // Loop through subdirectories
    for( s32 i=0; i<DirListing.GetCount(); i++ )
    {
        x_DebugMsg("CHECKING DIR: %s\n",DirListing[i]);
        if( (hFile = _findfirst( xfs("%s\\%s",(const char*)DirListing[i],pWildcard), &Data )) == -1L )
            continue;

        do
        {
            // handle folders
            if( (Data.attrib & _A_SUBDIR)==FALSE )
            {
                FileList.Append( xstring(xfs("%s\\%s",(const char*)DirListing[i],Data.name)) );
            }

        } while( _findnext( hFile, &Data ) == 0 );
    }
}
#endif

//=========================================================================

#if defined(X_EDITOR)
void BuildBlueprintList( xarray<xstring>& FileList )
{
    char cPath[MAX_PATH];
    for( s32 i = g_Project.GetFirstBlueprintDir( cPath ); i != -1; i = g_Project.GetNextBlueprintDir( i, cPath ) )
    {
        AppendFileListing( FileList, xfs("%s",cPath), "*.bpx" );
    }
}
#endif

//=========================================================================

#include "MeshUtil\RawMesh2.hpp"

void GatherMATXDependencies( xarray<xstring>& FileList, const char* pMATX, xbool bCheckForMaxFile )
{
    x_DebugMsg("MATX: %s\n", pMATX);

    // Add textures to FileList
    {
        rawmesh2 RM;

        x_try;
        RM.Load(pMATX);
        x_catch_begin;
            return;
        x_catch_end;

        for( s32 i=0; i<RM.m_nTextures; i++ )
            FileList.Append( xstring( RM.m_pTexture[i].FileName ) );
    }

    // Get source .max file name
    if( bCheckForMaxFile )
    {
        text_in     File;

        x_try;
        File.OpenFile( pMATX );
        x_catch_begin;
            return;
        x_catch_end;

        while( 1 )
        {
            if( !File.ReadHeader() )
                return;

            if( x_stricmp( File.GetHeaderName(), "SourceFile" ) == 0)
            {
                if( File.ReadFields() == FALSE )                    
                {
                    return;
                }
                char MaxName[256];
                File.GetString("FileName",MaxName);
                FileList.Append( xstring(MaxName) );
            }
        }
    }
}

//=========================================================================

#if defined(X_EDITOR)
void rsc_desc_mgr::ScanResources( void )
{
    s32 i;
    xbool bCheckTGAs=FALSE;
    xbool bCheckForMaxFile=FALSE;
    x_DebugMsg("** Starting Resource Scan **\n");

    // Be sure this is what the user wants to do.
    {
        generic_dialog Dialog;
        Dialog.SetTitle( "Do you wish to scan perforce for missing files?" );
        xstring Msg;
        Msg += "BASIC SCAN -\n";
        Msg += "    - Resource description files of included themes\n";
        Msg += "    - .matx, and other direct dependencies of resource descriptions\n";
        Msg += "    - .layer, .project, .projectinfo files\n";
        Msg += "    - .bpx blueprints of included themes\n";
        Msg += "\n";
        Msg += "BASIC SCAN + TGAs -\n";
        Msg += "    - Everything in the basic scan\n";
        Msg += "    - Confirm all .tgas referenced by matx files are in perforce\n";
        Msg += "\n";
        Msg += "BASIC SCAN + TGAs + MAX -\n";
        Msg += "    - Everything in the basic scan + tgas\n";
        Msg += "    - Confirm all .max files used to export .matx files are in perforce\n";
        Msg += "\n";
        Msg += "Each scan type takes more processing time.\n";
        Msg += "\n";
        Msg += "Which scan would you like to do?\n";
        Dialog.SetMessage( Msg );
        Dialog.AppendButton( "SCAN" );
        Dialog.AppendButton( "SCAN + TGA" );
        Dialog.AppendButton( "SCAN + TGA/MAX" );
        Dialog.AppendButton( "CANCEL" );
        s32 Ret = Dialog.Execute();
        if( (Ret==-1) || (Ret==3) ) return;
        if( (Ret==1)  || (Ret==2) ) bCheckTGAs = TRUE;
        if( Ret==2 )                bCheckForMaxFile = TRUE;
    }

    PERFORCE_SetupFileListing();

    xarray<xstring> FileList;
    xarray<xstring> MissingFiles;

    //
    // Scan resource files themselves
    //
    {
        char RscDescFileName[256];
        for( i=0; i<m_lRscDesc.GetCount(); i++ )
        {
            node& Node = m_lRscDesc[i];
            Node.pDesc->GetFullName(RscDescFileName);
            FileList.Append(RscDescFileName);
        }
    }

    //
    // Collect resource dependencies
    //
    for( i=0; i<m_lRscDesc.GetCount(); i++ )
    {
        node& Node = m_lRscDesc[i];

        // Get all the dependencies out of the resource descriptor
        xarray<xstring> Dependencies;
        Node.pDesc->OnGetCompilerDependencies( Dependencies );

        for( s32 j=0; j<Dependencies.GetCount(); j++ )
        {
            FileList.Append(Dependencies[j]);

            // If dependency is a .matx file then gather sub-dependencies
            if( bCheckTGAs )
            {
                char EXT[32];
                x_splitpath((const char*)Dependencies[j],NULL,NULL,NULL,EXT);
                if( x_stricmp(EXT,".matx")==0 )
                    GatherMATXDependencies( FileList, (const char*)Dependencies[j], bCheckForMaxFile );
            }
        }
    }

    //
    // Collect blueprints
    //
    BuildBlueprintList( FileList );

    //
    // Collect other level files
    //
    AppendFileListing( FileList, g_Project.GetWorkingPath(), "*.layer");
    AppendFileListing( FileList, g_Project.GetWorkingPath(), "*.project");
    AppendFileListing( FileList, g_Project.GetWorkingPath(), "*.projectinfo");

    //
    // Check if files are in P4
    //
    for( i=0; i<FileList.GetCount(); i++ )
    {
        if( !PERFORCE_DoesFileExist( FileList[i] ) )
        {
            MissingFiles.Append(FileList[i]);
        }
    }

    //
    // Build string of missing files to hand to dialog box.
    //
    generic_dialog Dialog;

    if( MissingFiles.GetCount() == 0 )
    {
        Dialog.SetTitle( "No Files Missing!" );
        Dialog.SetMessage( "Fantastic! All files exist in Perforce!\n" );
        Dialog.AppendButton( "OK" );
        Dialog.Execute();
    }
    else
    {
        Dialog.SetTitle( "Files Missing From Perforce..." );

        xstring Msg;
        Msg += "These files are being used but are not checked into Perforce.\n";
        Msg += "Would you like to add them?\n\n";
        for( i=0; i<MissingFiles.GetCount(); i++ )
            Msg += xfs("%s\n",(const char*)MissingFiles[i]);

        Dialog.SetMessage( (const char*)Msg );
        Dialog.AppendButton( "DON'T ADD" );
        Dialog.AppendButton( "ADD TO P4" );

        // Execute dialog and see which button was pressed
        s32 Ret = Dialog.Execute();

        if( (Ret==1) && (MissingFiles.GetCount() > 0) )
        {
            // Add files to perforce.
            for( i=0; i<MissingFiles.GetCount(); i++ )
            {
                xstring P4Output;
                xstring Input = xfs("p4 add \"%s\"",(const char*)MissingFiles[i]);
                ExecuteCMD( Input, P4Output );
            }

            generic_dialog Dialog;
            Dialog.Execute_OK("Files added to perforce","The files have been added to perforce.");
        }
    }
}
#endif

//=========================================================================

