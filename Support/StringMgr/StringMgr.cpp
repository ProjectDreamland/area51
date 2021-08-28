//==============================================================================
//
//  StringMgr.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "StringMgr.hpp"
#include "e_Virtual.hpp"
#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

//#define ENABLE_QA_STRINGTEST

string_mgr g_StringTableMgr;               // Instantiate the one and only string manager

#if !defined(X_RETAIL) || defined(X_QA)
  #if defined(X_QA) && defined(ENABLE_QA_STRINGTEST)
    xbool    g_bStringTest = TRUE; 
  #else
    xbool    g_bStringTest = FALSE; 
  #endif
    xbool    g_bShowStringID = FALSE;
#endif

xwchar s_NullString[] = { '<','n','u','l','l','>',0,0 };

//------------------------------------------------------------------------------

//==============================================================================
//  STRING MANAGER RESOURCE LOADER
//==============================================================================

class binstring_loader : public rsc_loader
{
public:
            binstring_loader ( const char* pType, const char* pExt ) : rsc_loader(pType,pExt) {}
    void*   PreLoad         ( X_FILE* pFP   );
    void*   Resolve         ( void*   pData );
    void    Unload          ( void*   pData );
    void*   PreLoad         ( X_FILE*& Fp, const char* pFileName );
};

binstring_loader BinaryStringLoader("Binary Text",".stringbin");

//==============================================================================

void* binstring_loader::PreLoad( X_FILE*& Fp, const char* pFileName )
{
    MEMORY_OWNER( "STRING DATA" );
    xbool Loaded = FALSE;

    // this just checks that the file exists. the unlocalized name is passed
    // to the loader, and will be converted again...
    // this is because it will fail if we remove the ENG_ name prefix in the future,
    // and because the loader may be called elsewhere.
    string_mgr::char_string CharString;
    const char* LocalizedName = g_StringTableMgr.GetLocalizedName( pFileName, CharString );

    Fp = x_fopen( LocalizedName, "rb" );

    if( Fp )
    {
        //
        // We are going to use the name of the file as the table name.
        //
        xstring FileName( pFileName );  
        
        // Find the last '/'.
        s32 i = 0;
        for( i = FileName.GetLength()-1; i >= 0 ; i--)
        {
            if( FileName[i] == '/' )
            {
                i++;
                break;
            }
        }

        xstring TableNameExt = FileName.Right( (FileName.GetLength() - i) );
        TableNameExt = TableNameExt.Left( TableNameExt.Find( '.' ) );

        s32 StrLen = TableNameExt.GetLength();
        char* pTableName = new char[ StrLen + 1 ];
        x_memset( pTableName, 0, StrLen+1 );

        x_strncpy( pTableName, (const char*)TableNameExt, StrLen );
        Loaded = g_StringTableMgr.LoadTable( pTableName, pFileName );

        if( Loaded )
        {
            return (void*)pTableName;
        }
        else
        {
            delete [] pTableName;
            return NULL;
        }
    }

    return NULL;
}

//==============================================================================

void* binstring_loader::PreLoad( X_FILE* pFP   )
{
    (void)pFP;
    return NULL;
}

//==============================================================================

void* binstring_loader::Resolve( void* pData )
{
    return pData;
}

//==============================================================================

void  binstring_loader::Unload( void* pData )
{
    if( pData != NULL )
    {
        g_StringTableMgr.UnloadTable( (const char*)pData );
        delete [] (char *)pData;
    }
}
//==============================================================================
// GLOBALS
//==============================================================================
xbool   string_mgr::m_Initialized = FALSE;

//==============================================================================
//  FUNCTIONS
//==============================================================================

string_table::string_table( void )
{
    m_pTableName    = NULL;
    m_pData         = NULL;
    m_nStrings      = 0;
}

//==============================================================================

string_table::~string_table( void )
{
    if( m_pData )
    {
        x_free( m_pData );

        m_pData     = NULL;
        m_nStrings  = 0;

        LOG_MESSAGE( "string_table::~string_table",
                     "Unloaded: %s", m_pTableName );
    }
}

//==============================================================================

