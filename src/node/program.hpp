#pragma once

#include "function/function.hpp"
#include "node/ast.hpp"
#include <vector>

namespace intrp {
class program : public ast_node_impl<program> {
  using Base = ast_node_impl<program>;
  std::vector<unique_ptr<intrp::function>> functions;

public:
  program(std::vector<unique_ptr<intrp::function>> &&funcs, yy::location loc)
      : functions{std::move(funcs)}, Base{loc} {}
  const std::vector<unique_ptr<intrp::function>> &get_funcs() const {
    return functions;
  }
};

} // namespace intrp