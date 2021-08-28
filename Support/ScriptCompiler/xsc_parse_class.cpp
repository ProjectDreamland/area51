//==============================================================================
//
//  xsc_parse_class
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
//  ParseClass
//==============================================================================

xsc_ast_node* xsc_parser::ParseClass( void )
{
    const xsc_token*    pClassToken = m_t;

    ReadToken();

    // Check for class name identifier
    if( m_t->Code == T_IDENTIFIER )
    {
        // Add class name to symbol table
        m_pClassSymbol = m_SymbolTable.AddSymbol( m_t->ValStr, m_t, symtype_class );
        if( m_pClassSymbol )
        {
            // Setup class symbol & activate new scope
            m_pClassSymbol->SymbolType  = symtype_class;
            m_pClassSymbol->pChildScope = m_SymbolTable.NewScope( m_pClassSymbol );
            m_SymbolTable.PushScope( m_pClassSymbol->pChildScope );

            ReadToken();

            // Is this a subclass?
            if( m_t->Code == T_COLON )
            {
                ReadToken();

                // Is this a possible class name?
                if( m_t->Code == T_IDENTIFIER )
                {
                    // Get symbol for superclass name
                    xwstring SuperClassName = m_Source.Mid( m_t->StartIndex, m_t->Length );
                    xsc_symbol* pSuperClassSymbol = m_SymbolTable.GetSymbol( SuperClassName );
                    ReadToken();

                    // Does it exist and is it a class?
                    if( pSuperClassSymbol && (pSuperClassSymbol->SymbolType == symtype_class) )
                    {
                        // Set class superclass pointer & super scope pointer, plus init bytesize of class
                        m_pClassSymbol->pSuperClass = pSuperClassSymbol;
                        m_pClassSymbol->pChildScope->SetSuperScope( pSuperClassSymbol->pChildScope );
                        m_pClassSymbol->pChildScope->SetStorageSize( pSuperClassSymbol->pChildScope->GetStorageSize() );
                        m_pClassSymbol->StorageSize = pSuperClassSymbol->StorageSize;
                    }
                    else
                    {
                        // Output error
                        m_Errors.Error( err_syntax, m_t, xfs("Undefined class '%ls'", (const xwchar*)SuperClassName) );
                    }
                }
                else
                {
                    // Expecting super class name
                    m_Errors.Error( err_syntax, m_t, xfs("Expecting a class name") );
                }
            }

            // Parse Body?
            if( m_t->Code == T_LBRACE )
            {
                ReadToken();

                // Create a node for this class definition
                m_pClassNode = m_AST.NewNode( ast_class );
                m_pClassNode->pToken  = pClassToken;
                m_pClassNode->pSymbol = m_pClassSymbol;

                s32 PrevIndex = m_Index-1;
                while( (m_t->Code != T_RBRACE ) && (PrevIndex != m_Index) )
                {
                    // Record previous index to detect no progress state
                    PrevIndex = m_Index;

                    // Parse a class member node
                    ParseClassMember();
                }

                Expect( T_RBRACE, err_syntax, L"Expecting '}'" );
            }
            else
            {
                // Class Declaration but not definition
                if( m_t->Code == T_SEMICOLON )
                {
                    // TODO: Just declare the class
                }
                else
                {
                    // Error, not { or ; after class definition
                    m_Errors.Error( err_syntax, m_t, "Expecting '{' or ';'" );
                }
            }

            // Pop the scope off the symbol tables stack
            m_SymbolTable.PopScope();

            // Fill in size of the class
            m_pClassSymbol->StorageSize = m_pClassSymbol->pChildScope->GetStorageSize();
        }
        else
        {
            // Identifer already defined
            m_Errors.Error( err_syntax, m_t, xfs("Symbol redefinition '%ls'", m_t->ValStr) );
        }
    }
    else
    {
        // Expecting class name
        m_Errors.Error( err_syntax, m_t, "Expecting a class name" );
    }

    // Return Class Node
    return m_pClassNode;
}

//==============================================================================

