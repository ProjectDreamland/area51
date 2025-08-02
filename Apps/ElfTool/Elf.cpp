//==============================================================================
// Elf.cpp
//==============================================================================

#include "stdafx.h"
#include "Elf.hpp"
#include "Elf_Mdebug.hpp"

//==============================================================================

elf::elf( void )
{
    m_Valid = FALSE;
}

elf::~elf( void )
{
}

//==============================================================================

xbool elf::Load( const char* pFileName )
{
    // Open the file
    X_FILE* pFile = x_fopen( pFileName, "rb" );
    if( pFile )
    {
        // Store file details
        m_FileName = pFileName;
        m_Size = x_flength( pFile );
        m_pData = (byte*)x_malloc( m_Size );
        ASSERT( m_pData );
        x_fread( m_pData, 1, m_Size, pFile );
        x_fclose( pFile );

        // Get the header pointer
        m_pHeader = (Elf32_Ehdr*)m_pData;

        // Check for a valid elf file
        if( (m_pHeader->e_ident[Elf32_Ehdr::EI_MAG0] == Elf32_Ehdr::ELFMAG0) &&
            (m_pHeader->e_ident[Elf32_Ehdr::EI_MAG1] == Elf32_Ehdr::ELFMAG1) &&
            (m_pHeader->e_ident[Elf32_Ehdr::EI_MAG2] == Elf32_Ehdr::ELFMAG2) &&
            (m_pHeader->e_ident[Elf32_Ehdr::EI_MAG3] == Elf32_Ehdr::ELFMAG3) &&
            (m_pHeader->e_machine == Elf32_Ehdr::EM_MIPS) &&
            (m_pHeader->e_version == Elf32_Ehdr::EV_CURRENT) )
        {
            // Verify sizeof struct is the same as the file format
            ASSERT( m_pHeader->e_shentsize == sizeof(Elf32_Shdr) );

            // Setup internal pointers for navigating sections
            m_pSections = (Elf32_Shdr*)(m_pData + m_pHeader->e_shoff);
            m_pShdrStrs = (const char*)(m_pData + m_pSections[m_pHeader->e_shstrndx].sh_offset );

            // Success
            m_Valid = TRUE;
        }
    }

    return m_Valid;
}

//==============================================================================

Elf32_Shdr* elf::FindSection( const char* pSectionName )
{
    if( m_Valid )
    {
        // Loop over all the sections finding symbol sections
        for( s32 i=1 ; i<m_pHeader->e_shnum ; i++ )
        {
            // Check for name match
            Elf32_Shdr& s = m_pSections[i];
            if( x_strcmp( pSectionName, m_pShdrStrs + s.sh_name ) == 0 )
                return &s;
        }
    }

    // Not found
    return NULL;
}

//==============================================================================

void elf::PrintSections( void )
{
    if( m_Valid )
    {
        // Print Header
        //         12345678 12345678 12345678 12345678 12345678 123456789012 '%s'
        x_printf( "Offset   Size     Address  Link     Info     Type         Name\n"
                  "------   ----     -------  ----     ----     ----         ----\n" );

        // Loop over all the sections printing details
        for( s32 i=1 ; i<m_pHeader->e_shnum ; i++ )
        {
            Elf32_Shdr& s = m_pSections[i];

            xstring TypeName;
            switch( s.sh_type )
            {
            case Elf32_Shdr::SHT_NULL:      TypeName = "SHT_NULL    "; break;
            case Elf32_Shdr::SHT_PROGBITS:  TypeName = "SHT_PROGBITS"; break;
            case Elf32_Shdr::SHT_SYMTAB:    TypeName = "SHT_SYMTAB  "; break;
            case Elf32_Shdr::SHT_STRTAB:    TypeName = "SHT_STRTAB  "; break;
            case Elf32_Shdr::SHT_RELA:      TypeName = "SHT_RELA    "; break;
            case Elf32_Shdr::SHT_HASH:      TypeName = "SHT_HASH    "; break;
            case Elf32_Shdr::SHT_DYNAMIC:   TypeName = "SHT_DYNAMIC "; break;
            case Elf32_Shdr::SHT_NOTE:      TypeName = "SHT_NOTE    "; break;
            case Elf32_Shdr::SHT_NOBITS:    TypeName = "SHT_NOBITS  "; break;
            case Elf32_Shdr::SHT_REL:       TypeName = "SHT_REL     "; break;
            case Elf32_Shdr::SHT_SHLIB:     TypeName = "SHT_SHLIB   "; break;
            case Elf32_Shdr::SHT_DYNSYM:    TypeName = "SHT_DYNSYM  "; break;
            default:                        TypeName.Format( "<0x%08x>", s.sh_type ); break;
            }

            x_printf( "%08x %08x %08x %08x %08x %s '%s'\n", s.sh_offset, s.sh_size, s.sh_addr, s.sh_link, s.sh_info, (const char*)TypeName, m_pShdrStrs + s.sh_name );
        }
    }
}

