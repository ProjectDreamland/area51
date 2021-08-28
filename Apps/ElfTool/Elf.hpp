//==============================================================================
// Elf.hpp
//==============================================================================

//==============================================================================

typedef u32 Elf32_Addr;
typedef u16 Elf32_Half;
typedef u32 Elf32_Off;
typedef s32 Elf32_Sword;
typedef u32 Elf32_Word;

//==============================================================================
// Elf32_Ehdr
//==============================================================================

struct Elf32_Ehdr
{
    enum magic
    {
        ELFMAG0     = 0x7f,
        ELFMAG1     = 'E',
        ELFMAG2     = 'L',
        ELFMAG3     = 'F'
    };

    enum ident_index
    {
        EI_MAG0     = 0,    // Magic 0
        EI_MAG1     = 1,    // Magic 1
        EI_MAG2     = 2,    // Magic 2
        EI_MAG3     = 3,    // Magic 3
        EI_CLASS    = 4,    // File class
        EI_DATA     = 5,    // Data encoding
        EI_VERSION  = 6,    // File version
        EI_PAD      = 7,    // Start of padding bytes
        EI_NIDENT   = 16    // Size of e_ident[]
    };

    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half      e_type;
    Elf32_Half      e_machine;
    Elf32_Word      e_version;
    Elf32_Addr      e_entry;
    Elf32_Off       e_phoff;
    Elf32_Off       e_shoff;
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;
    Elf32_Half      e_phentsize;
    Elf32_Half      e_phnum;
    Elf32_Half      e_shentsize;
    Elf32_Half      e_shnum;
    Elf32_Half      e_shstrndx;

    enum type
    {
        ET_NONE     = 0,
        ET_REL      = 1,
        ET_EXEC     = 2,
        ET_DYN      = 3,
        ET_CORE     = 4,
        ET_LOPROC   = 0xff00,
        ET_HIPROC   = 0xffff
    };

    enum machine
    {
        EM_NONE     = 0,
        EM_M32      = 1,    // AT&T WE 32100
        EM_SPARC    = 2,    // SPARC
        EM_386      = 3,    // Intel 80386
        EM_68K      = 4,    // Motorola 68000
        EM_88K      = 5,    // Motorola 88000
        EM_860      = 7,    // Intel 80860
        EM_MIPS     = 8,    // MIPS R3000
    };

    enum version
    {
        EV_NONE     = 0,    // Invalid version
        EV_CURRENT  = 1,    // Current version
    };
};

//==============================================================================
// Elf32_Shdr
//==============================================================================

struct Elf32_Shdr
{
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
    Elf32_Word  sh_flags;
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addrAlign;
    Elf32_Word  sh_entsize;

    enum type
    {
        SHT_NULL        = 0,
        SHT_PROGBITS    = 1,
        SHT_SYMTAB      = 2,
        SHT_STRTAB      = 3,
        SHT_RELA        = 4,
        SHT_HASH        = 5,
        SHT_DYNAMIC     = 6,
        SHT_NOTE        = 7,
        SHT_NOBITS      = 8,
        SHT_REL         = 9,
        SHT_SHLIB       = 10,
        SHT_DYNSYM      = 11,
        SHT_LOPROC      = 0x70000000,
        SHT_HIPROC      = 0x7fffffff,
        SHT_LOUSER      = 0x80000000,
        SHT_HIUSER      = 0xffffffff
    };

    enum section
    {
        SHN_UNDEF       = 0
    };

    enum flags
    {
        SHF_WRITE       = 0x01,
        SHF_ALLOC       = 0x02,
        SHF_EXECINSTR   = 0x04,
        SHF_MASKPROC    = 0xf0000000
    };
};

//==============================================================================
// Elf32_Sym
//==============================================================================

struct Elf32_Sym
{
    Elf32_Word  st_name;
    Elf32_Addr  st_value;
    Elf32_Word  st_size;
    u8          st_info;
    u8          st_other;
    Elf32_Half  st_shndx;

    enum binding
    {
        STB_LOCAL   = 0,
        STB_GLOBAL  = 1,
        STB_WEAK    = 2,
        STB_LOPROC  = 13,
        STB_HIPROC  = 15
    };

    enum type
    {
        STT_NOTYPE  = 0,
        STT_OBJECT  = 1,
        STT_FUNC    = 2,
        STT_SECTION = 3,
        STT_FILE    = 4,
        STT_LOPROC  = 13,
        STT_HIPROC  = 15
    };

    enum symbol
    {
        STN_UNDEF   = 0
    };
};

//==============================================================================
// elf
//==============================================================================

class symbol
{
public:
    xstring         m_Name;             // Name of symbol
    int             m_ID;               // Numeric ID
};

class symbol_ref
{
public:
    int             m_ReferenceID;      // ID of symbol this one references
};

class symbol_structure : public symbol
{
public:
    xarray<symbol*> m_pMembers;
};

class symbol_member : public symbol
{
public:
    symbol*         m_pStructure;
};

class elf_file
{
public:
                elf_file( void );
               ~elf_file( void );

    bool        ParseIdentifier                 ( const char*& p, const char*& pIdentifier, s32& cIdentifier );
    bool        ParseDecimal                    ( const char*& p, s32& Value );
    bool        ParseTypeRef                    ( const char*& p, s32& Type );

    void        ParseSymbol                     ( const char* p );
    bool        ParseSymbolType                 ( const char*& p );
    bool        ParseSymbolStructure            ( const char*& p );
    bool        ParseSymbolStructMember         ( const char*& p );
    bool        ParseSymbolStructMemberFunction ( const char*& p );
    bool        ParseSymbolStructMemberData     ( const char*& p );

public:
    xarray<symbol>              m_Symbols;      // Array of symbols
    xarray<symbol_structure>    m_pStructs;     // Array of structures

    const char*                 m_pSymbolName;
    s32                         m_cSymbolName;
    bool                        m_SymbolIsClass;
    s32                         m_SymbolID;
    s32                         m_SymbolByteLength;

    const char*                 m_pMemberName;
    s32                         m_cMemberName;
    s32                         m_MemberType;
    s32                         m_MemberBitOffset;
    s32                         m_MemberBitSize;
};

class elf
{
public:
                elf( void );
               ~elf( void );

    xbool       Load                    ( const char* pFileName );
    
    Elf32_Shdr* FindSection             ( const char* pSectionName );

    void        PrintSections           ( void );
    void        PrintSymbols            ( void );
    void        PrintMdebug             ( void );
    void        ParseMdebugSymbolString ( const char* pString );

    void        ProcessSymbolString     ( const char* p );

protected:
    xbool               m_Valid;        // Valid ELF
    xstring             m_FileName;     // File name
    s32                 m_Size;         // File length
    byte*               m_pData;        // Raw file
    Elf32_Ehdr*         m_pHeader;      // Elf header
    Elf32_Shdr*         m_pSections;    // Section header array
    const char*         m_pShdrStrs;    // Section header string table

    xarray<elf_file>    m_FileInfo;     // Array of info structures for each file
};

//==============================================================================
