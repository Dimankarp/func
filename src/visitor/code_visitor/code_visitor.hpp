#pragma once

#include "exception.hpp"
#include "location.hh"
#include "printer.hpp"
#include "type/type.hpp"
#include "visitor/code_visitor/instruction_writer.hpp"
#include "visitor/code_visitor/register_allocator.hpp"
#include "visitor/sym_table.hpp"
#include "visitor/visitor.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace func {

struct expr_result {
    std::unique_ptr<func::type> type_obj;
    uint8_t reg_num;
};

struct sym_info {
    std::string name;
    std::unique_ptr<func::type> type_obj;
    enum : char { STACK, ABS } access_type;
    uint16_t offset;
    yy::location declare_loc = yy::location{};
    bool is_delimeter = false;

    public:
    sym_info(const std::string& name,
             const std::unique_ptr<func::type>& type_obj,
             decltype(STACK) access_type,
             uint16_t offset,
             yy::location declare_loc = yy::location{},
             bool is_delimeter = false)
    : name{ name }, type_obj{ type_obj->clone() }, access_type{ access_type },
      offset{ offset }, declare_loc{ declare_loc }, is_delimeter{ is_delimeter } {}
    sym_info() = default;
    sym_info(const sym_info& sym)
    : name{ sym.name }, access_type{ sym.access_type }, offset{ sym.offset },
      declare_loc{ sym.declare_loc }, is_delimeter{ sym.is_delimeter } {
        if(sym.type_obj != nullptr)
            type_obj = sym.type_obj->clone();
    }
};

class code_visitor : public visitor<expr_result> {
    private:
    func::stream_proxy& debug_out;
    expr_result result;
    sym_table<sym_info> table;
    reg_allocator alloc;
    func::instr::instruction_writer writer;
    uint16_t stack_height = 0;
    uint16_t label_ind = 0;

    public:
    code_visitor(func::printer& printer);

    void visit(const binop_expression&) override;
    void visit(const unarop_expression&) override;
    void visit(const literal_expression&) override;
    void visit(const identifier_expression&) override;
    void visit(const function_call&) override;
    void visit(const subscript_expression&) override;

    void visit(const subscript_assign_statement&) override;
    void visit(const program&) override;
    void visit(const block_statement&) override;
    void visit(const return_statement&) override;
    void visit(const assign_statement&) override;
    void visit(const if_statement&) override;
    void visit(const while_statement&) override;
    void visit(const declaration&) override;
    void visit(const function&) override;

    expr_result&& extract_result() override { return std::move(result); }

    private:
    void declare_write_func();
    void declare_read_func();
};

} // namespace func
