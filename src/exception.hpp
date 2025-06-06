#pragma once

#include "location.hh"
#include <sstream>
#include <string>
namespace intrp {

struct syntax_exception {
  std::string reason;
  yy::location loc;
};

struct undeclared_variable_exception : public syntax_exception {};

struct unexpected_type_exception : public syntax_exception {};

struct symbol_redeclaratione_exception : public syntax_exception {
  symbol_redeclaratione_exception(std::string sym, yy::location origin_loc,
                                  yy::location current_loc) {
    this->loc = current_loc;
    std::stringstream ss{};
    ss << "Symbol redeclaration: " << sym << " originally declared at "
       << origin_loc;
    this->reason = ss.str();
  }
};

struct symbol_not_found_exception : public syntax_exception {};

struct not_enough_registers_exceptions {};

struct global_syntax_exception {
  std::string reason;
};

std::ostream &operator<<(std::ostream &outs, const syntax_exception &e);
std::ostream &operator<<(std::ostream &outs, const global_syntax_exception &e);
} // namespace intrp