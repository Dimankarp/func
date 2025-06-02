#pragma once

#include "function/function.hpp"
#include "node/statement.hpp"
namespace intrp {
class program : public statement {
  std::vector<unique_ptr<intrp::function>> functions;

public:
  program(std::vector<unique_ptr<intrp::function>> &&funcs,  yy::location loc);
  const std::vector<unique_ptr<intrp::function>> &get_funcs() const {
    return functions;
  }
  void accept(statement_visitor &visitor);
};

} // namespace intrp