//==============================================================================

void elf::PrintSymbols( void )
{
    if( m_Valid )
    {
        // Loop over all the sections finding symbol sections
        for( s32 i=1 ; i<m_pHeader->e_shnum ; i++ )
        {
            Elf32_Shdr& s = m_pSections[i];

            // Check that this is a symbol table section
            if( (s.sh_type == Elf32_Shdr::SHT_SYMTAB) ||
                (s.sh_type == Elf32_Shdr::SHT_DYNSYM) )
            {
                // Get pointer to base of strings for this section
                Elf32_Shdr& Strings = m_pSections[s.sh_link];
                const char* pStrings = (const char*)(m_pData + Strings.sh_offset);

                // Get pointer to first symbol and the number of symbols
                Elf32_Sym*  pSymbol = (Elf32_Sym*)(m_pData + s.sh_offset);
                s32         nSymbols = s.sh_size / sizeof(Elf32_Sym);

                // Loop through them and print the details
                for( s32 i=1 ; i<nSymbols ; i++ )
                {
                    x_printf( "%08x %08x %02x %02x %04x '%s'\n", pSymbol->st_value, pSymbol->st_size, pSymbol->st_info, pSymbol->st_other, pSymbol->st_shndx, pStrings + pSymbol->st_name );
                    pSymbol++;
                }
            }
        }
    }
}

//==============================================================================

void elf::PrintMdebug( void )
{
    // Parse out the .mdebug section from the elf
    Elf32_Shdr* pSection = FindSection( ".mdebug" );
	if( pSection )
	{
        MDB_HDRR* pHeader = (MDB_HDRR*)(m_pData + pSection->sh_offset);

        for( s32 iFile=0 ; iFile<pHeader->ifdMax ; iFile++ )
        {
            // Get the File header
            MDB_FDR* pFile = (MDB_FDR*)(m_pData + pHeader->cbFdOffset + (sizeof(MDB_FDR) * iFile));

            // Add an entry to the FileInfo table for this file
            m_FileInfo.Append();

            // Get the name of the source file
            const char* pString = (const char*)(m_pData + pHeader->cbSsOffset + pFile->issBase + pFile->rss);

            // Loop through all the symbols for this file
            for( s32 i=0 ; i<pFile->csym ; i++ )
            {
                // Get the symbol
                MDB_SYMR* pSym = (MDB_SYMR*)(m_pData + pHeader->cbSymOffset + ((pFile->isymBase + i) * sizeof(MDB_SYMR)));

                // Print the symbol details in CSV form
                if( pSym->st == MDB_SYMR::ST_NIL )
                {
                    const char* pString = (const char*)(m_pData + pHeader->cbSsOffset + pFile->issBase + pSym->iss);

                    m_FileInfo[iFile].ParseSymbol( pString );
	            }
            }
        }
    }
}

//==============================================================================

elf_file::elf_file( )
{
}

elf_file::~elf_file( )
{
}

//==============================================================================

bool isIdentifierFirst( char c )
{
    return( isalpha(c) || (c == '_') );
}

bool isIdentifier( char c )
{
    return( isalpha(c) || isdigit(c) || (c == '_') );
}


bool elf_file::ParseIdentifier( const char*& p, const char*& pIdentifier, s32& cIdentifier )
{
    pIdentifier = p;
    cIdentifier = 0;

    if( isIdentifierFirst( *p ) )
    {
        while( isIdentifier( *p ) )
        {
            cIdentifier++;
            p++;
        }
    }

    return cIdentifier != 0;
}

bool elf_file::ParseDecimal( const char*& p, s32& Value )
{
    bool Success = false;
    Value = 0;

    while( isdigit( *p ) )
    {
        Value *= 10;
        Value += (s32)(*p - '0');
        Success = true;
        p++;
    }

    return Success;
}

