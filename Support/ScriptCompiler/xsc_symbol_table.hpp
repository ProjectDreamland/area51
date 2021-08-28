//==============================================================================
//  
//  xsc_symbol_table.hpp
//  
//==============================================================================

#ifndef XSC_SYMBOL_TABLE_HPP
#define XSC_SYMBOL_TABLE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  External & Forward references
//==============================================================================

struct xsc_token;
class  xsc_symbol;
class  xsc_scope;
extern class xsc_symbol* g_pTvoid;

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  xsc_symbol
//==============================================================================

// Symbol visibility
enum symbol_visibility
{
    symvis_public,                                      // Public visibility
    symvis_protected,                                   // Protected visibility
    symvis_private,                                     // Private visibility
};

// Symbol flags
#define SYM_CONST           (1<<0)
#define SYM_STATIC          (1<<1)
#define SYM_VIRTUAL         (1<<2)
#define SYM_NATIVE          (1<<3)
#define SYM_MEMBER          (1<<4)
#define SYM_IMPORTED        (1<<5)
#define SYM_INITIALIZED     (1<<6)
#define SYM_DEFINED         (1<<7)

// Symbol types
enum symbol_type
{
    symtype_null,                                       // NULL
    symtype_keyword,                                    // Keyword symbol
    symtype_type,                                       // Type symbol
    symtype_typedef,                                    // Typedef symbol
    symtype_enum,                                       // Enum symbol
    symtype_enumvalue,                                  // Enum value symbol
    symtype_class,                                      // Class symbol
    symtype_field,                                      // Struct/Class Field symbol
    symtype_method,                                     // Struct/Class Method symbol
    symtype_argument,                                   // Method Argument symbol
    symtype_local,                                      // Local variable
};

class typeref
{
public:
    xsc_symbol*         pType;                                      // Type symbol
    xbool               IsReference;                                // Reference type

                        typeref         ( );
                        typeref         ( xsc_symbol* apType, xbool aIsReference = FALSE );
    bool                operator !=     ( const typeref& t );
    s32                 GetByteSize     ( void );
};

class xsc_symbol
{
//==============================================================================
//  Data
//==============================================================================
public:
    // Symbol Common Data
    s32                 GUID;                                       // Only used for loading symbols
    xwstring            Name;                                       // Name string
    symbol_type         SymbolType;                                 // Symbol Type
    const xsc_token*    pDefiningToken;                             // Token that defines this symbol
    xsc_scope*          pParentScope;                               // Owning Scope for this symbol
    xsc_scope*          pChildScope;                                // Scope for members of this Class / Method
    typeref             Type;                                       // Type of this field or method

    // Class specific
    const xsc_symbol*   pSuperClass;                                // SuperClass

    // Method specific
    xwstring            Signature;                                  // Signature string for methods

    // Attributes
    symbol_visibility   Visibility;                                 // Public / Protected / Private
    s32                 Flags;                                      // Flags
    s32                 enumValue;                                  // enum value

    // Storage Data
    s32                 StorageOffset;                              // Offset for storage
    s32                 StorageSize;                                // Size of storage

//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_symbol      ( void );
                       ~xsc_symbol      ( void );

    // Flag Access Functions
    xbool               IsConst         ( void );
    xbool               IsStatic        ( void );
    xbool               IsVirtual       ( void );
    xbool               IsNative        ( void );
    xbool               IsMember        ( void );
    xbool               IsImported      ( void );
    xbool               IsInitialized   ( void );
    xbool               IsDefined       ( void );
    void                SetConst        ( xbool State );
    void                SetStatic       ( xbool State );
    void                SetVirtual      ( xbool State );
    void                SetNative       ( xbool State );
    void                SetMember       ( xbool State );
    void                SetImported     ( xbool State );
    void                SetInitialized  ( xbool State );
    void                SetDefined      ( xbool State );

    const xsc_symbol&   operator =      ( const xsc_symbol& s );
};

//==============================================================================
//  xsc_scope
//==============================================================================

