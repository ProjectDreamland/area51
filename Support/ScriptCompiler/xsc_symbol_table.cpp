//==============================================================================
//
//  xsc_symbol_table
//
//==============================================================================

#include "xsc_symbol_table.hpp"
#include "x_memfile.hpp"
#include "xsc_map.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  typeref
//==============================================================================

typeref::typeref()
{
    pType = g_pTvoid;
    IsReference = FALSE;
}

typeref::typeref( xsc_symbol* apType, xbool aIsReference )
{
    pType       = apType;
    IsReference = aIsReference;
}

bool typeref::operator != ( const typeref& t )
{
    return ( (pType       != t.pType      ) ||
             (IsReference != t.IsReference) );
}

s32 typeref::GetByteSize( void )
{
    return IsReference ? 4 : pType->StorageSize;
}

//==============================================================================
//  xsc_symbol
//==============================================================================

xsc_symbol::xsc_symbol( void )
{
    SymbolType          = symtype_null;
    pDefiningToken      = 0;
    pParentScope        = 0;
    pChildScope         = 0;
    Type.pType          = 0;
    Type.IsReference    = 0;
    pSuperClass         = 0;
    Visibility          = symvis_public;
    Flags               = 0;
    enumValue           = 0;
    StorageOffset       = 0;
    StorageSize         = 0;
}

xsc_symbol::~xsc_symbol( void )
{
}

xbool xsc_symbol::IsConst         ( void )
{
    return (Flags & SYM_CONST);
}

xbool xsc_symbol::IsStatic        ( void )
{
    return (Flags & SYM_STATIC);
}

xbool xsc_symbol::IsVirtual       ( void )
{
    return (Flags & SYM_VIRTUAL);
}

xbool xsc_symbol::IsNative        ( void )
{
    return (Flags & SYM_NATIVE);
}

xbool xsc_symbol::IsMember        ( void )
{
    return (Flags & SYM_MEMBER);
}

xbool xsc_symbol::IsImported      ( void )
{
    return (Flags & SYM_IMPORTED);
}

xbool xsc_symbol::IsInitialized   ( void )
{
    return (Flags & SYM_INITIALIZED);
}

xbool xsc_symbol::IsDefined       ( void )
{
    return (Flags & SYM_DEFINED);
}

void  xsc_symbol::SetConst        ( xbool State )
{
    Flags = (Flags & ~SYM_CONST) | (State ? SYM_CONST : 0);
}

void  xsc_symbol::SetStatic       ( xbool State )
{
    Flags = (Flags & ~SYM_STATIC) | (State ? SYM_STATIC : 0);
}

void  xsc_symbol::SetVirtual      ( xbool State )
{
    Flags = (Flags & ~SYM_VIRTUAL) | (State ? SYM_VIRTUAL : 0);
}

void  xsc_symbol::SetNative       ( xbool State )
{
    Flags = (Flags & ~SYM_NATIVE) | (State ? SYM_NATIVE : 0);
}

void  xsc_symbol::SetMember       ( xbool State )
{
    Flags = (Flags & ~SYM_MEMBER) | (State ? SYM_MEMBER : 0);
}

void  xsc_symbol::SetImported     ( xbool State )
{
    Flags = (Flags & ~SYM_IMPORTED) | (State ? SYM_IMPORTED : 0);
}

void  xsc_symbol::SetInitialized  ( xbool State )
{
    Flags = (Flags & ~SYM_INITIALIZED) | (State ? SYM_INITIALIZED : 0);
}

void  xsc_symbol::SetDefined      ( xbool State )
{
    Flags = (Flags & ~SYM_DEFINED) | (State ? SYM_DEFINED : 0);
}

const xsc_symbol& xsc_symbol::operator = ( const xsc_symbol& s )
{
    Name            = s.Name;
    SymbolType      = s.SymbolType;
    pDefiningToken  = s.pDefiningToken;
    pParentScope    = s.pParentScope;
    pChildScope     = s.pChildScope;
    Type            = s.Type;
    pSuperClass     = s.pSuperClass;
    Signature       = s.Signature;
    Visibility      = s.Visibility;
    Flags           = s.Flags;
    enumValue       = s.enumValue;
    StorageOffset   = s.StorageOffset;
    StorageSize     = s.StorageSize;

    return *this;
}

//==============================================================================
//  xsc_scope
//==============================================================================

