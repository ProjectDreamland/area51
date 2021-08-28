//=============================================================================
//
// Expression.cpp
//
// Copyright (c) 2002 Max McGuire
//
// Created by Max McGuire (mmcguire@ironlore.com)
//
//=============================================================================

#include "stdafx.h"
#include "Expression.h"

#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include <stack>
#include <string>
#include <strstream>
#include <map>


Expression::Token::Token()
{
}


Expression::Token::Token(float _value)
    : type(typeValue), value(_value)
{
}


Expression::Token::Token(Function _function)
    : type(typeFunction), function(_function) 
{
}


Expression::Token::Token(UserFunction _userFunction)
    : type(typeUserFunction), userFunction(_userFunction) 
{
}


Expression::Token::Token(int _variableIndex)
    : type(typeVariable), variableIndex(_variableIndex)
{
}


Expression::Expression()
    : compiledCode(NULL)
{
}


bool Expression::Initialize(const char* expression, const char* variableName[], int numVariables,
                            UserFunction function[], const char* functionName[], int numFunctions)
{
   
    std::vector<Token> postfixTokens;

    if (!Parse(expression, postfixTokens, variableName, numVariables, function, functionName, numFunctions))
    {
        return false;    
    }

    if (!Compile(postfixTokens))
    {
        return false;    
    }

    return true;

}


