//==============================================================================
//
//  xsc_parse_compound_statement
//
//==============================================================================

#include "xsc_parser.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  ParseCompoundStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseCompoundStatement( xbool NewScope )
{
    // Expect start of compound statement
    Expect( T_LBRACE, err_syntax, L"Expecting '{'" );

    // Create node for compound statement
    xsc_ast_node* pCompoundNode = m_AST.NewNode( ast_compound_statement );

    // Push a new symbol scope onto stack
    if( NewScope )
    {
        // Get storage position from previous scope
        s32 StorageBase = 0;
        xsc_scope*   pCurrentScope = m_SymbolTable.GetCurrentScope();
        if( pCurrentScope->GetOwningSymbol() == NULL )
            StorageBase = pCurrentScope->GetStorageSize();

        // Make new scope
        xsc_scope* pCompoundScope = m_SymbolTable.NewScope( );
        m_SymbolTable.PushScope( pCompoundScope );
        pCompoundNode->pScope = pCompoundScope;
        pCompoundScope->SetStorageSize( StorageBase );
    }

    // Parse the scope
    s32 PrevIndex = m_Index-1;
    while( !IsEOF(m_t) && (m_t->Code != T_RBRACE) && (m_Index != PrevIndex) )
    {
        // Set previous index so we can detect no progress conditions
        PrevIndex = m_Index;

        // Parse a statement & add to AST
        xsc_ast_node* pStatementNode = ParseStatement();
        if( pStatementNode )
            pCompoundNode->Children.Append() = pStatementNode;
    }

    // Read past the "}"
    Expect( T_RBRACE, err_syntax, L"Expecting '}'" );

    // Pop symbol scope from stack
    if( NewScope )
    {
        m_SymbolTable.PopScope( );
    }

    // Return compound node
    return pCompoundNode;
}

//==============================================================================
