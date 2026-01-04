#pragma once

#include "node/ast.hpp"
#include "parser.tab.hpp"
#include <string>
#include <unordered_map>

#define YY_DECL yy::parser::symbol_type yylex(driver &drv)
YY_DECL;

using var_table = std::unordered_map<std::string, intrp::lit_val>;

class driver {

public:
  driver();
  std::unique_ptr<intrp::ast_node> result;
  // Run the parser on file F.  Return 0 on success.
  int parse(const std::string &f);
  // The name of the file being parsed.
  std::string file;
  std::string output_file;
  // Whether to generate parser debug traces.
  bool trace_parsing;

  // Handling the scanner.
  void scan_begin();
  void scan_end();
  // Whether to generate scanner debug traces.
  bool trace_scanning;
  // The token's location used by the scanner.
  yy::location location;
};