const char* string_mgr::GetLocalizedName( const char* pFileName, char_string& LocalizedName ) const
{
    char pDrive[X_MAX_DRIVE], pPath[X_MAX_PATH], pName[X_MAX_FNAME], pExt[X_MAX_EXT];
    ASSERT( pFileName );

    x_splitpath(pFileName, pDrive, pPath, pName, pExt);
    ASSERT(x_strlen(pName) > 3);
    ASSERTS( x_strlen(pName) < (s32)sizeof( LocalizedName ), xfs( "Inc char_string to %d chars", x_strlen( pName ) ) );

    char* Name = LocalizedName.m_String;
    x_sprintf(Name, "%s%s%s%s%s", pDrive, pPath, x_GetLocaleString(), pName + 3, pExt);

#if defined(X_DEBUG) && defined(ctetrick)
    if( x_strcmp(pFileName, Name) != 0 )
        LOG_MESSAGE("string_mgr::GetLocalizedName", "Stringbin file (%s) returned as (%s)", pFileName, Name);
#endif
    
    return (const char *)Name;
}

//==============================================================================

xbool string_table::Load( const char* pTableName, const char* pFileName )
{
    xbool Success = FALSE;

    string_mgr::char_string CharString;
    const char* LocalizedName = g_StringTableMgr.GetLocalizedName( pFileName, CharString );

    X_FILE* pFile = x_fopen( LocalizedName, "rb" );
    if( pFile )
    {
        s32 Length = x_flength( pFile );

        m_pData = (byte*)x_malloc( Length );
        ASSERT( m_pData );

        VERIFY( x_fread( m_pData, 1, Length, pFile ) == Length );
        x_fclose( pFile );

        m_pTableName = pTableName;
        m_nStrings = *(s32*)m_pData;
        Success = TRUE;

        LOG_MESSAGE( "string_table::Load",
                     "Loaded: %s - %s",
                     pTableName, pFileName );
    }
    else
    {
        LOG_ERROR( "string_table::Load",
                   "Failed to load string table: %s - %s",
                   pTableName, pFileName );
        ASSERT( FALSE );
    }

    return Success;
}

//==============================================================================
s32 string_table::GetCount( void ) const
{
    return( m_nStrings );
}

//==============================================================================
#if !defined(X_RETAIL) || defined(X_QA)
// function for string tests. to flush out hard coded text in game. 
// Substitutes X's for all printable text.
void FillString( const xwchar* pString )
{
    ASSERTS(!g_bShowStringID, "Shouldn't be here if displaying string IDs.");    

    xwchar* pText = (xwchar*)pString;

    while( *pText )
    {
        // skip over any control codes.
        if(( *pText > 0x10 ) && (*pText != 0x20))
        {
            // check for button code pairs
            if( *pText == 0xAB)
            {
                while(*pText && *pText != 0xBB)
                    pText++;
            }
            // check for bracket pairs
            if( *pText == '<')
            {
                while(*pText && *pText != '>')
                    pText++;
            }
            if( *pText == 0 ) break;
            if( (*pText != 0xBB) && (*pText != '>') )
                *pText = 'x';
        }
        pText++;
    }
}
#endif

//==============================================================================

const xwchar* string_table::GetAt( s32 Index ) const
{
    const xwchar* pString = NULL;

    if( (Index >= 0) && (Index < m_nStrings) )
    {
        s32* pIndex = (s32*)(m_pData+4); 
        pString = (const xwchar*)(m_pData+4+4*m_nStrings+pIndex[Index]); 

#if !defined(X_RETAIL) || defined(X_QA)
        if( g_bShowStringID )
            return pString;
#endif
        pString += x_wstrlen( pString );
        pString++;

#if !defined(X_RETAIL) || defined(X_QA)
        if( g_bStringTest )
            FillString(pString);    // modifies string
#endif

        return pString;
    }

    return NULL;
}

//==============================================================================
s32 w2sstricmp( const xwchar* pStr1, const char* pStr2 )
{
    s32 C1, C2;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    do
    {
        C2 = (s32)(*(pStr1++));
        if( (C2 >= 'A') && (C2 <= 'Z') )
        {
            C2 -= ('A' - 'a');
        }

        C1 = (s32)(*(pStr2++));
        if( (C1 >= 'A') && (C1 <= 'Z') )
        {
            C1 -= ('A' - 'a');
        }

    } while( C1 && (C1 == C2) );

    return( C2 - C1 );
}