xsc_scope::xsc_scope( )
{
    // Initialize
    m_Flags         = 0;
    m_pOwningSymbol = NULL;
    m_pSuperScope   = NULL;
    m_Symbols.SetCapacity( 128 );
    m_StorageSize   = 0;
}

//==============================================================================
//  ~xsc_scope
//==============================================================================

xsc_scope::~xsc_scope( void )
{
    // Delete all symbols
    for( s32 i=0 ; i<m_Symbols.GetCount() ; i++ )
    {
        delete m_Symbols[i];
    }
}

//==============================================================================
//  AddSymbol
//==============================================================================

xsc_symbol* xsc_scope::AddSymbol( const xwstring&    Name,
                                  const xsc_token*   pToken,
                                  symbol_type        SymbolType,
                                  xsc_symbol*        pType )
{
    // TODO: Turn this into a hash for speed

    // Try to find this symbol first
    xsc_symbol* pSymbol = GetSymbol( Name );

    // Check if symbol found
    if( pSymbol == NULL )
    {
        pSymbol = new xsc_symbol;
        ASSERT( pSymbol );

        // Add pointer to symbol into list, double size if at end of array
        if( m_Symbols.GetCount() == m_Symbols.GetCapacity() )
            m_Symbols.SetCapacity( m_Symbols.GetCapacity() * 2);
        m_Symbols.Append() = pSymbol;

        // Setup the default symbol data
        pSymbol->Name               = Name;
        pSymbol->SymbolType         = SymbolType;
        pSymbol->pDefiningToken     = pToken;
        pSymbol->pParentScope       = this;
        pSymbol->Type               = typeref(pType,FALSE);
    }
    else
    {
        // Symbol found, return NULL to flag an error
        pSymbol = NULL;
    }

    return pSymbol;
}

//==============================================================================
//  AddSymbol
//==============================================================================

xsc_symbol* xsc_scope::AddSymbol( xsc_symbol* pSymbol )
{
	xsc_symbol* pNewSymbol = NULL;

    ASSERT( pSymbol );

    // Try to find this symbol first
	pNewSymbol = GetSymbol( pSymbol->Name );

    // Check if symbol found
    if( pNewSymbol == NULL )
    {
        // Add pointer to symbol into list, double size if at end of array
        if( m_Symbols.GetCount() == m_Symbols.GetCapacity() )
            m_Symbols.SetCapacity( m_Symbols.GetCapacity() * 2);
        m_Symbols.Append() = pSymbol;

        pNewSymbol = pSymbol;
    }
    else
    {
        // Symbol found, return NULL to flag an error
        pNewSymbol = NULL;
    }

    return pNewSymbol;
}

//==============================================================================
//  GetNumSymbols
//==============================================================================

s32 xsc_scope::GetNumSymbols( void ) const
{
    return m_Symbols.GetCount();
}

//==============================================================================
//  GetSymbol
//==============================================================================

xsc_symbol* xsc_scope::GetSymbol( s32 iSymbol ) const
{
    ASSERT( (iSymbol >= 0) && (iSymbol < m_Symbols.GetCount()) );
    return m_Symbols[iSymbol];
}

//==============================================================================
//  GetSymbol
//==============================================================================

xsc_symbol* xsc_scope::GetSymbol( const xwstring& Name ) const
{
    s32         i;
    xsc_symbol* pSymbol = NULL;

    // TODO: Turn this into a hash for speed
    for( i=0 ; i<m_Symbols.GetCount() ; i++ )
    {
        // Check for match
        if( Name == m_Symbols[i]->Name )
        {
            // Match, set return value
            pSymbol = m_Symbols[i];
        }
    }

    // Check super scope for a match if we are not importing
    if( (pSymbol == NULL) && (m_pSuperScope != NULL) && !(m_Flags & SCOPE_IMPORTED) )
    {
        pSymbol = m_pSuperScope->GetSymbol( Name );
    }

    return pSymbol;
}

//==============================================================================
//  SetOwningSymbol
//==============================================================================

void xsc_scope::SetOwningSymbol( xsc_symbol* pOwningSymbol )
{
    m_pOwningSymbol = pOwningSymbol;
}

//==============================================================================
//  GetOwningSymbol
//==============================================================================

xsc_symbol* xsc_scope::GetOwningSymbol( void ) const
{
    return m_pOwningSymbol;
}

