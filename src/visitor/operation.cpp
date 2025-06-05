#include "visitor/operation.hpp"
#include "node/expression.hpp"
#include <iostream>
#include <variant>

namespace intrp {

expr_result expr_add(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b) {
  expect<intrp::int_type>(a);
  expect<intrp::int_type>(b);
  auto r = alloc.alloc();
  w.add(r, a.reg_num, b.reg_num);
  return expr_result{std::make_unique<intrp::int_type>(), r};
}

} // namespace intrp