//==============================================================================

xwchar* Ansi2UpperWide( const char* pSrc, string_mgr::wchar_string& WideString )
{
    ASSERT( pSrc );
    u32 Length = x_strlen( pSrc )+1;
    ASSERTS( Length <= ( sizeof( WideString ) / 2 ), xfs( "Inc wide_string to %d chars", Length ) );
	xwchar* pWide = WideString.m_String;
    char* pTemp = (char*)pWide;
    for( u32 i=0;i<Length;i++ )
    {
#ifdef TARGET_GCN
        pTemp[0]=0;
        pTemp[1]= x_toupper( pSrc[i] );
        pTemp+=2;
#else   
        pTemp[1]=0;
        pTemp[0]= x_toupper( pSrc[i] );
        pTemp+=2;
#endif
    }
    return pWide;
}

//==============================================================================
const xwchar* string_table::FindString( const char* lookupString ) const
{
    xwchar* pString = NULL;
    s32* pIndex = (s32*)(m_pData+4);
    
    s32 imax;
    s32 imin;
    xbool bFound = FALSE;

    string_mgr::wchar_string WideString;
    xwchar* pLookupWide = Ansi2UpperWide( lookupString, WideString );

    imax = m_nStrings;
    imin = 0;
    bFound = FALSE;

    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;
        pString = (xwchar*)(m_pData+4+4*m_nStrings+pIndex[i]);

        if( imax == imin+1 )
        {
            if( !x_wstrcmp( pString, pLookupWide ) )
                bFound = TRUE;
            else  //-- NOT FOUND
                break;
        }

        s32 cmpVal = x_wstrcmp( pString, pLookupWide );

        if( cmpVal == 0 )
            bFound = TRUE;
        else if ( cmpVal > 0 )
            imax = i;
        else
            imin = i;

        if( bFound )
        {
#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bShowStringID )
                return pString;
#endif
            pString += x_wstrlen(pString);
            pString++;

#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bStringTest )
                FillString(pString);    // modifies string
#endif

            return( pString );    
        }
    }
    return(NULL);
}

//==============================================================================
const xwchar* string_table::GetAt( const char* lookupString ) const
{
#if defined(TARGET_XBOX)
    char TARGETTAG[6] = {'_','X','B','O','X','\0'};
#elif defined(TARGET_PS2)
    char TARGETTAG[5] = {'_','P','S','2','\0'};
    char ALTTARGET[6] = {'_','S','C','E','E','\0'};
#elif defined(TARGET_PC)
    char TARGETTAG[4] = {'_','P','C','\0'};
#else
    char TARGETTAG[1] = {'\0'};
#endif

    const xwchar* pString = NULL;
    char LOOKUPSTRING_TAG[256];

#if defined(TARGET_PS2)
    char LOOKUPSTRING_ALT[256];
    ASSERT( ( x_strlen(ALTTARGET) + x_strlen(lookupString) ) < (s32)sizeof( LOOKUPSTRING_ALT ) );
    x_strncpy( LOOKUPSTRING_ALT, lookupString, sizeof( LOOKUPSTRING_ALT ) );  //-- ID
    x_strncat( LOOKUPSTRING_ALT, ALTTARGET, 7 );     //-- TAG
#endif

    ASSERT( ( x_strlen(TARGETTAG) + x_strlen(lookupString) ) < (s32)sizeof( LOOKUPSTRING_TAG ) );
    x_strncpy( LOOKUPSTRING_TAG, lookupString, sizeof( LOOKUPSTRING_TAG ) );  //-- ID
    x_strncat( LOOKUPSTRING_TAG, TARGETTAG, 7 );     //-- TAG

    // look first for SCEE tags
#if defined(TARGET_PS2)
    if(x_GetTerritory() == XL_TERRITORY_EUROPE)
    {
        pString = FindString( LOOKUPSTRING_ALT );
        if( pString )
        {
            return( pString );
        }
    }
#endif

    //-- Look for the ID with the Tag hooked to the end.
    pString = FindString( LOOKUPSTRING_TAG );
    if( pString )
    {
        return( pString );
    }
    else
    {
        //-- Didnt find it with a platform tag now look normaly.
        pString = FindString( lookupString );
        if( pString )
            return( pString );
    }

    return NULL;
}