//==============================================================================
//  SetSuperScope
//==============================================================================

void xsc_scope::SetSuperScope( xsc_scope* pSuperScope )
{
    m_pSuperScope = pSuperScope;
}

//==============================================================================
//  GetSuperScope
//==============================================================================

xsc_scope* xsc_scope::GetSuperScope( void ) const
{
    return m_pSuperScope;
}

//==============================================================================
//  SetFlags
//==============================================================================

void xsc_scope::SetFlags( u32 Flags )
{
    m_Flags = Flags;
}

//==============================================================================
//  GetFlags
//==============================================================================

u32 xsc_scope::GetFlags( void ) const
{
    return m_Flags;
}

//==============================================================================
//  SetStorageSize
//==============================================================================

void xsc_scope::SetStorageSize( s32 StorageSize )
{
    m_StorageSize = StorageSize;
}

//==============================================================================
//  GetStorageSize
//==============================================================================

s32 xsc_scope::GetStorageSize( void ) const
{
    return m_StorageSize;
}

//==============================================================================
//  Dump
//==============================================================================

xstring xsc_scope::Dump( xbool Recurse, s32 Indent ) const
{
    s32     i;
    s32     j;
    xstring Output;

    // Loop though symbols in scope
    for( i=0 ; i<m_Symbols.GetCount() ; i++ )
    {
        xsc_symbol& s = *m_Symbols[i];

        // Dump symbol data
        switch( s.SymbolType )
        {
        case symtype_keyword:
            Output += "<symtype_keyword  >"; break;
        case symtype_type:
            Output += "<symtype_type     >"; break;
        case symtype_typedef:
            Output += "<symtype_typedef  >"; break;
        case symtype_enum:
            Output += "<symtype_enum     >"; break;
        case symtype_enumvalue:
            Output += "<symtype_enumvalue>"; break;
        case symtype_class:
            Output += "<symtype_class    >"; break;
        case symtype_field:
            Output += "<symtype_field    >"; break;
        case symtype_method:
            Output += "<symtype_method   >"; break;
        case symtype_argument:
            Output += "<symtype_argument >"; break;
        case symtype_local:
            Output += "<symtype_local    >"; break;
        }
        Output += " ";

        for( j=0 ; j<Indent ; j++ )
            Output += " ";

        if( s.Type.pType )
            Output += s.Type.pType->Name;
        else if( (s.SymbolType != symtype_typedef) && (s.SymbolType != symtype_enum) && (s.SymbolType != symtype_class) )
            Output += "<no type>";
        Output += " ";

        Output += s.Name;
        Output += " ";
        Output.AddFormat( "%d %d", s.StorageOffset, s.StorageSize );
        Output += "\n";

        // Dump child scope
        if( Recurse && s.pChildScope )
            Output += s.pChildScope->Dump( Recurse, Indent+2 );
    }

    // Return dump
    return Output;
}

//==============================================================================
//  xsc_symbol_table
//==============================================================================

xsc_symbol_table::xsc_symbol_table( void )
{
}

//==============================================================================
//  ~xsc_symbol_table
//==============================================================================

xsc_symbol_table::~xsc_symbol_table( void )
{
    // Delete all scopes
    for( s32 i=0 ; i<m_Scopes.GetCount() ; i++ )
    {
        delete m_Scopes[i];
    }
}

//==============================================================================
//  NewScope
//==============================================================================

xsc_scope* xsc_symbol_table::NewScope( xsc_symbol* pSymbol )
{
    // Create new scope and set owning symbol
    xsc_scope* pScope = new xsc_scope( );
    ASSERT( pScope );
    pScope->SetOwningSymbol( pSymbol );

    // Add to list of scopes
    m_Scopes.Append() = pScope;

    // return scope
    return pScope;
}

//==============================================================================
//  AddScope
//==============================================================================

void xsc_symbol_table::AddScope( xsc_scope* pScope )
{
    // Add to list of scopes
    m_Scopes.Append() = pScope;
}

//==============================================================================
//  PushScope
//==============================================================================

void xsc_symbol_table::PushScope( xsc_scope* pScope )
{
    ASSERT( pScope );

    // Push scope onto stack
    m_ScopeStack.Append() = pScope;
}

//==============================================================================
//  PopScope
//==============================================================================

