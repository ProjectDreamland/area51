//==============================================================================
//
//  xsc_parse_statement
//
//==============================================================================

#include "xsc_parser.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "xsc_compiler.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  ParseStatement
//==============================================================================

xsc_ast_node* xsc_parser::ParseStatement( void )
{
    // Parse the statement
    switch( m_t->Code )
    {
    case T_LBRACE:
        return ParseCompoundStatement();
        break;
    case T_IF:
        return ParseIfStatement();
        break;
    case T_WHILE:
        return ParseWhileStatement();
        break;
    case T_FOR:
        return ParseForStatement();
        break;
    case T_CONTINUE:
        return ParseContinueStatement();
        break;
    case T_RETURN:
        return ParseReturnStatement();
    case T_SEMICOLON:
        {
            ReadToken();
            // Empty statement
            // TODO: Check for suspicious use of empty statements
            return m_AST.NewNode( ast_empty_statement );
        }
    case T_IDENTIFIER:
        {
            // If token is a type then it must be a variable definition?
            if( TokenIsType( m_t ) )
            {
                // Group node
                xsc_ast_node* pVarDefsNode = m_AST.NewNode( ast_var_defs );

                // Save Type Token & Symbol
                const xsc_token* pTypeToken  = m_t;
                xsc_symbol*      pType       = GetTypeFromToken( pTypeToken );
                if( pType == NULL )
                {
                    // Symbol not found or not of the correct type
                    m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
                    pType = g_pTs32;
                }
                xbool IsReference = FALSE;
                if( LookAhead()->Code == T_AND_OP )
                {
                    IsReference = TRUE;
                    ReadToken();
                }

                // Variable Declaration
                do
                {
                    // Skip Type or Comma
                    ReadToken();

                    // Is this an identifier?
                    if( m_t->Code == T_IDENTIFIER )
                    {
                        const xsc_token* pSymbolToken = m_t;
                        ReadToken();

                        // Create new symbol
                        xsc_symbol* pSymbol = m_SymbolTable.AddSymbol( pSymbolToken->ValStr, pSymbolToken, symtype_local, pType );
                        if( pSymbol )
                        {
                            // Setup symbol
                            xsc_scope* pScope = m_SymbolTable.GetCurrentScope();
                            ASSERT( pScope );
                            pSymbol->Type          = typeref(pType,IsReference);
                            pSymbol->StorageOffset = pScope->GetStorageSize();
                            pSymbol->StorageSize   = pSymbol->Type.GetByteSize();
                            pScope->SetStorageSize( pSymbol->StorageOffset + pSymbol->StorageSize );

                            // Create node & Add to var_defs
                            xsc_ast_node* pNode = m_AST.NewNode( ast_var_def );
                            pNode->Type    = pSymbol->Type;
                            pNode->pSymbol = pSymbol;
                            pNode->pToken  = pSymbolToken;
                            pVarDefsNode->Children.Append() = pNode;

                            // Process initializer
                            // TODO: Handle array initializers
                            if( m_t->Code == T_ASSIGN )
                            {
                                UnreadToken( 2 );
                                xsc_ast_node* pExpressionNode = ParseExpression( TRUE );
                                pNode->Children.Append() = pExpressionNode;
                            }

                            // TODO: Process constructor
                            //       Parse argument list building signature and match to available constructors
                            else if( m_t->Code == T_LPAREN )
                            {
                            }

                        }
                        else
                        {
                            // Error symbol already defined
                            m_Errors.Error( err_syntax, pSymbolToken, xfs("Symbol redefinition '%ls'", pSymbolToken->ValStr) );
                        }

                        // Make sure reference vars are initialized
                        if( pSymbol->Type.IsReference && !pSymbol->IsInitialized() )
                        {
                            m_Errors.Error( err_syntax, pSymbolToken, xfs("Reference type '%ls' must be initialized", pSymbolToken->ValStr) );
                        }
                    }
                    else
                    {
                        // Report error
                        m_Errors.Error( err_syntax, m_t, "Expecting a field identifier" );
                    }
                } while( m_t->Code == T_COMMA_OP );

                // End of statement
                Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );
                return pVarDefsNode;
            }
        }
    default:
        {
            // Expression statement
            xsc_ast_node* pExpressionNode = ParseExpression( TRUE );
            Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );

            // Return the expression node
            return pExpressionNode;
        }
    }

    // No matches, return NULL
    return NULL;
}

//==============================================================================