bool Expression::Parse(const char* infix, std::vector<Token>& postfixTokens, const char* variableName[], int numVariables, UserFunction function[], const char* functionName[], int numFunctions)
{
	try 
	{
		typedef std::map<std::string, int> VariableIndexMap;   
		typedef std::map<std::string, int> FunctionIndexMap;   

		VariableIndexMap variableIndexMap;
		FunctionIndexMap functionIndexMap;

		int i;
    
		for (i = 0; i < numVariables; ++i)
		{
			variableIndexMap.insert(VariableIndexMap::value_type(variableName[i], i));    
		}
    
		for (i = 0; i < numFunctions; ++i)
		{
			functionIndexMap.insert(FunctionIndexMap::value_type(functionName[i], i));    
		}

		// Modified from source code by Rex Jaeshke available at
		// http://www.programmersheaven.com/zone3/cat414/16136.htm

		std::stack<Token> stack;

		// Flag to keep track of whether or not the parser is ready for a unary
		// operator to occur in the token stream. This happens after a number
		// or a '(' or on the first token.

		bool readyForUnaryOperator = true;

		// Push a '(' on the stack. This sentinel allows us to detect when we
		// flush out the stack on completion.
    
		stack.push(Token::functionLParen);

		while (*infix != '\0')
		{

			if (isspace(*infix))
			{

				// Ignore white space.

    			++infix;

			}
			else if (*infix == '.' || isdigit(*infix))
			{

				// Parse a real number.

				float result = 0;

				while (isdigit(*infix))
				{
					result = result * 10 + (*infix) - '0';
					++infix;
				}

				if (*infix == '.')
				{

					++infix;

					float multiplier = 0.1f;

					do
					{
						result += ((*infix) - '0') * multiplier;
						multiplier *= 0.1f; 
						++infix;
					}
					while (isdigit(*infix));                
				}

                if( (*infix == 'e') || (*infix == 'E') )
                {
                    ++infix;

                    bool negative = false;

                    if( (*infix == '-') )
                    {
                        ++infix;
                        negative = true;
                    }
                    else if( (*infix == '+') )
                    {
                        ++infix;
                    }

                    int exponent = 0;
                    do
                    {
                        exponent *= 10;
                        exponent += (*infix -'0');
                        ++infix;
                    } while( isdigit(*infix) );

                    if( negative )
                        exponent = -exponent;

                    result *= pow( 10, exponent );
                }

				postfixTokens.push_back(Token(result));
				readyForUnaryOperator = false;

			}
			else if (isalpha(*infix))
			{

				int length = 0;

				// Parse an identifier.

				while (isalpha(infix[length]) || isdigit(infix[length]) || infix[length] == '_')
				{
					++length;
				}

				// Pop the operators of higher precedence (this is the special
				// case where there aren't any)

				readyForUnaryOperator = true;
            
				if (length == 3 && strncmp(infix, "abs", length) == 0)
				{
					stack.push(Token::functionAbs);
				}
				else if (length == 3 && strncmp(infix, "sin", length) == 0)
				{
					stack.push(Token::functionSin);
				}
				else if (length == 3 && strncmp(infix, "cos", length) == 0)
				{
					stack.push(Token::functionCos);
				}
				else if (length == 4 && strncmp(infix, "sqrt", length) == 0)
				{
					stack.push(Token::functionSqrt);
				}
				else if (length == 2 && strncmp(infix, "pi", length) == 0)
				{
					stack.push(Token::functionPi);
					readyForUnaryOperator = false;
				}
				else
				{

					std::string name(infix, infix + length);

					// Check if the token is a variable.

					VariableIndexMap::const_iterator iterator
						= variableIndexMap.find(name);

					if (iterator == variableIndexMap.end())
					{

						// Check if the token is a function.

						FunctionIndexMap::const_iterator iterator
							= functionIndexMap.find(name);

						if (iterator == functionIndexMap.end())
						{
							return false;
						}

						stack.push(Token(function[iterator->second]));
						readyForUnaryOperator = false;

					}
					else
					{
						postfixTokens.push_back(Token(iterator->second));
						readyForUnaryOperator = false;
					}
            
				}

				infix += length;

			}
			else if (*infix == '(')
			{

				// Push any '(' on the stack. These sentinels allows us to detect
				// when have flushed out the stack when handling ')' and operators.
			
				stack.push(Token::functionLParen);

				readyForUnaryOperator = true;
    			++infix;
        
			}
			else if (*infix == ')')
			{

				// Have a ')' so pop off the stack and put into postfix list until a
				// '(' is popped. Discard the '('.
				
				while (stack.top().type != Token::typeFunction ||
					   stack.top().function != Token::functionLParen)
				{
					postfixTokens.push_back(stack.top());
					stack.pop();
				}

				stack.pop();
            
				readyForUnaryOperator = false;
    			++infix;

			}
			else if (*infix == '*' || *infix == '/')
			{
            
				// Have a '*' or '/'. Pop off any operators of equal or higher
				// precedence and put them into postfix list. If a '(' or lower
				// precedence operator (such as '+' or '-') is popped, put it back and
				// stop looking. Push new '*' or '/'.
				
				while (stack.top().type != Token::typeFunction ||
					  (stack.top().function != Token::functionLParen &&
					   stack.top().function != Token::functionAdd    &&
					   stack.top().function != Token::functionSub))            
				{
					postfixTokens.push_back(Token(stack.top()));
					stack.pop();
				}
    
				if (*infix == '*')
				{
					stack.push(Token::functionMul);
				}
				else
				{
					stack.push(Token::functionDiv);
				}
            
				readyForUnaryOperator = true;

    			++infix;

			}
			else if (*infix == '+' || *infix == '-')
			{

				if (readyForUnaryOperator)
				{

					// Pop the operators of higher precedence (this is the special
					// case where there aren't any)

					if (*infix == '-')
					{
						stack.push(Token::functionUnarySub);
					}
                
				}
				else
				{

					// Have a '+' or '-'. Pop off any operators of equal or higher
					// precedence (that includes all of them) and put them into
					// postfix list. If a '(' is popped, put it back and stop looking.
					// Push new '+' or '-'.

					while (stack.top().type != Token::typeFunction ||
						   stack.top().function != Token::functionLParen)
					{
						postfixTokens.push_back(stack.top());
						stack.pop();
					}

					if (*infix == '+')
					{
						stack.push(Token::functionAdd);
					}
					else
					{
						stack.push(Token::functionSub);
					}
            
				}
            
				readyForUnaryOperator = true;
    			++infix;

			}
			else
			{
				return false;
			}
		
		}

		// Have processed all input characters. New flush stack until we find
		// the '(' originally pushed onto the stack.
    
		while (stack.top().type != Token::typeFunction ||
			   stack.top().function != Token::functionLParen)
		{
			postfixTokens.push_back(Token(stack.top()));
			stack.pop();
		}
	} catch(...)
	{
		TRACE("EXPRESSION PARSING ERROR!!!!\n");
		return false;
	}

    return true;

}