xsc_scope* xsc_symbol_table::PopScope( void )
{
    ASSERT( m_ScopeStack.GetCount() > 0 );

    // Pop off the stack
    xsc_scope* pScope = m_ScopeStack[m_ScopeStack.GetCount()-1];
    m_ScopeStack.Delete( m_ScopeStack.GetCount()-1 );

    // Return scope
    return pScope;
}

//==============================================================================
//  AddSymbol
//==============================================================================

xsc_symbol* xsc_symbol_table::AddSymbol( const xwstring&    Name,
                                         const xsc_token*   pToken,
                                         symbol_type        SymbolType,
                                         xsc_symbol*        pType )
{
    ASSERT( m_ScopeStack.GetCount() > 0 );

    // Add symbol to last scope on stack
    xsc_symbol* pSymbol = m_ScopeStack[m_ScopeStack.GetCount()-1]->AddSymbol( Name, pToken, SymbolType, pType );

    // Return symbol
    return pSymbol;
}

//==============================================================================
//  GetSymbol
//==============================================================================

xsc_symbol* xsc_symbol_table::GetSymbol( const xwstring& Name, xsc_scope* pScope ) const
{
    xsc_symbol* pSymbol = NULL;

    // Is this a search in a specific scope?
    if( pScope != NULL )
    {
        // Specific scoped search
        pSymbol = pScope->GetSymbol( Name );
    }
    else
    {
        // Regular stacked scope search
        s32 i = m_ScopeStack.GetCount()-1;
        while( (i >= 0) && (pSymbol == NULL) )
        {
            // Search current scope for symbol and move to next scope
            pSymbol = m_ScopeStack[i]->GetSymbol( Name );
            i--;
        }
    }

    // Return symbol
    return pSymbol;
}

//==============================================================================
//  GetCurrentScope
//==============================================================================

xsc_scope* xsc_symbol_table::GetCurrentScope( void ) const
{
    ASSERT( m_ScopeStack.GetCount() > 0 );
    return m_ScopeStack[m_ScopeStack.GetCount()-1];
}

//==============================================================================
//  Save
//==============================================================================

xbool xsc_symbol_table::Save( const char* pFileName )
{
    xbool       Success = TRUE;
    xmemfile    MemFile;
    s32         i;
    s32         j;

#ifdef X_DEBUG
	xsc_map		GUIDS;
	GUIDS.Add( 0, 0 );
#endif

    // Write symbol scopes
    MemFile += (s32)m_Scopes.GetCount();
    for( i=0 ; i<m_Scopes.GetCount() ; i++ )
    {
        xsc_scope* pScope = m_Scopes[i];
        ASSERT( pScope );

        // Write scope data
        MemFile += (s32)pScope;
        MemFile += (s32)pScope->m_Flags;
        MemFile += (s32)pScope->m_pOwningSymbol;
        MemFile += (s32)pScope->m_pSuperScope;
        MemFile += (s32)pScope->m_StorageSize;

#ifdef X_DEBUG
		GUIDS.Add( (s32)pScope, 1 );
		ASSERT( GUIDS.FindByKey( (s32)pScope->m_pOwningSymbol ) );
		ASSERT( GUIDS.FindByKey( (s32)pScope->m_pSuperScope   ) );
#endif
        // Write Symbols
        MemFile += (s32)pScope->GetNumSymbols();
        for( j=0 ; j<pScope->GetNumSymbols() ; j++ )
        {
            // Get symbol pointer
            xsc_symbol* pSymbol = pScope->GetSymbol( j );
            ASSERT( pSymbol );

            // Write symbol data
            MemFile += (s32)pSymbol;
            MemFile +=      pSymbol->Name;
            MemFile += (s32)pSymbol->SymbolType;
            MemFile += (s32)pSymbol->pParentScope;
            MemFile += (s32)pSymbol->pChildScope;
            MemFile += (s32)pSymbol->Type.pType;
            MemFile +=      pSymbol->Type.IsReference;
            MemFile += (s32)pSymbol->pSuperClass;
            MemFile +=      pSymbol->Signature;
            MemFile += (s32)pSymbol->Visibility;
            MemFile += (s32)pSymbol->Flags;
            MemFile += (s32)pSymbol->enumValue;
            MemFile += (s32)pSymbol->StorageOffset;
            MemFile += (s32)pSymbol->StorageSize;

#ifdef X_DEBUG
			GUIDS.Add( (s32)pSymbol, 1 );
			ASSERT( GUIDS.FindByKey( (s32)pSymbol->pParentScope ) );
//			ASSERT( GUIDS.FindByKey( (s32)pSymbol->pChildScope  ) );
			ASSERT( GUIDS.FindByKey( (s32)pSymbol->pSuperClass  ) );
#endif
        }
    }

    // Save file
    Success &= MemFile.SaveFile( pFileName );

    return Success;
}

