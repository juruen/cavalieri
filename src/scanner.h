#ifndef SCANNER_H
#define SCANNER_H

#ifndef YY_DECL

#define	YY_DECL	\
    queryparser::Parser::token_type	\
    queryparser::Scanner::lex( \
      queryparser::Parser::semantic_type* yylval,	\
      queryparser::Parser::location_type* yylloc \
    )
#endif

#ifndef __FLEX_LEXER_H
#define yyFlexLexer QueryparserFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

#include "parser.h"

namespace queryparser {

class Scanner : public QueryparserFlexLexer
{
public:
    Scanner(std::istream* arg_yyin = 0,
	    std::ostream* arg_yyout = 0);

    virtual ~Scanner();

    virtual Parser::token_type lex(
        Parser::semantic_type* yylval,
        Parser::location_type* yylloc
        );
    void set_debug(bool b);
};

}

#endif
