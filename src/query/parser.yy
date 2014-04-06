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
    std::string*		unquotedstringVal;
    class QueryNode*	querynode;
}

%token			END	     0	"end of file"
%token			EOL		"end of line"
%token <integerVal> INTEGER		 "integer"
%token <doubleVal> 	DOUBLE		 "double"
%token <stringVal> 	STRING		 "string"
%token <stringVal> 	EQUAL      "equal"
%token <stringVal> 	GREATER    "greater"
%token <stringVal> 	GREATER_EQ "greater_eq"
%token <stringVal> 	LESSER     "lesser"
%token <stringVal> 	LESSER_EQ  "lesser_eq"
%token <stringVal> 	LIKE       "like"
%token <unquotedstringVal> 	UNQUOTEDSTRING		"unquotedstring"

%token TRUE
%token TAGGED
%left OR AND NOT

%type <querynode> all action expr
%type <stringVal> operator

%{

#include "driver.h"
#include "scanner.h"

#undef yylex
#define yylex driver.lexer->lex

%}

%% /*** Grammar Rules ***/

all : '(' TRUE ')'
      {
        $$ = new QueryTrue();
      }

operator: EQUAL
            {
              $$ = $1;
            }
         |
          LIKE
            {
              $$ = $1;
            }
         | GREATER
            {
              $$ = $1;
            }
         | GREATER_EQ
            {
              $$ = $1;
            }
         | LESSER
            {
              $$ = $1;
            }
         | LESSER_EQ
            {
              $$ = $1;
            }

action : TAGGED EQUAL STRING
          {
            delete $2;
            $$ = new QueryTagged($3);
          }
        | UNQUOTEDSTRING operator STRING
          {
            $$ = new QueryField($1, $3, $2);
          }
        | UNQUOTEDSTRING operator INTEGER
          {
            $$ = new QueryField($1, $3, $2);
          }
        | UNQUOTEDSTRING operator DOUBLE
          {
            $$ = new QueryField($1, $3, $2);
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
      | NOT expr
        {
          $$ = new QueryNot($2);
        }
      | action
        {
          $$ = $1;
        }

start :   all
          {
            driver.query.expression.reset($1);
          }
        | expr
          {
            driver.query.expression.reset($1);
          }

%%

void queryparser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