bool Expression::Compile(const std::vector<Token>& postfixTokens)
{

    const unsigned char byteCodeTable[][2] =
        {
            { 0xde, 0xc1 },   // faddp   st(0), st(1)
            { 0xde, 0xe9 },   // fsubrp  st(0), st(1) 
            { 0xde, 0xc9 },   // fmulp   st(0), st(1)
            { 0xde, 0xf9 },   // fdivrp  st(0), st(1)
            { 0xd9, 0xe0 },   // fchs
            { 0xd9, 0xe1 },   // fabs
            { 0xd9, 0xfe },   // fsin
            { 0xd9, 0xff },   // fcos
            { 0xd9, 0xfa },   // fsqrt
            { 0xd9, 0xeb },   // fldpi       
        };
    
    std::strstream buffer;

    // Output the initialization code.
    
    // push ebp
    
    buffer.put(0x55);

    // mov ebp, esp

    buffer.put(0x8b);
    buffer.put(0xec);

    // sub esp, 4
    
    buffer.put(0x83);
    buffer.put(0xec);
    buffer.put(0x04);	 

    // Tracks the amount of the floating point stack which is currently used.

    const int maxStackSize = 8;

    int usedStackSize = 0;

    // Compile and output the code.

    for (unsigned int i = 0; i < postfixTokens.size(); ++i)
    {

        if (postfixTokens[i].type == Token::typeValue)
        {

            if (usedStackSize == maxStackSize)
            {
                return false;
            }

            // Move the value into a temporary storage variable and then
            // load it onto the floating point stack.

            // mov DWORD PTR [ebp - 4], value

            buffer.put(0xc7);
            buffer.put(0x45); 
            buffer.put(0xfc); 
            buffer.put(((char*)(&postfixTokens[i].value))[0]);
            buffer.put(((char*)(&postfixTokens[i].value))[1]);
            buffer.put(((char*)(&postfixTokens[i].value))[2]);
            buffer.put(((char*)(&postfixTokens[i].value))[3]);

            // fld DWORD PTR [ebp - 4]
            
            buffer.put(0xd9);
            buffer.put(0x45);
            buffer.put(0xfc);

            ++usedStackSize;

        }
        else if (postfixTokens[i].type == Token::typeVariable)
        {

            if (usedStackSize == maxStackSize)
            {
                return false;
            }            

            // fld DWORD PTR [esi + variableIndex * 4]
            
            buffer.put(0xd9);
            buffer.put(0x46);
            buffer.put(postfixTokens[i].variableIndex * 4);

            ++usedStackSize;

        }
        else if (postfixTokens[i].type == Token::typeUserFunction)
        {
            
            // Make a function call to a user function.

            // push ecx

            buffer.put(0x51);

            // fstp	DWORD PTR [esp]

            buffer.put(0xd9);
            buffer.put(0x1c);
            buffer.put(0x24);

            // mov eax, postfixTokens[i].userFunction

            buffer.put(0xb8);
            buffer.put(((char*)(&postfixTokens[i].userFunction))[0]);
            buffer.put(((char*)(&postfixTokens[i].userFunction))[1]);
            buffer.put(((char*)(&postfixTokens[i].userFunction))[2]);
            buffer.put(((char*)(&postfixTokens[i].userFunction))[3]);

            // call eax

            buffer.put(0xff);
            buffer.put(0xd0);

            // add esp, 4
            
            buffer.put(0x83);
            buffer.put(0xc4);
            buffer.put(0x04);
        
        }
        else if (postfixTokens[i].type == Token::typeFunction)
        {

            switch (postfixTokens[i].function)
            {
            
            case Token::functionAdd:
            case Token::functionSub:
            case Token::functionMul:
            case Token::functionDiv:
                
                if (usedStackSize < 2)
                {
                    return false;
                }
                
                --usedStackSize;
                break;
                
            case Token::functionUnarySub:
            case Token::functionAbs:
            case Token::functionSin:
            case Token::functionCos:
            case Token::functionSqrt:

                if (usedStackSize < 1)
                {
                    return false;
                }
                break;

            case Token::functionPi:

                if (usedStackSize == maxStackSize)
                {
                    return false;
                }

                ++usedStackSize;
                break;

            }

            buffer.put(byteCodeTable[postfixTokens[i].function][0]);
            buffer.put(byteCodeTable[postfixTokens[i].function][1]);
        
        }
    
    }

    // Output the return code.

    // mov	 esp, ebp
    
    buffer.put(0x8b);
    buffer.put(0xe5);

    // pop	 ebp

    buffer.put(0x5d);
    
    // ret    
    
    buffer.put(0xc3);

    // Save the buffer for later execution.

    compiledCode = buffer.str();

    return true;

}


float Expression::Evaluate(const float variable[]) const
{

    if (compiledCode == NULL)
    {
        return 0;
    }

    const void* code = compiledCode;

    __asm
    {
        push esi
        mov  esi, variable
        call code
        pop  esi
        jmp  end
    };

    return 0;

end:

    // Return value is set in the compiled code by storing the result on the
    // floating point stack.

    ;    

}


Expression::~Expression()
{
    free(compiledCode);
}
