%{ /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>

#include "expression.h"

%}

/*** yacc/bison Declarations ***/

/* Require bison 2.3 or later */
%require "2.3"

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug

/* start symbol is named "start" */
%start start

/* write out a header file containing the token defines */
%defines

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* namespace to enclose parser in */
%name-prefix="queryparser"

%define "parser_class_name" "Parser"

%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};

%parse-param { class Driver& driver }

/* verbose error messages */
%error-verbose

%union {
    int integerVal;
    double doubleVal;
    std::string*		stringVal;
    class QueryNode*	querynode;
}

%token			END	     0	"end of file"
%token			EOL		"end of line"
%token <integerVal> 	INTEGER		"integer"
%token <doubleVal> 	DOUBLE		"double"
%token <stringVal> 	STRING		"string"

%token TAGGED
%left OR AND

%type <querynode> action expr

%{

#include "driver.h"
#include "scanner.h"

#undef yylex
#define yylex driver.lexer->lex

%}

%% /*** Grammar Rules ***/

action : TAGGED '=' STRING
          {
            $$ = new QueryTagged($3);
          }

expr: '(' expr ')'
        {
          $$ = $2;
        }
      | expr AND expr
        {
          $$ = new QueryAnd($1, $3);
        }
      | expr OR expr
        {
          $$ = new QueryOr($1, $3);
        }
      | action
        {
          $$ = $1;
        }

start : expr
          {
            driver.query.expressions.push_back($1);
          }

%%

void queryparser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
