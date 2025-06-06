%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.8"
%defines

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
  #include "node/expression.hpp"
  #include "node/statement.hpp"
  #include "node/program.hpp"
  #include "function/function.hpp"
  #include "type/type.hpp"
  #include <string>
  #include <variant>

  class driver;
  using std::vector, std::unique_ptr, std::string;
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
  END  0    "end of file"
  EOS       ";" // end of statement
  COMMA     ","
  
  MINUS     "-"
  PLUS      "+"
  MUL       "*"
  DIV       "/"
  MOD       "%"
  LPAR      "("
  RPAR      ")"
  LBRACK    "["
  RBRACK    "]"
  ASSIGN    "="
  LESS      "<"
  GRTR      ">"
  EQUALS    "=="
  NOTEQ     "!="
  OR        "||"
  AND       "&&"
  NOT       "!"
  LCURL     "{"
  RCURL     "}"
  WHILE     "while"
  IF        "if"
  ELSE      "else"

  RETURN    "return"
  INT_T     "int"
  BOOL_T    "bool"
  STRING_T  "string"
  VOID_T    "void"
;

%token <std::string> STR "str"
%token <std::string> ID "id"
%token <int>  NUM "num"
%token <bool> BOOLS "bools"

%type  <std::unique_ptr<intrp::expression>> exp
%type  <std::unique_ptr<intrp::statement>> statement
%type  <std::unique_ptr<intrp::block_statement>> statements
%type  <std::unique_ptr<intrp::block_statement>> block

%type  <std::unique_ptr<intrp::function>> function
%type  <std::vector<std::unique_ptr<intrp::function>>> functions


%type  <std::vector<intrp::parameter>> param_list
%type  <std::vector<intrp::parameter>> params
%type  <intrp::parameter> param

%type  <std::vector<unique_ptr<intrp::expression>>> arg_list
%type  <std::vector<unique_ptr<intrp::expression>>> args

%type  <unique_ptr<intrp::type>>  type 
%type  <unique_ptr<intrp::type>>  func_res_type
%type  <std::vector<unique_ptr<intrp::type>>>  func_type
%type  <std::vector<unique_ptr<intrp::type>>>  func_type_rec


%%
%start program;
program: functions {drv.result = std::make_unique<intrp::program>(std::move($1), @$);};

functions:
  %empty {
    $$ = std::vector<std::unique_ptr<intrp::function>>();
  }
| functions function {
    $1.push_back(std::move($2));
    $$ = std::move($1);
};

function:
  func_res_type "id" "(" param_list ")" block {
    $$ = std::make_unique<intrp::function>(std::move($1), $2, std::move($4), std::move($6), @$);
  }


param_list:
  %empty {
    $$ = std::vector<intrp::parameter>();
  }
| params {
    $$ = std::move($1);
};

params:
  param {
    $$ = std::vector<intrp::parameter>();
    $$.push_back(std::move($1));
  }
| params "," param  {
  $1.push_back(std::move($3));
  $$ = std::move($1);
};

param:
  type "id" {
    $$ = intrp::parameter(std::move($1), $2);
  };



block:
  "{" "}" {
    $$ = std::make_unique<intrp::block_statement>(@$);
    }
| statement {
  $$ = std::make_unique<intrp::block_statement>(@$);
  $$->add_statement(std::move($1));
  }
| "{" statements "}" {
  $$ = std::move($2);
};




statements:
 statement {
  $$ = std::make_unique<intrp::block_statement>(@$);
  $$->add_statement(std::move($1));
  }
| statements statement {
    $1->add_statement(std::move($2));
    $$ = std::move($1);
};


%precedence "if";
%precedence "else";

statement: 
  type "id" ";" {$$ = std::make_unique<intrp::assign_statement>(std::move($1), $2, @$);}
| type "id" "=" exp ";" {$$ = std::make_unique<intrp::assign_statement>(std::move($1), $2, std::move($4), @$);}
| "id" "=" exp ";" {$$ = std::make_unique<intrp::assign_statement>($1, std::move($3), @$);}
| exp "[" exp "]" "=" exp ";" {$$ = std::make_unique<intrp::subscript_assign_statement>(std::move($1), std::move($3), std::move($6),  @$);}
| exp "(" arg_list ")" ";" {$$ = std::make_unique<intrp::function_call>(std::move($1), std::move($3), @$);}
| "if" "(" exp ")" block %prec "if" {$$ = std::make_unique<intrp::if_statement>(std::move($3), std::move($5), @$);}
| "if" "(" exp ")" block "else" block %prec "else" {
  auto s = std::make_unique<intrp::if_statement>(std::move($3), std::move($5), @$);
  (*s).add_else(std::move($7));
  $$ = std::move(s);}
