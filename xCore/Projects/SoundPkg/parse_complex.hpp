#ifndef __PARSE_COMPLEX_H
#define __PARSE_COMPLEX_H

#include "../../support/tokenizer/tokenizer.hpp"
#include "parse.hpp"

// Complex included at the top level within a control file
void            ParseComplex(token_stream &Tok,parse_output &Output,parse_variables &Default,xbool SkipLabel);
// Complex included within a complex definition
complex_effect *ParseComplexComplex(token_stream &Tok,parse_output &Output,s32 ownerid,parse_variables &Default);
complex_effect *ParseComplexSimple(token_stream &Tok,parse_output &Output,xbool ForceLooped,s32 ownerid,parse_variables &Default);

#endif