//==============================================================================
const xwchar* string_table::GetSubTitleSpeaker( const char* lookupString ) const
{
    xwchar* pString = NULL;
    s32* pIndex = (s32*)(m_pData+4);

    string_mgr::wchar_string WideString;
    xwchar* pLookupWide = Ansi2UpperWide( lookupString, WideString );

    s32 imax = m_nStrings;
    s32 imin = 0;
    xbool    bFound = FALSE;
    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;

        pString = (xwchar*)(m_pData+4+4*m_nStrings+pIndex[i]);

        if( imax == imin+1 )
        {
            if( !x_wstrcmp( pString, pLookupWide ) )
            {
                bFound = TRUE;
            }
            else
            {
                //-- NOT FOUND
                break;
            }
        }

        s32 cmpVal = x_wstrcmp( pString, pLookupWide );

        if( cmpVal == 0 )
            bFound = TRUE;
        else if ( cmpVal > 0 )
            imax = i;
        else
            imin = i;

        if ( bFound )
        {
#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bShowStringID )
                return pString;
#endif
            // Skip Title
            pString += x_wstrlen(pString);
            pString++;

            // Skip String
            pString += x_wstrlen(pString);
            pString++; // Skip Null

#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bStringTest )
                FillString(pString);    // modifies string
#endif

            // Return Speaker
            return pString;
        }
    }

    return NULL;
}

//==============================================================================

const xwchar* string_table::GetSoundDescString( const char* lookupString ) const
{
    xwchar* pString = NULL;
    s32* pIndex = (s32*)(m_pData+4);

    string_mgr::wchar_string WideString;
    xwchar* pLookupWide = Ansi2UpperWide( lookupString, WideString );

    s32 imax = m_nStrings;
    s32 imin = 0;
    xbool    bFound = FALSE;
    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;

        pString = (xwchar*)(m_pData+4+4*m_nStrings+pIndex[i]);

        if( imax == imin+1 )
        {
            if( !x_wstrcmp( pString, pLookupWide ) )
            {
                bFound = TRUE;
            }
            else
            {
                //-- NOT FOUND
                break;
            }
        }

        s32 cmpVal = x_wstrcmp( pString, pLookupWide );

        if( cmpVal == 0 )
            bFound = TRUE;
        else if ( cmpVal > 0 )
            imax = i;
        else
            imin = i;

        if ( bFound )
        {
#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bShowStringID )
                return pString;
#endif
            // Skip Title
            pString += x_wstrlen(pString);
            pString++;

            // Skip String
            pString += x_wstrlen(pString);
            pString++; // Skip Null
            
            // Skip Subtitle string.
            pString += x_wstrlen(pString);
            pString++;

#if !defined(X_RETAIL) || defined(X_QA)
            if( g_bStringTest )
                FillString(pString);    // modifies string
#endif

            // Return Speaker
            return pString;
        }
    }

    return NULL;
}

//==============================================================================

string_mgr::string_mgr( void )
{
    ASSERT( !m_Initialized );
    m_Initialized = TRUE;
    m_Tables.SetCapacity( 64 );
    m_Tables.SetLocked( TRUE );
}

//==============================================================================

string_mgr::~string_mgr( void )
{
    ASSERT( m_Initialized );
    m_Initialized = FALSE;
}

//==============================================================================
s32 string_mgr::GetStringCount( const char* pTableName )
{
    s32 numStrings = 0;

    const string_table* pTable  = FindTable( pTableName );
    if( pTable )
    {
        numStrings = pTable->GetCount();
    }

    return numStrings;
}

//==============================================================================

