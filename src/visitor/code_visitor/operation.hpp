#pragma once

#include "codegen/location.hh"
#include "exception.hpp"
#include "type/type.hpp"
#include "visitor/code_visitor/code_visitor.hpp"
#include "visitor/code_visitor/instruction_writer.hpp"
#include "visitor/code_visitor/register_allocator.hpp"
#include <memory>
namespace func {

expr_result expr_add(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);

expr_result expr_sub(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);

expr_result expr_mul(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);
expr_result expr_div(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);

expr_result expr_rem(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);
expr_result expr_less(instr::instruction_writer& w,
                      reg_allocator& alloc,
                      const expr_result& a,
                      const expr_result& b);
expr_result expr_grtr(instr::instruction_writer& w,
                      reg_allocator& alloc,
                      const expr_result& a,
                      const expr_result& b);

expr_result expr_eq(instr::instruction_writer& w,
                    reg_allocator& alloc,
                    const expr_result& a,
                    const expr_result& b);
expr_result expr_neq(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);
expr_result expr_or(instr::instruction_writer& w,
                    reg_allocator& alloc,
                    const expr_result& a,
                    const expr_result& b);
expr_result expr_and(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b);

expr_result
expr_minus(instr::instruction_writer& w, reg_allocator& alloc, const expr_result& a);
expr_result
expr_not(instr::instruction_writer& w, reg_allocator& alloc, const expr_result& a);

} // namespace func