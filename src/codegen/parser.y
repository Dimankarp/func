%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.8"
%defines

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include "node/expression.hpp"
  #include "node/statement.hpp"
  #include <string>
  #include <variant>

  class driver;
}

// The parsing context.
%param { driver& drv }


// Adding locations support
%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.hpp"
#include "node/expression.hpp"
#include "node/statement.hpp"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  EOS     ";" // end of statement
  
  MINUS   "-"
  PLUS    "+"
  MUL     "*"
  DIV     "/"
  MOD     "%"
  LPAR    "("
  RPAR    ")"
  ASSIGN  "="
  LESS    "<"
  GRTR    ">"
  LESS_EQ "<="
  GRTR_EQ ">="
  EQUALS  "=="
  NOTEQ   "!="
  LCURL   "{"
  RCURL   "}"
  WHILE   "while"
  IF      "if"
  ELSE    "else"

  PRINT   "print"
;

%token <std::string> STR "string"
%token <std::string> ID "identifier"
%token <int> NUM "number"
%type  <std::unique_ptr<intrp::expression>> exp
%type  <std::unique_ptr<intrp::statement>> statement
%type  <std::unique_ptr<intrp::block_statement>> statements


// %printer { yyoutput << $$; } <*>;

%%
%start program;
program: statements {drv.result = std::move($1);};

statements:
  %empty {
    $$ = std::make_unique<intrp::block_statement>(@$);
  }
| statements statement {
    $1->add_statement(std::move($2));
    $$ = std::move($1);
};

%precedence "if";
%precedence "else";

statement: 
  "print" exp ";"          {$$ = std::make_unique<intrp::print_statement>(std::move($2), @$);}
| "identifier" "=" exp ";" {$$ = std::make_unique<intrp::assign_statement>($1, std::move($3), @$);}
| "if" exp "{" statements "}" %prec "if" {$$ = std::make_unique<intrp::if_statement>(std::move($2), std::move($4), @$);}
| "if" exp "{" statements "}" "else" "{" statements "}" %prec "else" {
  auto s = std::make_unique<intrp::if_statement>(std::move($2), std::move($4), @$);
  (*s).add_else(std::move($8));
  $$ = std::move(s);}
| "while" exp "{" statements "}" {$$ = std::make_unique<intrp::while_statement>(std::move($2), std::move($4), @$);};

%left "<" ">" "<=" ">=" "==" "!=";
%left "+" "-";
%left "*" "/";
%left "%";
%precedence UMINUS;


exp:
  exp "+" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::ADD, std::move($1), std::move($3), @$);}
| exp "-" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::SUB, std::move($1), std::move($3), @$);}
| exp "*" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::MUL, std::move($1), std::move($3), @$);}
| exp "/" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::DIV, std::move($1), std::move($3), @$);}
| exp "%" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::MOD, std::move($1), std::move($3), @$);}
| exp "<" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::LESS, std::move($1), std::move($3), @$);}
| exp ">" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::GRTR, std::move($1), std::move($3), @$);}
| exp "<=" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::LEQ, std::move($1), std::move($3), @$);}
| exp ">=" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::GREQ, std::move($1), std::move($3), @$);}
| exp "==" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::EQ, std::move($1), std::move($3), @$);}
| exp "!=" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::NEQ, std::move($1), std::move($3), @$);}
| "-" exp %prec UMINUS  {$$ = std::make_unique<intrp::unarop_expression>(intrp::unarop::MINUS, std::move($2), @$);}
| "string"              {$$ = std::make_unique<intrp::literal_expression>(intrp::expr_t($1), @$);}
| "number"              {$$ = std::make_unique<intrp::literal_expression>(intrp::expr_t($1), @$);}
| "identifier"          {$$ = std::make_unique<intrp::identifier_expression>($1, @$);};

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
