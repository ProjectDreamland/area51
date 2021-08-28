//==============================================================================
//
//  MapFile.cpp
//
//==============================================================================

#include "stdafx.h"
#include "MapFile.h"

//==============================================================================

IMPLEMENT_DYNCREATE( CMapFile, CObject );

#define PRIME_SECTION   100
#define PRIME_OBJECT    1000
#define PRIME_SYMBOL    10000

//==============================================================================

CMapFile::CMapFile()
{
    m_Initialized = false;
}

//==============================================================================

CMapFile::~CMapFile()
{
    // If it has been initialized then kill it
    if( m_Initialized )
        Kill();
}

//==============================================================================

xbool CMapFile::Init( const char* pFileName )
{
    // Kill if already initialized
    if( m_Initialized )
        Kill();

    // Load the file
    m_Initialized = LoadFile( pFileName );

    // Done
    return m_Initialized;
}

//==============================================================================

void CMapFile::Kill( void )
{
    // Clear the data
    m_Sections.SetCount( 0 );
    m_Objects .SetCount( 0 );
    m_Symbols .SetCount( 0 );
}

//==============================================================================

xbool CMapFile::ishex( char c )
{
    c = toupper(c);
    return( ((c>='0') && (c<='9')) || ((c>='A') && (c<='F')) );
}

//==============================================================================

s32 CMapFile::tohex( char c )
{
    c = toupper(c);
    if( c > '9' )
        return c-'A'+10;
    else
        return c-'0';
}

//==============================================================================

u32 CMapFile::ReadHex( xstring& s, s32& i )
{
    s32 v = 0;
    while( (i < s.GetLength()) && (ishex(s[i])) )
    {
        v <<= 4;
        v += tohex(s[i]);
        i++;
    }
    return v;
}

//==============================================================================

void CMapFile::SkipSpaces( xstring&s, s32& i )
{
    while( (i < s.GetLength()) && (isspace(s[i])) )
        i++;
}

//==============================================================================

void CMapFile::SkipField( xstring& s, s32& i )
{
    SkipSpaces( s, i );
    while( (i < s.GetLength()) && !(isspace(s[i])) )
        i++;
}

//==============================================================================

void CMapFile::SkipToEOL( xstring& s, s32& i )
{
    while( (i < s.GetLength()) && (s[i] != 0x0a) )
    {
        if( s[i] == 0x0d ) s[i] = 0;
        i++;
    }
    if( i < s.GetLength() )
        s[i++] = 0;
}

//==============================================================================
//  LoadFile
//==============================================================================

static int fn_symbol_address_sort( const void* a1, const void* a2 )
{
    CMapFile::symbol* p1 = (CMapFile::symbol*)a1;
    CMapFile::symbol* p2 = (CMapFile::symbol*)a2;

    if( p1->Address < p2->Address )
        return -1;
    else if( p1->Address > p2->Address )
        return 1;
    else
        return 0;
}

xbool CMapFile::LoadFile( const char* pFileName )
{
    xbool   Success         = TRUE;
    s32     iCurrentSection = 0;
    s32     iCurrentObject  = 0;

    // Save file name
    m_FileName = pFileName;

    // Prime array
    m_Sections.SetCapacity( PRIME_SECTION );
    m_Objects .SetCapacity( PRIME_OBJECT  );
    m_Symbols .SetCapacity( PRIME_SYMBOL  );

    // Add undefined section
    {
        section& s  = m_Sections.Append();
        s.iName     = 0;
        s.Address   = 0;
        s.Size      = 0;
    }

    // Add undefined object
    {
        object& s   = m_Objects.Append();
        s.iName     = 0;
        s.Address   = 0;
        s.Size      = 0;
    }

    // Add undefined symbol
    {
        symbol& s   = m_Symbols.Append();
        s.iName     = 0;
        s.Address   = 0;
        s.Size      = 0;
        s.iSection  = iCurrentSection;
        s.iObject   = iCurrentObject;
    }

    // Load mapfile
    if( m_MapFile.LoadFile( pFileName ) )
    {
        s32         LineIndex   = 0;
        s32         Index       = 0;

        // Skip to first data line
        while( (Index < m_MapFile.GetLength()) && (m_MapFile[Index] != '0') )
            Index++;

        while( Index < m_MapFile.GetLength() )
        {

            LineIndex = Index;

            u32 Address = ReadHex    ( m_MapFile, Index );
                          SkipSpaces ( m_MapFile, Index );
            u32 Size    = ReadHex    ( m_MapFile, Index );
                          SkipField  ( m_MapFile, Index );
                          SkipSpaces ( m_MapFile, Index );

            // Section?
            if( ((Index-LineIndex) == 24) )
            {
                section& s  = m_Sections.Append();
                s.iName     = Index;
                s.Address   = Address;
                s.Size      = Size;

                iCurrentSection = m_Sections.GetCount()-1;
            }

            // Object?
            if( ((Index-LineIndex) == 40) )
            {
                object& s   = m_Objects.Append();
                s.iName     = Index;
                s.Address   = Address;
                s.Size      = Size;

                iCurrentObject = m_Objects.GetCount()-1;
            }

            // Symbol?
            if( ((Index-LineIndex) == 48) && (Size > 0) )
            {
                symbol& s  = m_Symbols.Append();
                s.iName    = Index;
                s.Address  = Address;
                s.Size     = Size;
                s.iSection = iCurrentSection;
                s.iObject  = iCurrentObject;
            }

            // Skip to next line
            SkipToEOL( m_MapFile, Index );
        }
    }
    else
    {
        x_printf( "Error - Failed to load map file '%s'\n", pFileName );
        Success = FALSE;
    }

    // Sort the symbols into address order
    x_qsort( &m_Symbols[0], m_Symbols.GetCount(), sizeof(symbol), fn_symbol_address_sort );

    return Success;
}

