#include "visitor/code_visitor/operation.hpp"
#include "codegen/location.hh"
#include "type/type.hpp"
#include "type/type_checker.hpp"

namespace func {

expr_result expr_add(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.add(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}

expr_result expr_sub(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.sub(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}

expr_result expr_mul(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.mul(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}

expr_result expr_div(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.div(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}

expr_result expr_rem(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.rem(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}

expr_result expr_less(instr::instruction_writer& w,
                      reg_allocator& alloc,
                      const expr_result& a,
                      const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.slt(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

expr_result expr_grtr(instr::instruction_writer& w,
                      reg_allocator& alloc,
                      const expr_result& a,
                      const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r1 = alloc.alloc();
    auto r2 = alloc.alloc();
    w.sge(r1, a.reg_num, b.reg_num);
    w.sne(r2, a.reg_num, b.reg_num);
    w.and_op(r1, r1, r2);
    alloc.dealloc(r2);
    return expr_result{ std::make_unique<func::bool_type>(), r1 };
}

expr_result expr_eq(instr::instruction_writer& w,
                    reg_allocator& alloc,
                    const expr_result& a,
                    const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();

    w.seq(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

expr_result expr_neq(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    expect_types(int_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();

    w.sne(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

expr_result expr_or(instr::instruction_writer& w,
                    reg_allocator& alloc,
                    const expr_result& a,
                    const expr_result& b) {
    expect_types(bool_type{}, *a.type_obj, yy::location{});
    expect_types(bool_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.or_op(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

expr_result expr_and(instr::instruction_writer& w,
                     reg_allocator& alloc,
                     const expr_result& a,
                     const expr_result& b) {
    expect_types(bool_type{}, *a.type_obj, yy::location{});
    expect_types(bool_type{}, *b.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.and_op(r, a.reg_num, b.reg_num);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

expr_result
expr_minus(instr::instruction_writer& w, reg_allocator& alloc, const expr_result& a) {
    expect_types(int_type{}, *a.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.sub(r, 0, a.reg_num);
    return expr_result{ std::make_unique<func::int_type>(), r };
}
expr_result
expr_not(instr::instruction_writer& w, reg_allocator& alloc, const expr_result& a) {
    expect_types(bool_type{}, *a.type_obj, yy::location{});
    auto r = alloc.alloc();
    w.xori(r, a.reg_num, 1);
    return expr_result{ std::make_unique<func::bool_type>(), r };
}

} // namespace func