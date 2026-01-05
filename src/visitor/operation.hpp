#pragma once

#include "exception.hpp"
#include "location.hh"
#include "type/type.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/instruction_writer.hpp"
#include "visitor/register_allocator.hpp"
#include "visitor/print_visitor.hpp"
#include <memory>
namespace cmplr {


template <typename T> T expect(const expr_result &exp) {
  if (exp.type_obj->get_type() != T::static_get_type())
    throw unexpected_type_exception({cmplr::types_to_string(exp.type_obj->get_type()) + " but expected " + cmplr::types_to_string(T::static_get_type()) , yy::location{}});
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

                     expr_result expr_minus(instr::instruction_writer &w, reg_allocator &alloc,
                      const expr_result &a);
                      expr_result expr_not(instr::instruction_writer &w, reg_allocator &alloc,
                        const expr_result &a);

} // namespace cmplr