//==============================================================================
//  Load
//==============================================================================

xbool xsc_symbol_table::Load( const char* pFileName )
{
    xbool       Success = TRUE;
    xmemfile    MemFile;
    s32         i;
    s32         j;

    // Load the file
    Success = MemFile.LoadFile( pFileName );
    if( Success )
    {
        xarray<xsc_scope*>  Scopes;
        xarray<xsc_symbol*> Symbols;
        xsc_map             ScopeMap;
        xsc_map             SymbolMap;

        // Add NULL pointer into both maps
        ScopeMap.Add( 0, 0 );
        SymbolMap.Add( 0, 0 );

        // Read scopes
        s32 NumScopes = MemFile.Read_s32();
        for( i=0 ; i<NumScopes ; i++ )
        {
            // Create scope
            xsc_scope* pScope = new xsc_scope;
            Scopes.Append() = pScope;

            // Read scope data
            pScope->m_GUID          =               MemFile.Read_s32();
            pScope->m_Flags         = (u32)         MemFile.Read_s32() | SCOPE_IMPORTED;
            pScope->m_pOwningSymbol = (xsc_symbol*) MemFile.Read_s32();
            pScope->m_pSuperScope   = (xsc_scope*)  MemFile.Read_s32();
            pScope->m_StorageSize   = (s32)         MemFile.Read_s32();

            ScopeMap.Add( pScope->m_GUID, (s32)pScope );

            // Read symbols
            s32 NumSymbols = MemFile.Read_s32();
            for( j=0 ; j<NumSymbols ; j++ )
            {
                // Create symbol
                xsc_symbol* pSymbol = new xsc_symbol;
                Symbols.Append( pSymbol );

                // Read symbol data
                pSymbol->GUID             =                     MemFile.Read_s32();
                pSymbol->Name             =                     MemFile.Read_xwstring();
                pSymbol->SymbolType       = (symbol_type)       MemFile.Read_s32();
                pSymbol->pParentScope     = (xsc_scope*)        MemFile.Read_s32();
                pSymbol->pChildScope      = (xsc_scope*)        MemFile.Read_s32();
                pSymbol->Type.pType       = (xsc_symbol*)       MemFile.Read_s32();
                pSymbol->Type.IsReference = (xbool)             MemFile.Read_s32();
                pSymbol->pSuperClass      = (xsc_symbol*)       MemFile.Read_s32();
                pSymbol->Signature        =                     MemFile.Read_xwstring();
                pSymbol->Visibility       = (symbol_visibility) MemFile.Read_s32();
                pSymbol->Flags            =                     MemFile.Read_s32() | SYM_IMPORTED;
                pSymbol->enumValue        =                     MemFile.Read_s32();
                pSymbol->StorageOffset    =                     MemFile.Read_s32();
                pSymbol->StorageSize      =                     MemFile.Read_s32();

//                pScope->AddSymbol( pSymbol );
                SymbolMap.Add( pSymbol->GUID, (s32)pSymbol );
            }
        }

/*
        // Remap any type symbols to the real types
        for( i=0 ; i<Symbols.GetCount() ; i++ )
        {
            xsc_symbol* pSymbol = Symbols[i];
            ASSERT( pSymbol );
            if( pSymbol->SymbolType == symtype_type )
            {
                // Change entry in map
                xsc_symbol* pTypeSymbol = m_Scopes[0]->GetSymbol( pSymbol->Name );
                ASSERT( pTypeSymbol );
                xsc_map::entry* pEntry = SymbolMap.FindByValue( (s32)pSymbol );
                ASSERT( pEntry );
                pEntry->Value = (s32)pTypeSymbol;

                // Remove this symbol from the list of symbols
                Symbols[i] = 0;
            }
        }
*/

        // Remap all the scopes
        for( i=0 ; i<Scopes.GetCount() ; i++ )
        {
            xsc_scope* pScope = Scopes[i];
            ASSERT( pScope );

            // Remap pointers
            pScope->m_pOwningSymbol = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pScope->m_pOwningSymbol );
            pScope->m_pSuperScope   = (xsc_scope*) ScopeMap .ValueFromKey( (s32)pScope->m_pSuperScope   );

            // Add Scope into Symbol Table
            AddScope( pScope );
        }

        // Remap all the symbols
        for( i=0 ; i<Symbols.GetCount() ; i++ )
        {
            xsc_symbol* pSymbol = Symbols[i];

            // Symbol could be NULL so ignore those
            if( pSymbol )
            {
                // Remap pointers
                pSymbol->pParentScope = (xsc_scope* )ScopeMap .ValueFromKey( (s32)pSymbol->pParentScope );
                pSymbol->pChildScope  = (xsc_scope* )ScopeMap .ValueFromKey( (s32)pSymbol->pChildScope  );
                pSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                pSymbol->pSuperClass  = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->pSuperClass  );

                // Add into scope
                pSymbol->pParentScope->AddSymbol( pSymbol );
            }
        }
    }

    return Success;
}