void xsc_parser::ParseClassMember( void )
{
    xbool       IsConst     = FALSE;
    xbool       IsStatic    = FALSE;
    xbool       IsVirtual   = FALSE;
    xbool       IsNative    = FALSE;
    xbool       IsReference = FALSE;
    xwstring    Signature;

    // Read any type qualifiers
    while( (m_t->Code == T_CONST) || (m_t->Code == T_STATIC) || (m_t->Code == T_VIRTUAL) || (m_t->Code == T_NATIVE) )
    {
        switch( m_t->Code )
        {
        case T_CONST:
            IsConst   = TRUE; break;
        case T_STATIC:
            IsStatic  = TRUE; break;
        case T_VIRTUAL:
            IsVirtual = TRUE; break;
        case T_NATIVE:
            IsNative  = TRUE; break;
        }

        // Read next token
        ReadToken();
    }

    // Save Type Token
    const xsc_token* pTypeToken = m_t;
    ReadToken();

    // Check for reference type
    if( m_t->Code == T_AND_OP )
    {
        IsReference = TRUE;
        ReadToken();
    }

    // Save Name Token
    const xsc_token* pNameToken = m_t;

    // Does it look valid? eg. the next 2 tokens are identifiers
    if( (pTypeToken->Code == T_IDENTIFIER) && (pNameToken->Code == T_IDENTIFIER) )
    {
        // Method Definition?
        if( LookAhead(0)->Code == T_LPAREN )
        {
            // Skip Name & LPAREN
            ReadToken();
            ReadToken();

            // Locate the type symbol & add into signature
            xsc_symbol* pTypeSymbol = GetTypeFromToken( pTypeToken );
            if( pTypeSymbol == NULL )
            {
                // Symbol not found or not of the correct type
                m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
                pTypeSymbol = g_pTs32;
            }
            typeref MethodReturnType( pTypeSymbol, IsReference );

            Signature += GetSignatureFromType( MethodReturnType );

            // Create symbol for method
            m_pMethodSymbol = m_SymbolTable.AddSymbol( pNameToken->ValStr, pNameToken, symtype_method, pTypeSymbol );
            ASSERT( m_pMethodSymbol );
            m_pMethodSymbol->SetMember ( TRUE );
            m_pMethodSymbol->SetConst  ( IsConst   );
            m_pMethodSymbol->SetStatic ( IsStatic  );
            m_pMethodSymbol->SetVirtual( IsVirtual );
            m_pMethodSymbol->SetNative ( IsNative  );
            m_pMethodSymbol->Type        = MethodReturnType;
            m_pMethodSymbol->StorageSize = m_pMethodSymbol->Type.GetByteSize();

            // Create ast node for method
            xsc_ast_node* pMethodNode = m_AST.NewNode( ast_class_methoddef );
            pMethodNode->pSymbol = m_pMethodSymbol;
            pMethodNode->pToken  = pNameToken;
            pMethodNode->Type    = m_pMethodSymbol->Type;

            // Add Method node to Class
            m_pClassNode->Children.Append() = pMethodNode;

            // Create scope for Method
            m_pMethodScope = m_SymbolTable.NewScope( m_pMethodSymbol );
            m_pMethodSymbol->pChildScope = m_pMethodScope;
            m_SymbolTable.PushScope( m_pMethodScope );

            // Setup argument storage offset counter
            s32 ArgumentStorageOffset = 0;

            // Process implicit this for non-static methods
            if( !IsStatic )
            {
                xsc_symbol* pThisSymbol    = m_pMethodScope->AddSymbol( xwstring("this"), 0, symtype_argument, m_pClassSymbol );
                pThisSymbol->Type          = typeref(m_pClassSymbol,TRUE);
                pThisSymbol->StorageOffset = 0;
                pThisSymbol->StorageSize   = 4;
                ArgumentStorageOffset     += pThisSymbol->StorageSize;
            }

            // Process arguments, look for special case of a 'void' argument list
            if( (m_t->Code == T_IDENTIFIER) && (GetTypeFromToken( m_t ) == g_pTvoid) && (LookAhead()->Code == T_RPAREN) )
            {
                // Special case of a void argument list
                ReadToken();
            }
            else
            {
                // Keep going as long as it looks like an argument
                while( m_t->Code == T_IDENTIFIER )
                {
                    // Save type and name tokens
                    xbool IsReference = FALSE;
                    const xsc_token* pTypeToken = m_t;
                    ReadToken();
                    if( m_t->Code == T_AND_OP )
                    {
                        IsReference = TRUE;
                        ReadToken();
                    }
                    const xsc_token* pNameToken = m_t;
                    ReadToken();

                    // Make sure name is an identifier
                    if( pNameToken->Code == T_IDENTIFIER )
                    {
                        // Locate the type symbol
                        xsc_symbol* pTypeSymbol = GetTypeFromToken( pTypeToken );
                        if( pTypeSymbol == NULL )
                        {
                            // Symbol not found or not of the correct type
                            m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
                            pTypeSymbol = g_pTs32;
                        }
                        if( pTypeSymbol == g_pTvoid )
                        {
                            // Error - void argument
                            m_Errors.Error( err_syntax, pTypeToken, "Arguments cannot have type 'void'" );
                        }
                        else
                        {
                            // Create symbol for argument
                            xsc_symbol* pArgumentSymbol  = m_SymbolTable.AddSymbol( pNameToken->ValStr, pNameToken, symtype_argument, pTypeSymbol );
                            if( pArgumentSymbol )
                            {
                                // Set symbol size and position in class, plus increase size for class
                                pArgumentSymbol->Type          = typeref(pTypeSymbol,IsReference);
                                pArgumentSymbol->StorageOffset = ArgumentStorageOffset;
                                pArgumentSymbol->StorageSize   = pArgumentSymbol->Type.GetByteSize();
                                ArgumentStorageOffset         += pArgumentSymbol->StorageSize;
                            }
                            else
                            {
                                // Error symbol already defined
                                m_Errors.Error( err_syntax, pNameToken, xfs("Symbol redefinition '%ls'", pNameToken->ValStr) );
                            }

                            // Add argument type into signature
                            Signature += GetSignatureFromType( pArgumentSymbol->Type );
                        }
                    }
                    else
                    {
                        // Error
                        m_Errors.Error( err_syntax, pNameToken, "Expecting argument name" );
                    }

                    // Skip comma argument separator
                    if( m_t->Code == T_COMMA_OP )
                    {
                        ReadToken();
                    }
                }
            }

            // Expect a closing right paren
            Expect( T_RPAREN, err_syntax, L"Expecting ')'" );

            // Set size in scope and symbol of method
            m_pMethodScope->SetStorageSize( ArgumentStorageOffset );
            m_pMethodSymbol->StorageOffset = ArgumentStorageOffset;

            // Parse the method body
            if( m_t->Code == T_LBRACE )
            {
                // Parse method body
                xsc_ast_node* pMethodBodyNode = ParseCompoundStatement( TRUE );
                pMethodNode->Children.Append() = pMethodBodyNode;

                // The method had been defined
                m_pMethodSymbol->SetDefined( TRUE );
            }
            else if( m_t->Code == T_SEMICOLON )
            {
                // Just a declaration, not a definition
                ReadToken();
            }
            else
            {
                // Error
                m_Errors.Error( err_syntax, m_t, "Expecting '{' or ';'" );
            }

            // Save signature
            m_pMethodSymbol->Signature = Signature;

            // Pop Method scope off stack
            m_SymbolTable.PopScope();

            // Clear global Method Symbol pointer
            m_pMethodSymbol = NULL;
        }
        else
        {
            // Must be a field defintion

            // Locate the type symbol & add into signature
            xsc_symbol* pTypeSymbol = GetTypeFromToken( pTypeToken );
            if( pTypeSymbol == NULL )
            {
                // Symbol not found or not of the correct type
                m_Errors.Error( err_syntax, pTypeToken, xfs("Undefined type '%ls'", pTypeToken->ValStr) );
                pTypeSymbol = g_pTs32;
            }
            Signature += GetSignatureFromType( typeref(pTypeSymbol,IsReference) );

            // Check for a class used within the same class
            if( !IsReference && (pTypeSymbol == m_pClassSymbol) )
            {
                m_Errors.Error( err_syntax, m_t, xfs("Class '%ls' has recursive field definition", pTypeSymbol->Name) );
            }

            // Field Definition
            do
            {
                // Skip Comma
                if( m_t->Code == T_COMMA_OP )
                    ReadToken();

                // Is this an identifier?
                if( m_t->Code == T_IDENTIFIER )
                {
                    const xsc_token* pSymbolToken = m_t;
                    ReadToken();

                    // Create new symbol
                    xsc_symbol* pFieldSymbol = m_SymbolTable.AddSymbol( pSymbolToken->ValStr, pSymbolToken, symtype_field, pTypeSymbol );
                    if( pFieldSymbol )
                    {
                        // Set Qualifiers
                        pFieldSymbol->SetMember ( TRUE );
                        pFieldSymbol->SetConst  ( IsConst   );
                        pFieldSymbol->SetStatic ( IsStatic  );
                        pFieldSymbol->SetVirtual( IsVirtual );
                        pFieldSymbol->SetNative ( IsNative  );

                        // Set Type
                        pFieldSymbol->Type.IsReference = IsReference;

                        // TODO: Deal with Static symbols allocating from different storage
                        // Set symbol size and position in class, plus increase size for class
                        xsc_scope* pClassScope      = m_pClassSymbol->pChildScope;
                        pFieldSymbol->Signature     = Signature;
                        pFieldSymbol->StorageSize   = pTypeSymbol->StorageSize;
                        pFieldSymbol->StorageOffset = m_pClassSymbol->pChildScope->GetStorageSize();
                        pClassScope->SetStorageSize( pClassScope->GetStorageSize() + pFieldSymbol->StorageSize );

                        // Create node & Add to Class
                        xsc_ast_node* pFieldNode = m_AST.NewNode( ast_class_fielddef );
                        pFieldNode->Type    = pFieldSymbol->Type;
                        pFieldNode->pSymbol = pFieldSymbol;
                        pFieldNode->pToken  = pSymbolToken;
                        m_pClassNode->Children.Append() = pFieldNode;
                    }
                    else
                    {
                        // Error symbol already defined
                        m_Errors.Error( err_syntax, pSymbolToken, xfs("Symbol redefinition '%s'", pSymbolToken->ValStr) );
                    }
                }
                else
                {
                    // Report error
                    m_Errors.Error( err_syntax, m_t, "Expecting a field identifier" );
                }
            } while( m_t->Code == T_COMMA_OP );

            Expect( T_SEMICOLON, err_syntax, L"Expecting ';'" );
        }
    }
}

//==============================================================================