bool elf_file::ParseTypeRef( const char*& p, s32& Type )
{
    bool Success = true;
    s32 RefID;

    if( (p[0] == 'a') && (p[1] == 'r') && (p[2] == '1') )
    {
        p += 4; // Skip ar1;
        s32 ArrayLow;
        s32 ArrayHigh;
        VERIFY( ParseDecimal( p, ArrayLow  ) );
        ASSERT( *p == ';' );
        p++;
        VERIFY( ParseDecimal( p, ArrayHigh ) );
        ASSERT( *p == ';' );
        p++;
        VERIFY( ParseTypeRef( p, RefID ) );
    }
    else if( (p[0] == '#') && (p[1] == '#') )
    {
        p += 2;
        VERIFY( ParseTypeRef( p, RefID ) );
        ASSERT( *p == ';' );
        p++; // Skip ;
    }
    else if( (p[0] == 'x') && (p[1] == 's') )
    {
        p += 2;
        const char* pIdentifier;
        s32         cIdentifier;
        VERIFY( ParseIdentifier( p, pIdentifier, cIdentifier ) );
        ASSERT( *p == ':' );
        p++; // Skip :
    }
    else if( p[0] == 'f' )
    {
        p += 1;
        VERIFY( ParseTypeRef( p, RefID ) );
    }
    else if( p[0] == '*' )
    {
        p += 1;
        VERIFY( ParseTypeRef( p, RefID ) );
    }
    else if( p[0] == '&' )
    {
        p += 1;
        VERIFY( ParseTypeRef( p, RefID ) );
    }
    else if( ParseDecimal( p, RefID ) )
    {
        Type = RefID;
    }
    else
    {
        // Unknown type expression
        ASSERT( 0 );
    }

    // Deal with assignment operator
    if( *p == '=' )
    {
        p++;
        VERIFY( ParseTypeRef( p, RefID ) );
    }

    return Success;
}

void elf_file::ParseSymbol( const char* p )
{
    // First parse the symbol name
    if( ParseIdentifier( p, m_pSymbolName, m_cSymbolName ) )
    {
        // Next expect :
        if( *p++ == ':' )
        {
            m_SymbolIsClass = false;

            if( *p == 'T' )
            {
                m_SymbolIsClass = true;
                p++;
            }

            switch( *p++ )
            {
            case 't':
                ParseSymbolType( p );
                break;
            }
        }
    }
}

bool elf_file::ParseSymbolType( const char*& p )
{
    bool Success = false;

    // Parse the symbol ID
    if( ParseDecimal( p, m_SymbolID ) )
    {
        // Check for =
        if( *p++ == '=' )
        {
            switch( *p )
            {
            case 's':
                p++;
                Success = ParseSymbolStructure( p );
                break;
            default:
                break;
            }
        }
    }

    return Success;
}

bool elf_file::ParseSymbolStructure( const char*& p )
{
    bool Success = false;

    // Parse byte length of symbol
    if( ParseDecimal( p, m_SymbolByteLength ) )
    {
        // Print symbol info
        xstring s( m_pSymbolName );
        s = s.Left( m_cSymbolName );
        x_printf( "'%s' %d\n", (const char*)s, m_SymbolByteLength );

        // Iterate over structure members
        while( *p != ';' )
        {
            // Parse the member
            Success &= ParseSymbolStructMember( p );

            // Consume the ';'
            p++;
        }
    }

    return Success;
}

bool elf_file::ParseSymbolStructMember( const char*& p )
{
    bool Success = false;

    // Parse the name of the member
    if( ParseIdentifier( p, m_pMemberName, m_cMemberName ) )
    {
        // Expecting ':'
        if( *p++ == ':' )
        {
            // Check for function
            if( *p == ':' )
            {
                p++;
                Success = ParseSymbolStructMemberFunction( p );
            }
            else
            {
                Success = ParseSymbolStructMemberData( p );
            }
        }
    }

    return Success;
}

bool elf_file::ParseSymbolStructMemberFunction( const char*& p )
{
    return false;
}

bool elf_file::ParseSymbolStructMemberData( const char*& p )
{
    bool Success = false;

    // Parse the symbol storage class
    if( *p == '/' )
    {
        p += 2;
    }

    // Parse the type
    if( ParseTypeRef( p, m_MemberType ) )
    {
        if( *p++ == ',' )
        {
            // Parse the offset
            if( ParseDecimal( p, m_MemberBitOffset ) )
            {
                if( *p++ == ',' )
                {
                    // Parse the size
                    if( ParseDecimal( p, m_MemberBitSize ) )
                    {
                        // Success, we have parsed a member data field
                        Success = true;
                    }
                }
            }
        }
    }

    ASSERT( Success == true );
    return Success;
}

//==============================================================================