//==============================================================================
//  Import
//==============================================================================

xbool xsc_symbol_table::Import( const xsc_symbol_table& SymbolTable )
{
    xbool       Success = TRUE;
    s32         i;
    s32         j;
    s32         k;

    xsc_map     ScopeMap;
    xsc_map     SymbolMap;

    // Symbol table we are importing to must have a global scope at least
    ASSERT( m_Scopes.GetCount() > 0 );
    ASSERT( SymbolTable.m_Scopes.GetCount() > 0 );

    // Setup initial mappings
    SymbolMap.Add( 0, 0 );
    ScopeMap.Add( 0, 0 );
    ScopeMap.Add( (s32)SymbolTable.m_Scopes[0], (s32)m_Scopes[0] );

    // Import
    xsc_scope* pScope = SymbolTable.m_Scopes[0];
    ASSERT( pScope );
    for( i=0 ; i<pScope->GetNumSymbols() ; i++ )
    {
        xsc_symbol* pSymbol = pScope->GetSymbol( i );
        ASSERT( pSymbol );

        // Check for type
        if( pSymbol->SymbolType == symtype_type )
        {
            xsc_symbol* pType = m_Scopes[0]->GetSymbol( pSymbol->Name );
            ASSERT( pType );
            SymbolMap.Add( (s32)pSymbol, (s32)pType );
        }

        // Check for typedef
        else if( pSymbol->SymbolType == symtype_typedef )
        {
            // Check for duplicate
            xsc_symbol* pExistingSymbol = m_Scopes[0]->GetSymbol( pSymbol->Name );
            if( pExistingSymbol )
            {
                // Remap the typedef to the existing one
                SymbolMap.Add( (s32)pSymbol, (s32)pExistingSymbol );
            }
            else
            {
                // Import the symbol
                xsc_symbol* pNewSymbol = new xsc_symbol;
                ASSERT( pNewSymbol );
                *pNewSymbol = *pSymbol;
                pNewSymbol->Flags |= SYM_IMPORTED;
                pNewSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                pNewSymbol->pParentScope = (xsc_scope* )ScopeMap .ValueFromKey( (s32)pSymbol->pParentScope );
                m_Scopes[0]->AddSymbol( pNewSymbol );

                SymbolMap.Add( (s32)pSymbol, (s32)pNewSymbol );
            }
        }

        // Check for class
        else if( pSymbol->SymbolType == symtype_class )
        {
            // Check for duplicate
            xsc_symbol* pExistingSymbol = m_Scopes[0]->GetSymbol( pSymbol->Name );
            if( pExistingSymbol )
            {
                // Remap the class to the existing one
                SymbolMap.Add( (s32)pSymbol, (s32)pExistingSymbol );
                ScopeMap. Add( (s32)pSymbol->pChildScope, (s32)pExistingSymbol->pChildScope );
            }
            else
            {
                // Create class symbol
                xsc_symbol* pNewSymbol = new xsc_symbol;
                ASSERT( pNewSymbol );
                *pNewSymbol = *pSymbol;
                pNewSymbol->Flags |= SYM_IMPORTED;
                pNewSymbol->pParentScope = (xsc_scope* )ScopeMap .ValueFromKey( (s32)pSymbol->pParentScope );
                pNewSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                pNewSymbol->pSuperClass  = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->pSuperClass  );
                m_Scopes[0]->AddSymbol( pNewSymbol );

                SymbolMap.Add( (s32)pSymbol, (s32)pNewSymbol );

                // Create class scope
                xsc_scope* pClassScope = pSymbol->pChildScope;
                xsc_scope* pNewClassScope = NewScope( pNewSymbol );
                pNewSymbol->pChildScope = pNewClassScope;
                pNewClassScope->m_Flags = pClassScope->m_Flags;
                pNewClassScope->m_StorageSize = pClassScope->m_StorageSize;
                pNewClassScope->m_pSuperScope   = (xsc_scope*) ScopeMap .ValueFromKey( (s32)pClassScope->m_pSuperScope   );

                ScopeMap.Add( (s32)pSymbol->pChildScope, (s32)pClassScope );

                // Loop through class scope
                for( j=0 ; j<pClassScope->GetNumSymbols() ; j++ )
                {
                    xsc_symbol* pSymbol = pClassScope->GetSymbol( j );
                    ASSERT( pSymbol );

                    // Is it a field?
                    if( pSymbol->SymbolType == symtype_field )
                    {
                        xsc_symbol* pNewSymbol = new xsc_symbol;
                        ASSERT( pNewSymbol );
                        *pNewSymbol = *pSymbol;
                        pNewSymbol->Flags |= SYM_IMPORTED;
                        pNewSymbol->pParentScope = pNewClassScope;
                        pNewSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                        pNewClassScope->AddSymbol( pNewSymbol );
                    }

                    // Is it a method?
                    else if( pSymbol->SymbolType == symtype_method )
                    {
                        xsc_symbol* pNewSymbol = new xsc_symbol;
                        ASSERT( pNewSymbol );
                        *pNewSymbol = *pSymbol;
                        pNewSymbol->Flags |= SYM_IMPORTED;
                        pNewSymbol->pParentScope = pNewClassScope;
                        pNewSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                        pNewClassScope->AddSymbol( pNewSymbol );

                        xsc_scope* pMethodScope = pSymbol->pChildScope;
                        xsc_scope* pNewMethodScope = NewScope( pNewSymbol );
                        pNewSymbol->pChildScope = pNewMethodScope;
                        pNewMethodScope->m_Flags = pMethodScope->m_Flags;
                        pNewMethodScope->m_StorageSize = pMethodScope->m_StorageSize;

                        // Loop through method scope
                        for( k=0 ; k<pMethodScope->GetNumSymbols() ; k++ )
                        {
                            xsc_symbol* pSymbol = pMethodScope->GetSymbol( k );
                            ASSERT( pSymbol );

                            // Is it an argument
                            if( pSymbol->SymbolType == symtype_argument )
                            {
                                xsc_symbol* pNewSymbol = new xsc_symbol;
                                ASSERT( pNewSymbol );
                                *pNewSymbol = *pSymbol;
                                pNewSymbol->Flags |= SYM_IMPORTED;
                                pNewSymbol->pParentScope = pNewMethodScope;
                                pNewSymbol->Type.pType   = (xsc_symbol*)SymbolMap.ValueFromKey( (s32)pSymbol->Type.pType   );
                                pNewMethodScope->AddSymbol( pNewSymbol );
                            }

                            // Must be illegal
                            else
                            {
                                ASSERT( 0 );
                            }
                        }
                    }

                    // Must be illegal
                    else
                    {
                        ASSERT( 0 );
                    }
                }
            }
        }

        // Must be illegal
        else
        {
            ASSERT( 0 );
        }
    }

    return Success;
}

//==============================================================================
//  Dump
//==============================================================================

xstring xsc_symbol_table::Dump( void ) const
{
    s32     i;
    xstring Output;

    Output = "Symbol Table\n"
             "------------\n";

    // Dump all scopes
    for( i=0 ; i<m_Scopes.GetCount() ; i++ )
    {
        Output.AddFormat( "Scope %3d (%3d)\n"
                          "---------------\n", i, m_Scopes[i]->GetStorageSize() );
        if( m_Scopes[i]->GetOwningSymbol() )
            Output.AddFormat( "Owning Symbol = '%ls'\n", m_Scopes[i]->GetOwningSymbol()->Name );
        Output += m_Scopes[i]->Dump( FALSE );
        Output += "\n";
    }

    // Return dump
    return Output;
}

//==============================================================================
