#pragma once

#include "exception.hpp"
#include "type/type.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/register_allocator.hpp"
#include <memory>
namespace intrp {

template <typename T> T expect(const expr_result &exp) {
  if (exp.type_obj->get_type() != T::static_get_type())
    throw unexpected_type_exception();
  return dynamic_cast<const T &>(*exp.type_obj);
}

expr_result expr_add(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);

expr_result expr_sub(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);

expr_result expr_mul(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);
expr_result expr_div(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);

expr_result expr_rem(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);
expr_result expr_less(instr::instruction_writer &w, reg_allocator &alloc,
                      const expr_result &a, const expr_result &b);
expr_result expr_grtr(instr::instruction_writer &w, reg_allocator &alloc,
                      const expr_result &a, const expr_result &b);

expr_result expr_eq(instr::instruction_writer &w, reg_allocator &alloc,
                    const expr_result &a, const expr_result &b);
expr_result expr_neq(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);
expr_result expr_or(instr::instruction_writer &w, reg_allocator &alloc,
                    const expr_result &a, const expr_result &b);
expr_result expr_and(instr::instruction_writer &w, reg_allocator &alloc,
                     const expr_result &a, const expr_result &b);

} // namespace intrp