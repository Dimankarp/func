#pragma once

#include "location.hh"
#include <string>
namespace intrp {

struct syntax_exception {
  std::string reason;
  yy::location loc;
};

struct undeclared_variable_exception : public syntax_exception {};

struct unexpected_type_exception : public syntax_exception {};

std::ostream &operator<<(std::ostream &outs, const syntax_exception &e);
} // namespace intrp