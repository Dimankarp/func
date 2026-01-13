#pragma once

#include "codegen/location.hh"
#include "codegen/parser.tab.hpp"
#include "node/ast.hpp"
#include <memory>
#include <string>

#define YY_DECL yy::parser::symbol_type yylex(driver& drv)
YY_DECL;

class driver {

    public:
    driver();
    std::unique_ptr<func::ast_node> result;
    // Run the parser on file F.  Return 0 on success.
    int parse(const std::string& f);
    // The name of the file being parsed.
    std::string file;
    // Whether to generate parser debug traces.
    bool trace_parsing{ false };
    // Handling the scanner.
    void scan_begin();
    void scan_end();
    // Whether to generate scanner debug traces.
    bool trace_scanning{ false };
    // The token's location used by the scanner.
    yy::location location;
};