| "while" "(" exp ")" block {$$ = std::make_unique<intrp::while_statement>(std::move($3), std::move($5), @$);};
| "return" exp ";" {$$ = std::make_unique<intrp::return_statement>(std::move($2), @$);};
| "return" ";"     {$$ = std::make_unique<intrp::return_statement>(nullptr, @$);};


%left "||";
%left "&&"; 
%left "<" ">" "==" "!="; 
%left "+" "-";
%left "*" "/";
%left "%";
%precedence UMINUS;
%precedence "!";
%left "("; 
%left "["; 

exp:
  exp "+" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::ADD, std::move($1), std::move($3), @$);}
| exp "-" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::SUB, std::move($1), std::move($3), @$);}
| exp "*" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::MUL, std::move($1), std::move($3), @$);}
| exp "/" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::DIV, std::move($1), std::move($3), @$);}
| exp "%" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::MOD, std::move($1), std::move($3), @$);}
| exp "<" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::LESS, std::move($1), std::move($3), @$);}
| exp ">" exp           {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::GRTR, std::move($1), std::move($3), @$);}
| exp "==" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::EQ, std::move($1), std::move($3), @$);}
| exp "!=" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::NEQ, std::move($1), std::move($3), @$);}
| exp "||" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::OR, std::move($1), std::move($3), @$);}
| exp "&&" exp          {$$ = std::make_unique<intrp::binop_expression>(intrp::binop::AND, std::move($1), std::move($3), @$);}
| "-" exp %prec UMINUS  {$$ = std::make_unique<intrp::unarop_expression>(intrp::unarop::MINUS, std::move($2), @$);}
| "!" exp               {$$ = std::make_unique<intrp::unarop_expression>(intrp::unarop::NOT, std::move($2), @$);}
| "str"                 {$$ = std::make_unique<intrp::literal_expression>(intrp::lit_val($1), @$);}
| "num"                 {$$ = std::make_unique<intrp::literal_expression>(intrp::lit_val($1), @$);}
| "bools"               {$$ = std::make_unique<intrp::literal_expression>(intrp::lit_val($1), @$);}
| "id"                  {$$ = std::make_unique<intrp::identifier_expression>($1, @$);}
| exp "(" arg_list ")"  {$$ = std::make_unique<intrp::function_call>(std::move($1), std::move($3), @$);}
| exp "[" exp "]"       {$$ = std::make_unique<intrp::subscript_expression>(std::move($1), std::move($3), @$);}
| "(" exp ")"           {$$ = std::move($2);}



arg_list:
  %empty {
    $$ = std::vector<unique_ptr<intrp::expression>>();
  }
| args {
    $$ = std::move($1);
};

args:
  exp {
    $$ = std::vector<unique_ptr<intrp::expression>>();
    $$.push_back(std::move($1));
    }
| args "," exp {
  $1.push_back(std::move($3));
  $$ = std::move($1);
};



type:
  "int"     {$$ = std::make_unique<intrp::int_type>();}
| "bool"    {$$ = std::make_unique<intrp::bool_type>();}
| "string"  {$$ = std::make_unique<intrp::string_type>();}
| "(" func_type ")"  {$$ = std::make_unique<intrp::function_type>(std::move($2));};


func_type:
  type "-" func_type_rec {
    $3.insert($3.begin(), std::move($1));
    $$ = std::move($3);
    }
| "void" "-" func_res_type {
  auto typevec = vector<unique_ptr<intrp::type>>();
  typevec.push_back(std::make_unique<intrp::void_type>());
  typevec.push_back(std::move($3));
  $$ = std::move(typevec);
  };

func_res_type:
  type {$$ = std::move(std::move($1));}
| "void" {$$ = std::make_unique<intrp::void_type>();};


 func_type_rec:
  type "-" func_type_rec {
  $3.insert($3.begin(), std::move($1));
  $$ = std::move($3);
  }
| func_res_type {
  $$ = vector<unique_ptr<intrp::type>>();
  $$.push_back(std::move($1));
};

%%

void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
