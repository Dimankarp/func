#pragma once

#include "function/function.hpp"
#include "node/statement.hpp"
namespace cmplr {
class program : public statement {
  std::vector<unique_ptr<cmplr::function>> functions;

public:
  program(std::vector<unique_ptr<cmplr::function>> &&funcs,  yy::location loc);
  const std::vector<unique_ptr<cmplr::function>> &get_funcs() const {
    return functions;
  }
  void accept(statement_visitor &visitor);
};

} // namespace cmplr