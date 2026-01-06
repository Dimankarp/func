#pragma once

#include "function/function.hpp"
#include "node/statement.hpp"
namespace func {
class program : public statement {
  std::vector<unique_ptr<func::function>> functions;

public:
  program(std::vector<unique_ptr<func::function>> &&funcs,  yy::location loc);
  const std::vector<unique_ptr<func::function>> &get_funcs() const {
    return functions;
  }
  void accept(statement_visitor &visitor);
};

} // namespace func