//==============================================================================

const char* CMapFile::AddressToSymbol( u32 Address )
{
    if( !m_Initialized )
        return NULL;

    s32     imin = 0;
    s32     imax = m_Symbols.GetCount()-1;
    symbol* pSymbol = NULL;

    // Binary search of the symbols
    while( imax >= imin )
    {
        s32 i = (imin+imax)/2;

        // Check for hit
        if( (Address - m_Symbols[i].Address) < m_Symbols[i].Size )
        {
            // Get pointer to symbol
            pSymbol = &m_Symbols[i];
            break;
        }
        else
        {
            // Refine the search
            if( m_Symbols[i].Address < Address )
                imin = i+1;
            else
                imax = i-1;
        }
    }

    // Found it
    if( pSymbol )
        return &m_MapFile[pSymbol->iName];

/*
    // Search the object files for a hit
    for( i=0 ; i<m_Objects.GetCount() ; i++ )
    {
        // Check for hit
        if( (Address - m_Objects[i].Address) < m_Objects[i].Size )
        {
            // Hit symbol, increment counter and bail
            m_Objects[i].Hits++;
            return;
        }
    }
*/

    // Failed
    return NULL;
}

//==============================================================================

void CMapFile::Serialize(CArchive& ar)
{
    CObject::Serialize( ar );

	if( ar.IsStoring() )
	{
        CString t;
        s32     i;

        // Initialized
        ar << m_Initialized;

        if( m_Initialized )
        {
            // File name
            t = m_FileName; ar << t;

            // Write file contents
            ar << m_MapFile.GetLength();
            ar.Write( &m_MapFile[0], m_MapFile.GetLength() );

            // Write sections
            ar << m_Sections.GetCount();
            for( i=0 ; i<m_Sections.GetCount() ; i++ )
            {
                section& s = m_Sections[i];
                ar << s.iName;
                ar << s.Address;
                ar << s.Size;
            }

            // Write objects
            ar << m_Objects.GetCount();
            for( i=0 ; i<m_Objects.GetCount() ; i++ )
            {
                object& o = m_Objects[i];
                ar << o.iSection;
                ar << o.iName;
                ar << o.Address;
                ar << o.Size;
            }

            // Write symbols
            ar << m_Symbols.GetCount();
            for( i=0 ; i<m_Symbols.GetCount() ; i++ )
            {
                symbol& s = m_Symbols[i];
                ar << s.iSection;
                ar << s.iObject;
                ar << s.iName;
                ar << s.Address;
                ar << s.Size;
            }
        }
	}
	else
	{
        CString t;
        s32     Length;
        s32     Count;
        s32     i;

        // Initialized
        ar >> m_Initialized;
        if( m_Initialized )
        {
            // File name
            ar >> t;
            m_FileName = (const char*)t;

            // Read file contents
            ar >> Length;
            m_MapFile.SetLength( Length );
            ar.Read( &m_MapFile[0], Length );

            // Read sections
            ar >> Count;
            m_Sections.SetCapacity( Count );
            for( i=0 ; i<Count ; i++ )
            {
                section& s = m_Sections.Append();
                ar >> s.iName;
                ar >> s.Address;
                ar >> s.Size;
            }

            // Read objects
            ar >> Count;
            m_Objects.SetCapacity( Count );
            for( i=0 ; i<Count ; i++ )
            {
                object& o = m_Objects.Append();
                ar >> o.iSection;
                ar >> o.iName;
                ar >> o.Address;
                ar >> o.Size;
            }

            // Read symbols
            ar >> Count;
            m_Symbols.SetCapacity( Count );
            for( i=0 ; i<Count ; i++ )
            {
                symbol& s = m_Symbols.Append();
                ar >> s.iSection;
                ar >> s.iObject;
                ar >> s.iName;
                ar >> s.Address;
                ar >> s.Size;
            }
        }
	}
}

//==============================================================================