// Scope flags
#define SCOPE_IMPORTED      (1<<0)

class xsc_scope
{
    friend class xsc_symbol_table;

//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_scope           ( );
                       ~xsc_scope           ( void );

    xsc_symbol*         AddSymbol           ( const xwstring&   Name,
                                              const xsc_token*  pToken,
                                              symbol_type       SymbolType,
                                              xsc_symbol*       pType = NULL );         // Add a symbol
    xsc_symbol*         AddSymbol           ( xsc_symbol*       pSymbol );              // Add a symbol

    s32                 GetNumSymbols       ( void ) const;                             // Get number of symbols
    xsc_symbol*         GetSymbol           ( s32               iSymbol ) const;        // Get symbol at position
    xsc_symbol*         GetSymbol           ( const xwstring&   Name ) const;           // Find a symbol

    void                SetOwningSymbol     ( xsc_symbol*       pOwningSymbol );        // Set symbol that owns this scope
    xsc_symbol*         GetOwningSymbol     ( void ) const;                             // Get symbol that owns this scope

    void                SetSuperScope       ( xsc_scope*        pSuperScope );          // Set super scope
    xsc_scope*          GetSuperScope       ( void ) const;                             // Get super scope

    void                SetFlags            ( u32               Flags );                // Set Flags
    u32                 GetFlags            ( void ) const;                             // Get Flags

    void                SetStorageSize      ( s32               StorageSize );          // Set StorageSize
    s32                 GetStorageSize      ( void ) const;                             // Get StorageSize

    xstring             Dump                ( xbool             Recurse = FALSE,
                                              s32               Indent = 0 ) const;     // Dump

//==============================================================================
//  Data
//==============================================================================
protected:
    s32                 m_GUID;                                                         // Only used for loading scopes
    u32                 m_Flags;                                                        // Flags
    xsc_symbol*         m_pOwningSymbol;                                                // Owning symbol
    xsc_scope*          m_pSuperScope;                                                  // Super scope we inherit from
    xarray<xsc_symbol*> m_Symbols;                                                      // Symbols
    s32                 m_StorageSize;                                                  // Sum of symbols in scope
};

//==============================================================================
//  xsc_symbol_table
//==============================================================================

class xsc_symbol_table
{
//==============================================================================
//  Functions
//==============================================================================
public:
                        xsc_symbol_table    ( void );
                       ~xsc_symbol_table    ( void );

    xsc_scope*          NewScope            ( xsc_symbol*       pSymbol = NULL );       // Create a new symbol scope
    void                AddScope            ( xsc_scope*        pScope );               // Add a new symbol scope

    void                PushScope           ( xsc_scope*        pScope );               // Push a new symbol scope
    xsc_scope*          PopScope            ( void );                                   // Pop a symbol scope

    xsc_symbol*         AddSymbol           ( const xwstring&   Name,
                                              const xsc_token*  pToken,
                                              symbol_type       SymbolType,
                                              xsc_symbol*       pType = NULL );         // Add a symbol
    xsc_symbol*         GetSymbol           ( const xwstring&   Name,
                                              xsc_scope*        pScope = NULL ) const;  // Get a symbol

    xsc_scope*          GetCurrentScope     ( void ) const;                             // Get current symbol scope

    // Serialization
    xbool               Save                ( const char* pFileName );                  // Save symbol table
    xbool               Load                ( const char* pFileName );                  // Load symbol table
    xbool               Import              ( const xsc_symbol_table&   SymbolTable );  // Import symbol table

    // Debugging
    xstring             Dump                ( void ) const;                             // Dump contents to STDOUT

//==============================================================================
//  Data
//==============================================================================
protected:
    xarray<xsc_scope*>          m_ScopeStack;                                           // Active symbol scopes
    xarray<xsc_scope*>          m_Scopes;                                               // All symbol scopes
};

//==============================================================================
#endif // XSC_SYMBOL_TABLE_HPP
//==============================================================================
