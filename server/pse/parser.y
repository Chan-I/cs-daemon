%define api.pure full
%lex-param {core_yyscan_t scanner}
%parse-param {core_yyscan_t scanner}{module *mod}

%define parse.trace
%define parse.error verbose
//%define api.prefix {module_yy}
%name-prefix "module_yy"

%{
#include <stdio.h>
#include "pse/module.h"
#include "parser.tab.h"
#include "scanner.h"

void module_yyerror (core_yyscan_t scanner, module *mod, char const *msg);
extern int module_scanner_errmsg(const char *msg, core_yyscan_t *scanner);
extern int module_scanner_errposition(const int location, core_yyscan_t *scanner);
%}

%code requires
{
#include "pse/ast.h"
#include "pse/module.h"
}

%define api.value.type union /* Generate YYSTYPE from these types:  */
%token <long>           NUMBER     "number"
%token <const char *>   STRING     "string"
%token <const char *>   IDENTIFIER "identifier"

%token TOK_EOF 0 "end of file"
%token TOK_LPAREN "("
%token TOK_RPAREN ")"

%type <ast_node_sexp*> sexp
%type <ast_node_atom*> atom
%type <ast_node_list*> list

%%
%start sexps;

sexps:
	sexp        { mod->root = $1; };

sexp:
  atom          { $$ = new_sexp_node(ST_ATOM, $1); }
| "(" list ")"  { $$ = new_sexp_node(ST_LIST, $2); };

list:
  %empty        { $$ = new_list_node(); }
| list sexp     { $$ = $1; add_node_to_list($$, $2); };

atom:
  "number"      { $$ = new_atom_node(AT_NUMBER, (void *)(&$1)); }
| "identifier"  { $$ = new_atom_node(AT_IDENTIFIER, (void *)$1); }
| "string"      { $$ = new_atom_node(AT_STRING, (void *)$1); };

%%

void module_yyerror (core_yyscan_t scanner, module *mod, char const *msg)
{
  module_scanner_errmsg("cypher error", scanner);
}

