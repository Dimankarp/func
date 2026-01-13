%{
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "driver.hpp"
# include "codegen/parser.tab.hpp"


// Pacify warnings in yy_init_buffer (observed with Flex 2.6.4)
// and GCC 7.3.0.
#if defined __GNUC__ && 7 <= __GNUC__
# pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
%}

%option noyywrap nounput batch debug noinput

id    [a-zA-Z_][a-zA-Z_0-9]*
int   [0-9]+
str   \"([^\n"\\]|\\.)*\" 
spc   [ \t]
bools (true|false)
ln_cmmnt \/\/[^\n]*\n
mltln_cmmnt \/\*([^*]|\*+[^*\/])*\*+\/
%{
  // Code run each time a pattern is matched.
  // Updates current column of loc.
  # define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step();
%}

{spc}+   loc.step();
[\n]+    loc.lines (yyleng); loc.step();
{ln_cmmnt} loc.lines(1); loc.step();
{mltln_cmmnt} {
  int lines = 0;
  
  for(int i = 0; i < yyleng; i++)
    lines+= yytext[i]=='\n';

  if(lines > 0)loc.lines(lines);
  loc.step();
}

";"      return yy::parser::make_EOS(loc);
","      return yy::parser::make_COMMA(loc);

"-"      return yy::parser::make_MINUS(loc);
"+"      return yy::parser::make_PLUS(loc);
"*"      return yy::parser::make_MUL(loc);
"/"      return yy::parser::make_DIV(loc);
"%"      return yy::parser::make_MOD(loc);
"("      return yy::parser::make_LPAR(loc);
")"      return yy::parser::make_RPAR(loc);
"["      return yy::parser::make_LBRACK(loc);
"]"      return yy::parser::make_RBRACK(loc);
"="      return yy::parser::make_ASSIGN(loc);

"<"      return yy::parser::make_LESS(loc);
">"      return yy::parser::make_GRTR(loc);
"=="     return yy::parser::make_EQUALS(loc);
"!="     return yy::parser::make_NOTEQ(loc);
"||"     return yy::parser::make_OR(loc);
"&&"     return yy::parser::make_AND(loc);
"!"      return yy::parser::make_NOT(loc);

"{"      return yy::parser::make_LCURL(loc); 
"}"      return yy::parser::make_RCURL(loc); 
"while"  return yy::parser::make_WHILE(loc); 
"if"     return yy::parser::make_IF(loc); 
"else"   return yy::parser::make_ELSE(loc); 

"return"  return yy::parser::make_RETURN(loc);

"int"    return yy::parser::make_INT_T(loc);
"bool"   return yy::parser::make_BOOL_T(loc);
"string" return yy::parser::make_STRING_T(loc);
"void"   return yy::parser::make_VOID_T(loc);

{str}      {
  std::string str_lit {yytext};
  auto len = str_lit.length();
  str_lit.erase(0, 1).erase(len-2, len);

  // replacing \n with newline character
  const std::string from_arr[] = {"\\n", "\\\""};
  const std::string to_arr[] = {"\n", "\""};

  for(int i = 0; i < 2; i++) {
    const std::string& from = from_arr[i];
    const std::string& to = to_arr[i];

    size_t pos =0;
    while((pos=str_lit.find(from, pos)) != std::string::npos) {
      str_lit.replace(pos, from.length(), to);
      pos += to.length();
    }
  }
  return yy::parser::make_STR(str_lit, loc);
}

{int}      {
  errno = 0;
  long n = strtol(yytext, NULL, 10);
  if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
    throw yy::parser::syntax_error (loc, "integer is out of range: "
                                    + std::string(yytext));
  return yy::parser::make_NUM(n, loc);
}

{bools} {
  std::string bool_str{yytext};
  return yy::parser::make_BOOLS(bool_str=="true", loc);
}

{id}     return yy::parser::make_ID(yytext, loc);

.          {
             throw yy::parser::syntax_error
               (loc, "invalid character: " + std::string(yytext));
}
<<EOF>>    return yy::parser::make_END (loc);
%%

void
driver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
      exit (EXIT_FAILURE);
    }
}

void
driver::scan_end ()
{
  fclose (yyin);
}
