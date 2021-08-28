//=============================================================================
//
// Expression.h
//
// Copyright (c) 2002 Max McGuire
//
// Created by Max McGuire (mmcguire@ironlore.com)
//
//=============================================================================

#ifndef EXPRESSION_H
#define EXPRESSION_H

#pragma warning (disable : 4786)
#pragma warning (disable : 4305) // warning C4305: 'argument' : truncation from 'const int' to 'char'
#pragma warning (disable : 4309) // warning C4309: 'argument' : truncation of constant value

#include <vector>
#include <map>
#include <string>


/**
 *
 * Expression evaluating class.
 *
 * The class supports binary addition, subtraction, multiplication, division,
 * unary minus, parenthesis, abs(), sin(), cos(), sqrt(), pi and user supplied
 * C functions.
 *
 * The one caveat is that because of the way the compiler uses the processor's
 * floating point stack, expressions are limited in their complexity (otherwise
 * they will overflow the stack).  In practice this doesn't seem to be a problem
 * for the types of expressions you would normally want to use with this class.
 * 
 */

class Expression
{
    
public:

    typedef float (__cdecl *UserFunction)(float);

    /**
     * Constructor.
     */
    
    Expression();

    /**
     *
     * Initializes the class with an infix expression. The expression must be
     * initialized before it can be evaluated.
     * 
     * @param expression An infix expression.
     * @param variableName An array of variable names which can appear in the
     * expression.
     * @param numVariables The number of elements in the variable array.
     * @param function An array of C function pointers which are callable from
     * the expression.
     * @param function An array of user supplied functions which can be called
     * from inside the expression.
     * @param functionName An array of user supplied function names which can
     * appear in the expression.
     * @param numFunctions The number of elements in the function and
     * functionName arrays.
     *
     * @return Returns true if the expression was initialized successfully
     * or false if otherwise. Possible reasons for failure are syntax errors
     * or floating point stack overflow.
     *
     */
    
    bool Initialize(const char* expression, const char* variableName[], int numVariables,
        UserFunction function[], const char* functionName[], int numFunctions);

    /**
     *
     * Evaluates the expression with a set of variable substitutions and
     * returns the result.
     *
     * @param variable An array of values to substitute for the variables the
     * expression can contain. This array should have the same number of
     * elements as the array passed into the Initialize() method.
     *
     */
    
    float Evaluate(const float variable[]) const;

    /**
     * Destructor.
     */
    
    virtual ~Expression();

private:

    class Token
    {

    public:
    
        enum Type
        {
            typeFunction,
            typeValue,
            typeVariable,
            typeUserFunction,
        };
    
        enum Function
        {
            
            functionAdd      = 0,
            functionSub      = 1,
            functionMul      = 2,
            functionDiv      = 3,
            functionUnarySub = 4,
            functionAbs      = 5,
            functionSin      = 6,
            functionCos      = 7,
            functionSqrt     = 8,
            functionPi       = 9,
            
            // Only used during parsing.
            
            functionLParen,
            functionRParen,
        
        };

        Token();
        Token(float value);
        Token(Function function);
        Token(UserFunction userFunction);
        Token(int variableIndex);

    public:

        Type type;

        union
        {

            Function function;

            UserFunction userFunction;

            float value;    
    
            int variableIndex;

        };

    };

    /**
     * Disallow copying.
     */
    
    Expression(const Expression& expression);

    /**
     * Disallow copying.
     */
    
    Expression& operator =(const Expression& expression);

    /**
     * Tokenizes an infix expression and converts it to postfix.
     */

    bool Parse(const char* expression, std::vector<Token>& postfixTokens, const char* variableName[], int numVariables,
        UserFunction function[], const char* functionName[], int numFunctions);
    
    /**
     * Compiles a postfix expression.
     */

    bool Compile(const std::vector<Token>& postfixTokens);

private:

    /// The compiled expression code.

    void* compiledCode;

};

#endif