xbool string_mgr::LoadTable( const char* pTableName, const char* pFileName )
{
    CONTEXT( "string_mgr::LoadTable" );

    // If the table is already loaded then just return.
    if( FindTable( pTableName ) != 0 )
        return TRUE;

    // Create a new table
    string_table* pTable = new string_table;
    ASSERT( pTable );

    // Load the table
    xbool Success = pTable->Load( pTableName, pFileName );

    // Keep it or heave it.
    if( Success )
    {
		x_DebugMsg( 7, "********* %s Loaded\n", pFileName );
        LOG_MESSAGE( "string_mgr::LoadTable",
                     "Loaded: %s - %s",
                     pTableName, pFileName );
        m_Tables.Append( pTable );
    }
    else
    {
        delete pTable;
        LOG_ERROR( "string_mgr::LoadTable", 
                   "Failed to load string table: %s - %s",
                   pTableName, pFileName );
        ASSERT( FALSE );
    }

    // Return success code
    return Success;
}

//==============================================================================

void string_mgr::UnloadTable( const char* pTableName )
{
    s32 iTable = -1;
    
    // Find Table
    for( s32 i=0 ; i<m_Tables.GetCount() ; i++ )
    {
        if( x_strcmp( m_Tables[i]->m_pTableName, pTableName ) == 0 )
        {
            iTable = i;
            break;
        }
    }

    // Found?
    if( iTable != -1 )
    {
		x_DebugMsg( 7, "********* %s UnLoaded\n", pTableName );
        LOG_MESSAGE( "string_mgr::UnloadTable",
                     "Unloaded: %s", pTableName );
        delete m_Tables[iTable];
        m_Tables.Delete( iTable );
    }
    else
    {
        ASSERT( FALSE );
    }
}

//==============================================================================

const string_table* string_mgr::FindTable( const char* pTableName ) const
{
    // Find Table
    for( s32 i=0 ; i<m_Tables.GetCount() ; i++ )
    {
        if( x_stricmp(m_Tables[i]->m_pTableName, pTableName) == 0 )
        {
            return m_Tables[i];
        }
    }

    return NULL;
}

//==============================================================================
// string_mgr
//==============================================================================

const xwchar* string_mgr::operator () ( const char* pTableName, s32 Index, const xbool bLogNULLString ) const
{
    const string_table* pTable  = FindTable( pTableName );
    if( pTable )
    {
        const xwchar* pString = pTable->GetAt( Index );
        if( pString )
            return pString;
    }

    // String not found
    if( bLogNULLString )
        LOG_ERROR( "string_table::()", "Failed to find string %d in table '%s'", Index, pTableName );
    return s_NullString;
}

//==============================================================================

const xwchar* string_mgr::operator () ( const char* pTableName, const char* TitleString, const xbool bLogNULLString ) const
{
    const string_table* pTable  = FindTable( pTableName );
    if( pTable )
    {
        const xwchar* pString = pTable->GetAt( TitleString );
        if( pString )
            return pString;
    }

    // String not found
    if( bLogNULLString )
        LOG_ERROR( "string_table::()", "Failed to find string '%s' in table '%s'", TitleString, pTableName );
    return s_NullString;
}

//==============================================================================
const xwchar* string_mgr::GetSubTitleSpeaker( const char* pTableName, const char* SpeakerString, const xbool bLogNULLString ) const
{
    const string_table* pTable  = FindTable( pTableName );
    if( pTable )
    {
        const xwchar* pString = pTable->GetSubTitleSpeaker( SpeakerString );
        if( pString )
            return pString;
    }

    // String not found
    if( bLogNULLString )
        LOG_ERROR( "string_table::GetSubTitleSpeaker", "Failed to find subtitle string '%s' in table '%s'", SpeakerString, pTableName );
    return s_NullString;
}

//==============================================================================
const xwchar* string_mgr::GetSoundDescString( const char* pTableName, const char* SpeakerString, const xbool bLogNULLString ) const
{
    const string_table* pTable  = FindTable( pTableName );
    if( pTable )
    {
        const xwchar* pString = pTable->GetSoundDescString( SpeakerString );
        if( pString )
            return pString;
    }

    // String not found
    if( bLogNULLString )
        LOG_ERROR( "string_table::GetSoundDescString", "Failed to find sounddesc string '%s' in table '%s'", SpeakerString, pTableName );
    return s_NullString;
}

//==============================================================================
