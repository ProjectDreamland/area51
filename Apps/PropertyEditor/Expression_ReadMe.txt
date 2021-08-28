This is a class that I wrote to take infix simple mathematical expressions
and compile them into Intel Architecture 32 machine code. The expressions
can later be evaluated with variable substitutions.  This is an updated version
of the original code which includes support for calling user supplied functions
from within the expression.

Here's a simple example of the class's usage: 


    Expression expression;

    const char* variableName[] = { "angle", "rand" };
    const char* functionName[] = { "sqr" };

    Expression::UserFunction function[] = { sqr };  

    expression.Initialize("1 + 2 * sqr(cos(angle * pi)) + rand", variableName, 2, function, functionName, 1);
  
    float variableValue[2];
 
    variableValue[0] = 27;
    variableValue[1] = rand();
  
    float result = expression.Evaluate(variableValue);
 

The infix to postfix code was adapted from a freely available routine by Rex Jaeshke.
I expanded this code to use floating point numbers, support unary minus and allow
functions and variables. 

Possible uses might be using mathematical expressions in scripts to control material
properies, particle systems, etc. 

Please report any bugs you find. 

Max 
